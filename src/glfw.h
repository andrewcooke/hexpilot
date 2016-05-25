
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"


int create_glfw_context(const lulog *log, GLFWwindow **window);
int load_opengl_functions(const lulog *log);

#endif
