#include "database.h"
#include <hiredis/adapters/libuv.h>
#include <hiredis/async.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

static void on_connect(const redisAsyncContext *c, int status)
{
    SPDLOG_DEBUG("Connected!");
}

static void on_disconnect(const redisAsyncContext *c, int status)
{
    SPDLOG_DEBUG("Disconnected");
}

Database::Database(uv_loop_t *loop, const std::string_view &host,
                   const std::uint16_t port)
    : redis_ctx{}
{
    this->redis_ctx = redisAsyncConnect(host.data(), port);
    if (!this->redis_ctx)
        throw std::runtime_error("Failed to connect to Redis database");

    if (this->redis_ctx->err)
        throw std::runtime_error(
            std::string(
                "Failed to connect to Redis database. Error message: ") +
            this->redis_ctx->errstr);

    redisLibuvAttach(this->redis_ctx, loop);
    redisAsyncSetConnectCallback(this->redis_ctx, on_connect);
    redisAsyncSetDisconnectCallback(this->redis_ctx, on_disconnect);
}

Database::~Database()
{
    if (this->redis_ctx)
        redisAsyncFree(this->redis_ctx);
}
