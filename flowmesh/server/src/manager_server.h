#ifndef MANAGER_SERVER_H
#define MANAGER_SERVER_H

#include <cstdint>
#include <string_view>
#include <uv.h>

class ManagerServer {
  public:
    ManagerServer(uv_loop_t *loop, const std::string_view &host,
                  const std::uint16_t port);
};

#endif // MANAGER_SERVER_H