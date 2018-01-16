
#include <status.h>
#include "lu/internal.h"

#include "glfw.h"
#include "vertices.h"
#include "tiles.h"
#include "shaders.h"
#include "world.h"
#include "geometry.h"
#include "programs.h"


static int init_keys(lulog *log, user_action *action) {
	int status = LU_OK;
	keys k = {};
	try(set_keys(log, &k, "+/-", 61, 1, 45, 0,
			15, 10, 0,
			0.1, 10, camera_zoom));
	try(luary_control_push(log, action->controls, &k, 1));
	try(set_keys(log, &k, "left/right", 262, 0, 263, 0,
			0.3, 5, 5,
			-0.5, 0.5, ship_rotation));
	try(luary_control_push(log, action->controls, &k, 0));
	try(set_keys(log, &k, "up/down",
			265, 0, 264, 0,
			2, 10, 0,
			0, 4, ship_speed));
	try(luary_control_push(log, action->controls, &k, 0));
	finally:
	return status;
}

static int respond_to_user(lulog *log, double dt, user_action *action, float *variables) {
	int status = LU_OK, width, height;
	try(controls_update(log, dt, action->controls, variables));
	glfwGetFramebufferSize(action->window, &width, &height);
	gl_try(glViewport(0, 0, width, height));
	variables[buffer_x] = width; variables[buffer_y] = height;
	finally:
	return status;
}


static luglc hex_colour = {0,0.5,0.2};

static int send_hex_data(lulog *log, model *model, world *world) {
	int status = LU_OK;
	// this builds the whole geometry buffer so must be done before ship
	gl_try(glBindBuffer(GL_UNIFORM_BUFFER, world->geometry_buffer->name));
	geometry_buffer buffer = {};
	luglc_copy(&hex_colour, &buffer.model_colour);
	flight_data *data = (flight_data*) world->data;
	luglm_copy(&data->geometry.hex_to_camera, &buffer.model_to_camera);
	luglm_copy(&data->geometry.hex_to_camera_n, &buffer.model_to_camera_n);
	luglm_copy(&data->geometry.camera_to_clip, &buffer.camera_to_clip);
	luglm_copy(&data->geometry.camera_to_clip_n, &buffer.camera_to_clip_n);
	buffer.line_width = 0.002;
	gl_try(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(geometry_buffer), &buffer));
	finally:
	GL_CLEAN(glBindBuffer(GL_UNIFORM_BUFFER, 0))
	return status;
}

static luglc ship_cyan = {0,1,1};

static int build_hexagon(lulog *log, programs *programs, world *world) {
	int status = LU_OK;
	model *model = NULL;
	luary_vnorm *vertices = NULL;
	try(model_mk(log, &model, &send_hex_data, &draw_triangle_edges));
	try(hexagon_vnormal_strips(log, 0, 5, 10, 0.4, 1, &vertices, &model->offsets, &model->counts));
	try(buffer_mk(log, &model->vertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
			vertices->vn, vertices->mem.used * sizeof(*vertices->vn)));
	try(interleaved_vnorm_vao(log, model->vertices, &model->vao));
	try(model_push(log, world, model));
	finally:
	status = luary_vnorm_free(&vertices, status);
	return status;
}

static int send_ship_data(lulog *log, model *model, world *world) {
	int status = LU_OK;
	// this patches ship-specific changes into existing geometry buffer
	gl_try(glBindBuffer(GL_UNIFORM_BUFFER, world->geometry_buffer->name))
	gl_try(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ship_cyan), &ship_cyan))
	luglm ship_to_camera = {};
	flight_data *data = (flight_data*) world->data;
	luglm_mult(&data->geometry.hex_to_camera, &data->geometry.ship_to_hex, &ship_to_camera);
	gl_try(glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(ship_to_camera), &ship_to_camera))
	luglm ship_to_camera_n = {};
	luglm_mult(&data->geometry.hex_to_camera_n, &data->geometry.ship_to_hex_n, &ship_to_camera_n);
	gl_try(glBufferSubData(GL_UNIFORM_BUFFER, 80, sizeof(ship_to_camera_n), &ship_to_camera_n))
	finally:
	GL_CLEAN(glBindBuffer(GL_UNIFORM_BUFFER, 0))
	return status;
}

// TODO - remove ugly dangerous explicit offsets
static int build_ship(lulog *log, programs *programs, world *world) {
	int status = LU_OK;
	model *model = NULL;
	luary_vnorm *vertices = NULL;
	try(model_mk(log, &model, &send_ship_data, &draw_triangle_edges));
	try(ship_vnormal_strips(log, 0.03, &vertices, &model->offsets, &model->counts));
	try(buffer_mk(log, &model->vertices, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
			vertices->vn, vertices->mem.used * sizeof(*vertices->vn)));
	try(interleaved_vnorm_vao(log, model->vertices, &model->vao));
	try(model_push(log, world, model));
	finally:
	status = luary_vnorm_free(&vertices, status);
	return status;
}

static int build_geometry(lulog *log, programs *programs, world *world) {
	int status = LU_OK;
	try(buffer_mk(log, &world->geometry_buffer, GL_UNIFORM_BUFFER, GL_STREAM_DRAW,
			NULL, sizeof(geometry_buffer)));
	// http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
	gl_try(GLuint index = glGetUniformBlockIndex(programs->triangle_edges, "geometry"))
	gl_try(glUniformBlockBinding(programs->triangle_edges, index, 1))
	gl_try(glBindBufferBase(GL_UNIFORM_BUFFER, 1, world->geometry_buffer->name))
	finally:
	return status;
}

/**
 * copy a frame to the display buffer.
 */
