
#ifndef HP_VERTICES_H
#define HP_VERTICES_H

#include "lu/structs.h"

typedef struct vnorm {
    ludata_fxyzw v;
    ludata_fxyzw n;
} vnorm;

typedef struct luarray_vnorm {
    vnorm *v;
    lumem mem;
} luarray_vnorm;

int luarray_mkvnormn(lulog *log, luarray_vnorm **vnorm, size_t n);
int luarray_freevnorm(luarray_vnorm **vnorm, int prev_status);
int luarray_reservevnorm(lulog *log, luarray_vnorm *vnorm, size_t n);
int luarray_pushvnorm(lulog *log, luarray_vnorm *vnorm, ludata_fxyzw v, ludata_fxyzw n);
size_t luarray_sizevnorm(luarray_vnorm *vnorm);
int luarray_dumpvnorm(lulog *log, luarray_vnorm *vnorm, const char *name, size_t nlines);

#endif
