#include "src/manager_server.h"
#include "src/proxy_server.h"

#if !defined(NDEBUG) &&                                                        \
    !defined(SPDLOG_ACTIVE_LEVEL) // in case TRACE wasn't set
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif
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

    ProxyServer proxy_server(&loop, "127.0.0.1", 1080);
    ManagerServer manager_server(&loop, "127.0.0.1", 1081);

    SPDLOG_INFO("Initialized both proxy and manager servers");

    int ret = uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);

    return ret;
}