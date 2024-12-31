#ifndef MANAGER_SERVER_H
#define MANAGER_SERVER_H

#include "database.h"
#include <cstdint>
#include <string_view>
#include <uv.h>

class ManagerServer {
  public:
    ManagerServer(uv_loop_t *loop, Database *database,
                  const std::string_view &host, const std::uint16_t port);

  private:
    uv_tcp_t *server;
    Database *database;
};

#endif // MANAGER_SERVER_H