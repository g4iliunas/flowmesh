#include "utils.h"

void alloc_buf(uv_handle_t *handle, std::size_t suggested_size, uv_buf_t *buf)
{
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}