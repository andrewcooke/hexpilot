
#ifndef HP_GLFW_H
#define HP_GLFW_H

#include "lu/log.h"

#include "glad.h"
#include <GLFW/glfw3.h>

int glfw_init(lulog *log, GLFWwindow **window);
int glfw_set_key_callback(lulog *log, GLFWwindow *window, GLFWkeyfun callback, void *data);
int glfw_close(int status);

typedef struct timing {
    double previous;
    double fps;
    double seconds;
    int frame_count;
} timing;

int init_timing(lulog *log, timing *clock);
double update_timing(lulog *log, timing *clock);

#endif
