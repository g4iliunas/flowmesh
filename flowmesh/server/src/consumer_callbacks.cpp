#include "consumer.h"
#include "socks5.h"
#include <any>
#include <fmt/ranges.h>
#include <memory>
#include <spdlog/spdlog.h>

struct AuthQuery {
    std::string password;
    Consumer *consumer;
};

void Consumer::handle_auth(const socks5::credentials &creds)
{
    /*
    validate auth
    */

    this->get_manager()->get_database()->get_provider(
        creds.username,
        [](Database::HashMap &hashmap, std::any args) {
            auto q =
                std::unique_ptr<AuthQuery>(std::any_cast<AuthQuery *>(args));

            if (hashmap.empty()) {
                SPDLOG_DEBUG("Database result is empty");
                return;
            }

            SPDLOG_DEBUG("*** Query: pw:{}; result:{}", q->password, hashmap);

            socks5::reply rep = q->password == hashmap["proxy_password"]
                                    ? socks5::reply::SUCCEEDED
                                    : socks5::reply::GENERAL_FAILURE;
            q->consumer->send_request_response(rep);
        },
        new AuthQuery{std::string(creds.password), this});
}

void Consumer::handle_request(const socks5::raw_conn_address &raw_addr)
{
    /*
     * if there is not a provider assigned, find it in manager server
     */
    this->send_request_response(socks5::reply::SUCCEEDED);
}

void Consumer::handle_relay(const std::string_view &buf)
{
    SPDLOG_DEBUG("Relaying {} bytes", buf.length());
}