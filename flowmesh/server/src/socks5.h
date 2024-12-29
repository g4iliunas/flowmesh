#ifndef SOCKS5_H
#define SOCKS5_H

#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>

namespace socks5 {
static const int VERSION = 0x05;
static const int SUBNEGOTIATION_VERSION = 0x01;

enum class state {
    NONE = -1,
    IDENT,
    AUTH,
    REQUEST,
};

enum class methods {
    NO_AUTH = 0x00,
    GSSAPI = 0x01,
    USER_PASS = 0x02,
    NO_ACCEPTABLE = 0xFF,
};

enum class address_type {
    IPV4 = 0x01,
    DOMAIN = 0x03,
    IPV6 = 0x04,
};

enum class reply {
    SUCCEEDED = 0x00,
    GENERAL_FAILURE = 0x01,
    CONNECTION_NOT_ALLOWED = 0x02,
    NETWORK_UNREACHABLE = 0x03,
    HOST_UNREACHABLE = 0x04,
    CONNECTION_REFUSED = 0x05,
    TTL_EXPIRED = 0x06,
    COMMAND_NOT_SUPPORTED = 0x07,
    ADDRESS_TYPE_NOT_SUPPORTED = 0x08,
};

enum class command {
    CONNECT = 0x01,
    BIND = 0x02,
    UDP_ASSOCIATE = 0x03,
};

struct credentials {
    std::string_view username;
    std::string_view password;
};

struct raw_conn_address {
    std::string_view addr;
    std::uint16_t netport;
    bool is_domain;
};

std::string_view parse_ident(std::string_view buf);
std::optional<credentials> parse_auth(std::string_view buf);
std::optional<raw_conn_address> parse_request(std::string_view buf);

}; // namespace socks5

#endif