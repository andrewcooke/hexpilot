
#ifndef HP_FRAMES_H
#define HP_FRAMES_H

#include "glfw.h"
#include "lu/log.h"


typedef struct frame {
    int width;
    int height;
    GLuint render;
    GLuint texture;
    GLuint depth;
} frame;

int init_frame(lulog *log, GLFWwindow *window, frame *frame);
int free_frame_contents(lulog *log, frame *frame);
int check_frame(lulog *log, GLFWwindow *window, frame *frame);

#endif
