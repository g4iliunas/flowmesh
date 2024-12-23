#ifndef MANAGER_SERVER_H
#define MANAGER_SERVER_H

#include <stdbool.h>
#include <uv.h>

typedef struct fm_manager_client {
    int state;
    bool disconnect;
} fm_manager_client_t;

uv_tcp_t *fm_manager_server_init(uv_loop_t *loop, const char *address,
                                 const uint16_t port);

#endif