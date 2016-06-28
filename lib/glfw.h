
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "lu/log.h"

#include "glad.h"
#include <GLFW/glfw3.h>

#include "keys.h"

int init_glfw(lulog *log);
int create_glfw_context(lulog *log, GLFWwindow **window);
int load_opengl_functions(lulog *log);
int set_window_callbacks(lulog *log, GLFWwindow *window, user_action *action);

typedef struct timing {
    double previous;
    double fps;
    double seconds;
    int frame_count;
} timing;

int init_timing(lulog *log, timing *clock);
double update_timing(lulog *log, timing *clock);

#endif
