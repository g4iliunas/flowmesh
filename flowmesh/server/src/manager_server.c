#include "manager_server.h"
#include "log.h"
#include <stdlib.h>

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size,
                         uv_buf_t *buf)
{
    (void)handle;

    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
#ifndef NDEBUG
    log_trace("Allocated a buffer %p of len %u", buf->base, buf->len);
#endif
}

static void client_free(uv_handle_t *client)
{
#ifndef NDEBUG
    log_debug("Freeing client handle");
#endif
    if (client->data)
        free(client->data);
    free((void *)client);
}

static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread == 0)
        return;

    if (nread < 0) {
        if (buf->base != NULL && buf->len != 0)
            free(buf->base);

        if (nread == UV_EOF) {
            // handle eof
            log_warn("Connection closed");
        }
        else {
            // handle error
            log_warn("Error encountered");
        }

        uv_close((uv_handle_t *)client, client_free);
        return;
    }
#ifndef NDEBUG
    log_debug("Received %d bytes of data", nread);
#endif

    free(buf->base);
}

static void on_new_connection(uv_stream_t *server, int status)
{
#ifndef NDEBUG
    log_debug("Got a connection!");
#endif
    if (status < 0) {
        log_error("New connection error: %s", uv_strerror(status));
        return;
    }

    uv_tcp_t *client = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    if (!client) {
        log_error("Failed to allocate a provider client");
        return;
    }

    // integrate client into server loop
    uv_tcp_init(server->loop, client);

    if (uv_accept(server, (uv_stream_t *)client) == 0) {
        fm_manager_client_t *ctx =
            (fm_manager_client_t *)malloc(sizeof(fm_manager_client_t));

        if (!ctx) {
            uv_close((uv_handle_t *)client, client_free);
            log_error("Failed to allocate provider context");
            return;
        }

        // initialize ctx
        ctx->state = 0;
        ctx->disconnect = false;

        // assign uv handle to carry the ctx
        client->data = (void *)ctx;

        // start the read/write cycle
        uv_read_start((uv_stream_t *)client, alloc_buffer, read_cb);
    }
    else {
        // if we fail to accept the client, close it
        uv_close((uv_handle_t *)client, client_free);
    }
}

uv_tcp_t *fm_manager_server_init(uv_loop_t *loop, const char *address,
                                 const uint16_t port)
{
    uv_tcp_t *proxy_server = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    if (!proxy_server)
        return NULL;

    uv_tcp_init(loop, proxy_server);
    struct sockaddr_in sin = {0};
    uv_ip4_addr(address, port, &sin);
    uv_tcp_bind(proxy_server, (struct sockaddr *)&sin, 0);

    if (uv_listen((uv_stream_t *)proxy_server, SOMAXCONN, on_new_connection) !=
        0) {
        free(proxy_server);
        return NULL;
    }

    return proxy_server;
}