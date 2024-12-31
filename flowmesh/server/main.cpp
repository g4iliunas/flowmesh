#include "src/database.h"
#include "src/manager_server.h"
#include "src/proxy_server.h"
#include <spdlog/spdlog.h>
#include <uv.h>

int main(void)
{
    spdlog::set_pattern("[%^%L%$] [%Y-%m-%d %H:%M:%S.%e] [%s:%#] %v");

#if SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_TRACE
    spdlog::set_level(spdlog::level::trace);
    spdlog::info("Set to trace level");
#elif SPDLOG_ACTIVE_LEVEL == SPDLOG_LEVEL_DEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Set to debug level");
#endif

    SPDLOG_INFO("libuv version: {}", uv_version_string());

    uv_loop_t loop;
    uv_loop_init(&loop);

    SPDLOG_TRACE("trace test");
    SPDLOG_DEBUG("debug test");

    Database database(&loop, "127.0.0.1", 6379);
    ManagerServer manager_server(&loop, &database, "127.0.0.1", 1081);
    ProxyServer proxy_server(&loop, &manager_server, "127.0.0.1", 1080);

    SPDLOG_INFO("Initialized both proxy and manager servers");

    int ret = uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);

    return ret;
}