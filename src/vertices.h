
#ifndef HP_VERTICES_H
#define HP_VERTICES_H

#include "lu/structs.h"

typedef struct vnorm {
    ludta_fxyzw v;
    ludta_fxyzw n;
} vnorm;

typedef struct luary_vnorm {
    vnorm *v;
    lumem mem;
} luary_vnorm;

int luary_mkvnormn(lulog *log, luary_vnorm **vnorm, size_t n);
int luary_freevnorm(luary_vnorm **vnorm, int prev_status);
int luary_reservevnorm(lulog *log, luary_vnorm *vnorm, size_t n);
int luary_pushvnorm(lulog *log, luary_vnorm *vnorm, ludta_fxyzw v, ludta_fxyzw n);
size_t luary_sizevnorm(luary_vnorm *vnorm);
int luary_dumpvnorm(lulog *log, luary_vnorm *vnorm, const char *name, size_t nlines);

#endif
