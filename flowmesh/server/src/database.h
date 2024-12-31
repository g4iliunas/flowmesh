#ifndef DATABASE_H
#define DATABASE_H

#include "provider.h"
#include <cstdint>
#include <hiredis/hiredis.h>
#include <string_view>
#include <uv.h>

class Database {
  public:
    Database(uv_loop_t *loop, const std::string_view &host,
             const std::uint16_t port);
    ~Database();

  private:
    redisAsyncContext *redis_ctx;
};

#endif