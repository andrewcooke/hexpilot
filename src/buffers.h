
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

typedef struct luary_buffer {
    buffer *b;
    lumem mem;
} luary_buffer;

int luary_mkbuffern(lulog *log, luary_buffer **buffer, size_t n);
int luary_freebuffer(luary_buffer **buffer, int prev_status);
int luary_reservebuffer(lulog *log, luary_buffer *buffer, size_t n);
int luary_pushbuffer(lulog *log, luary_buffer *buffer,
        GLuint name, GLenum target, GLenum usage, size_t count, size_t bytes, size_t chunk);
size_t luary_sizebuffer(luary_buffer *buffer);

int load_buffer(lulog *log, GLenum target, GLenum usage,
        const void *data, size_t count, size_t chunk, luary_buffer **buffers);

int bind_buffer(lulog *log, buffer *buffer);
int bind_buffers(lulog *log, luary_buffer *buffers);

int unbind_buffer(lulog *log, buffer *buffer);
int unbind_buffers(lulog *log, luary_buffer *buffers);

#endif
