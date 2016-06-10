
#ifndef HP_MODELS_H
#define HP_MODELS_H

#include "lu/arrays.h"

#include "buffers.h"
#include "keys.h"


struct model;

typedef int draw(lulog *log, struct model *model);

typedef struct model {
    GLuint vao;
    buffer *vertices;
    luary_int32 *offsets;
    luary_uint32 *counts;
    draw *draw;
} model;

typedef struct luary_model {
    model **m;
    lumem mem;
} luary_model;

int luary_mkmodeln(lulog *log, luary_model **models, size_t n);
int luary_freemodel(luary_model **models, int prev_status);
int luary_reservemodel(lulog *log, luary_model *models, size_t n);
int luary_pushmodel(lulog *log, luary_model *models, model *model);
size_t luary_sizemodel(luary_model *models);

typedef struct universe {
    lulog *log;
    GLuint program;
    float *variables;
    luary_model *models;
    buffer *geometry;
    user_action *action;
} universe;

int mkuniverse(lulog *log, universe **universe,
        GLuint program, size_t n_variables, GLFWwindow *window);
int free_universe(universe **universe, int status);
int mkmodel(lulog *log, model **model);
int free_model(model **model, int status);
int push_model(universe *universe, model *model);
int draw_multi_arrays(lulog *log, model *model);

#endif
