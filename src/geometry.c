
#include <geometry.h>
#include <math.h>

#include "lu/internal.h"
#include "lu/vectors.h"
#include "lu/minmax.h"

#include "status_codes.h"
#include "buffers.h"
#include "keys.h"


// assumes zeroed on stack
int init_geometry(lulog *log, float *variables) {
	variables[far_z] = -100;  // scan hexagon limits?
	variables[near_z] = -1;
	variables[camera_zoom] = 1;
	variables[camera_elevation] = M_PI/4;
	variables[camera_distance] = 4;
	variables[ship_angle] = 0;
	variables[ship_z] = 1;
	return LU_OK;
}

static int calculate_physics(lulog *log, double dt, float *variables) {
	int status = LU_OK;
	variables[ship_angle] += variables[ship_rotation];
	variables[ship_x] -= variables[ship_speed] * sinf(variables[ship_angle]);
	variables[ship_y] -= variables[ship_speed] * cosf(variables[ship_angle]);
	finally:return status;
}

static int normal_transform(lulog *log, luglm *m, luglm *n) {
	int status = LU_OK;
	int i;
	luglm copy = {}, inv = {};
	luglm_copy(m, &copy);
	for (i = 0; i < 4; ++i) {
		copy[luglm_idx(i, 3)] = 0;
		copy[luglm_idx(3, i)] = 0;
	}
	copy[luglm_idx(3, 3)] = 1;
	try(lumat_inv(log, &copy, &inv));
	luglm_trans(&inv, n);
	finally:return status;
}

static int calculate_geometry(lulog *log, float *variables, flight_geometry *geometry) {

	int status = LU_OK;
	luglm ship_to_hex = {}, world_to_camera = {}, m = {};

	// the hex surface goes from -1 to 1 in z and extends outwards in x, y
	// from the origin.  that seems quite reasonable for the world, too.
	// so we use hex coords as the world space.

	luglm_rotz(-variables[ship_angle] + M_PI/2, &geometry->ship_to_hex);
	luglm_offset(-variables[ship_x], -variables[ship_y], variables[ship_z], &m);
	luglm_mult_inplace(&m, &geometry->ship_to_hex);
	try(normal_transform(log, &geometry->ship_to_hex, &geometry->ship_to_hex_n))

    		// camera space is described in "learning modern 3d graphics
    		// programming", page 60 onwards.  the projection plane is at
    		// z=-1 and includes [-1,1] in x and y.  the camera is at (0,0)
    		// looking towards -ve z.

    		// move to ship location
    		luglm_offset(variables[ship_x], variables[ship_y], 0, &geometry->hex_to_camera);
	// orient so that we are looking along the ship's velocity
	// ie rotate until y axis points along ship
	luglm rotation = {};  // save this part to use on light direction
	luglm_rotz(variables[ship_angle], &rotation);
	// tilt to the camera angle
	luglm_rotx(variables[camera_elevation] - M_PI/2, &m);
	luglm_mult_inplace(&m, &rotation);
	luglm_mult_inplace(&rotation, &geometry->hex_to_camera);
	// retreat back along camera view
	luglm_offset(0, 0, -variables[camera_distance], &m);
	luglm_mult_inplace(&m, &geometry->hex_to_camera);
	try(normal_transform(log,  &geometry->hex_to_camera,  &geometry->hex_to_camera_n))

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
	luglm_set(scale_x,       0,           0,           0,
			0, scale_y,           0,           0,
			0,       0, (n+f)/(n-f), 2*f*n/(n-f),
			0,       0,          -1,           0, &geometry->camera_to_clip);
	try(normal_transform(log,  &geometry->camera_to_clip,  &geometry->camera_to_clip_n))

    		finally:return status;
}

int update_geometry(lulog *log, double dt, float *variables, void *data) {
	int status = LU_OK;
	try(calculate_physics(log, dt, variables))
    		try(calculate_geometry(log, variables, (flight_geometry*)data))
			finally:return status;
}


