#include "socks5.h"

int socks5_parse_identifier(uint8_t *buf, const size_t buf_len,
                            uint8_t **methods_out, uint8_t **nmethods_out)
{
    if (buf_len < 2)
        return 1;

    uint8_t *ver = &buf[0];
    uint8_t *nmethods = &buf[1];
    if (*ver != SOCKS5_VERSION || buf_len != (size_t)(*nmethods + 2))
        return 1;

    uint8_t *methods = &buf[2];
    *methods_out = methods;
    *nmethods_out = nmethods;
    return 0;
}

int socks5_parse_auth(uint8_t *buf, const size_t buf_len, uint8_t **ulen_out,
                      char **uname_out, uint8_t **plen_out, char **passwd_out)
{
    if (buf_len < 2)
        return 1;

    uint8_t *ver = &buf[0];
    uint8_t *ulen = &buf[1];
    if (*ver != SOCKS5_SUBNEGOTIATION_VERSION || *ulen == 0 ||
        buf_len < (size_t)(*ulen + 3)) // including ver, uname, plen byte fields
        return 1;

    char *uname = (char *)&buf[2];
    uint8_t *plen = (uint8_t *)&uname[*ulen];
    if (*plen == 0 ||
        buf_len != (size_t)(*ulen + *plen +
                            3)) // including ver, uname, plen byte fields
        return 1;

    char *passwd = (char *)&plen[1];

    *ulen_out = ulen;
    *uname_out = uname;
    *plen_out = plen;
    *passwd_out = passwd;
    return 0;
}

int socks5_parse_request(uint8_t *buf, const size_t buf_len,
                         socks5_address_type_t **addr_type_out,
                         uint8_t **addr_out, uint16_t **netport_out)
{
    if (buf_len < 4)
        return 1;

    uint8_t *ver = &buf[0];
    uint8_t *cmd = &buf[1];
    uint8_t *rsv = &buf[2];
    uint8_t *atyp = &buf[3];

    if (*ver != SOCKS5_VERSION || *cmd != SOCKS5_CMD_CONNECT || *rsv != 0x00)
        return 1;

    switch (*atyp) {
    case SOCKS5_ADDR_TYPE_IPV4: {
        if (buf_len != 10)
            return 1;

        uint8_t *ipv4 = &buf[4];
        uint16_t *netport = (uint16_t *)&ipv4[4];

        *addr_out = ipv4;
        *netport_out = netport;
        break;
    }
    case SOCKS5_ADDR_TYPE_DOMAIN: {
        if (buf_len < 5)
            return 1;

        uint8_t *domainlen = &buf[4];
        if (*domainlen == 0 || buf_len != (size_t)(*domainlen + 7))
            return 1;

        uint8_t *domain = &domainlen[1];
        uint16_t *netport = (uint16_t *)&domain[*domainlen];

        *addr_out = domainlen;
        *netport_out = netport;
        break;
    }
    case SOCKS5_ADDR_TYPE_IPV6: {
        if (buf_len != 22)
            return 1;

        uint8_t *ipv6 = &buf[4];
        uint16_t *netport = (uint16_t *)&ipv6[16];

        *addr_out = ipv6;
        *netport_out = netport;
        break;
    }
    default: {
        return 1;
    }
    }

    *addr_type_out = atyp;
    return 0;
}