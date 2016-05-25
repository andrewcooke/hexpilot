
#include "lu/log.h"
#include "lu/status.h"
#include "lu/array_macros.h"

#include "error_codes.h"
#include "buffers.h"


LUARRAY_MKBASE(buffer, luarray_buffer, buffer, b)

int luarray_pushbuffer(lulog *log, luarray_buffer *buffers,
        GLuint name, GLenum target, GLenum usage, size_t count, size_t bytes, size_t chunk) {
    LU_STATUS
    LU_CHECK(luarray_reservebuffer(log, buffers, 1))
    buffers->b[buffers->mem.used++] = (buffer){name, target, usage, count, bytes, chunk};
    LU_NO_CLEANUP
}


int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, luarray_buffer *buffers) {
    LU_STATUS
    GLuint buffer;
    size_t bytes = count * chunk;
    HP_GLCHECK(glGenBuffers(1, &buffer))
    HP_GLCHECK(glBindBuffer(target, buffer))
    HP_GLCHECK(glBufferData(target, bytes, data, usage))
    HP_GLCHECK(glBindBuffer(target, 0))
    LU_CHECK(luarray_pushbuffer(log, buffers,
            buffer, target, usage, count, bytes, chunk))
    luinfo(log, "Loaded %zu bytes (%zu x %zu) to buffer %u (%zu)",
            bytes, count, chunk, buffer, buffers->mem.used);
    LU_NO_CLEANUP
}
