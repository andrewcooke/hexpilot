
#ifndef HP_VERTICES_H
#define HP_VERTICES_H

#include "lu/structs.h"
#include "lu/vectors.h"
#include "lu/arrays.h"
#include "lu/data/fxyzw.h"
#include "lu/data/ijz.h"
#include "lu/data/int32.h"
#include "lu/data/void.h"

#include "data/uint32.h"
#include "data/vecf4.h"


typedef luglv vnorm[2];

typedef struct luary_vnorm {
    vnorm *vn;
    lumem mem;
} luary_vnorm;

int luary_mkvnorm(lulog *log, luary_vnorm **vnorm, size_t n);
int luary_freevnorm(luary_vnorm **vnorm, int prev_status);
int luary_reservevnorm(lulog *log, luary_vnorm *vnorm, size_t n);
int luary_pushvnorm(lulog *log, luary_vnorm *vnorm, luglv *v, luglv *n);
int luary_dumpvnorm(lulog *log, luary_vnorm *vnorm, const char *name, size_t nlines);

int strips(lulog *log, luary_ijz *ijz,
        luary_uint32 **indices, luary_uint32 **offsets, luary_uint32 **counts);
int ijz2fxyzw(lulog *log, luary_ijz *ijz, float step, luary_fxyzw **fxyzw);
int ijz2vecf4(lulog *log, luary_ijz *ijz, float step, luary_vecf4 **f4);
int offsets2void(lulog *log, luary_uint32 *in, size_t chunk, luary_void **out);
int normalize_z(lulog *log, luary_ijz *vertices);
int uniquify(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
        luary_uint32 *counts, luary_ijz *vertices);
int normals(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
        luary_uint32 *counts, luary_vecf4 *vertices, luary_vnorm **vnorms);
int uint2int(lulog *log, luary_uint32 *in, luary_int32 **out);

#endif
