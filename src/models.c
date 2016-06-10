
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

int mkuniverse(lulog *log, universe **universe,
        GLuint program, size_t n_variables, GLFWwindow *window) {
    LU_STATUS
    LU_ALLOC(log, *universe, 1)
    (*universe)->log = log;
    (*universe)->program = program;
    LU_ALLOC(log, (*universe)->variables, n_variables)
    LU_CHECK(luary_mkmodeln(log, &(*universe)->models, 1))
    LU_ALLOC(log, (*universe)->action, 1)
    LU_CHECK(luary_mkcontroln(log, &(*universe)->action->controls, 1))
    (*universe)->action->log = log;
    (*universe)->action->window = window;
    LU_NO_CLEANUP
}

int mkmodel(lulog *log, model **model) {
    LU_STATUS
    LU_ALLOC(log, *model, 1)
    (*model)->draw = &draw_multi_arrays;
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
        status = luary_freemodel(&(*universe)->models, status);
        free(*universe);
        *universe = NULL;
    }
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
    GL_CHECK(glBindVertexArray(0))
    LU_RETURN
}
