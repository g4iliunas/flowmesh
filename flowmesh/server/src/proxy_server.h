#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "socks5.h"
#include <stdbool.h>
#include <stdint.h>
#include <uv.h>

typedef enum {
    FM_SOCKS5_STATE_NONE,
    FM_SOCKS5_STATE_IDENTIFIER,
    FM_SOCKS5_STATE_AUTHENTICATION,
    FM_SOCKS5_STATE_AUTHORIZED,
} fm_socks5_state_t;

typedef struct {
    // TODO: indicate the type of proxy protocol
    uint32_t state;
    bool disconnect; // used for ensuring that data is written before
                     // disconnecting the tcp handle
} fm_proxy_client_t;

/*
 * Returns a dynamically allocated TCP handle of proxy server
 */
uv_tcp_t *fm_proxy_server_init(uv_loop_t *loop, const char *address,
                               const uint16_t port);

void fm_proxy_auth_ack(uv_stream_t *client, bool ok);

/*
 * Type must not be domain
 */
void fm_proxy_request_ack(uv_stream_t *client, socks5_address_type_t type,
                          bool ok);

#endif // PROXY_SERVER_H