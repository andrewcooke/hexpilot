
#include "lu/log.h"
#include "lu/tiles.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "models.h"
#include "universe.h"
#include "worlds.h"
#include "shaders.h"
#include "buffers.h"
#include "tiles.h"
#include "error_codes.h"
#include "geometry.h"

#include "flight_simple.h"


static int display(lulog *log, world *world) {
    LU_STATUS
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClearDepth(1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    for (size_t i = 0; i < world->models->mem.used; ++i) {
    	GL_CHECK(glUseProgram(world->models->m[i]->program))
        LU_CHECK(world->models->m[i]->send(log, world->models->m[i], world))
        LU_CHECK(world->models->m[i]->draw(log, world->models->m[i]))
    }
LU_CLEANUP
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}


static int build_flat(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "lit_model.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

static int init_opengl(lulog *log) {
    LU_STATUS
    GL_CHECK(glEnable(GL_CULL_FACE))
    GL_CHECK(glCullFace(GL_BACK))
    GL_CHECK(glFrontFace(GL_CW))
    GL_CHECK(glEnable(GL_DEPTH_TEST))
    GL_CHECK(glDepthMask(GL_TRUE))
    GL_CHECK(glDepthFunc(GL_LEQUAL))
    GL_CHECK(glDepthRange(0.0f, 1.0f))
    GL_CHECK(glEnable(GL_MULTISAMPLE))  // see also GLFW_SAMPLES in glfw.c
//    GL_CHECK(glEnable(GL_DEPTH_CLAMP))
    LU_NO_CLEANUP
}

static int with_glfw(lulog *log) {

    LU_STATUS
    GLFWwindow *window = NULL;
    universe *universe = NULL;

    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    LU_CHECK(init_opengl(log))

    LU_CHECK(mkuniverse(log, &universe))
	LU_CHECK(build_flat(log, &universe->programs.flat))
	LU_CHECK(build_flight_simple(log, universe->programs.flat, window, &universe->flight))

    double tik[2] = {glfwGetTime(), 0};
    double fpszero = glfwGetTime(); int fcount = 0;
    while (!glfwWindowShouldClose(window)) {
        tik[1] = glfwGetTime();
        if (tik[1] > fpszero+1) {
//            ludebug(log, "%0.1f fps", fcount / (tik[1] - fpszero));
            fpszero = tik[1]; fcount = 0;
        }
        LU_CHECK(respond_to_user(log, tik[1] - tik[0], universe->flight))
        LU_CHECK(update_geometry(log, tik[1] - tik[0], universe->flight->variables, universe->flight->geometry))
        LU_CHECK(display(log, universe->flight))
        glfwSwapBuffers(window);
        glfwPollEvents();
        tik[0] = tik[1]; fcount++;
    }
    ludebug(log, "Clean exit");
LU_CLEANUP
    glfwTerminate();
    status = free_universe(&universe, status);
    LU_RETURN
}


int main(int argc, char** argv) {
    LU_STATUS
    lulog *log = NULL;
    lulog_mkstderr(&log, lulog_level_debug);
    LU_ASSERT(sizeof(GLuint) == sizeof(unsigned int), HP_ERR, log,
            "Unexpected int size (%zu != %zu)", sizeof(GLuint), sizeof(unsigned int))
    LU_CHECK(init_glfw(log))
    LU_CHECK(with_glfw(log))
LU_CLEANUP
    if (log) status = log->free(&log, status);
    return status ? 1 : 0;
}
