#ifndef UTILS_H
#define UTILS_H

#include <uv.h>
#include <cstddef>

void alloc_buf(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf);

#endif