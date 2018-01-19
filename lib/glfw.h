
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "lu/log.h"

#include "glad.h"
#include <GLFW/glfw3.h>

/**
 * @file
 *
 * @brief Wrapper around GLFW functionality.
 *
 * GLFW manages the "gap" between OpenGL and the local OS.  It is
 * responsible for starting and shutting-down OpenGL, handling
 * key events, etc.
 *
 * Note that library static data are used for logging and status
 * handling.
 */
/// @{

/// Start-up GLFW, glad, and OpenGL.
int glfw_init(lulog *log, GLFWwindow **window);
/// Set the keypress callback and associated data pointer.
int glfw_set_key_callback(lulog *log, GLFWwindow *window, GLFWkeyfun callback, void *data);
/// Shut-down GLFW (can be called even if glfw_init was not called).
int glfw_final(int status);
/// Timing data
typedef struct {
    double last_update;  ///< Time when last updated.
    double fps;  ///< Current FPS estimate.
    double fps_interval_start;  ///< Time when we last zeroed the FPS counter.
    int frame_count;  ///< Frame counter.
} glfw_timing;
/// Initialize the timing information.
int glfw_timing_init(lulog *log, glfw_timing *clock);
/// @brief Update the timing information (call once per frame).
/// @return The time passed (in seconds) during this frame.
double glfw_timing_update(lulog *log, glfw_timing *clock);

/// @}
#endif
