
#ifndef HP_WORLD_H
#define HP_WORLD_H

#include "models.h"
#include "geometry.h"


typedef struct world {
    float *variables;
    geometry *geometry;
    buffer *geometry_buffer;
    user_action *action;
    luary_model *models;
} world;

int mkworld(lulog *log, world **world, size_t n_variables, GLFWwindow *window);
int free_world(world **world, int status);
int push_model(lulog *log, world *world, model *model);

int respond_to_user(lulog *log, double dt, world *world);

#endif
