
#include <math.h>

#include "lu/status.h"
#include "lu/vectors.h"
#include "lu/minmax.h"

#include "error_codes.h"
#include "buffers.h"
#include "keys.h"
#include "geometry.h"


int init_keys(lulog *log, user_action *action) {
    LU_STATUS
    keys k = {};
    LU_CHECK(set_keys(log, &k, "+/-", 61, 1, 45, 0,
            15, 10, 0,
            0.1, 10, camera_zoom))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 1))
    LU_CHECK(set_keys(log, &k, "left/right", 262, 0, 263, 0,
            0.3, 5, 5,
            -0.5, 0.5, ship_rotation))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
    LU_CHECK(set_keys(log, &k, "up/down",
            265, 0, 264, 0,
            2, 10, 0,
            0, 4, ship_speed))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
LU_CLEANUP
    free(k.name);
    LU_RETURN
}

// assumes zeroed on stack
int init_geometry(lulog *log, float *variables) {
    variables[far_z] = -1000;  // scan hexagon limits?
    variables[near_z] = -1;
    variables[camera_zoom] = 1;
    variables[camera_elevation] = M_PI/4;
    variables[camera_distance] = 4;
    variables[ship_angle] = 0;
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
    lumat_f4 x = {};
    LU_CHECK(lumat_invf4(log, m, &x));
    lumat_trnf4(&x, n);
    LU_NO_CLEANUP
}

static int calculate_geometry(lulog *log, float *variables, geometry *geometry) {

    LU_STATUS
    lumat_f4 ship_to_hex = {}, world_to_camera = {}, m = {};

    // the hex surface goes from -1 to 1 in z and extends outwards in x, y
    // from the origin.  that seems quite reasonable for the world, too.
    // so we use hex coords as the world space.

    lumat_offf4_3(-variables[ship_x], -variables[ship_y], -variables[ship_z], &geometry->ship_to_hex);
    LU_CHECK(normal_transform(log, &geometry->ship_to_hex, &geometry->ship_to_hex_n))

    // camera space is described in "learning modern 3d graphics
    // programming", page 60 onwards.  the projection plane is at
    // z=-1 and includes [-1,1] in x and y.  the camera is at (0,0)
    // looking towards -ve z.
    lumat_idnf4(&geometry->hex_to_camera);
    // move to ship location
    lumat_offf4_3(variables[ship_x], variables[ship_y], 0, &m);
    lumat_mulf4_in(&m, &geometry->hex_to_camera);
    // orient so that we are looking along the ship's velocity
    // ie rotate until y axis points along ship
    lumat_rotf4_z(variables[ship_angle], &m);
    lumat_mulf4_in(&m, &geometry->hex_to_camera);
    // tilt to the camera angle
    lumat_rotf4_x(variables[camera_elevation] - M_PI/2, &m);
    lumat_mulf4_in(&m, &geometry->hex_to_camera);
    // retreat back along camera view
    lumat_offf4_3(0, 0, -variables[camera_distance], &m);
    lumat_mulf4_in(&m, &geometry->hex_to_camera);
    LU_CHECK(normal_transform(log,  &geometry->hex_to_camera,  &geometry->hex_to_camera_n))

    // transform light direction to camera space
    luvec_f4 light_model =
        {variables[light_x], variables[light_y], variables[light_z], 0};
    luvec_mulf4(&geometry->hex_to_camera, &light_model, &geometry->camera_light_pos);

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
    LU_NO_CLEANUP
}

int update_geometry(lulog *log, double dt, float *variables, geometry *geometry) {
    LU_STATUS
    LU_CHECK(calculate_physics(log, dt, variables))
    LU_CHECK(calculate_geometry(log, variables, geometry))
    LU_NO_CLEANUP
}


