#ifndef PROXY_CLIENT_H
#define PROXY_CLIENT_H

#include "client.h"
#include "manager_server.h"
#include "socks5.h"
#include <uv.h>

class ProxyClient : public Client {
  public:
    ProxyClient(ManagerServer *database);
    void read_start();
    void write(std::vector<uv_buf_t> &bufs, bool disconnect);
    bool is_disconnect() const { return this->disconnect; }

  private:
    socks5::state passed_state;
    bool disconnect; // set to true when the client should be disconnected after
                     // writing is done
    bool is_ipv6;

    void handle_buf(const std::string_view buf);

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
    ManagerServer *manager;
};

#endif // PROXY_CLIENT_H