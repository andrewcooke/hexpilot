
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/dynamic_memory.h"

#include "models.h"
#include "error_codes.h"


LUARY_MKBASE(model, luary_model, model*, m)

int luary_pushmodel(lulog *log, luary_model *models, model *model) {
    LU_STATUS
    LU_CHECK(luary_reservemodel(log, models, 1))
    models->m[models->mem.used++] = model;
    LU_NO_CLEANUP
}

int mkuniverse(lulog *log, universe **universe, size_t n_variables, GLFWwindow *window) {
    LU_STATUS
    LU_ALLOC(log, *universe, 1)
    (*universe)->log = log;
    LU_ALLOC(log, (*universe)->variables, n_variables)
    LU_CHECK(luary_mkmodeln(log, &(*universe)->models, 1))
    LU_ALLOC(log, (*universe)->geometry, 1);
    LU_ALLOC(log, (*universe)->action, 1)
    LU_CHECK(luary_mkcontroln(log, &(*universe)->action->controls, 1))
    (*universe)->action->log = log;
    (*universe)->action->window = window;
    LU_NO_CLEANUP
}

int mkmodel(lulog *log, model **model, send *send, draw *draw, GLuint program) {
    LU_STATUS
    LU_ALLOC(log, *model, 1)
    (*model)->program = program;
    (*model)->draw = draw;
    (*model)->send = send;
    LU_NO_CLEANUP
}

int free_universe(universe **universe, int prev) {
    LU_STATUS
    if (universe && *universe) {
        if ((*universe)->models) {
            for (size_t i = 0; i < (*universe)->models->mem.used; ++i) {
                status = luary_freeint32(&(*universe)->models->m[i]->offsets, status);
                status = luary_freeuint32(&(*universe)->models->m[i]->counts, status);
                free((*universe)->models->m[i]); (*universe)->models->m[i] = NULL;
            }
        }
        free((*universe)->geometry);
        free((*universe)->geometry_buffer);
        status = luary_freemodel(&(*universe)->models, status);
        if ((*universe)->action) {
            LU_CHECK(free_keys((*universe)->action))
            free((*universe)->action);
        }
        free(*universe);
        *universe = NULL;
    }
LU_CLEANUP
    LU_RETURN2(prev)
}

int push_model(universe *universe, model *model) {
    return luary_pushmodel(universe->log, universe->models, model);
}

int draw_multi_arrays(lulog *log, model *model) {
    LU_STATUS
    GL_CHECK(glBindVertexArray(model->vao))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
LU_CLEANUP
    GL_CLEAN(glBindVertexArray(0))
    LU_RETURN
}

static luvec_f3 hex_red = {1,0,0};

int send_hex_data(lulog *log, model *model, universe *universe) {
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

int send_ship_data(lulog *log, model *model, universe *universe) {
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

