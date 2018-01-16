
#ifndef HP_BUFFERS_H
#define HP_BUFFERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"

/**
 * @file
 *
 * @brief Simple wrapper round OpenGL buffers.
 *
 * An OpenGL buffer is a chunk of memory that is shared with the GPU.
 * Here the interface is simplified with the following assumptions:
 * * The data re loaded when the buffer is first created.
 * * The data do not change.
 * * The buffer is associated with a single target.
 * Given those assumptions, the only actions are creating the buffer
 * (with data), and binding / unbinding.
 */
/// @{

/// Metadata associated with a buffer.
typedef struct buffer {
    GLuint name;  ///< The buffer name (an integer, provided by OpenGL).
    GLenum target;  ///< The (single) target to which the buffer is bound.
} buffer;

/// Construct a buffer and load the data.
int buffer_mk(
		lulog *log,  ///< [in] Destination for error messages.
		buffer **buffer,  ///< [out] The newly created buffer.
		GLenum target,  ///< [in] Where the buffer will be bound.
		GLenum usage,  ///< [in] The usage pattern for the data.
        const void *data,  ///< [in] The data to be stored.
		size_t bytes  ///< [in] The amount of data to be stored.
		);

int buffer_bind(lulog *log, buffer *buffer);
int buffer_unbind(lulog *log, buffer *buffer);

/// @}
#endif
