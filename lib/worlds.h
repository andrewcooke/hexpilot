
#ifndef HP_WORLD_H
#define HP_WORLD_H

#include "models.h"


struct world;

typedef int respond(lulog *log, double delta, user_action *action, float *variables);
typedef int update(lulog *log, double delta, float *variables, void *data);

typedef struct world {
    float *variables;
    void *data;
    buffer *data_buffer;
    user_action *action;
    luary_model *models;
    respond *respond;
    update *update;
} world;

int mkworld(lulog *log, world **world, size_t n_variables, size_t data_size,
		GLFWwindow *window, respond *respond, update *update);
int free_world(world **world, int status);
int push_model(lulog *log, world *world, model *model);

int update_world(lulog *log, double delta, world *world);
int display_world(lulog *log, world *world);

#endif
