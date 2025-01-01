cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(flowmesh_server)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
find_package(spdlog REQUIRED)
find_package(hiredis REQUIRED)

add_executable(flowmesh_server main.cpp
    src/proxy_server.cpp
    src/manager_server.cpp
    src/client.cpp
    src/proxy_client.cpp
    src/utils.cpp
    src/socks5.cpp
    src/consumer_callbacks.cpp
    src/database.cpp
    src/provider.cpp
)

target_include_directories(flowmesh_server PRIVATE ${LIBUV_INCLUDE_DIRS})
target_compile_options(flowmesh_server PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowmesh_server PRIVATE PkgConfig::LIBUV spdlog::spdlog hiredis::hiredis) 


if (DEFINED TRACE)
    message("TRACE is enabled")
    target_compile_definitions(flowmesh_server PUBLIC SPDLOG_ACTIVE_LEVEL=0)
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("DEBUG is enabled")
    target_compile_definitions(flowmesh_server PUBLIC SPDLOG_ACTIVE_LEVEL=1)
endif()

#if (DEFINED TRACE OR CMAKE_BUILD_TYPE STREQUAL "Debug")
#    message("Address sanitizer is enabled")
#    target_compile_options(flowmesh_server PRIVATE -fsanitize=address)
#    target_link_options(flowmesh_server PRIVATE -fsanitize=address)
#endif()