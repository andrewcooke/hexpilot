
#ifndef HP_TILES_H
#define HP_TILES_H

#include "lu/log.h"
#include "lu/arrays.h"

#include "vertices.h"


int hexagon_vertex_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_fxyzw **vertices, luarray_uint32 **indices,
		luarray_void **offsets, luarray_uint32 **counts);

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_vnorm **vertices, luarray_int32 **offsets, luarray_uint32 **counts);

#endif
