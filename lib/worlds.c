
#include "lu/status.h"

#include "error_codes.h"
#include "worlds.h"
#include "universe.h"


int mkworld(lulog *log, world **world, size_t n_variables, size_t data_size,
		GLFWwindow *window, respond *respond, update *update) {
    LU_STATUS
    LU_ALLOC(log, *world, 1)
    LU_ALLOC(log, (*world)->variables, n_variables)
    LU_CHECK(luary_mkmodeln(log, &(*world)->models, 1))
    LU_ALLOC_SIZE(log, (*world)->data, data_size);
    LU_ALLOC(log, (*world)->action, 1)
    LU_CHECK(luary_mkcontroln(log, &(*world)->action->controls, 1))
    (*world)->action->log = log;
    (*world)->action->window = window;
    (*world)->respond = respond;
    (*world)->update = update;
    LU_NO_CLEANUP
}

int free_world(world **world, int prev) {
    LU_STATUS
    if (world && *world) {
        if ((*world)->models) {
            for (size_t i = 0; i < (*world)->models->mem.used; ++i) {
                status = luary_freeint32(&(*world)->models->m[i]->offsets, status);
                status = luary_freeuint32(&(*world)->models->m[i]->counts, status);
                free((*world)->models->m[i]); (*world)->models->m[i] = NULL;
            }
        }
        free((*world)->data);
        free((*world)->data_buffer);
        status = luary_freemodel(&(*world)->models, status);
        if ((*world)->action) {
            LU_CHECK(free_keys((*world)->action))
            free((*world)->action);
        }
        free(*world);
        *world = NULL;
    }
LU_CLEANUP
    LU_RETURN2(prev)
}

int push_model(lulog *log, world *world, model *model) {
    return luary_pushmodel(log, world->models, model);
}


int update_world(lulog *log, double delta, world *world) {
	LU_STATUS
    LU_CHECK(world->respond(log, delta, world->action, world->variables))
    LU_CHECK(world->update(log, delta, world->variables, world->data))
	LU_NO_CLEANUP
}

int display_world(lulog *log, void *programs, world *world) {
    LU_STATUS
    GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f))
    GL_CHECK(glClearDepth(1.0f))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    for (size_t i = 0; i < world->models->mem.used; ++i) {
        LU_CHECK(world->models->m[i]->send(log, world->models->m[i], world))
        LU_CHECK(world->models->m[i]->draw(log, world->models->m[i], programs))
    }
    LU_NO_CLEANUP
}