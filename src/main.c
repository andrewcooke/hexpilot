
#include <GLFW/glfw3.h>

#include "error_codes.h"
#include "lu/log.h"
#include "lu/status.h"


static int display(lulog *log) {
    LU_STATUS
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f( 0.5f, -0.5f);
    glVertex2f( 0.5f,  0.5f);
    glVertex2f(-0.5f,  0.5f);
    glEnd();
    glFlush();
    LU_NO_CLEANUP
}

int main_loop(lulog *log) {

    LU_STATUS
    GLFWwindow *window = NULL;

    LU_ASSERT(window = glfwCreateWindow(320, 320, "hexpilot", NULL, NULL),
            HP_ERR_GLFW, log, "Could not create window")

    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

LU_CLEANUP
    glfwTerminate();
    LU_RETURN
}


static lulog *LOG = NULL;

void on_error(int error, const char *message) {
    luerror(LOG, "%d: %s", error, message);
}

int main(int argc, char** argv) {
    LU_STATUS
    lulog_mkstderr(&LOG, lulog_level_debug);
    glfwSetErrorCallback(on_error);
    LU_ASSERT(glfwInit(), HP_ERR_GLFW, LOG, "Could not start GLFW")
    LU_CHECK(main_loop(LOG))
LU_CLEANUP
    return status ? 1 : 0;
}
