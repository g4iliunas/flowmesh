#include "proxy_server.h"
#include "proxy_client.h"
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
    ProxyClient *pclient{};

    try {
        pclient = new ProxyClient;
    }
    catch (const std::exception &e) {
        SPDLOG_WARN("Failed to allocate memory for client: {}", e.what());
        return;
    }

    uv_tcp_t *client = pclient->get_client();
    uv_tcp_init(server->loop, client);

    if (uv_accept(server, reinterpret_cast<uv_stream_t *>(client)) != 0) {
        SPDLOG_ERROR("Failed to accept connection");
        delete pclient;
        return;
    }

    SPDLOG_DEBUG("Finished accepting the connection");
    pclient->read_start();
}

ProxyServer::ProxyServer(uv_loop_t *loop, const std::string_view &host,
                         const std::uint16_t port)
{
    uv_tcp_t *server = new uv_tcp_t;
    uv_tcp_init(loop, server);
    sockaddr_in sin{};
    uv_ip4_addr(host.data(), port, &sin);
    uv_tcp_bind(server, reinterpret_cast<const sockaddr *>(&sin), 0);
    if (uv_listen(reinterpret_cast<uv_stream_t *>(server), SOMAXCONN,
                  on_connection) != 0)
        throw std::runtime_error("Failed to listen on proxy server");
}