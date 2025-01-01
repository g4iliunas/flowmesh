#ifndef CLIENT_H
#define CLIENT_H

#include "utils.h"
#include <spdlog/spdlog.h>
#include <uv.h>
#include <vector>

template <typename Derived> class Client {
  public:
    Client() : disconnect(false)
    {
        SPDLOG_DEBUG("Constructing client");
        this->client = new uv_tcp_t;
    }

    ~Client()
    {
        SPDLOG_DEBUG("Destroying client");
        uv_close(reinterpret_cast<uv_handle_t *>(this->client),
                 [](uv_handle_t *handle) { delete handle; });
    }

    uv_tcp_t *get_client() { return this->client; }

    void read_start()
    {
        uv_read_start(
            reinterpret_cast<uv_stream_t *>(this->get_client()), alloc_buf,
            [](uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
                auto *client = reinterpret_cast<Derived *>(stream->data);
                if (nread < 0) {
                    if (nread != UV_EOF)
                        SPDLOG_WARN("Read error: {}", uv_strerror(nread));
                    else
                        SPDLOG_DEBUG("Client disconnected");

                    delete client;

                    if (buf->base != nullptr)
                        delete[] buf->base;
                    return;
                }

                SPDLOG_DEBUG("Read {} bytes", nread);
                client->handle_buf(std::string_view(buf->base, nread));
                delete[] buf->base;
            });
    }

    void write(std::vector<uv_buf_t> &bufs, bool disconnect)
    {
        this->disconnect = disconnect;
        uv_write_t *req = new uv_write_t;
        uv_write(req, reinterpret_cast<uv_stream_t *>(this->get_client()),
                 bufs.data(), bufs.size(), [](uv_write_t *req, int status) {
                     auto *client =
                         reinterpret_cast<Derived *>(req->handle->data);
                     delete req;

                     if (status < 0) {
                         SPDLOG_ERROR("Write error: {}", uv_strerror(status));
                         delete client;
                         return;
                     }

                     if (client->is_disconnect()) {
                         SPDLOG_DEBUG(
                             "Disconnecting client because the flag was set");
                         delete client;
                         return;
                     }
                 });
    }

    bool is_disconnect() const { return this->disconnect; }

  private:
    uv_tcp_t *client;
    bool disconnect;
};

#endif // CLIENT_H
