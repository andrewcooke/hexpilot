
#include "lu/log.h"
#include "lu/tiles.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "models.h"
#include "shaders.h"
#include "buffers.h"
#include "tiles.h"
#include "error_codes.h"
#include "geometry.h"


static int display(universe *universe) {
    LU_STATUS
    lulog *log = universe->log;
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClearDepth(1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    GL_CHECK(glUseProgram(universe->program))
    for (size_t i = 0; i < universe->models->mem.used; ++i) {
        LU_CHECK(universe->models->m[i]->send(log, universe->models->m[i], universe))
        LU_CHECK(universe->models->m[i]->draw(log, universe->models->m[i]))
    }
LU_CLEANUP
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}

static const char* vertex_shader =
        "#version 330\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec4 normal;\n"
        "layout(std140) uniform geometry {\n"
        "  vec3 colour;\n"
        "  vec4 camera_light_pos;\n"
        "  mat4 model_to_camera;\n"
        "  mat4 model_to_camera_n;\n"
        "  mat4 camera_to_clip;\n"
        "};\n"
        "flat out vec4 interpColour;\n"
        "void main(){\n"
        "  vec4 c_position = model_to_camera * position;\n"
        "  vec4 c_normal = vec4(normalize((model_to_camera_n * normal).xyz), 0);\n"
        "  float brightness_1 = clamp(dot(c_normal, camera_light_pos), 0, 1);\n"
        "  float brightness_2 = clamp(dot(c_normal, vec4(0,0,1,0)), 0, 1);\n"
        "  float brightness = 0.6 * brightness_1 + 0.1 * brightness_2;\n"
        "  interpColour = vec4(brightness * colour, 1.0);\n"
        "  gl_Position = camera_to_clip * c_position;\n"
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

static int build_hexagon(universe *universe, GLuint program) {
    LU_STATUS
    lulog *log = universe->log;
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_hex_data));
    LU_CHECK(hexagon_vnormal_strips(log, 0, 5, 10, 0.4, 1, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, program, model->vertices, &model->vao))
    push_model(universe, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int build_ship(universe *universe, GLuint program) {
    LU_STATUS
    lulog *log = universe->log;
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_ship_data));
    LU_CHECK(ship_vnormal_strips(log, 0.03, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, program, model->vertices, &model->vao))
    push_model(universe, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int build_geometry(universe *universe, GLuint program) {
    LU_STATUS
    lulog *log = universe->log;
    LU_CHECK(load_buffer(log, GL_UNIFORM_BUFFER, GL_STREAM_DRAW,
            NULL, 1, sizeof(geometry_buffer), &universe->geometry_buffer));
    // http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
    GL_CHECK(GLuint index = glGetUniformBlockIndex(program, "geometry"))
    GL_CHECK(glUniformBlockBinding(program, index, 1))
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 1, universe->geometry_buffer->name))
    LU_NO_CLEANUP
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
    GLuint program;
    universe *universe = NULL;

    LU_CHECK(create_glfw_context(log, &window))
    LU_CHECK(load_opengl_functions(log))
    LU_CHECK(init_opengl(log))

    LU_CHECK(build_program(log, &program))
    LU_CHECK(mkuniverse(log, &universe, program, n_variables, window))
    LU_CHECK(init_keys(log, universe->action))
    LU_CHECK(init_geometry(log, universe->variables))
    LU_CHECK(set_window_callbacks(log, window, universe->action))
    LU_CHECK(build_geometry(universe, program))
    LU_CHECK(build_hexagon(universe, program))
    LU_CHECK(build_ship(universe, program))

    double tik[2] = {glfwGetTime(), 0};
    double fpszero = glfwGetTime(); int fcount = 0;
    while (!glfwWindowShouldClose(window)) {
        tik[1] = glfwGetTime();
        if (tik[1] > fpszero+1) {
//            ludebug(log, "%0.1f fps", fcount / (tik[1] - fpszero));
            fpszero = tik[1]; fcount = 0;
        }
        LU_CHECK(respond_to_user(log, tik[1] - tik[0],
                universe->action, universe->variables))
        LU_CHECK(update_geometry(log, tik[1] - tik[0],
                universe->variables, universe->geometry))
        LU_CHECK(display(universe))
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
