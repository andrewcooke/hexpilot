
#include <status_codes.h>
#include <string.h>

#include "lu/log.h"
#include "lu/array_macros.h"

#include "buffers.h"


int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, buffer **buffer) {
    int status = LU_OK;
    size_t bytes = chunk * count;
    lu_alloc(log, *buffer, 1);
    (*buffer)->target = target;
    gl_try(glGenBuffers(1, &(*buffer)->name));
    try(bind_buffer(log, *buffer));
    gl_try(glBufferData(target, bytes, data, usage));
    try(unbind_buffer(log, *buffer));
    luinfo(log, "Loaded %zu bytes (%zu x %zu) to buffer %u",
    		bytes, count, chunk, buffer);
    finally:
	return status;
}

int bind_buffer(lulog *log, buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, buffer->name));
    finally:
	return status;
}

int unbind_buffer(lulog *log, buffer *buffer) {
    int status = LU_OK;
    gl_try(glBindBuffer(buffer->target, 0));
    finally:
	return status;
}
