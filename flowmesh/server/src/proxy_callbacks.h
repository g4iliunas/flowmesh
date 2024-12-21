#ifndef PROXY_CALLBACKS_H
#define PROXY_CALLBACKS_H

#include "socks5.h"
#include <stdint.h>
#include <uv.h>

void cb_proxy_auth(uv_stream_t *client, const char *uname, uint8_t ulen,
                   const char *passwd, uint8_t plen);

void cb_conn_request(uv_stream_t *client, socks5_address_type_t *addr_type,
                     uint8_t *addr, uint16_t *netport);

#endif