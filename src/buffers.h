
#ifndef HP_BUFFERS_H
#define HP_BUFFERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"

int load_buffer(lulog *log, GLenum target,
        const void *data, size_t size, luarray_uint *buffers);

#endif
