
#ifndef HP_GEOMETRY_H
#define HP_GEOMETRY_H

#include "lu/log.h"

#include "glad.h"


typedef enum variable_index {
    ship_sx,
    ship_sy,
    ship_vx,
    ship_vy,
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

int init_geometry(lulog *log, float *variables);
int update_geometry(lulog *log, GLuint program, float *variables);

#endif
