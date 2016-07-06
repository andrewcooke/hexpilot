
#ifndef HP_FLIGHT_H
#define HP_FLIGHT_H


#include "worlds.h"
#include "universe.h"

#include "geometry.h"


typedef struct flight_data {
    flight_geometry geometry;
    GLuint render;
    GLuint texture;
    buffer *quad_buffer;
    GLuint quad_vao;
} flight_data;

int build_flight(lulog *log, void *programs, GLFWwindow *window, world **world);

#endif