static int build_render(lulog *log, programs *programs, flight_data *data) {
	int status = LU_OK;
	float quad[] = {-1,-1, -1,1, 1,-1, 1,1};
	try(buffer_mk(log, &data->quad_buffer, GL_ARRAY_BUFFER, GL_STATIC_DRAW, quad, sizeof(quad)));
	gl_try(glGenVertexArrays(1, &data->quad_vao));
	gl_try(glBindVertexArray(data->quad_vao));
	try(buffer_bind(log, data->quad_buffer));
	gl_try(glEnableVertexAttribArray(0));
	gl_try(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
	finally:
	status = lu_both(status, buffer_unbind(log, data->quad_buffer));
	return status;
}

static int before_display_blur(lulog *log, void *programs, world *world) {
	int status = LU_OK;
	flight_data *data = (flight_data*)world->data;

	try(rescale_frame(log, world->action->window, &data->single));
	try(rescale_frame(log, world->action->window, &data->tmp1));
	try(rescale_frame(log, world->action->window, &data->tmp2));
	try(rescale_frame(log, world->action->window, &data->multiple));

	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, data->single.render));
	gl_try(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	finally:
	return status;
}

/**
 * assuming that the current display has been rendered into buffer
 * single.render, combine that with the previous blurred display,
 * and move the result to output.  then blur further and save for
 * next iteration.  requires tmp1 and tmp2 for workspace.
 */
static int after_display_blur(lulog *log, void *v, world *world) {

	int status = LU_OK;
	flight_data *data = (flight_data*)world->data;
	programs *p = (programs*)v;

	gl_try(glDisable(GL_DEPTH_TEST))

	// blit from single into tmp1 to resolve anti-aliasing
	gl_try(glBindFramebuffer(GL_READ_FRAMEBUFFER, data->single.render));
	gl_try(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data->tmp1.render));
	gl_try(glBlitFramebuffer(0, 0, data->single.width, data->single.height,
			0, 0, data->single.width, data->single.height, GL_COLOR_BUFFER_BIT, GL_NEAREST));

	// add multiple and tmp1 into tmp2
	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, data->tmp2.render));
	gl_try(glClear(GL_COLOR_BUFFER_BIT));  // TODO - needed?
	gl_try(glUseProgram(p->merge_frames.name));
	try(use_uniform_texture(log, p->merge_frames.frame1, data->tmp1.texture));
	try(use_uniform_texture(log, p->merge_frames.frame2, data->multiple.texture));
	gl_try(glBindVertexArray(data->quad_vao));
	gl_try(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	// copy tmp2 into output
	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	gl_try(glClear(GL_COLOR_BUFFER_BIT));  // TODO - needed?
	gl_try(glUseProgram(p->copy_frame.name));
	try(use_uniform_texture(log, p->copy_frame.frame, data->tmp2.texture));
	gl_try(glBindVertexArray(data->quad_vao));
	gl_try(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	for (int loop = 0; loop < 4; ++loop) {

		// blur tmp2 horizontally into tmp1
		gl_try(glBindFramebuffer(GL_FRAMEBUFFER, data->tmp1.render));
		gl_try(glClear(GL_COLOR_BUFFER_BIT));  // TODO - needed?
		gl_try(glUseProgram(p->blur.name));
		try(use_uniform_texture(log, p->blur.frame, data->tmp2.texture));
		gl_try(glUniform1i(p->blur.horizontal, 1));
		gl_try(glBindVertexArray(data->quad_vao));
		gl_try(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		// blur tmp1 vertically into tmp2
		gl_try(glBindFramebuffer(GL_FRAMEBUFFER, data->tmp2.render));
		gl_try(glClear(GL_COLOR_BUFFER_BIT));  // TODO - needed?
		gl_try(glUseProgram(p->blur.name));
		try(use_uniform_texture(log, p->blur.frame, data->tmp1.texture));
		gl_try(glUniform1i(p->blur.horizontal, 0));
		gl_try(glBindVertexArray(data->quad_vao));
		gl_try(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	}

	// copy tmp2 into multiple to save
	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, data->multiple.render));
	gl_try(glClear(GL_COLOR_BUFFER_BIT));  // TODO - needed?
	gl_try(glUseProgram(p->copy_frame.name));
	try(use_uniform_texture(log, p->copy_frame.frame, data->tmp2.texture));
	gl_try(glBindVertexArray(data->quad_vao));
	gl_try(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

	finally:
	gl_try(glBindVertexArray(0)); // TODO try after finally!!!!!!!!!!!!!!!!!!!!
	gl_try(glBindTexture(GL_TEXTURE_2D, 0));
	gl_try(glEnable(GL_DEPTH_TEST));
	GL_CLEAN(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	return status;
}

int build_flight_blur(lulog *log, void *v, GLFWwindow *window, world **world) {
	int status = LU_OK;
	programs *p = (programs*) v;
	try(world_mk(log, world, n_variables, sizeof(flight_data), window,
			&respond_to_user, &update_geometry, &before_display_blur, &after_display_blur));
	flight_data *data = (flight_data*)(*world)->data;
	try(init_frame(log, window, &data->single, 1, 1));
	try(init_frame(log, window, &data->multiple, 0, 0));
	try(init_frame(log, window, &data->tmp1, 0, 0));
	try(init_frame(log, window, &data->tmp2, 0, 0));
	try(init_keys(log, (*world)->action));
	try(init_geometry(log, (*world)->variables));
	try(build_render(log, p, data));
	try(build_geometry(log, p, *world));
	try(build_hexagon(log, p, *world));
	try(build_ship(log, p, *world));
	try(set_window_callbacks(log, window, (*world)->action));
	finally:
	return status;
}

