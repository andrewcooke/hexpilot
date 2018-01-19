
#include <stdlib.h>
#include <math.h>
#include <status.h>
#include <string.h>

#include "lu/log.h"
#include "lu/internal.h"
#include "lu/gl/vectors.h"

#include "glfw.h"


static lulog *LOG = NULL;
static int STATUS = LU_OK;

#define glfw_try(operation) operation; {\
	if (STATUS) {\
		status = HP_ERR_GLFW;\
		STATUS = LU_OK;\
		goto finally;\
	}}

static void on_error(int error, const char *message) {
    luerror(LOG, "GLFW: %s (code %d)", message, error);
    STATUS = error;
}

static int create_glfw_context(lulog *log, GLFWwindow **window) {
    int status = LU_OK;
    // not clear to me to what extent these duplicate or conflict with glad
    glfw_try(glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3));
    glfw_try(glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3));
    glfw_try(glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1));
    glfw_try(glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE));
    glfw_try(glfwWindowHint(GLFW_SAMPLES, 4));
    assert(*window = glfwCreateWindow(320, 320, "hexpilot", NULL, NULL),
            HP_ERR_GLFW, log, "Could not create window");
    glfw_try(glfwMakeContextCurrent(*window));
    glfw_try(glfwSwapInterval(1));  // 0 to see raw fps
    finally:
	return status;
}

static int load_opengl_functions(lulog *log) {
    int status = LU_OK;
    assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress),
            HP_ERR_GLAD, log, "Could not load OpenGL via glad");
    assert(GLVersion.major > 1, HP_ERR_OPENGL, log,
            "Bad OpenGL version: %d.%d", GLVersion.major, GLVersion.minor);
    luinfo(log, "OpenGL %s, GLSL %s",
            glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    finally:
	return status;
}


int glfw_init(lulog *log, GLFWwindow **window) {
	int status = LU_OK;
    glfw_try(glfwSetErrorCallback(on_error));
    glfw_try(glfwInit());
	try(create_glfw_context(log, window));
	try(load_opengl_functions(log));
	finally:
	return status;
}

int glfw_set_key_callback(lulog *log, GLFWwindow *window, GLFWkeyfun callback, void *data) {
	int status = LU_OK;
    glfw_try(glfwSetWindowUserPointer(window, data));
    glfw_try(glfwSetKeyCallback(window, callback));
	finally:
	return status;
}

int glfw_final(int prev) {
	int status = LU_OK;
	glfw_try(glfwTerminate());
	finally:
	STATUS = LU_OK;
	LOG = NULL;
	return lu_both(prev, status);
}


int glfw_timing_init(lulog *log, glfw_timing *clock) {
    int status = LU_OK;
    glfw_timing zero = {};
    *clock = zero;
    clock->last_update = clock->fps_interval_start = glfwGetTime();
    finally:
	return status;
}

double glfw_timing_update(lulog *log, glfw_timing *clock) {
    double now = glfwGetTime();
    if (now > clock->fps_interval_start+1) {
        double fps = clock->frame_count / (now - clock->fps_interval_start);
        if (abs(fps - clock->fps) > clock->fps / 60.0) {
            // by default glfw syncs to 60fps (more exactly,
            // whatever freq the monitor uses).  see glfw.c
            // to enable free-spinning.
            clock->fps = fps;
            ludebug(log, "FPS: %0.1f", fps);
        }
        clock->fps_interval_start = now;
        clock->frame_count = 0;
    }
    double delta = now - clock->last_update;
    clock->last_update = now;
    clock->frame_count++;
    return delta;
}
