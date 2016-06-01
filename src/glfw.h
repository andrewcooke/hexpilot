
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"


typedef struct user_action {
    lulog *log;
    GLFWwindow *window;
    int framebuffer_size_change;
    int any_change;
} user_action;

int create_glfw_context(lulog *log, GLFWwindow **window);
int load_opengl_functions(lulog *log);
int set_window_callbacks(lulog *log, GLFWwindow *window, user_action **action);

#endif
