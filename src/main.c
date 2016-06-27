
#include "lu/log.h"
#include "lu/tiles.h"
#include "lu/status.h"
#include "lu/arrays.h"

#include "glfw.h"
#include "models.h"
#include "universe.h"
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
    for (size_t i = 0; i < universe->models->mem.used; ++i) {
    	GL_CHECK(glUseProgram(universe->models->m[i]->program))
        LU_CHECK(universe->models->m[i]->send(log, universe->models->m[i], universe))
        LU_CHECK(universe->models->m[i]->draw(log, universe->models->m[i]))
    }
LU_CLEANUP
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}


static int build_flat(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "lit_model.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

static luvec_f3 hex_red = {1,0,0};

static int send_hex_data(lulog *log, model *model, universe *universe) {
    LU_STATUS
//    ludebug(log, "Sending hex geometry");
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, universe->geometry_buffer->name))
    geometry_buffer buffer = {};
    luvec_cpyf3(&hex_red, &buffer.colour);
    luvec_cpyf4(&universe->geometry->camera_light_pos, &buffer.camera_light_pos);
    lumat_cpyf4(&universe->geometry->hex_to_camera, &buffer.model_to_camera);
    lumat_cpyf4(&universe->geometry->hex_to_camera_n, &buffer.model_to_camera_n);
    lumat_cpyf4(&universe->geometry->camera_to_clip, &buffer.camera_to_clip);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(buffer), &buffer))
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_NO_CLEANUP
}

static luvec_f3 ship_cyan = {0,1,1};

static int build_hexagon(universe *universe) {
    LU_STATUS
    lulog *log = universe->log;
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_hex_data, &draw_multi_arrays, universe->programs.flat));
    LU_CHECK(hexagon_vnormal_strips(log, 0, 5, 10, 0.4, 1, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, model->program, model->vertices, &model->vao))
    push_model(universe, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int send_ship_data(lulog *log, model *model, universe *universe) {
    LU_STATUS
//    ludebug(log, "Sending ship geometry");
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, universe->geometry_buffer->name))
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ship_cyan), &ship_cyan))
    lumat_f4 ship_to_camera = {};
    lumat_mulf4(&universe->geometry->hex_to_camera, &universe->geometry->ship_to_hex, &ship_to_camera);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(ship_to_camera), &ship_to_camera))
    lumat_f4 ship_to_camera_n = {};
    lumat_mulf4(&universe->geometry->hex_to_camera_n, &universe->geometry->ship_to_hex_n, &ship_to_camera_n);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 96, sizeof(ship_to_camera_n), &ship_to_camera_n))
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_NO_CLEANUP
}

static int build_ship(universe *universe) {
    LU_STATUS
    lulog *log = universe->log;
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_ship_data, &draw_multi_arrays, universe->programs.flat));
    LU_CHECK(ship_vnormal_strips(log, 0.03, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, model->program, model->vertices, &model->vao))
    push_model(universe, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int build_geometry(universe *universe) {
    LU_STATUS
    lulog *log = universe->log;
    LU_CHECK(load_buffer(log, GL_UNIFORM_BUFFER, GL_STREAM_DRAW,
            NULL, 1, sizeof(geometry_buffer), &universe->geometry_buffer));
    // http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
    GL_CHECK(GLuint index = glGetUniformBlockIndex(universe->programs.flat, "geometry"))
    GL_CHECK(glUniformBlockBinding(universe->programs.flat, index, 1))
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

    LU_CHECK(build_flat(log, &program))
    LU_CHECK(mkuniverse(log, &universe, n_variables, window))
	LU_CHECK(build_flat(log, &universe->programs.flat))
    LU_CHECK(init_keys(log, universe->action))
    LU_CHECK(init_geometry(log, universe->variables))
    LU_CHECK(set_window_callbacks(log, window, universe->action))
    LU_CHECK(build_geometry(universe))
    LU_CHECK(build_hexagon(universe))
    LU_CHECK(build_ship(universe))

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
