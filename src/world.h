
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
    // render new frame to single, blit that to tmp1 to sample, then add to
    // multiple in tmp1.  copy tmp1 to output then blut via tmp2 into multiple.
    frame single;
    frame multiple;
    frame tmp1;
    frame tmp2;
} flight_data;

int build_flight(lulog *log, void *programs, GLFWwindow *window, world **world);

#endif
