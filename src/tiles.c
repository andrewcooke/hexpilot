
#include "lu/status.h"
#include "lu/tiles.h"
#include "lu/random.h"

#include "tiles.h"


int hexagon(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_xyz **vertices, luarray_uint **indices, luarray_uint **offsets) {
    LU_STATUS
    lutile_config *config = NULL;
    lutile_tile *hexagon = NULL;
    luarray_ijz *ijz = NULL;
    LU_CHECK(lutile_defaultconfig(log, &config, seed))
    LU_CHECK(lutile_mkhexagon(log, &hexagon, side, subsamples, octweight))
    LU_CHECK(hexagon->enumerate(hexagon, log, config, -1, &ijz))
    LU_CHECK(lutile_strips(log, ijz, indices, offsets))
    LU_CHECK(lutile_ijz2xyz(log, ijz, step, vertices))
LU_CLEANUP
    status = lutile_freeconfig(&config, status);
    if (hexagon) status = hexagon->free(&hexagon, status);
    status = luarray_freeijz(&ijz, status);
    LU_RETURN
}
