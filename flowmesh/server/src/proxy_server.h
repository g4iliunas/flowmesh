#ifndef PROXY_SERVER_H
#define PROXY_SERVER_H

#include <cstdint>
#include <string_view>
#include <uv.h>

class ProxyServer {
  public:
    ProxyServer(uv_loop_t *loop, const std::string_view &host,
                const std::uint16_t port);
};

#endif // PROXY_SERVER_H