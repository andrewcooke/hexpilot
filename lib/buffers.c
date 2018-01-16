
#include <status.h>
#include <string.h>

#include "lu/log.h"
#include "lu/arrays.h"
#include "lu/memory.h"

#include "buffers.h"


int buffer_mk(lulog *log, buffer **buffer, GLenum target, GLenum usage,
        const void *data, size_t bytes) {
    int status = LU_OK;
    LU_ALLOC(log, *buffer, 1);
    (*buffer)->target = target;
    gl_try(glGenBuffers(1, &(*buffer)->name));
    try(buffer_bind(log, *buffer));
    gl_try(glBufferData(target, bytes, data, usage));
    try(buffer_unbind(log, *buffer));
    luinfo(log, "Loaded %zu bytes to buffer %u", bytes, buffer);
    finally:
	return status;
}

int buffer_bind(lulog *log, buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, buffer->name));
    finally:
	return status;
}

int buffer_unbind(lulog *log, buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, 0));
    finally:
	return status;
}
