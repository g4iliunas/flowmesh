cmake_minimum_required(VERSION 3.10)
project(redistest)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
find_package(hiredis REQUIRED)

add_executable(redistest main.cpp)

target_include_directories(redistest PRIVATE ${LIBUV_INCLUDE_DIRS})
target_link_libraries(redistest PRIVATE PkgConfig::LIBUV hiredis::hiredis) 