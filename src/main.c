
#include <glfw.h>
#include "lu/log.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "shaders.h"
#include "buffers.h"
#include "tiles.h"
#include "error_codes.h"


static int display(lulog *log, GLuint program, luarray_buffer *buffers, luarray_uint *offsets) {
    LU_STATUS
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
    GL_CHECK(glUseProgram(program))
    LU_CHECK(bind_buffers(log, buffers))
    GL_CHECK(glEnableVertexAttribArray(0))
    GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0))
    for (size_t i = 0; i < offsets->mem.used-1; ++i) {
        GL_CHECK(glDrawElements(GL_TRIANGLE_STRIP, offsets->i[i+1] - offsets->i[i],
                GL_UNSIGNED_INT, (void*)(offsets->i[i] * buffers->b[1].chunk)))
    }
    GL_CHECK(glDisableVertexAttribArray(0))
    LU_CHECK(unbind_buffers(log, buffers))
    GL_CHECK(glUseProgram(0))
    LU_NO_CLEANUP
}

static int build_buffers(lulog *log, luarray_buffer **buffers, luarray_uint **offsets) {
    LU_STATUS
    luarray_fxyzw *vertices = NULL;
    luarray_uint *indices = NULL;
    LU_CHECK(hexagon(log, 0, 3, 3, 0.1, 1.0, &vertices, &indices, offsets))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->fxyzw, vertices->mem.used, sizeof(*vertices->fxyzw), buffers));
    LU_CHECK(luarray_dumpfxyzw(log, vertices, "vertices", 2))
    LU_CHECK(load_buffer(log, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
            indices->i, indices->mem.used, sizeof(*indices->i), buffers))
    LU_CHECK(luarray_dumpuint(log, indices, "indices", 2))
    // create and select this, since only one is needed
    GLuint vao;
    GL_CHECK(glGenVertexArrays(1, &vao))
    GL_CHECK(glBindVertexArray(vao))
LU_CLEANUP
    status = luarray_freefxyzw(&vertices, status);
    status = luarray_freeuint(&indices, status);
    LU_RETURN
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
        "  outputColor = vec4(0.5f, 0.0f, 0.0f, 1.0f);\n"
        "}\n";

static int build_program(lulog *log, GLuint *program) {
    LU_STATUS
    luarray_uint *shaders = NULL;
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, &shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

static int with_glfw(lulog *log) {
    LU_STATUS
    GLFWwindow *window = NULL;
    luarray_buffer *buffers = NULL;
    luarray_uint *offsets = NULL;
    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    GLuint program;
    LU_CHECK(build_program(log, &program))
    LU_CHECK(build_buffers(log, &buffers, &offsets))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log, program, buffers, offsets))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ludebug(log, "Clean exit");
LU_CLEANUP
    glfwTerminate();
    status = luarray_freebuffer(&buffers, status);
    status = luarray_freeuint(&offsets, status);
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
