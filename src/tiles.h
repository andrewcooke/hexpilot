
#ifndef HP_TILES_H
#define HP_TILES_H

#include "lu/log.h"
#include "lu/arrays.h"

int hexagon(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_xyzw **vertices, luarray_uint **indices, luarray_uint **offsets);

#endif
