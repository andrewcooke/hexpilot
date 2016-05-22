
#include <GLFW/glfw3.h>
#include "lu/log.h"

void display() {
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
}

static lulog *LOG = NULL;

void on_error(int error, char *message) {
    luerror(LOG, "%d: %s", error, message);
}
 
int main(int argc, char** argv) {
    lulog_mkstderr(&LOG, lulog_level_debug);
    GLFWwindow *window = NULL;
    if (!glfwInit()) return 1;
    window = glfwCreateWindow(320, 320, "OpenGL Setup Test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window)) {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
