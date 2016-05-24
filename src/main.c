
#include "lu/log.h"
#include "lu/status.h"

#include "init.h"
#include "error_codes.h"


static int display(const lulog *log, GLuint program, luarray_gluint *buffers) {
    LU_STATUS
    HP_GLCHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    HP_GLCHECK(glClear(GL_COLOR_BUFFER_BIT))
    HP_GLCHECK(glUseProgram(program))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers->i[0]))
    HP_GLCHECK(glEnableVertexAttribArray(0))
    HP_GLCHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0))
    HP_GLCHECK(glDrawArrays(GL_TRIANGLES, 0, 3))
    HP_GLCHECK(glDisableVertexAttribArray(0))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
    HP_GLCHECK(glUseProgram(0))
    LU_NO_CLEANUP
}

static const float vertices[] = {
        0.75f, 0.75f, 0.0f, 1.0f,
        0.75f, -0.75f, 0.0f, 1.0f,
        -0.75f, -0.75f, 0.0f, 1.0f,
};

static int load_data(const lulog *log, const void *data, size_t size, luarray_gluint *buffers) {
    LU_STATUS
    GLuint buffer;
    HP_GLCHECK(glGenBuffers(1, &buffer))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, buffer))
    HP_GLCHECK(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
    LU_CHECK(luarray_pushgluint(log, buffers, buffer))
    luinfo(log, "Loaded %zu bytes to buffer %d", size, buffers->mem.used);
    LU_NO_CLEANUP
}

static int build_data(const lulog *log, luarray_gluint *buffers) {
    LU_STATUS
    LU_CHECK(load_data(log, vertices, sizeof(vertices), buffers))
    // create and select this, since only one is neede
    GLuint vao;
    HP_GLCHECK(glGenVertexArrays(1, &vao))
    HP_GLCHECK(glBindVertexArray(vao))
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

static int build_program(const lulog *log, GLuint *program) {
    LU_STATUS
    luarray_gluint *shaders = NULL;
    LU_CHECK(luarray_mkgluintn(log, &shaders, 2))
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, shaders))
    LU_CHECK(link_program(log, shaders, program));
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
    GLuint program;
    LU_CHECK(build_program(log, &program))
    luarray_gluint *buffers = NULL;
    LU_CHECK(luarray_mkgluintn(log, &buffers, 1));
    LU_CHECK(build_data(log, buffers))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log, program, buffers))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
LU_CLEANUP
    glfwTerminate();
    status = luarray_freegluint(&buffers, status);
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
