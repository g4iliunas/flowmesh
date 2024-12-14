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