
#ifndef HP_FRAME_BUFFERS_H
#define HP_FRAME_BUFFERS_H

#include "glfw.h"
#include "lu/log.h"

/**
 * @file
 *
 * @brief Wrapper round OpenGL frame buffers.
 *
 * An OpenGL frame buffer is an in-memory "display surface".  It's
 * a container for a texture and an optional depth buffer.
 * Scenes can be rendered to buffers and their contents manipulated.
 */
/// @{

/// Metadata associated with a frame buffer
typedef struct {
    GLuint name;  ///< The frame buffer name (an integer, provided by OpenGL).
    int multisample_aa;  ///< Enable multisample antialias?
    int width;  ///< Width in pixels.
    int height;  ///< Height in pixels.
    GLuint texture;  ///< The texture contained.
    GLuint depth;  ///< The optional depth buffer.
} frame_buffer;

int frame_buffer_mk(
		lulog *log,  ///< [in] Destination for error messages.
		frame_buffer *frame,  ///< [out] The newly created frame buffer.
		GLFWwindow *window,  ///< [in] The GLFW context, to get the window size.
		int multisample_aa,  ///< [in] Whether to use multi-sampling.
		int depth  ///< [in] Whether to use a depth buffer.
		);

/// Rescale the the buffer if necessary.
int frame_buffer_rescale(lulog *log, frame_buffer *frame, GLFWwindow *window);

/// @}
#endif
