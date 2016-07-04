
#ifndef HP_MODELS_H
#define HP_MODELS_H

#include "lu/arrays.h"

#include "buffers.h"
#include "keys.h"


struct model;
struct world;
struct programs;

typedef int send(lulog *log, struct model *model, struct world *world);
typedef int draw(lulog *log, struct model *model, struct programs *programs);

typedef struct model {
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

int mkmodel(lulog *log, model **model, send *send, draw *draw);
int free_model(model **model, int status);
int draw_triangles(lulog *log, model *model, struct programs *programs);
int draw_lines_and_triangles(lulog *log, model *model, struct programs *programs);

#endif
