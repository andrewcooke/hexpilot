
#include "lu/log.h"
#include "lu/status.h"

#include "error_codes.h"
#include "buffers.h"


int load_buffer(lulog *log, GLenum target,
        const void *data, size_t size, luarray_uint *buffers) {
    LU_STATUS
    GLuint buffer;
    HP_GLCHECK(glGenBuffers(1, &buffer))
    HP_GLCHECK(glBindBuffer(target, buffer))
    HP_GLCHECK(glBufferData(target, size, data, GL_STATIC_DRAW))
    HP_GLCHECK(glBindBuffer(target, 0))
    LU_CHECK(luarray_pushuint(log, buffers, buffer))
    luinfo(log, "Loaded %zu bytes to buffer %d", size, buffers->mem.used);
    LU_NO_CLEANUP
}

