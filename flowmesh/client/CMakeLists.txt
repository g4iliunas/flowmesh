cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
project(flowmesh_client)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
find_package(spdlog REQUIRED)

add_executable(flowmesh_client main.cpp)

target_include_directories(flowmesh_client PRIVATE ${LIBUV_INCLUDE_DIRS})
target_compile_options(flowmesh_client PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(flowmesh_client PRIVATE PkgConfig::LIBUV spdlog::spdlog)

if (DEFINED TRACE)
    message("TRACE is enabled")
    target_compile_definitions(flowmesh_server PUBLIC SPDLOG_ACTIVE_LEVEL=0)
elseif (NOT DEFINED NDEBUG)
    message("NDEBUG is enabled")
    target_compile_definitions(flowmesh_server PUBLIC SPDLOG_ACTIVE_LEVEL=1)
endif()