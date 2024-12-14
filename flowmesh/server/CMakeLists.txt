cmake_minimum_required(VERSION 3.10)
project(flowmesh_server)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
add_executable(flowmesh_server main.c src/log.c src/proxy_server.c src/socks5.c)
# link libuv
target_include_directories(flowmesh_server PRIVATE ${LIBUV_INCLUDE_DIRS})
target_link_libraries(flowmesh_server PkgConfig::LIBUV)
# definitions
target_compile_definitions(flowmesh_server PRIVATE LOG_USE_COLOR)