#include "proxy_client.h"
#include "client.h"
#include "socks5.h"
#include "utils.h"
#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <spdlog/spdlog.h>

ProxyClient::ProxyClient()
    : Client(), passed_state(socks5::state::NONE), disconnect(false),
      is_ipv6(false)
{
    SPDLOG_DEBUG("Constructing a proxy client");
    this->get_client()->data = this;
}

void ProxyClient::read_start()
{
    uv_read_start(
        reinterpret_cast<uv_stream_t *>(this->get_client()), alloc_buf,
        [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
            auto *pclient = reinterpret_cast<ProxyClient *>(stream->data);
            if (nread < 0) {
                if (nread != UV_EOF)
                    SPDLOG_WARN("Read error: {}", uv_strerror(nread));
                else
                    SPDLOG_DEBUG("Client disconnected");

                delete pclient;

                if (buf->base != nullptr)
                    delete[] buf->base;
                return;
            }

            SPDLOG_DEBUG("Read {} bytes", nread);
            pclient->handle_buf(std::string_view(buf->base, nread));
            delete[] buf->base;
        });
}

void ProxyClient::write(std::vector<uv_buf_t> &bufs, bool disconnect)
{
    this->disconnect = disconnect;
    uv_write_t *req = new uv_write_t;
    uv_write(
        req, reinterpret_cast<uv_stream_t *>(this->get_client()), bufs.data(),
        bufs.size(), [](uv_write_t *req, int status) {
            auto *pclient = reinterpret_cast<ProxyClient *>(req->handle->data);
            delete req;

            if (status < 0) {
                SPDLOG_ERROR("Write error: {}", uv_strerror(status));
                delete pclient;
                return;
            }

            if (pclient->is_disconnect()) {
                SPDLOG_DEBUG("Disconnecting client because the flag was set");
                delete pclient;
                return;
            }
        });
}

void ProxyClient::handle_buf(const std::string_view buf)
{
    SPDLOG_DEBUG("Handling {} bytes", buf.length());

    switch (this->passed_state) {
    case socks5::state::NONE: {
        this->handle_stage_ident(buf);
        break;
    }
    case socks5::state::IDENT: {
        this->handle_stage_auth(buf);
        break;
    }
    case socks5::state::AUTH: {
        this->handle_stage_request(buf);
        break;
    }
    case socks5::state::REQUEST: {
        this->handle_relay(buf);
        break;
    }
    default: {
        break;
    }
    }
}

void ProxyClient::handle_stage_ident(const std::string_view &buf)
{
    std::string_view methods = socks5::parse_ident(buf);
    SPDLOG_DEBUG("SOCKS5 client suggested {} methods", methods.length());

    if (methods.empty()) {
        SPDLOG_WARN("Invalid SOCKS5 identification");
        delete this; // disconnect
        return;
    }

    char outmethod = std::find(methods.begin(), methods.end(),
                               static_cast<char>(socks5::methods::USER_PASS)) !=
                             methods.end()
                         ? static_cast<char>(socks5::methods::USER_PASS)
                         : static_cast<char>(socks5::methods::NO_ACCEPTABLE);

    char response[2] = {socks5::VERSION, outmethod};
    std::vector<uv_buf_t> bufs = {{response, sizeof(response)}};

    if (outmethod == static_cast<char>(socks5::methods::USER_PASS))
        this->passed_state = socks5::state::IDENT;

    // we only accept user pass auth
    this->write(bufs,
                outmethod != static_cast<char>(socks5::methods::USER_PASS));
}

void ProxyClient::handle_stage_auth(const std::string_view &buf)
{
    std::optional<socks5::credentials> _creds = socks5::parse_auth(buf);

    if (!_creds.has_value()) {
        SPDLOG_WARN("Failed to parse credentials");

        std::vector<uv_buf_t> bufs;
        char response[2] = {socks5::SUBNEGOTIATION_VERSION, 0x01};
        bufs.push_back({response, sizeof(response)});

        this->write(bufs, true);
        return;
    }

    socks5::credentials &creds = _creds.value();
    SPDLOG_DEBUG("Received credentials: {} {}", creds.username, creds.password);
    this->handle_auth(creds);
}

void ProxyClient::handle_stage_request(const std::string_view &buf)
{
    std::optional<socks5::raw_conn_address> conn_addr =
        socks5::parse_request(buf);

    if (!conn_addr.has_value()) {
        SPDLOG_WARN("Invalid SOCKS5 request");

        std::vector<uv_buf_t> bufs;
        char response[3] = {socks5::VERSION,
                            static_cast<char>(socks5::reply::GENERAL_FAILURE),
                            0x00};
        bufs.push_back({response, sizeof(response)});

        this->write(bufs, true);
        return;
    }

    socks5::raw_conn_address &raw_addr = conn_addr.value();
    if (raw_addr.addr.length() == 16)
        this->is_ipv6 = true;

#ifndef NDEBUG
    if (raw_addr.is_domain) {
        SPDLOG_DEBUG("Received request: {} {}", raw_addr.addr,
                     raw_addr.netport);
    }
    else {
        char buf[INET6_ADDRSTRLEN];

        if (raw_addr.addr.length() == 4)
            inet_ntop(AF_INET, raw_addr.addr.data(), buf, INET_ADDRSTRLEN);
        else if (raw_addr.addr.length() == 16)
            inet_ntop(AF_INET6, raw_addr.addr.data(), buf, INET6_ADDRSTRLEN);

        SPDLOG_DEBUG("Received request: {} {}", buf, raw_addr.netport);
    }
#endif
    this->handle_request(raw_addr);
}

void ProxyClient::send_auth_response(const bool success)
{
    char response[2];
    response[0] = socks5::SUBNEGOTIATION_VERSION;
    response[1] = success ? 0x00 : 0x01;

    std::vector<uv_buf_t> bufs;
    bufs.push_back({response, sizeof(response)});

    if (success)
        this->passed_state = socks5::state::AUTH;

    this->write(bufs, !success);
}

void ProxyClient::send_request_response(const socks5::reply status)
{
    char response[socks5::IPV6_REQ_LEN] = {0};
    response[0] = socks5::VERSION;
    response[1] = static_cast<char>(status);
    response[2] = 0x00;

    std::vector<uv_buf_t> bufs;
    if (status != socks5::reply::SUCCEEDED) {
        bufs.push_back({response, 3});
        this->write(bufs, true);
        return;
    }

    response[3] = this->is_ipv6 ? static_cast<char>(socks5::address_type::IPV6)
                                : static_cast<char>(socks5::address_type::IPV4);

    bufs.push_back({response, static_cast<std::size_t>(
                                  this->is_ipv6 ? socks5::IPV6_REQ_LEN
                                                : socks5::IPV4_REQ_LEN)});

    this->passed_state = socks5::state::REQUEST;
    this->write(bufs, false);
}