
#include "lu/log.h"
#include "lu/status.h"

#include "glfw.h"
#include "universe.h"
#include "worlds.h"
#include "error_codes.h"

#include "programs.h"
#include "flight_geometry.h"
#include "flight_simple.h"


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
        double delta = tik[1] - tik[0];
        LU_CHECK(universe->flight->respond(log, delta, universe->flight->action, universe->flight->variables))
        LU_CHECK(universe->flight->update(log, tik[1] - tik[0], universe->flight->variables, universe->flight->data))
        LU_CHECK(display_world(log, universe->flight))
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
    // is this always true?  unclear to me, but we use pre-built uint32 arrays
    LU_ASSERT(sizeof(GLuint) == sizeof(uint32_t), HP_ERR, log,
            "Unexpected int size (%zu != %zu)", sizeof(GLuint), sizeof(uint32_t))
    LU_CHECK(init_glfw(log))
    LU_CHECK(with_glfw(log))
LU_CLEANUP
    if (log) status = log->free(&log, status);
    return status ? 1 : 0;
}
