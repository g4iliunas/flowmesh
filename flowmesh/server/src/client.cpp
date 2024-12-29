#include "client.h"
#include <spdlog/spdlog.h>
#include <uv.h>

Client::Client() : client{} { SPDLOG_DEBUG("Constructing client"); }

Client::~Client()
{
    SPDLOG_DEBUG("Destroying client");
    uv_close(reinterpret_cast<uv_handle_t *>(&this->client), nullptr);
}