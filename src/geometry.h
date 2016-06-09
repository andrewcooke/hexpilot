
#ifndef HP_GEOMETRY_H
#define HP_GEOMETRY_H

#include "lu/log.h"

#include "glad.h"
#include "buffers.h"


typedef enum variable_index {
    ship_speed,
    ship_rotation,
    ship_x,
    ship_y,
    ship_angle,
    camera_elevation,
    camera_distance,
    camera_zoom,
    buffer_x,
    buffer_y,
    near_z,
    far_z,
    light_x,
    light_y,
    light_z,
    n_variables
} variable_index;

typedef struct geometry_data {
    luvec_f4 light_camera;
    lumat_f4 model_camera;
    lumat_f4 model_camera_n;
    lumat_f4 camera_clip;
} geometry_data;

int init_geometry(lulog *log, float *variables);
int update_geometry(lulog *log, double dt, GLuint program, float *variables, buffer *buffer);

#endif
