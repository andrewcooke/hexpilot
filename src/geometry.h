
#ifndef HP_GEOMETRY_H
#define HP_GEOMETRY_H

#include "lu/log.h"
#include "lu/gl/matrices.h"

#include "../lib/data_buffers.h"
#include "glad.h"
#include "glfw.h"

struct world;

typedef enum variable_index {
    ship_speed,
    ship_rotation,
    ship_x,
    ship_y,
    ship_z,
    ship_angle,
    camera_elevation,
    camera_distance,
    camera_zoom,
    buffer_x,
    buffer_y,
    near_z,
    far_z,
    n_variables
} variable_index;

typedef struct flight_geometry {
    luglm ship_to_hex;
    luglm ship_to_hex_n;
    luglm hex_to_camera;
    luglm hex_to_camera_n;
    luglm camera_to_clip;
    luglm camera_to_clip_n;
} flight_geometry;

typedef struct geometry_buffer {
    luglc model_colour;
    float padding;
    luglm model_to_camera;
    luglm model_to_camera_n;
    luglm camera_to_clip;
    luglm camera_to_clip_n;
    float line_width;
} geometry_buffer;

int init_geometry(lulog *log, float *variables);
int update_geometry(lulog *log, double dt, float *variables, void *data);

#endif
