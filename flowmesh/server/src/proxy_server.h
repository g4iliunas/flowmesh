#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include "manager_server.h"
#include <cstdint>
#include <string_view>
#include <uv.h>

class ProxyServer {
  public:
    ProxyServer(uv_loop_t *loop, ManagerServer *manager,
                const std::string_view &host, const std::uint16_t port);
    ~ProxyServer();

  private:
    uv_tcp_t *server;
};

#endif // PROXY_SERVER_H