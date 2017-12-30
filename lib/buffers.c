
#include <status_codes.h>
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/array_macros.h"

#include "buffers.h"


int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, buffer **buffer) {
    int status = LU_OK;
    size_t bytes = chunk * count;
    LU_ALLOC(log, *buffer, 1)
    (*buffer)->target = target;
    GL_CHECK(glGenBuffers(1, &(*buffer)->name))
    try(bind_buffer(log, *buffer))
    GL_CHECK(glBufferData(target, bytes, data, usage))
    try(unbind_buffer(log, *buffer))
    luinfo(log, "Loaded %zu bytes (%zu x %zu) to buffer %u",
            bytes, count, chunk, buffer);
    exit:return status;
}

int bind_buffer(lulog *log, buffer *buffer) {
    int status = LU_OK;
    GL_CHECK(glBindBuffer(buffer->target, buffer->name))
    exit:return status;
}

int unbind_buffer(lulog *log, buffer *buffer) {
    int status = LU_OK;
    GL_CHECK(glBindBuffer(buffer->target, 0))
    exit:return status;
}
