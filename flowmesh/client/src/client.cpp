#include "client.h"
#include <spdlog/spdlog.h>
#include <uv.h>

static void on_connect(uv_connect_t *req, int status)
{
    if (status < 0) {
        SPDLOG_ERROR("Connect error {}", uv_strerror(status));

        delete req;
        return;
    }

    SPDLOG_INFO("Connected");
    delete req;
}

Client::Client(uv_loop_t *loop, const std::string_view &server_host,
               const std::uint16_t server_port)
{
    this->server = new uv_tcp_t;
    uv_tcp_init(loop, this->server);
    this->server->data = this;
    sockaddr_in sin{};
    uv_ip4_addr(server_host.data(), server_port, &sin);
    uv_tcp_connect(new uv_connect_t, this->server,
                   reinterpret_cast<const sockaddr *>(&sin), on_connect);
}

Client::~Client() {
    uv_close(reinterpret_cast<uv_handle_t *>(this->server),
             [](uv_handle_t *handle) { delete handle; });
}