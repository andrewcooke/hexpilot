
#ifndef HP_GEOMETRY_H
#define HP_GEOMETRY_H

#include "lu/log.h"

#include "glad.h"
#include "buffers.h"
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
    lumat_f4 ship_to_hex;
    lumat_f4 ship_to_hex_n;
    lumat_f4 hex_to_camera;
    lumat_f4 hex_to_camera_n;
    lumat_f4 camera_to_clip;
    lumat_f4 camera_to_clip_n;
} flight_geometry;

typedef struct geometry_buffer {
    luvec_f3 model_colour;
    float padding;
    lumat_f4 model_to_camera;
    lumat_f4 model_to_camera_n;
    lumat_f4 camera_to_clip;
    lumat_f4 camera_to_clip_n;
} geometry_buffer;

int init_geometry(lulog *log, float *variables);
int update_geometry(lulog *log, double dt, float *variables, void *data);

#endif
