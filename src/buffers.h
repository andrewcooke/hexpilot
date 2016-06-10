
#ifndef HP_BUFFERS_H
#define HP_BUFFERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"


typedef struct buffer {
    GLuint name;
    GLenum target;
} buffer;

int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, buffer **buffer);

int bind_buffer(lulog *log, buffer *buffer);
int unbind_buffer(lulog *log, buffer *buffer);

#endif
