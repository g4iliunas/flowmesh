cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_STANDARD 99)
project(flowmesh_server)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
add_executable(flowmesh_server main.c src/log.c src/proxy_server.c src/proxy_callbacks.c src/socks5.c)
target_compile_options(flowmesh_server PRIVATE -Wall -Wextra -Wpedantic -Werror)
# link libuv
target_include_directories(flowmesh_server PRIVATE ${LIBUV_INCLUDE_DIRS})
target_link_libraries(flowmesh_server PkgConfig::LIBUV)
# definitions
target_compile_definitions(flowmesh_server PRIVATE LOG_USE_COLOR)