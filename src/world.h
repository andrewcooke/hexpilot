
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
    frame blurred;
} flight_data;

int build_flight(lulog *log, void *programs, GLFWwindow *window, world **world);

#endif
