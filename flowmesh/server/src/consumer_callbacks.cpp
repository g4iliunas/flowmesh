#include "proxy_client.h"
#include <spdlog/spdlog.h>

void ProxyClient::handle_auth(const socks5::credentials &creds)
{
    /*
    request for a provider to manager server, find a provider that matches
    credentials.
    */

    this->send_auth_response(true);
}

void ProxyClient::handle_request(const socks5::raw_conn_address &raw_addr)
{
    this->send_request_response(socks5::reply::SUCCEEDED);
}

void ProxyClient::handle_relay(const std::string_view &buf)
{
    SPDLOG_DEBUG("Relaying {} bytes", buf.length());
}