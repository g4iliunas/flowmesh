#include "consumer.h"
#include "socks5.h"
#include <any>
#include <fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <utility>

void Consumer::handle_auth(const socks5::credentials &creds)
{
    /*
    request for a provider to manager server, find a provider that matches
    credentials.
    */

    this->get_manager()->get_database()->get_provider(
        creds.username,
        [](Database::HashMap &hashmap, std::any args) {
            auto *p = std::any_cast<std::pair<std::string, Consumer *> *>(args);

            if (hashmap.empty()) {
                SPDLOG_DEBUG("Database result is empty");
                delete p;
                return;
            }

            SPDLOG_DEBUG("Queried for: pw:{}; Result: {}", p->first, hashmap);
            socks5::reply rep = p->first == hashmap["proxy_password"]
                                    ? socks5::reply::SUCCEEDED
                                    : socks5::reply::GENERAL_FAILURE;
            p->second->send_request_response(rep);
            delete p;
        },
        new std::pair<std::string, Consumer *>(creds.password, this));
}

void Consumer::handle_request(const socks5::raw_conn_address &raw_addr)
{
    this->send_request_response(socks5::reply::SUCCEEDED);
}

void Consumer::handle_relay(const std::string_view &buf)
{
    SPDLOG_DEBUG("Relaying {} bytes", buf.length());
}