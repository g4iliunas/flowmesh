#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <uv.h>

void alloc_buf([[maybe_unused]] uv_handle_t *handle, std::size_t suggested_size,
               uv_buf_t *buf);

#endif