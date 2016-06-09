
#include <math.h>

#include "lu/status.h"
#include "lu/vectors.h"
#include "lu/minmax.h"

#include "error_codes.h"
#include "geometry.h"
#include "buffers.h"


// assumes zeroed on stack
int init_geometry(lulog *log, float *variables) {
    variables[far_z] = -1000;  // scan hexagon limits?
    variables[near_z] = -1;
    variables[camera_zoom] = 1;
    variables[camera_elevation] = M_PI/4;
    variables[camera_distance] = 4;
    variables[ship_vx] = 0;
    variables[ship_vy] = 1;
    variables[light_x] = 1;
    variables[light_y] = 1;
    variables[light_z] = 1;
    return LU_OK;
}

int calculate_geometry(lulog *log, float *variables, geometry_data *data) {

    LU_STATUS
    lumat_f4 model_world = {}, world_camera = {}, m = {};

    // the hex surface goes from -1 to 1 in z and extends outwards in x, y
    // from the origin.  that seems quite reasonable for the world, too.
    lumat_idnf4(&model_world);

    // camera space is described in "learning modern 3d graphics
    // programming", page 60 onwards.  the projection plane is at
    // z=-1 and includes [-1,1] in x and y.  the camera is at (0,0)
    // looking towards -ve z.
    lumat_idnf4(&world_camera);
    float dz = variables[camera_distance] * sinf(variables[camera_elevation]);
    lumat_offf4_3(0, 0, -dz, &m);
    lumat_mulf4_in(&m, &world_camera);
    // orient so that we are looking along the ship's velocity
    // ie rotate until y axis points along ship
    lumat_rotf4_z(atan2(variables[ship_vy], variables[ship_vx]), &m);
    lumat_mulf4_in(&m, &world_camera);
    // move left/right etc relative to ship orientation
    lumat_offf4_3(variables[ship_sx], variables[ship_sy], 0, &m);
    lumat_mulf4_in(&m, &world_camera);
    // tilt to the camera angle
    lumat_rotf4_x(variables[camera_elevation] - M_PI/2, &m);
    lumat_mulf4_in(&m, &world_camera);

    // combine model_world and world_camera
    lumat_mulf4(&world_camera, &model_world, &data->model_camera);

    // the equivalent for normals
    LU_CHECK(lumat_invf4(log, &data->model_camera, &m))
    lumat_trnf4(&m, &data->model_camera_n);

    // transform light direction to camera space
    luvec_f4 light_model =
        {variables[light_x], variables[light_y], variables[light_z], 0};
    luvec_mulf4(&data->model_camera, &light_model, &data->light_camera);

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
                      0,       0,          -1,           0, &data->camera_clip);
    LU_NO_CLEANUP
}

int send_geometry(lulog *log, GLuint program, geometry_data *data, buffer *buffer) {
    LU_STATUS
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, buffer->name))
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(*data), data))
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_NO_CLEANUP
}

int update_geometry(lulog *log, GLuint program, float *variables, buffer *buffer) {
    LU_STATUS
    geometry_data data = {};
    LU_CHECK(calculate_geometry(log, variables, &data))
    LU_CHECK(send_geometry(log, program, &data, buffer))
    LU_NO_CLEANUP
}


