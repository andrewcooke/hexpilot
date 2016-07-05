
#ifndef HP_TILES_H
#define HP_TILES_H

#include "lu/log.h"
#include "lu/arrays.h"

#include "vertices.h"


int hexagon_vertex_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_fxyzw **vertices, luary_uint32 **indices,
		luary_void **offsets, luary_uint32 **counts);

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts);

int ship_vnormal_strips(lulog *log, double step,
        luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts);

#endif
