
#include <geometry.h>
#include <world.h>
#include "lu/log.h"
#include "lu/status.h"

#include "glfw.h"
#include "universe.h"
#include "worlds.h"
#include "error_codes.h"

#include "programs.h"


static int init_opengl(lulog *log) {
    LU_STATUS
    GL_CHECK(glEnable(GL_CULL_FACE))
    GL_CHECK(glCullFace(GL_BACK))
    GL_CHECK(glFrontFace(GL_CW))
    GL_CHECK(glEnable(GL_DEPTH_TEST))
    GL_CHECK(glDepthMask(GL_TRUE))
    GL_CHECK(glDepthFunc(GL_LEQUAL))
    GL_CHECK(glDepthRange(0.0f, 1.0f))
    GL_CHECK(glEnable(GL_MULTISAMPLE))
    GL_CHECK(glClearColor(0, 0, 0, 1))
    LU_NO_CLEANUP
}

static int with_glfw(lulog *log) {

    LU_STATUS
    GLFWwindow *window = NULL;
    universe *universe = NULL;
    timing clock;

    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    LU_CHECK(init_opengl(log))

    LU_CHECK(mkuniverse(log, &universe, sizeof(programs)))
    LU_CHECK(build_black(log, &((programs*)universe->programs)->black))
    LU_CHECK(build_lit_per_vertex(log, &((programs*)universe->programs)->lit_per_vertex))
    LU_CHECK(build_direct_texture(log, &((programs*)universe->programs)->direct_texture))
    LU_CHECK(build_merge_frames(log, &((programs*)universe->programs)->merge_frames))
    LU_CHECK(build_blur(log, &((programs*)universe->programs)->blur))
//    LU_CHECK(build_flight_blur(log, universe->programs, window, &universe->flight))
    LU_CHECK(build_flight_direct(log, universe->programs, window, &universe->flight))

    LU_CHECK(init_timing(log, &clock));
    while (!glfwWindowShouldClose(window)) {
        double delta = update_timing(log, &clock);
        LU_CHECK(update_world(log, delta, universe->flight))
        LU_CHECK(display_world(log, universe->programs, universe->flight))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ludebug(log, "Clean exit");

LU_CLEANUP
    glfwTerminate();
    // TODO - need to free world buffers etc
    status = free_universe(&universe, status);
    LU_RETURN
}


int main(int argc, char** argv) {
    LU_STATUS
    lulog *log = NULL;
    lulog_mkstderr(&log, lulog_level_debug);
    // is this always true?  unclear to me, but we use pre-built uint32 arrays
    LU_ASSERT(sizeof(GLuint) == sizeof(uint32_t), HP_ERR, log,
            "Unexpected int size (%zu != %zu)", sizeof(GLuint), sizeof(uint32_t))
    LU_CHECK(init_glfw(log))
    LU_CHECK(with_glfw(log))
LU_CLEANUP
    if (log) status = log->free(&log, status);
    return status ? 1 : 0;
}
