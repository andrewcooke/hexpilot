
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/vectors.h"

#include "glfw.h"
#include "error_codes.h"


int create_glfw_context(lulog *log, GLFWwindow **window) {
    LU_STATUS
    // not clear to me to what extent these duplicate or conflict with glad
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    LU_ASSERT(*window = glfwCreateWindow(320, 320, "hexpilot", NULL, NULL),
            HP_ERR_GLFW, log, "Could not create window")
    glfwMakeContextCurrent(*window);
    LU_NO_CLEANUP
}

int load_opengl_functions(lulog *log) {
    LU_STATUS
    LU_ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress),
            HP_ERR_GLAD, log, "Could not load OpenGL via glad")
    LU_ASSERT(GLVersion.major > 1, HP_ERR_OPENGL, log,
            "Bad OpenGL version: %d.%d", GLVersion.major, GLVersion.minor)
    luinfo(log, "OpenGL %s, GLSL %s",
            glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    LU_NO_CLEANUP
}

static void key_callback(GLFWwindow *window, int key, int scancode, int act, int mods) {
    user_action *action = glfwGetWindowUserPointer(window);
    double now = glfwGetTime();
//    ludebug(action->log, "Key %d mods %d", key, mods);
    for (size_t i = 0; i < action->controls->mem.used; ++i) {
        for (size_t j = 0; j < 2; ++j) {
            if (key == action->controls->c[i].k.keys[j] &&
                    mods == action->controls->c[i].k.mods[j]) {
                switch(act) {
                case GLFW_PRESS:
//                    ludebug(action->log, "Press key %d (%d) mods %d", key, scancode, mods);
                    action->controls->c[i].v.pressed[j] = now;
                    break;
                case GLFW_RELEASE:
//                    ludebug(action->log, "Release key %d (%d) mods %d", key, scancode, mods);
                    action->controls->c[i].v.released[j] = now;
                    break;
//                default:
//                    ludebug(action->log, "Ignored action for key %d (%d) mods %d", key, scancode, mods);
                }
//            } else {
//                ludebug(action->log, "No match for key %d mods %d",
//                        action->controls->c[i].k.keys[j], action->controls->c[i].k.mods[j]);
            }
        }
    }
}

int set_window_callbacks(lulog *log, GLFWwindow *window, user_action **action) {
    LU_STATUS
    keys k = {};
    LU_ALLOC(log, *action, 1)
    (*action)->log = log;
    (*action)->window = window;
    LU_CHECK(luary_mkcontroln(log, &(*action)->controls, 1))
    LU_CHECK(set_keys(log, &k, "zoom", 45, 0, 61, 1, 15, 5, 0.5, 100, &lumat_sclf4_3))
    LU_CHECK(luary_pushcontrol(log, (*action)->controls, &k, 1))
    LU_CHECK(set_keys(log, &k, "roll", 262, 0, 263, 0, 15, 5, -M_PI, M_PI, &lumat_rotf4_z))
    LU_CHECK(luary_pushcontrol(log, (*action)->controls, &k, 0))
    LU_CHECK(set_keys(log, &k, "pitch", 264, 0, 265, 0, 15, 5, 0, 0.5 * M_PI, &lumat_rotf4_x))
    LU_CHECK(luary_pushcontrol(log, (*action)->controls, &k, 0.25 * M_PI))
    glfwSetWindowUserPointer(window, *action);
    glfwSetKeyCallback(window, &key_callback);
LU_CLEANUP
    free(k.name);
    LU_RETURN
}
