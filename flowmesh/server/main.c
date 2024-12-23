#include "src/log.h"
#include "src/proxy_server.h"
#include "src/manager_server.h"

#include <stdlib.h>
#include <uv.h>

int main(void)
{
    log_info("libuv version: %s", uv_version_string());

    uv_loop_t loop;
    uv_loop_init(&loop);

    uv_tcp_t *proxy_server = fm_proxy_server_init(&loop, "127.0.0.1", 1080);
    if (!proxy_server) {
        log_error("Failed to initialize the proxy server");
        return 1;
    }

    uv_tcp_t *manager_server = fm_manager_server_init(&loop, "127.0.0.1", 1081);
    if (!manager_server) {
        free(proxy_server);
        log_error("Failed to initialize the manager server");
        return 1;
    }

    log_info("Running both proxy and manager servers");

    int ret = uv_run(&loop, UV_RUN_DEFAULT);
    uv_loop_close(&loop);

    free(proxy_server);
    free(manager_server);

    return ret;
}