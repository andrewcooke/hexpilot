
#include <glfw.h>
#include "lu/log.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "shaders.h"
#include "buffers.h"
#include "error_codes.h"


static int display(const lulog *log, GLuint program, luarray_uint *buffers) {
    LU_STATUS
    HP_GLCHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    HP_GLCHECK(glClear(GL_COLOR_BUFFER_BIT))
    HP_GLCHECK(glUseProgram(program))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, buffers->i[0]))
    HP_GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers->i[1]))
    HP_GLCHECK(glEnableVertexAttribArray(0))
    HP_GLCHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0))
    HP_GLCHECK(glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)0))
    HP_GLCHECK(glDisableVertexAttribArray(0))
    HP_GLCHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0))
    HP_GLCHECK(glBindBuffer(GL_ARRAY_BUFFER, 0))
    HP_GLCHECK(glUseProgram(0))
    LU_NO_CLEANUP
}

static const float vertices[] = {
        0.75f, 0.75f, 0.0f, 1.0f,
        0.75f, -0.75f, 0.0f, 1.0f,
        -0.75f, -0.75f, 0.0f, 1.0f,
        -0.75f, 0.75f, 0.0f, 1.0f,
};

static const unsigned int indices[] = {
        0, 1, 2,
};

static int build_buffers(const lulog *log, luarray_uint *buffers) {
    LU_STATUS
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, vertices, sizeof(vertices), buffers))
    LU_CHECK(load_buffer(log, GL_ELEMENT_ARRAY_BUFFER, indices, sizeof(indices), buffers))
    // create and select this, since only one is needed
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
    luarray_uint *shaders = NULL;
    LU_CHECK(luarray_mkuintn(log, &shaders, 2))
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, shaders))
    LU_CHECK(link_program(log, shaders, program));
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        HP_GLCHECK(glDeleteShader(shaders->i[i]))
    }
LU_CLEANUP
    status = luarray_freeuint(&shaders, status);
    LU_RETURN
}

static int with_glfw(const lulog *log) {
    LU_STATUS
    GLFWwindow *window = NULL;
    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    GLuint program;
    LU_CHECK(build_program(log, &program))
    luarray_uint *buffers = NULL;
    LU_CHECK(luarray_mkuintn(log, &buffers, 1));
    LU_CHECK(build_buffers(log, buffers))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log, program, buffers))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ludebug(log, "Clean exit");
LU_CLEANUP
    glfwTerminate();
    status = luarray_freeuint(&buffers, status);
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
    LU_ASSERT(sizeof(GLuint) == sizeof(unsigned int), HP_ERR, LOG,
            "Unexpected int size (%zu != %zu)", sizeof(GLuint), sizeof(unsigned int))
    LU_ASSERT(glfwInit(), HP_ERR_GLFW, LOG, "Could not start GLFW")
    LU_CHECK(with_glfw(LOG))
LU_CLEANUP
    return status ? 1 : 0;
}
