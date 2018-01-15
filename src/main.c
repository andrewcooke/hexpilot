
#include <geometry.h>
#include <status.h>
#include <world.h>
#include "lu/log.h"
#include "lu/internal.h"

#include "glfw.h"
#include "universe.h"
#include "worlds.h"
#include "programs.h"


static int init_opengl(lulog *log) {
	int status = LU_OK;
	gl_try(glEnable(GL_CULL_FACE));
	gl_try(glCullFace(GL_BACK));
	gl_try(glFrontFace(GL_CW));
	gl_try(glEnable(GL_DEPTH_TEST));
	gl_try(glDepthMask(GL_TRUE));
	gl_try(glDepthFunc(GL_LEQUAL));
	gl_try(glDepthRange(0.0f, 1.0f));
	gl_try(glEnable(GL_MULTISAMPLE));
	gl_try(glClearColor(0, 0, 0, 1));
	finally:
	return status;
}

static int main_with_glfw(lulog *log) {

	int status = LU_OK;
	GLFWwindow *window = NULL;
	universe *universe = NULL;
	timing clock;

	try(create_glfw_context(log, &window));
	try(load_opengl_functions(log));
	try(init_opengl(log));

	try(universe_mk(log, &universe, sizeof(programs)));

	try(build_triangle_edges(log, &((programs*)universe->programs)->triangle_edges));
	try(build_direct_texture(log, &((programs*)universe->programs)->copy_frame));
	try(build_merge_frames(log, &((programs*)universe->programs)->merge_frames));
	try(build_blur(log, &((programs*)universe->programs)->blur));

	try(build_flight_blur(log, universe->programs, window, &universe->flight));

	try(init_timing(log, &clock));
	while (!glfwWindowShouldClose(window)) {
		double delta = update_timing(log, &clock);
		try(world_update(log, delta, universe->flight));
		try(world_display(log, universe->programs, universe->flight));
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ludebug(log, "Clean finally");

	finally:
	glfwTerminate();
	// TODO - need to free world buffers etc
	status = universe_free(&universe, status);
	return status;
}


int main(int argc, char** argv) {
	int status = LU_OK;
	lulog *log = NULL;
	try(lulog_stderr_mk(&log, lulog_level_debug));
	// is this always true?  unclear to me, but we use pre-built uint32 arrays
	assert(sizeof(GLuint) == sizeof(uint32_t), HP_ERR, log,
			"Unexpected int size (%zu != %zu)", sizeof(GLuint), sizeof(uint32_t))
	try(init_glfw(log));
	try(main_with_glfw(log));
	finally:
	if (log) status = log->free(&log, status);
	return status ? 1 : 0;
}
