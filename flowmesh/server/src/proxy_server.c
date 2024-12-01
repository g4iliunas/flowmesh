#include "proxy_server.h"
#include "log.h"
#include <stdlib.h>
#include <uv.h>

uv_loop_t *_loop = NULL;

static void
alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char *)malloc(suggested_size);
    buf->len = suggested_size;
}

static void client_close_cb(uv_handle_t *client)
{
#ifndef NDEBUG
    log_debug("Freeing client handle");
#endif
    free((void *)client);
}

static void read_cb(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread == 0) {
        return;
    }
    else if (nread < 0) {
        // handle errors
        if (nread == UV_EOF) {
            // handle eof
            log_warn("Connection closed");
        }
        else {
            // handle error 
            log_warn("Error encountered");
        }

        uv_close((uv_handle_t *)client, client_close_cb);
        goto cleanup;
    }
#ifndef DEBUG
    log_debug("Received %d bytes of data", nread);
#endif

    // we process data here

cleanup:
    if (buf->base)
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
        log_error("Failed to allocate a proxy client");
        return;
    }

    uv_tcp_init(_loop, client);
    if (uv_accept(server, (uv_stream_t *)client) == 0)
        uv_read_start((uv_stream_t *)client, alloc_buffer, read_cb);
    else
        uv_close((uv_handle_t *)client, client_close_cb);
}

uv_tcp_t *
fm_proxy_server_init(uv_loop_t *loop, const char *address, const uint16_t port)
{
    _loop = loop;
    uv_tcp_t *proxy_server = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    if (!proxy_server)
        return NULL;
    uv_tcp_init(loop, proxy_server);
    struct sockaddr_in sin = {0};
    uv_ip4_addr(address, port, &sin);
    uv_tcp_bind(proxy_server, (struct sockaddr *)&sin, 0);
    if (uv_listen((uv_stream_t *)proxy_server, 128, on_new_connection) != 0) {
        free(proxy_server);
        return NULL;
    }
    return proxy_server;
}