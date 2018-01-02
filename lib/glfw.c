
#include <stdlib.h>
#include <math.h>
#include <status.h>
#include <string.h>

#include "lu/log.h"
#include "lu/internal.h"
#include "lu/gl/vectors.h"

#include "glfw.h"


static lulog *LOG = NULL;

static void on_error(int error, const char *message) {
    luerror(LOG, "GLFW: %s (code %d)", error, message);
}

int init_glfw(lulog *log) {
    int status = LU_OK;
    LOG = log;
    glfwSetErrorCallback(on_error);
    assert(glfwInit(), HP_ERR_GLFW, log, "Could not start GLFW");
    finally:
	return status;
}

int create_glfw_context(lulog *log, GLFWwindow **window) {
    int status = LU_OK;
    // not clear to me to what extent these duplicate or conflict with glad
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    assert(*window = glfwCreateWindow(320, 320, "hexpilot", NULL, NULL),
            HP_ERR_GLFW, log, "Could not create window");
    glfwMakeContextCurrent(*window);
    glfwSwapInterval(1);  // 0 to see raw fps
    finally:
	return status;
}

int load_opengl_functions(lulog *log) {
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

static void key_callback(GLFWwindow *window, int key, int scancode, int act, int mods) {
    user_action *action = glfwGetWindowUserPointer(window);
    double now = glfwGetTime();
    for (size_t i = 0; i < action->controls->mem.used; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            if (key == action->controls->c[i].k.keys[j] &&
                    mods == action->controls->c[i].k.mods[j]) {
                switch(act) {
                case GLFW_PRESS:
                    ludebug(action->log, "Pressed %s (%d/%d)",
                            action->controls->c[i].k.name, key, mods);
                    action->controls->c[i].v.pressed[j] = now;
                    break;
                case GLFW_RELEASE:
                    action->controls->c[i].v.released[j] = now;
                    break;
                }
                return;
            }
        }
    }
    if (act == GLFW_PRESS) ludebug(action->log, "Unused key (%d/%d)", key, mods);
}

int set_window_callbacks(lulog *log, GLFWwindow *window, user_action *action) {
    int status = LU_OK;
    glfwSetWindowUserPointer(window, action);
    glfwSetKeyCallback(window, &key_callback);
    finally:
	return status;
}


int init_timing(lulog *log, timing *clock) {
    int status = LU_OK;
    timing zero = {};
    *clock = zero;
    clock->previous = clock->seconds = glfwGetTime();
    finally:
	return status;
}

double update_timing(lulog *log, timing *clock) {
    double now = glfwGetTime();
    if (now > clock->seconds+1) {
        double fps = clock->frame_count / (now - clock->seconds);
        if (abs(fps - clock->fps) > 0.1 * clock->fps) {
            // by default glfw syncs to 60fps (more exactly,
            // whatever freq the monitor uses).  see glfw.c
            // to enable free-spinning.
            clock->fps = fps;
            ludebug(log, "FPS: %0.1f", fps);
        }
        clock->seconds = now;
        clock->frame_count = 0;
    }
    double delta = now - clock->previous;
    clock->previous = now;
    clock->frame_count++;
    return delta;
}

