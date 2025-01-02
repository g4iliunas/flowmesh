#ifndef DATABASE_H
#define DATABASE_H

#include "provider.h"
#include <cstdint>
#include <functional>
#include <hiredis/hiredis.h>
#include <string_view>
#include <uv.h>

#include <unordered_map>
#include <any>


class Database {
  public:
    Database(uv_loop_t *loop, const std::string_view &host,
             const std::uint16_t port);
    ~Database();

    using HashMap = std::unordered_map<std::string, std::string>;
    using HashMapCallback = std::function<void(HashMap &, std::any args)>;

    struct Query {
        HashMapCallback callback;
        std::any args;
    };

    void get_provider(const std::string_view &proxy_username,
                      HashMapCallback callback, std::any args);

    void get_provider_by_token(const std::string_view &hashed_token,
                               HashMapCallback callback, std::any args);

  private:
    redisAsyncContext *redis_ctx;
};

#endif // DATABASE_H