#ifndef CLIENT_H
#define CLIENT_H

#include <cstddef>
#include <string_view>
#include <uv.h>

class Client {
  public:
    Client(uv_loop_t *loop, const std::string_view &server_host,
           const std::uint16_t server_port);
    ~Client();

  private:
    uv_tcp_t *server;
};

#endif // CLIENT_H