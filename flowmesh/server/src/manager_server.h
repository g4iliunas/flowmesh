#ifndef MANAGER_SERVER_H
#define MANAGER_SERVER_H

#include <uv.h>
#include <string_view>

class ManagerServer {
  public:
    ManagerServer(uv_loop_t *loop, const std::string_view &host,
                  const std::uint16_t port);
};

#endif // MANAGER_SERVER_H