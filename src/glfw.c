
#include <stdlib.h>
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/arrays.h"

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

static void size_callback(GLFWwindow* window, int width, int height) {
    user_action *action = glfwGetWindowUserPointer(window);
    ludebug(action->log, "Framebuffer changed to %d,%d", width, height);
    action->framebuffer_size_change = 1;
    action->any_change = 1;
}

int set_window_callbacks(lulog *log, GLFWwindow *window, user_action **action) {
    LU_STATUS
    LU_ALLOC(log, *action, 1)
    (*action)->log = log;
    (*action)->window = window;
    glfwSetWindowUserPointer(window, *action);
    glfwSetFramebufferSizeCallback(window, &size_callback);
    LU_NO_CLEANUP
}
