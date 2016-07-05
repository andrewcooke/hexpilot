
#include "lu/status.h"

#include "error_codes.h"
#include "glfw.h"
#include "vertices.h"
#include "tiles.h"
#include "shaders.h"

#include "geometry.h"
#include "flight.h"


static int init_keys(lulog *log, user_action *action) {
    LU_STATUS
    keys k = {};
    LU_CHECK(set_keys(log, &k, "+/-", 61, 1, 45, 0,
            15, 10, 0,
            0.1, 10, camera_zoom))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 1))
    LU_CHECK(set_keys(log, &k, "left/right", 262, 0, 263, 0,
            0.3, 5, 5,
            -0.5, 0.5, ship_rotation))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
    LU_CHECK(set_keys(log, &k, "up/down",
            265, 0, 264, 0,
            2, 10, 0,
            0, 4, ship_speed))
    LU_CHECK(luary_pushcontrol(log, action->controls, &k, 0))
    LU_NO_CLEANUP
}

static int respond_to_user(lulog *log, double dt, user_action *action, float *variables) {
    LU_STATUS
    LU_CHECK(update_controls(log, dt, action->controls, variables))
    int width, height;
    glfwGetFramebufferSize(action->window, &width, &height);
    GL_CHECK(glViewport(0, 0, width, height))
    variables[buffer_x] = width; variables[buffer_y] = height;
    LU_NO_CLEANUP
}


static luvec_f3 hex_red = {1,0,0};

static int send_hex_data(lulog *log, model *model, world *world) {
    LU_STATUS
//    ludebug(log, "Sending hex geometry");
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, world->data_buffer->name))
    geometry_buffer buffer = {};
    luvec_cpyf3(&hex_red, &buffer.colour);
    geometry *g = (geometry*) world->data;
    luvec_cpyf4(&g->camera_light_pos, &buffer.camera_light_pos);
    lumat_cpyf4(&g->hex_to_camera, &buffer.model_to_camera);
    lumat_cpyf4(&g->hex_to_camera_n, &buffer.model_to_camera_n);
    lumat_cpyf4(&g->camera_to_clip, &buffer.camera_to_clip);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(geometry_buffer), &buffer))
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_NO_CLEANUP
}

static luvec_f3 ship_cyan = {0,1,1};

static int build_hexagon(lulog *log, programs *programs, world *world) {
    LU_STATUS
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_hex_data, &draw_lines_and_triangles));
    LU_CHECK(hexagon_vnormal_strips(log, 0, 5, 10, 0.4, 1, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, model->vertices, &model->vao))
    push_model(log, world, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int send_ship_data(lulog *log, model *model, world *world) {
    LU_STATUS
//    ludebug(log, "Sending ship geometry");
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, world->data_buffer->name))
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ship_cyan), &ship_cyan))
    lumat_f4 ship_to_camera = {};
    geometry *g = (geometry*) world->data;
    lumat_mulf4(&g->hex_to_camera, &g->ship_to_hex, &ship_to_camera);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(ship_to_camera), &ship_to_camera))
    lumat_f4 ship_to_camera_n = {};
    lumat_mulf4(&g->hex_to_camera_n, &g->ship_to_hex_n, &ship_to_camera_n);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 96, sizeof(ship_to_camera_n), &ship_to_camera_n))
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_NO_CLEANUP
}

static int build_ship(lulog *log, programs *programs, world *world) {
    LU_STATUS
    model *model = NULL;
    luary_vnorm *vertices = NULL;
    LU_CHECK(mkmodel(log, &model, &send_ship_data, &draw_lines_and_triangles));
    LU_CHECK(ship_vnormal_strips(log, 0.03, &vertices, &model->offsets, &model->counts))
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW,
            vertices->vn, vertices->mem.used, sizeof(*vertices->vn), &model->vertices))
    LU_CHECK(interleaved_vnorm_vao(log, model->vertices, &model->vao))
    push_model(log, world, model);
LU_CLEANUP
    status = luary_freevnorm(&vertices, status);
    LU_RETURN
}

static int build_geometry(lulog *log, programs *programs, world *world) {
    LU_STATUS
    LU_CHECK(load_buffer(log, GL_UNIFORM_BUFFER, GL_STREAM_DRAW,
            NULL, 1, sizeof(geometry), &world->data_buffer));
    // http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
    GL_CHECK(GLuint index = glGetUniformBlockIndex(programs->lit_per_vertex, "geometry"))
    GL_CHECK(glUniformBlockBinding(programs->lit_per_vertex, index, 1))
    GL_CHECK(index = glGetUniformBlockIndex(programs->black, "geometry"))
    GL_CHECK(glUniformBlockBinding(programs->black, index, 1))
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 1, world->data_buffer->name))
    LU_NO_CLEANUP
}

int build_flight(lulog *log, programs *programs, GLFWwindow *window, world **world) {
    LU_STATUS
    LU_CHECK(mkworld(log, world, n_variables, sizeof(geometry), window,
            &respond_to_user, &update_geometry))
    LU_CHECK(init_keys(log, (*world)->action))
    LU_CHECK(init_geometry(log, (*world)->variables))
    LU_CHECK(build_geometry(log, programs, *world))
    LU_CHECK(build_hexagon(log, programs, *world))
    LU_CHECK(build_ship(log, programs, *world))
    LU_CHECK(set_window_callbacks(log, window, (*world)->action))
    LU_NO_CLEANUP
}
