#include "proxy_server.h"
#include "consumer.h"
#include "manager_server.h"
#include <spdlog/spdlog.h>
#include <sys/socket.h> // somaxconn
#include <uv.h>

static void on_connection(uv_stream_t *server, int status)
{
    if (status < 0) {
        SPDLOG_ERROR("Failed to accept connection: {}", uv_strerror(status));
        return;
    }

    SPDLOG_INFO("Received a connection");
    Consumer *consumer{};

    try {
        consumer =
            new Consumer(reinterpret_cast<ManagerServer *>(server->data));
    }
    catch (const std::exception &e) {
        SPDLOG_WARN("Failed to allocate memory for client: {}", e.what());
        return;
    }

    uv_tcp_t *client = consumer->get_client();
    uv_tcp_init(server->loop, client);

    if (uv_accept(server, reinterpret_cast<uv_stream_t *>(client)) != 0) {
        SPDLOG_ERROR("Failed to accept connection");
        delete consumer;
        return;
    }

    SPDLOG_DEBUG("Finished accepting the connection");
    consumer->read_start();
}

ProxyServer::ProxyServer(uv_loop_t *loop, ManagerServer *manager,
                         const std::string_view &host, const std::uint16_t port)
{
    this->server = new uv_tcp_t;
    uv_tcp_init(loop, this->server);
    this->server->data = manager;
    sockaddr_in sin{};
    uv_ip4_addr(host.data(), port, &sin);
    uv_tcp_bind(this->server, reinterpret_cast<const sockaddr *>(&sin), 0);
    if (uv_listen(reinterpret_cast<uv_stream_t *>(this->server), SOMAXCONN,
                  on_connection) != 0)
        throw std::runtime_error("Failed to listen on proxy server");
}

ProxyServer::~ProxyServer()
{
    uv_close(reinterpret_cast<uv_handle_t *>(this->server),
             [](uv_handle_t *handle) { delete handle; });
}