
#ifndef HP_BUFFERS_H
#define HP_BUFFERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"


typedef struct buffer {
    GLuint name;
    GLenum target;
    GLenum usage;
    size_t count;  // number of values
    size_t bytes;  // total space consumed
    size_t chunk;  // size (bytes) of one value
} buffer;

typedef struct luarray_buffer {
    buffer *b;
    lumem mem;
} luarray_buffer;

int luarray_mkbuffern(lulog *log, luarray_buffer **buffer, size_t n);
int luarray_freebuffer(luarray_buffer **buffer, int prev_status);
int luarray_reservebuffer(lulog *log, luarray_buffer *buffer, size_t n);
int luarray_pushbuffer(lulog *log, luarray_buffer *buffer,
        GLuint name, GLenum target, GLenum usage, size_t count, size_t bytes, size_t chunk);
size_t luarray_sizebuffer(luarray_buffer *buffer);

int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, luarray_buffer **buffers);
int bind_buffers(lulog *log, luarray_buffer *buffers);
int unbind_buffers(lulog *log, luarray_buffer *buffers);

#endif
