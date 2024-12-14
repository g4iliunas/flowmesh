#include "socks5.h"

int socks5_parse_identifier(uint8_t *buf, const size_t buf_len,
                            uint8_t **methods_out, uint8_t **nmethods_out)
{
    if (buf_len < 2)
        return 1;

    uint8_t *ver = &buf[0];
    uint8_t *nmethods = &buf[1];
    if (*ver != SOCKS5_VERSION || buf_len != *nmethods + 2)
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
        buf_len < *ulen + 3) // including ver, uname, plen byte fields
        return 1;

    char *uname = (char *)&buf[2];
    uint8_t *plen = (uint8_t *)&uname[*ulen];
    if (*plen == 0 ||
        buf_len != *ulen + *plen + 3) // including ver, uname, plen byte fields
        return 1;

    char *passwd = (char *)&plen[1];

    *ulen_out = ulen;
    *uname_out = uname;
    *plen_out = plen;
    *passwd_out = passwd;
    
    return 0;
}