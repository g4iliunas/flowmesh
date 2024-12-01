#include "proxy_server.h"
#include "log.h"
#include <stdlib.h>
#include <uv.h>

uv_loop_t *_loop = NULL;

static void on_new_connection(uv_stream_t *server, int status)
{
#ifndef NDEBUG
    log_debug("Got a connection!");
#endif
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