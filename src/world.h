
#ifndef HP_FLIGHT_H
#define HP_FLIGHT_H


#include "../lib/frame_buffers.h"
#include "worlds.h"
#include "universe.h"

#include "geometry.h"


typedef struct flight_data {
    flight_geometry geometry;
    data_buffer *quad_buffer;
    GLuint quad_vao;
    frame_buffer single;
    frame_buffer multiple;
    frame_buffer tmp1;
    frame_buffer tmp2;
} flight_data;

int build_flight_blur(lulog *log, void *programs, GLFWwindow *window, world **world);

#endif
