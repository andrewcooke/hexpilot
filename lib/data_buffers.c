
#include <data_buffers.h>
#include <status.h>
#include <string.h>

#include "lu/log.h"
#include "lu/arrays.h"
#include "lu/memory.h"



int data_buffer_mk(lulog *log, data_buffer **buffer, GLenum target, GLenum usage,
        const void *data, size_t bytes) {
    int status = LU_OK;
    LU_ALLOC(log, *buffer, 1);
    (*buffer)->target = target;
    gl_try(glGenBuffers(1, &(*buffer)->name));
    try(data_buffer_bind(log, *buffer));
    gl_try(glBufferData(target, bytes, data, usage));
    try(data_buffer_unbind(log, *buffer));
    luinfo(log, "Loaded %zu bytes to buffer %u", bytes, buffer);
    finally:
	return status;
}

int data_buffer_bind(lulog *log, data_buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, buffer->name));
    finally:
	return status;
}

int data_buffer_unbind(lulog *log, data_buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, 0));
    finally:
	return status;
}
