#ifndef SOCKS5_H
#define SOCKS5_H

#include <stddef.h>
#include <stdint.h>

#define SOCKS5_VERSION 0x05
#define SOCKS5_SUBNEGOTIATION_VERSION 0x01

typedef enum {
    SOCKS5_AUTH_NO_AUTHENTICATION_REQUIRED = 0x00,
    SOCKS5_AUTH_GSSAPI = 0x01,
    SOCKS5_AUTH_USERNAME_PASSWORD = 0x02,
    SOCKS5_AUTH_NO_ACCEPTABLE_METHODS = 0xFF
} socks5_auth_method_t;

typedef enum {
    SOCKS5_CMD_CONNECT = 0x01,
    SOCKS5_CMD_BIND = 0x02,
    SOCKS5_CMD_UDP_ASSOCIATE = 0x03
} socks5_command_t;

typedef enum {
    SOCKS5_ADDR_TYPE_IPV4 = 0x01,
    SOCKS5_ADDR_TYPE_DOMAIN = 0x03,
    SOCKS5_ADDR_TYPE_IPV6 = 0x04
} socks5_address_type_t;

typedef enum {
    SOCKS5_REPLY_SUCCEEDED = 0x00,
    SOCKS5_REPLY_GENERAL_FAILURE = 0x01,
    SOCKS5_REPLY_NOT_ALLOWED = 0x02,
    SOCKS5_REPLY_NETWORK_UNREACHABLE = 0x03,
    SOCKS5_REPLY_HOST_UNREACHABLE = 0x04,
    SOCKS5_REPLY_CONNECTION_REFUSED = 0x05,
    SOCKS5_REPLY_TTL_EXPIRED = 0x06,
    SOCKS5_REPLY_COMMAND_NOT_SUPPORTED = 0x07,
    SOCKS5_REPLY_ADDRESS_TYPE_NOT_SUPPORTED = 0x08
} socks5_reply_t;

int socks5_parse_identifier(uint8_t *buf, const size_t buf_len,
                            uint8_t **methods_out, uint8_t **nmethods_out);

int socks5_parse_auth(uint8_t *buf, const size_t buf_len, uint8_t **ulen_out,
                      char **uname_out, uint8_t **plen_out, char **passwd_out);

#endif