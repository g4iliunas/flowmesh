#include "proxy_server.h"
#include "log.h"
#include "proxy_callbacks.h"
#include "socks5.h"
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // somaxconn
#include <uv.h>
#include <uv/unix.h>

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

static void on_write(uv_write_t *req, int status)
{
    if (status) {
        log_error("uv_write error");
        uv_close((uv_handle_t *)req->handle, client_free);
    }
    else {
#ifndef NDEBUG
        log_debug("Written data (%u bufs) to client", req->nbufs);
#endif
        fm_proxy_client_t *ctx = (fm_proxy_client_t *)req->handle->data;
        if (ctx->disconnect) {
#ifndef NDEBUG
            log_debug(
                "Disconnect indicator was set to true, closing the client");
#endif
            uv_close((uv_handle_t *)req->handle, client_free);
        }
    }
    free(req);
}

static void write2(uv_stream_t *stream, char *data, int len)
{
    uv_buf_t buffer[] = {{.base = data, .len = len}};
    uv_write_t *req = malloc(sizeof(uv_write_t));
    uv_write(req, stream, buffer, 1, on_write);
}

/*
 * Return code 1 means the client must be disconnected
 */
static int socks5_handle(uv_stream_t *client, const ssize_t *nread,
                         const uv_buf_t *buf)
{
    fm_proxy_client_t *ctx = (fm_proxy_client_t *)client->data;
#ifndef NDEBUG
    log_debug("Client state: %d", ctx->state);
#endif
    switch (ctx->state) {
    case FM_SOCKS5_STATE_NONE: {
        uint8_t *methods;
        uint8_t *nmethods;

        if (socks5_parse_identifier((uint8_t *)buf->base, *nread, &methods,
                                    &nmethods) != 0) {
#ifndef NDEBUG
            log_debug("Failed to parse SOCKS5 identifier");
#endif
            return 1;
        }
#ifndef NDEBUG
        log_debug("SOCKS5 identifier:");
        log_debug("*** methods location: %p", methods);
        log_debug("*** nmethods: %u", *nmethods);
#endif
        // find the username/password method
        bool found = false;
        for (uint8_t i = 0; i < *nmethods; i++) {
            if (methods[i] == SOCKS5_AUTH_USERNAME_PASSWORD) {
                found = true;
                break;
            }
        }

        uint8_t resp[2];
        resp[0] = SOCKS5_VERSION;

        if (!found) {
#ifndef NDEBUG
            log_debug("Did not find the Username/Password auth method, "
                      "abandoning client");
#endif
            resp[1] = SOCKS5_AUTH_NO_ACCEPTABLE_METHODS;
            ctx->disconnect = true;
        }
        else {
            resp[1] = SOCKS5_AUTH_USERNAME_PASSWORD;
            ctx->state = FM_SOCKS5_STATE_IDENTIFIER;
        }

        write2((uv_stream_t *)client, (char *)resp, sizeof(resp));
        break;
    }
    case FM_SOCKS5_STATE_IDENTIFIER: {
        uint8_t *ulen;
        char *uname;
        uint8_t *plen;
        char *passwd;

        if (socks5_parse_auth((uint8_t *)buf->base, *nread, &ulen, &uname,
                              &plen, &passwd) != 0) {
#ifndef NDEBUG
            log_debug("Failed to parse SOCKS5 authentication packet");
#endif
            return 1;
        }

        cb_proxy_auth(client, uname, *ulen, passwd, *plen);
        break;
    }
    case FM_SOCKS5_STATE_AUTHENTICATION: {
        socks5_address_type_t *addr_type;
        uint8_t *addr;
        uint16_t *netport;

        if (socks5_parse_request((uint8_t *)buf->base, *nread, &addr_type,
                                 &addr, &netport) != 0) {
#ifndef NDEBUG
            log_debug("Failed to parse SOCKS5 request packet");
#endif
            return 1;
        }

        cb_conn_request(client, addr_type, addr, netport);
        break;
    }
    case FM_SOCKS5_STATE_AUTHORIZED: {
#ifndef NDEBUG
        log_debug("Got data to relay");
#endif
        // TODO: add callback for relay?
        break;
    }
    }
    return 0;
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
    int ret = socks5_handle(client, &nread, buf);
    if (ret == 1)
        uv_close((uv_handle_t *)client, client_free);
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

    // integrate client into server loop
    uv_tcp_init(server->loop, client);

    if (uv_accept(server, (uv_stream_t *)client) == 0) {
        fm_proxy_client_t *ctx =
            (fm_proxy_client_t *)malloc(sizeof(fm_proxy_client_t));

        if (!ctx) {
            uv_close((uv_handle_t *)client, client_free);
            log_error("Failed to allocate a proxy client context");
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

/* Public functions */

uv_tcp_t *fm_proxy_server_init(uv_loop_t *loop, const char *address,
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

void fm_proxy_auth_ack(uv_stream_t *client, bool ok)
{
    fm_proxy_client_t *ctx = (fm_proxy_client_t *)client->data;
    char resp[2];
    resp[0] = SOCKS5_SUBNEGOTIATION_VERSION;

    if (!ok) {
        resp[1] = 0x01;
        ctx->disconnect = true;
    }
    else {
        resp[1] = 0x00;
        ctx->state = FM_SOCKS5_STATE_AUTHENTICATION;
    }

    write2((uv_stream_t *)client, (char *)resp, sizeof(resp));
}

void fm_proxy_request_ack(uv_stream_t *client, socks5_address_type_t type,
                          bool ok)
{
    if (type == SOCKS5_ADDR_TYPE_DOMAIN)
        return;

    fm_proxy_client_t *ctx = (fm_proxy_client_t *)client->data;
    char resp[22];

    resp[0] = SOCKS5_VERSION;
    resp[2] = 0x00;

    if (!ok) {
        resp[1] = 0x01;
        ctx->disconnect = true;
        write2(client, resp, 3);
        return;
    }

    resp[1] = 0x00;
    resp[3] = type;
    ctx->state = FM_SOCKS5_STATE_AUTHORIZED;

    if (type == SOCKS5_ADDR_TYPE_IPV4) {
        memset(&resp[4], 0, 6); // nullify ip and port
        write2(client, resp, 10);
    }
    else if (type == SOCKS5_ADDR_TYPE_IPV6) {
        memset(&resp[4], 0, 18); // nullify ip and port
        write2(client, resp, 22);
    }
}