#include "database.h"
#include <hiredis/adapters/libuv.h>
#include <hiredis/async.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <memory>

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

static void get_hashmap_cb(redisAsyncContext *c, void *r, void *privdata)
{
    if (!privdata)
        return;

    auto q = std::unique_ptr<Database::Query>(reinterpret_cast<Database::Query *>(privdata));
    redisReply *reply = reinterpret_cast<redisReply *>(r);

    Database::HashMap hashmap{};
    if (!reply) {
        SPDLOG_WARN("Couldnt fetch the result from the db");
        q->callback(hashmap, q->args);
        return;
    }

    // why would we waste cpu clocks when result isnt needed?
    if (!q->callback)
        return;

    for (std::size_t i = 0; i < reply->elements; i += 2)
        hashmap[reply->element[i]->str] = reply->element[i + 1]->str;

    q->callback(hashmap, q->args);
}

void Database::get_provider(const std::string_view &proxy_username,
                            HashMapCallback callback, std::any args)
{
    std::string query = "HGETALL provider:" + std::string(proxy_username);
    redisAsyncCommand(this->redis_ctx, get_hashmap_cb,
                      reinterpret_cast<void *>(new Query{callback, args}),
                      query.c_str());
}

void Database::get_provider_by_token(const std::string_view &hashed_token,
                                     HashMapCallback callback, std::any args)
{
    std::string query = "HGETALL provider_hash:" + std::string(hashed_token);
    redisAsyncCommand(this->redis_ctx, get_hashmap_cb,
                      reinterpret_cast<void *>(new Query{callback, args}),
                      query.c_str());
}