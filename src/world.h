
#ifndef HP_FLIGHT_H
#define HP_FLIGHT_H


#include "frames.h"
#include "worlds.h"
#include "universe.h"

#include "geometry.h"


typedef struct flight_data {
    flight_geometry geometry;
    buffer *quad_buffer;
    GLuint quad_vao;
    frame single;
    frame multiple;
    frame tmp1;
    frame tmp2;
} flight_data;

int build_flight_blur(lulog *log, void *programs, GLFWwindow *window, world **world);
int build_flight_direct(lulog *log, void *v, GLFWwindow *window, world **world);

#endif
