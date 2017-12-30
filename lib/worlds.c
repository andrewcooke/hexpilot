
#include "status_codes.h"
#include "lu/internal.h"

#include "worlds.h"
#include "universe.h"


int mkworld(lulog *log, world **world, size_t n_variables, size_t data_size,
		GLFWwindow *window, respond *respond, update *update, before *before, after *after) {
	int status = LU_OK;
	lu_alloc(log, *world, 1);
	lu_alloc(log, (*world)->variables, n_variables);
	try(luary_mkmodel(log, &(*world)->models, 1));
	lu_alloc_size(log, (*world)->data, data_size);
	lu_alloc(log, (*world)->action, 1);
	try(luary_mkcontrol(log, &(*world)->action->controls, 1));
	(*world)->action->log = log;
	(*world)->action->window = window;
	(*world)->respond = respond;
	(*world)->update = update;
	(*world)->before = before;
	(*world)->after = after;
	finally:
	return status;
}

int free_world(world **world, int prev) {
	int status = LU_OK;
	if (world && *world) {
		if ((*world)->models) {
			for (size_t i = 0; i < (*world)->models->mem.used; ++i) {
				status = luary_freeint32(&(*world)->models->m[i]->offsets, status);
				status = luary_freeuint32(&(*world)->models->m[i]->counts, status);
				free((*world)->models->m[i]); (*world)->models->m[i] = NULL;
			}
		}
		free((*world)->data);
		free((*world)->geometry_buffer);
		status = luary_freemodel(&(*world)->models, status);
		if ((*world)->action) {
			try(free_keys((*world)->action));
			free((*world)->action);
		}
		free(*world);
		*world = NULL;
	}
	finally:
	return prev ? prev : status;
}

int push_model(lulog *log, world *world, model *model) {
	return luary_pushmodel(log, world->models, model);
}


int update_world(lulog *log, double delta, world *world) {
	int status = LU_OK;
	try(world->respond(log, delta, world->action, world->variables));
	try(world->update(log, delta, world->variables, world->data));
	finally:
	return status;
}

int display_world(lulog *log, void *programs, world *world) {
	int status = LU_OK;
	if (world->before) {
		try(world->before(log, programs, world));
	}
	for (size_t i = 0; i < world->models->mem.used; ++i) {
		try(world->models->m[i]->send(log, world->models->m[i], world));
		try(world->models->m[i]->draw(log, world->models->m[i], programs));
	}
	if (world->after) {
		try(world->after(log, programs, world));
	}
	finally:
	return status;
}
