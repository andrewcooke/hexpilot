
#include <stdlib.h>

#include "lu/status.h"

#include "universe.h"


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

int push_model(struct universe *universe, model *model) {
    return luary_pushmodel(universe->log, universe->models, model);
}

