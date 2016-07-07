
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
    // render new frame to single, blit that to tmp to sample, then add to
    // multiple.  copy that to output.  then blur multiple in two passes,
    // ping-pong through tmp.
    frame single;
    frame multiple;
    frame tmp;
} flight_data;

int build_flight(lulog *log, void *programs, GLFWwindow *window, world **world);

#endif
