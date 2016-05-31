
#include <glfw.h>

#include "lu/log.h"
#include "lu/tiles.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "shaders.h"
#include "buffers.h"
#include "tiles.h"
#include "error_codes.h"


static int display(lulog *log, GLuint vao, luarray_int32 *offsets, luarray_uint32 *counts) {
    LU_STATUS
//    int32_t o[] = {0};
//    uint32_t c[] = {9};
    GL_CHECK(glBindVertexArray(vao))
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, offsets->i, counts->i, counts->mem.used));
//    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, o, c, 1));
LU_CLEANUP
    GL_CHECK(glBindVertexArray(0))
    LU_RETURN
}

static int build_buffers(lulog *log, luarray_buffer **buffers,
		luarray_int32 **offsets, luarray_uint32 **counts) {
    LU_STATUS
    luarray_vnorm *vertices = NULL;
    luarray_uint32 *indices = NULL;
    LU_CHECK(hexagon_vnormal_strips(log, 0, 3, 1, 0.3, 1.0, &vertices, offsets, counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->v, vertices->mem.used, sizeof(*vertices->v), buffers));
    LU_CHECK(luarray_dumpvnorm(log, vertices, "vertices", 2))
    LU_CHECK(luarray_dumpint32(log, *offsets, "offsets", 2))
    LU_CHECK(luarray_dumpuint32(log, *counts, "counts", 2))
LU_CLEANUP
    status = luarray_freevnorm(&vertices, status);
    status = luarray_freeuint32(&indices, status);
    LU_RETURN
}

static const char* vertex_shader =
        "#version 330\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec4 normal;\n"
        "flat out vec4 interpColour;\n"
        "void main(){\n"
        "  float brightness = dot(normal, vec4(1.0f, 1.0f, 1.0f, 1.0f));\n"
        "  brightness = clamp(brightness, 0.1, 0.9);\n"
        "  interpColour = brightness * vec4(1.0f, 0.0f, 0.0f, 1.0f);\n"
        "  gl_Position = position;\n"
        "}\n";

static const char* fragment_shader =
        "#version 330\n"
        "flat in vec4 interpColour;\n"
        "out vec4 outputColor;\n"
        "void main(){\n"
        "  outputColor = interpColour;\n"
        "}\n";

static int build_program(lulog *log, GLuint *program) {
    LU_STATUS
    luarray_uint32 *shaders = NULL;
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, &shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

static int build_vao(lulog *log, GLuint program, luarray_buffer *buffers,
        GLuint *vao) {
    LU_STATUS
    GL_CHECK(glGenVertexArrays(1, vao))
    GL_CHECK(glBindVertexArray(*vao))
    GL_CHECK(glUseProgram(program))
    LU_CHECK(bind_buffers(log, buffers))
    GL_CHECK(glEnableVertexAttribArray(0))
    GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, 0))
    GL_CHECK(glEnableVertexAttribArray(1))
    GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (void*)16))
LU_CLEANUP
    GL_CHECK(glBindVertexArray(0))
    LU_CHECK(unbind_buffers(log, buffers))
    LU_RETURN
}

static int with_glfw(lulog *log) {
    LU_STATUS
    GLFWwindow *window = NULL;
    luarray_buffer *buffers = NULL;
    luarray_uint32 *counts = NULL;
    luarray_int32 *offsets = NULL;
    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    GLuint program, vao;
    LU_CHECK(build_program(log, &program))
    LU_CHECK(build_buffers(log, &buffers, &offsets, &counts))
    LU_CHECK(build_vao(log, program, buffers, &vao))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log, vao, offsets, counts))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ludebug(log, "Clean exit");
LU_CLEANUP
    glfwTerminate();
    status = luarray_freebuffer(&buffers, status);
    status = luarray_freeint32(&offsets, status);
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
