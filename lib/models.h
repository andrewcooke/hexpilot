
#ifndef HP_MODELS_H
#define HP_MODELS_H

#include "lu/arrays.h"

#include "buffers.h"
#include "keys.h"
#include "geometry.h"


struct model;
struct universe;

typedef int send(lulog *log, struct model *model, struct universe *universe);
typedef int draw(lulog *log, struct model *model);

typedef struct model {
	GLuint program;
    GLuint vao;
    buffer *vertices;
    luary_int32 *offsets;
    luary_uint32 *counts;
    luvec_f3 colour;
    send *send;
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
int mkmodel(lulog *log, model **model, send *send, draw *draw, GLuint program);
int free_model(model **model, int status);
int push_model(universe *universe, model *model);
int draw_multi_arrays(lulog *log, model *model);

int send_hex_data(lulog *log, model *model, universe *universe);
int send_ship_data(lulog *log, model *model, universe *universe);

#endif
