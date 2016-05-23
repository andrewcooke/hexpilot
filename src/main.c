
#include "lu/log.h"
#include "lu/status.h"

#include "init.h"
#include "error_codes.h"


static int display(const lulog *log) {
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

static const char* vertex_shader =
        "#version 330\n"
        "layout(location = 0) in vec4 position;\n"
        "void main(){\n"
        "  gl_Position = position;\n"
        "}\n";

static const char* fragment_shader =
        "#version 330\n"
        "out vec4 outputColor;\n"
        "void main(){\n"
        "  outputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n";

static int build_program(const lulog *log) {
    LU_STATUS
    luarray_gluint *shaders = NULL;
    LU_CHECK(luarray_mkgluintn(log, &shaders, 2))
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, shaders))
    LU_CHECK(link_program(log, shaders));
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        HP_GLCHECK(glDeleteShader(shaders->i[i]))
    }
LU_CLEANUP
    status = luarray_freegluint(&shaders, status);
    LU_RETURN
}

static int with_glfw(const lulog *log) {
    LU_STATUS
    GLFWwindow *window = NULL;
    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    LU_CHECK(build_program(log))
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

static void on_error(int error, const char *message) {
    luerror(LOG, "%d: %s", error, message);
}

int main(int argc, char** argv) {
    LU_STATUS
    lulog_mkstderr(&LOG, lulog_level_debug);
    glfwSetErrorCallback(on_error);
    LU_ASSERT(glfwInit(), HP_ERR_GLFW, LOG, "Could not start GLFW")
    LU_CHECK(with_glfw(LOG))
LU_CLEANUP
    return status ? 1 : 0;
}
