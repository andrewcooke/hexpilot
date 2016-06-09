
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"

#include "keys.h"


typedef struct user_action {
    lulog *log;
    GLFWwindow *window;
    luary_control *controls;
} user_action;

int create_glfw_context(lulog *log, GLFWwindow **window);
int load_opengl_functions(lulog *log);
int set_window_callbacks(lulog *log, GLFWwindow *window, user_action **action);
int respond_to_user(lulog *log, double dt, user_action *action, float *variables);

#endif
