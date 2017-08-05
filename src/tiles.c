
#include <math.h>
#include <status_codes.h>
#include <string.h>

#include "lu/status.h"
#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/structs.h"
#include "lu/vectors.h"

#include "tiles.h"
#include "vertices.h"


static int hexagon_common(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double octweight,
        luary_ijz **vertices, luary_uint32 **indices,
		luary_uint32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    lutle_config *config = NULL;
    lutle_tile *hexagon = NULL;
    LU_CHECK(lutle_defaultconfig(log, &config, seed))
    LU_CHECK(lutle_mkhexagon(log, &hexagon, side, subsamples, octweight))
    LU_CHECK(hexagon->enumerate(hexagon, log, config, -1, vertices))
    LU_CHECK(normalize_z(log, *vertices));
    LU_CHECK(strips(log, *vertices, indices, offsets, counts))
LU_CLEANUP
    status = lutle_freeconfig(&config, status);
    if (hexagon) status = hexagon->free(&hexagon, status);
    LU_RETURN
}


int hexagon_vertex_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_fxyzw **vertices, luary_uint32 **indices,
        luary_void **offsets, luary_uint32 **counts) {
    LU_STATUS
    luary_ijz *ijz = NULL;
    luary_uint32 *ioffsets = NULL;
    LU_CHECK(hexagon_common(log, seed, side, subsamples, octweight,
            &ijz, indices, &ioffsets, counts))
    LU_CHECK(ijz2fxyzw(log, ijz, step, vertices))
    LU_CHECK(offsets2void(log, ioffsets, sizeof(*(*indices)->i), offsets))
LU_CLEANUP
    status = luary_freeuint32(&ioffsets, status);
    status = luary_freeijz(&ijz, status);
    LU_RETURN
}

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    luary_ijz *ijz = NULL;
    luary_uint32 *ioffsets = NULL, *indices = NULL;
    luary_vecf4 *f4 = NULL;
    LU_CHECK(hexagon_common(log, seed, side, subsamples, octweight,
            &ijz, &indices, &ioffsets, counts))
    LU_CHECK(uniquify(log, indices, ioffsets, *counts, ijz))
    LU_CHECK(ijz2vecf4(log, ijz, step, &f4))
    LU_CHECK(normals(log, indices, ioffsets, *counts, f4, vertices))
    LU_CHECK(uint2int(log, ioffsets, offsets))
LU_CLEANUP
    status = luary_freeuint32(&ioffsets, status);
    status = luary_freeuint32(&indices, status);
    status = luary_freeijz(&ijz, status);
    status = luary_freevecf4(&f4, status);
    LU_RETURN
}

int ship_vnormal_strips(lulog *log, double step,
        luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    float points[][4] = {{-1,-3,0,1},{-1,3,0,1},{5,0,0,1},{0,0,1,1}};
    uint32_t index[] = {1,0,2,3,1,0};
    size_t np = sizeof(points) / sizeof(points[0]), ni = sizeof(index) / sizeof(index[0]);
    luary_vecf4 *f4 = NULL;
    luary_uint32 *ioffsets = NULL, *indices = NULL;
    LU_CHECK(luary_mkvecf4(log, &f4, np))
    memcpy(f4->v, points, sizeof(points)); f4->mem.used = np;
    for (size_t i = 0; i < f4->mem.used; ++i) {
        luvec_sclf4_3in(step, &f4->v[i]);
    }
    LU_CHECK(luary_mkuint32(log, &indices, ni))
    memcpy(indices->i, index, sizeof(index)); indices->mem.used = ni;
    LU_CHECK(luary_mkuint32(log, &ioffsets, 1))
    LU_CHECK(luary_pushuint32(log, ioffsets, 0))
    LU_CHECK(luary_mkuint32(log, counts, 1))
    LU_CHECK(luary_pushuint32(log, *counts, ni))
    LU_CHECK(normals(log, indices, ioffsets, *counts, f4, vertices))
    LU_CHECK(uint2int(log, ioffsets, offsets))
    luary_dumpvnorm(log, *vertices, "Ship vnorms", (*vertices)->mem.used);
LU_CLEANUP
    status = luary_freevecf4(&f4, status);
    status = luary_freeuint32(&ioffsets, status);
    status = luary_freeuint32(&indices, status);
    LU_RETURN
}

