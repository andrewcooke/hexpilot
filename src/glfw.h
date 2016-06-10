
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "lu/log.h"

#include "glad.h"
#include <GLFW/glfw3.h>

#include "keys.h"


int create_glfw_context(lulog *log, GLFWwindow **window);
int load_opengl_functions(lulog *log);
int set_window_callbacks(lulog *log, GLFWwindow *window, user_action *action);

#endif
