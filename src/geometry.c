
#include <geometry.h>
#include <math.h>

#include "lu/status.h"
#include "lu/vectors.h"
#include "lu/minmax.h"

#include "error_codes.h"
#include "buffers.h"
#include "keys.h"


// assumes zeroed on stack
int init_geometry(lulog *log, float *variables) {
    variables[far_z] = -1000;  // scan hexagon limits?
    variables[near_z] = -1;
    variables[camera_zoom] = 1;
    variables[camera_elevation] = M_PI/4;
    variables[camera_distance] = 4;
    variables[ship_angle] = 0;
    variables[ship_z] = 1;
    variables[light_x] = 1;
    variables[light_y] = 1;
    variables[light_z] = 1;
    return LU_OK;
}

static int calculate_physics(lulog *log, double dt, float *variables) {
    LU_STATUS
    variables[ship_angle] += variables[ship_rotation];
    variables[ship_x] -= variables[ship_speed] * sinf(variables[ship_angle]);
    variables[ship_y] -= variables[ship_speed] * cosf(variables[ship_angle]);
    LU_NO_CLEANUP
}

static int normal_transform(lulog *log, lumat_f4 *m, lumat_f4 *n) {
    LU_STATUS
    int i;
    lumat_f4 copy = {}, inv = {};
    lumat_cpyf4(m, &copy);
    for (i = 0; i < 4; ++i) {
        copy[lumat_idx4(i, 3)] = 0;
        copy[lumat_idx4(3, i)] = 0;
    }
    copy[lumat_idx4(3, 3)] = 1;
    LU_CHECK(lumat_invf4(log, &copy, &inv));
    lumat_trnf4(&inv, n);
    LU_NO_CLEANUP
}

static int calculate_geometry(lulog *log, float *variables, flight_geometry *geometry) {

    LU_STATUS
    lumat_f4 ship_to_hex = {}, world_to_camera = {}, m = {};

    // the hex surface goes from -1 to 1 in z and extends outwards in x, y
    // from the origin.  that seems quite reasonable for the world, too.
    // so we use hex coords as the world space.

//    lumat_rotf4_z(-variables[ship_angle] + M_PI/2, &geometry->ship_to_hex);
    lumat_rotf4_z(-variables[ship_angle] + 0.8*M_PI/2, &geometry->ship_to_hex);
    lumat_offf4_3(-variables[ship_x], -variables[ship_y], variables[ship_z], &m);
    lumat_mulf4_in(&m, &geometry->ship_to_hex);
    LU_CHECK(normal_transform(log, &geometry->ship_to_hex, &geometry->ship_to_hex_n))

    // camera space is described in "learning modern 3d graphics
    // programming", page 60 onwards.  the projection plane is at
    // z=-1 and includes [-1,1] in x and y.  the camera is at (0,0)
    // looking towards -ve z.

    // move to ship location
    lumat_offf4_3(variables[ship_x], variables[ship_y], 0, &geometry->hex_to_camera);
    // orient so that we are looking along the ship's velocity
    // ie rotate until y axis points along ship
    lumat_f4 rotation = {};  // save this part to use on light direction
    lumat_rotf4_z(variables[ship_angle], &rotation);
    // tilt to the camera angle
    lumat_rotf4_x(variables[camera_elevation] - M_PI/2, &m);
    lumat_mulf4_in(&m, &rotation);
    lumat_mulf4_in(&rotation, &geometry->hex_to_camera);
    // retreat back along camera view
    lumat_offf4_3(0, 0, -variables[camera_distance], &m);
    lumat_mulf4_in(&m, &geometry->hex_to_camera);
    LU_CHECK(normal_transform(log,  &geometry->hex_to_camera,  &geometry->hex_to_camera_n))

    // transform light direction to camera space
    luvec_f4 model_light_pos =
        {variables[light_x], variables[light_y], variables[light_z], 0};
    luvec_mulf4(&rotation, &model_light_pos, &geometry->camera_light_pos);
    luvec_nrmf4_3in(&geometry->camera_light_pos);

    // from page 66 of LM3DGP, but with the signs of near_z and far_z
    // changed (for some reason the author decided those should be
    // positive values; here they are simply coords and so -ve).
    float scale_x = 1, scale_y = 1;
    if (variables[buffer_x] < variables[buffer_y]) {
        scale_x = variables[buffer_y] / variables[buffer_x];
    } else {
        scale_y = variables[buffer_x] / variables[buffer_y];
    }
    scale_x *= variables[camera_zoom]; scale_y *= variables[camera_zoom];
    float f = -variables[far_z], n = -variables[near_z];
    lumat_setf4(scale_x,       0,           0,           0,
                      0, scale_y,           0,           0,
                      0,       0, (n+f)/(n-f), 2*f*n/(n-f),
                      0,       0,          -1,           0, &geometry->camera_to_clip);
    LU_CHECK(normal_transform(log,  &geometry->camera_to_clip,  &geometry->camera_to_clip_n))

    LU_NO_CLEANUP
}

int update_geometry(lulog *log, double dt, float *variables, void *data) {
    LU_STATUS
    LU_CHECK(calculate_physics(log, dt, variables))
    LU_CHECK(calculate_geometry(log, variables, (flight_geometry*)data))
    LU_NO_CLEANUP
}


