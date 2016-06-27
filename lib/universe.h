
#ifndef HP_UNIVERSE_H
#define HP_UNIVERSE_H

#include "models.h"


typedef struct programs {
	GLuint flat;
} programs;

typedef struct universe {
    lulog *log;
    programs programs;
    float *variables;
    luary_model *models;
    geometry *geometry;
    buffer *geometry_buffer;
    user_action *action;
} universe;

int mkuniverse(lulog *log, universe **universe, size_t n_variables, GLFWwindow *window);
int free_universe(universe **universe, int status);
int push_model(struct universe *universe, model *model);

#endif
