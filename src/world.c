
#include "lu/status.h"

#include "error_codes.h"
#include "glfw.h"
#include "vertices.h"
#include "tiles.h"
#include "shaders.h"

#include "world.h"
#include "geometry.h"
#include "programs.h"


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
    // this builds the whole geometry buffer so must be done before ship
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, world->geometry_buffer->name))
    geometry_buffer buffer = {};
    luvec_cpyf3(&hex_red, &buffer.colour);
    flight_data *data = (flight_data*) world->data;
    luvec_cpyf4(&data->geometry.camera_light_pos, &buffer.camera_light_pos);
    lumat_cpyf4(&data->geometry.hex_to_camera, &buffer.model_to_camera);
    lumat_cpyf4(&data->geometry.hex_to_camera_n, &buffer.model_to_camera_n);
    lumat_cpyf4(&data->geometry.camera_to_clip, &buffer.camera_to_clip);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(geometry_buffer), &buffer))
LU_CLEANUP
    GL_CLEAN(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_RETURN
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
    LU_CLEAN(luary_freevnorm(&vertices, status))
    LU_RETURN
}

static int send_ship_data(lulog *log, model *model, world *world) {
    LU_STATUS
    // this patches ship-specific changes into existing geometry buffer
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, world->geometry_buffer->name))
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ship_cyan), &ship_cyan))
    lumat_f4 ship_to_camera = {};
    flight_data *data = (flight_data*) world->data;
    lumat_mulf4(&data->geometry.hex_to_camera, &data->geometry.ship_to_hex, &ship_to_camera);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 32, sizeof(ship_to_camera), &ship_to_camera))
    lumat_f4 ship_to_camera_n = {};
    lumat_mulf4(&data->geometry.hex_to_camera_n, &data->geometry.ship_to_hex_n, &ship_to_camera_n);
    GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, 96, sizeof(ship_to_camera_n), &ship_to_camera_n))
LU_CLEANUP
    GL_CLEAN(glBindBuffer(GL_UNIFORM_BUFFER, 0))
    LU_RETURN
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
    LU_CLEAN(luary_freevnorm(&vertices, status))
    LU_RETURN
}

static int build_geometry(lulog *log, programs *programs, world *world) {
    LU_STATUS
    LU_CHECK(load_buffer(log, GL_UNIFORM_BUFFER, GL_STREAM_DRAW,
            NULL, 1, sizeof(geometry_buffer), &world->geometry_buffer));
    // http://learnopengl.com/#!Advanced-OpenGL/Advanced-GLSL
    GL_CHECK(GLuint index = glGetUniformBlockIndex(programs->lit_per_vertex, "geometry"))
    GL_CHECK(glUniformBlockBinding(programs->lit_per_vertex, index, 1))
    GL_CHECK(index = glGetUniformBlockIndex(programs->black, "geometry"))
    GL_CHECK(glUniformBlockBinding(programs->black, index, 1))
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 1, world->geometry_buffer->name))
    LU_NO_CLEANUP
}

static int build_render(lulog *log, programs *programs, flight_data *data) {
    LU_STATUS
    float quad[] = {-1,-1, -1,1, 1,-1, 1,1};
    LU_CHECK(load_buffer(log, GL_ARRAY_BUFFER, GL_STATIC_DRAW, quad, 1, sizeof(quad), &data->quad_buffer))
    GL_CHECK(glGenVertexArrays(1, &data->quad_vao))
    GL_CHECK(glBindVertexArray(data->quad_vao))
    LU_CHECK(bind_buffer(log, data->quad_buffer))
    GL_CHECK(glEnableVertexAttribArray(0))
    GL_CHECK(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0))
LU_CLEANUP
    LU_CLEAN(unbind_buffer(log, data->quad_buffer))
    LU_RETURN
}

static int before_display(lulog *log, void *programs, world *world) {
    LU_STATUS
    flight_data *data = (flight_data*)world->data;

    LU_CHECK(check_frame(log, world->action->window, &data->single))
    LU_CHECK(check_frame(log, world->action->window, &data->tmp))
    LU_CHECK(check_frame(log, world->action->window, &data->multiple))

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, data->single.render))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    LU_NO_CLEANUP
}

static int after_display(lulog *log, void *v, world *world) {
    LU_STATUS
    flight_data *data = (flight_data*)world->data;
    programs *p = (programs*)v;
    GL_CHECK(glDisable(GL_DEPTH_TEST))

    GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, data->single.render))
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data->tmp.render))
    GL_CHECK(glBlitFramebuffer(0, 0, data->single.width, data->single.height,
            0, 0, data->single.width, data->single.height, GL_COLOR_BUFFER_BIT, GL_NEAREST))

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT))
    GL_CHECK(glUseProgram(p->direct_texture))
    GL_CHECK(glBindVertexArray(data->quad_vao))
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, data->tmp.texture))
    GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))

LU_CLEANUP
    GL_CHECK(glBindVertexArray(0))
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0))
    GL_CHECK(glEnable(GL_DEPTH_TEST))
    GL_CLEAN(glBindFramebuffer(GL_FRAMEBUFFER, 0))
    LU_RETURN
}

int build_flight(lulog *log, void *v, GLFWwindow *window, world **world) {
    LU_STATUS
    programs *p = (programs*) v;
    LU_CHECK(mkworld(log, world, n_variables, sizeof(flight_data), window,
            &respond_to_user, &update_geometry, &before_display, &after_display))
    flight_data *data = (flight_data*)(*world)->data;
    LU_CHECK(init_frame(log, window, &data->single, 1, 1))
    LU_CHECK(init_frame(log, window, &data->multiple, 0, 0))
    LU_CHECK(init_frame(log, window, &data->tmp, 0, 0))
    LU_CHECK(init_keys(log, (*world)->action))
    LU_CHECK(init_geometry(log, (*world)->variables))
    LU_CHECK(build_render(log, p, data))
    LU_CHECK(build_geometry(log, p, *world))
    LU_CHECK(build_hexagon(log, p, *world))
    LU_CHECK(build_ship(log, p, *world))
    LU_CHECK(set_window_callbacks(log, window, (*world)->action))
    LU_NO_CLEANUP
}

