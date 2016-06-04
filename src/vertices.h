
#ifndef HP_VERTICES_H
#define HP_VERTICES_H

#include "lu/structs.h"
#include "lu/vectors.h"


typedef luvec_f4 vnorm[2];

typedef struct luary_vnorm {
    vnorm *vn;
    lumem mem;
} luary_vnorm;

int luary_mkvnormn(lulog *log, luary_vnorm **vnorm, size_t n);
int luary_freevnorm(luary_vnorm **vnorm, int prev_status);
int luary_reservevnorm(lulog *log, luary_vnorm *vnorm, size_t n);
int luary_pushvnorm(lulog *log, luary_vnorm *vnorm, luvec_f4 *v, luvec_f4 *n);
size_t luary_sizevnorm(luary_vnorm *vnorm);
int luary_dumpvnorm(lulog *log, luary_vnorm *vnorm, const char *name, size_t nlines);

#endif
