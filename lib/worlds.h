
#ifndef HP_WORLD_H
#define HP_WORLD_H

#include "models.h"


struct world;

typedef int respond(lulog *log, double delta, user_action *action, float *variables);
typedef int update(lulog *log, double delta, float *variables, void *data);
typedef int before(lulog *log, void *programs, struct world *world);
typedef int after(lulog *log, void *programs, struct world *world);

typedef struct world {
    float *variables;
    void *data;
    buffer *geometry_buffer;
    user_action *action;
    luary_model *models;
    respond *respond;
    update *update;
    before *before;
    after *after;
} world;

int mkworld(lulog *log, world **world, size_t n_variables, size_t data_size,
		GLFWwindow *window, respond *respond, update *update, before *before, after *after);
int free_world(world **world, int status);
int push_model(lulog *log, world *world, model *model);

int update_world(lulog *log, double delta, world *world);
int display_world(lulog *log, void *programs, world *world);

#endif
