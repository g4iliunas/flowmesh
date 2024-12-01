#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <stdint.h>
#include <uv.h>

/*
* Returns a dynamically allocated TCP handle of proxy server
*/
uv_tcp_t *fm_proxy_server_init(
    uv_loop_t *loop, const char *address, const uint16_t port);

#endif // PROXY_SERVER_H