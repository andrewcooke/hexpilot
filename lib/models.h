
#ifndef HP_MODELS_H
#define HP_MODELS_H

#include "lu/data/int32.h"

#include "keys.h"
#include "data/uint32.h"
#include "data_buffers.h"


struct model;
struct world;
struct programs;

typedef int send(lulog *log, struct model *model, struct world *world);
typedef int draw(lulog *log, struct model *model, struct programs *programs);

typedef struct model {
    GLuint vao;
    data_buffer *vertices;
    luary_int32 *offsets;
    luary_uint32 *counts;
    luglc colour;
    send *send;
    draw *draw;
} model;

typedef struct luary_model {
    model **m;
    lumem mem;
} luary_model;

int luary_model_mk(lulog *log, luary_model **models, size_t n);
int luary_model_free(luary_model **models, int prev_status);
int luary_model_res(lulog *log, luary_model *models, size_t n);
int luary_model_push(lulog *log, luary_model *models, model *model);

int model_mk(lulog *log, model **model, send *send, draw *draw);

#endif
