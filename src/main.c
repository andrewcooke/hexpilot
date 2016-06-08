
#include "lu/log.h"
#include "lu/tiles.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "shaders.h"
#include "buffers.h"
#include "tiles.h"
#include "error_codes.h"
#include "geometry.h"


static int display(lulog *log, GLuint program, GLuint vao,
        luary_int32 *offsets, luary_uint32 *counts) {
    LU_STATUS
    GL_CHECK(glUseProgram(program))
    GL_CHECK(glBindVertexArray(vao))
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClearDepth(1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, offsets->i, counts->i, counts->mem.used));
LU_CLEANUP
    GL_CHECK(glBindVertexArray(0))
    LU_RETURN
}

static int build_buffers(lulog *log, luary_buffer **buffers,
		luary_int32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    luary_vnorm *vertices = NULL;
    luary_uint32 *indices = NULL;
    LU_CHECK(hexagon_vnormal_strips(log, 0, 5, 5, 1, 1.0, &vertices, offsets, counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), buffers));
    LU_CHECK(luary_dumpvnorm(log, vertices, "vertices", 4))
    LU_CHECK(luary_dumpint32(log, *offsets, "offsets", 10))
    LU_CHECK(luary_dumpuint32(log, *counts, "counts", 10))
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    status = luary_freeuint32(&indices, status);
    LU_RETURN
}

static const char* vertex_shader =
        "#version 330\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec4 normal;\n"
        "flat out vec4 interpColour;\n"
        "uniform mat4 transform;\n"
        "uniform mat4 ntransform;\n"
        "void main(){\n"
        "  vec4 t_position = transform * position;\n"
        "  vec4 t_normal = vec4(normalize((ntransform * normal).xyz), 0);\n"
        "  float brightness_1 = dot(t_normal, vec4(1,1,1,0));\n"
        "  float brightness_2 = dot(t_normal, vec4(0,0,1,0));\n"
        "  float brightness = clamp(0.05 * brightness_1 + 0.95 * brightness_2, 0, 1);\n"
        "  interpColour = vec4(brightness * vec3(1.0, 0.0, 0.0), 1.0);\n"
        "  gl_Position = t_position;\n"
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
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, &shaders))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

static int build_vao(lulog *log, GLuint program, luary_buffer *buffers,
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

static int init_opengl(lulog *log) {
    LU_STATUS
    GL_CHECK(glEnable(GL_CULL_FACE))
    GL_CHECK(glCullFace(GL_BACK))
    GL_CHECK(glFrontFace(GL_CW))
    GL_CHECK(glEnable(GL_DEPTH_TEST))
    GL_CHECK(glDepthMask(GL_TRUE))
    GL_CHECK(glDepthFunc(GL_LEQUAL))
    GL_CHECK(glDepthRange(0.0f, 1.0f))
//    GL_CHECK(glEnable(GL_DEPTH_CLAMP))
    LU_NO_CLEANUP
}

static int with_glfw(lulog *log) {
    LU_STATUS
    GLFWwindow *window = NULL;
    luary_buffer *buffers = NULL;
    luary_uint32 *counts = NULL;
    luary_int32 *offsets = NULL;
    GLuint program, vao;
    user_action *action = NULL;
    float variables[n_variables] = {};
    LU_CHECK(init_geometry(log, variables));
    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    LU_CHECK(init_opengl(log))
    LU_CHECK(set_window_callbacks(log, window, &action))
    LU_CHECK(build_program(log, &program))
    LU_CHECK(build_buffers(log, &buffers, &offsets, &counts))
    LU_CHECK(build_vao(log, program, buffers, &vao))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(respond_to_user(log, action, variables));
        LU_CHECK(update_geometry(log, program, variables));
        LU_CHECK(display(log, program, vao, offsets, counts))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ludebug(log, "Clean exit");
LU_CLEANUP
    glfwTerminate();
    free(action);
    status = luary_freebuffer(&buffers, status);
    status = luary_freeint32(&offsets, status);
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
