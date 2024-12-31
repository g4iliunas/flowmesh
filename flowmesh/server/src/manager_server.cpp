#include "manager_server.h"

ManagerServer::ManagerServer(uv_loop_t *loop, Database *database,
                             const std::string_view &host,
                             const std::uint16_t port)
    : database(database)
{
    // alloc server
    // ...
}