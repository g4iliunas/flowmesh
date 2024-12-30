#include "client.h"
#include <spdlog/spdlog.h>
#include <uv.h>

Client::Client() {
    SPDLOG_DEBUG("Constructing client");
    this->client = new uv_tcp_t;
}

Client::~Client()
{
    SPDLOG_DEBUG("Destroying client");
    uv_close(reinterpret_cast<uv_handle_t *>(this->client), [](uv_handle_t *handle){
        delete handle;
    });
}