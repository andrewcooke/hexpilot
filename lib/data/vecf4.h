
#ifndef HP_DATA_VECF$_H
#define HP_DATA_VECF$_H

#include "lu/arrays.h"
#include "lu/gl/types.h"


typedef struct luary_vecf4 {
    luglv *v;
    lumem mem;
} luary_vecf4;

int luary_vecf4_mk(lulog *log, luary_vecf4 **v, size_t n);
int luary_vecf4_free(luary_vecf4 **v, int prev_status);
int luary_vecf4_res(lulog *log, luary_vecf4 *v, size_t n);
int luary_vecf4_push(lulog *log, luary_vecf4 *v, float x, float y, float z, float w);

#endif
