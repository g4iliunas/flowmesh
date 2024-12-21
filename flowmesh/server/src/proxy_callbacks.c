#include "proxy_callbacks.h"
#include "log.h"
#include "proxy_server.h"

void cb_proxy_auth(uv_stream_t *client, const char *uname, uint8_t ulen,
                   const char *passwd, uint8_t plen)
{
#ifndef NDEBUG
    log_debug("Username: %.*s; password: %.*s", ulen, uname, plen, passwd);
#else
    (void)uname;
    (void)ulen;
    (void)passwd;
    (void)plen;
#endif

    /* Validate proxy credentials here */

    fm_proxy_auth_ack(client, true);
}

void cb_conn_request(uv_stream_t *client, socks5_address_type_t *addr_type,
                     uint8_t *addr, uint16_t *netport)
{
#ifndef NDEBUG
    log_debug("SOCKS5 request:");

    char ipstr[INET6_ADDRSTRLEN];
    if (*addr_type == SOCKS5_ADDR_TYPE_IPV4) {
        inet_ntop(AF_INET, addr, ipstr, sizeof(ipstr));
        log_debug("*** IP: %s", ipstr);
    }
    else if (*addr_type == SOCKS5_ADDR_TYPE_IPV6) {
        inet_ntop(AF_INET6, addr, ipstr, sizeof(ipstr));
        log_debug("*** IP: %s", ipstr);
    }
    else {
        log_debug("*** Domain: %.*s", addr[0], &addr[1]);
    }

    log_debug("*** Port: %u", ntohs(*netport));
#endif

    /* Handle connection here */

    fm_proxy_request_ack(client, *addr_type, true);
}