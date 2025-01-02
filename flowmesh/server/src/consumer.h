#ifndef CONSUMER_H
#define CONSUMER_H

#include "client.h"
#include "manager_server.h"
#include "socks5.h"
#include <uv.h>

class Consumer : public Client<Consumer> {
  public:
    Consumer(ManagerServer *manager_server);
    void handle_buf(const std::string_view buf);
    ManagerServer *get_manager() { return this->manager_server; }

  private:
    socks5::state passed_state;
    bool is_ipv6;

    void handle_stage_ident(const std::string_view &buf);
    void handle_stage_auth(const std::string_view &buf);
    void handle_stage_request(const std::string_view &buf);

    /* Callbacks */

    void handle_auth(const socks5::credentials &creds);
    void handle_request(const socks5::raw_conn_address &addr);
    void handle_relay(const std::string_view &buf);

    /* Response functions */

    void send_auth_response(const bool success);
    void send_request_response(const socks5::reply status);

  private:
    ManagerServer *manager_server;
};

#endif // CONSUMER_H