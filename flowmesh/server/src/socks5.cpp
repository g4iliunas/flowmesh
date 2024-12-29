#include "socks5.h"
#include <cstring>
#include <spdlog/spdlog.h>
#include <string_view>

std::string_view socks5::parse_ident(std::string_view buf)
{
    if (buf.length() < 2)
        return {};

    unsigned char nmethods = static_cast<unsigned char>(buf[1]);
    if (buf[0] != socks5::VERSION || nmethods == 0)
        return {};

    if (buf.length() != static_cast<std::size_t>(nmethods + 2))
        return {};

    return std::string_view(&buf[2], nmethods);
}

std::optional<socks5::credentials> socks5::parse_auth(std::string_view buf)
{
    if (buf.length() < 3)
        return {};

    unsigned char ulen = static_cast<unsigned char>(buf[1]);
    if (buf[0] != socks5::SUBNEGOTIATION_VERSION || ulen == 0)
        return {};

    if (buf.length() <
        static_cast<std::size_t>(ulen + 3)) // including ver, ulen, plen
        return {};

    std::string_view username;
    std::string_view password;

    username = std::string_view(&buf[2], ulen);

    unsigned char plen = static_cast<unsigned char>(buf[ulen + 2]);
    if (plen == 0)
        return {};

    if (buf.length() !=
        static_cast<std::size_t>(ulen + plen + 3)) // including ver, ulen, plen
        return {};

    password = std::string_view(&buf[ulen + 3], plen);

    return socks5::credentials{username, password};
}

std::optional<socks5::raw_conn_address>
socks5::parse_request(std::string_view buf)
{
    if (buf.length() < 5) // minimum length, in case we get a domain and 4th
                          // byte is domain length
        return {};

    if (buf[0] != socks5::VERSION ||
        static_cast<std::uint8_t>(buf[1]) !=
            static_cast<std::uint8_t>(socks5::command::CONNECT) ||
        buf[2] != 0)
        return {};

    socks5::address_type atyp = static_cast<socks5::address_type>(buf[3]);
    SPDLOG_TRACE("atyp: {}", static_cast<std::uint8_t>(atyp));

    std::string_view addr;
    std::uint16_t port;

    switch (atyp) {
    case socks5::address_type::IPV4: {
        if (buf.length() != 10)
            return {};

        addr = std::string_view(&buf[4], 4);
        std::memcpy(&port, &buf[8], 2);
        return socks5::raw_conn_address{addr, port, false};
    }
    case socks5::address_type::DOMAIN: {
        std::uint8_t domain_len = static_cast<std::uint8_t>(buf[4]);

        if (buf.length() != static_cast<std::size_t>(domain_len + 7))
            return {};

        addr = std::string_view(&buf[5], domain_len);
        std::memcpy(&port, &buf[domain_len + 5], 2);
        return socks5::raw_conn_address{addr, port, true};
    }
    case socks5::address_type::IPV6: {
        if (buf.length() != 22)
            return {};

        addr = std::string_view(&buf[4], 16);
        std::memcpy(&port, &buf[20], 2);
        return socks5::raw_conn_address{addr, port, false};
    }
    default:
        return {};
    }
}