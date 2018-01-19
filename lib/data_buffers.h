
#ifndef HP_DATA_BUFFERS_H
#define HP_DATA_BUFFERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"

/**
 * @file
 *
 * @brief Wrapper round OpenGL (data) buffers.
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
typedef struct {
    GLuint name;  ///< The buffer name (an integer, provided by OpenGL).
    GLenum target;  ///< The (single) target to which the buffer is bound.
} data_buffer;

/// Construct a buffer and load the data.
int data_buffer_mk(
		lulog *log,  ///< [in] Destination for error messages.
		data_buffer **buffer,  ///< [out] The newly created buffer.
		GLenum target,  ///< [in] Where the buffer will be bound.
		GLenum usage,  ///< [in] The usage pattern for the data.
        const void *data,  ///< [in] The data to be stored.
		size_t bytes  ///< [in] The amount of data to be stored.
		);

/// Bind the buffer to its target.
int data_buffer_bind(lulog *log, data_buffer *buffer);
/// Unbind the buffer from its target.
int data_buffer_unbind(lulog *log, data_buffer *buffer);

/// @}
#endif
