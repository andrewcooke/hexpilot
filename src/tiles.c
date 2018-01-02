
#include <math.h>
#include <status.h>
#include <string.h>

#include "lu/internal.h"
#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/gl/vectors.h"

#include "tiles.h"
#include "vertices.h"


static int hexagon_common(lulog *log, uint64_t seed,
		size_t side, size_t subsamples, double octweight,
		luary_ijz **vertices, luary_uint32 **indices,
		luary_uint32 **offsets, luary_uint32 **counts) {
	int status = LU_OK;
	lutle_config *config = NULL;
	lutle_tile *hexagon = NULL;
	try(lutle_defaultconfig(log, &config, seed));
	try(lutle_mkhexagon(log, &hexagon, side, subsamples, octweight));
	try(hexagon->enumerate(hexagon, log, config, -1, vertices));
	try(normalize_z(log, *vertices));
	try(strips(log, *vertices, indices, offsets, counts));
	finally:
	status = lutle_freeconfig(&config, status);
	if (hexagon) status = hexagon->free(&hexagon, status);
	return status;
}


int hexagon_vertex_strips(lulog *log, uint64_t seed,
		size_t side, size_t subsamples, double step, double octweight,
		luary_fxyzw **vertices, luary_uint32 **indices,
		luary_void **offsets, luary_uint32 **counts) {
	int status = LU_OK;
	luary_ijz *ijz = NULL;
	luary_uint32 *ioffsets = NULL;
	try(hexagon_common(log, seed, side, subsamples, octweight,
			&ijz, indices, &ioffsets, counts));
	try(ijz2fxyzw(log, ijz, step, vertices));
	try(offsets2void(log, ioffsets, sizeof(*(*indices)->i), offsets));
	finally:
	status = luary_freeuint32(&ioffsets, status);
	status = luary_freeijz(&ijz, status);
	return status;
}

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
		size_t side, size_t subsamples, double step, double octweight,
		luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts) {
	int status = LU_OK;
	luary_ijz *ijz = NULL;
	luary_uint32 *ioffsets = NULL, *indices = NULL;
	luary_vecf4 *f4 = NULL;
	try(hexagon_common(log, seed, side, subsamples, octweight,
			&ijz, &indices, &ioffsets, counts));
	try(uniquify(log, indices, ioffsets, *counts, ijz));
	try(ijz2vecf4(log, ijz, step, &f4));
	try(normals(log, indices, ioffsets, *counts, f4, vertices));
	try(uint2int(log, ioffsets, offsets));
	finally:
	status = luary_freeuint32(&ioffsets, status);
	status = luary_freeuint32(&indices, status);
	status = luary_freeijz(&ijz, status);
	status = luary_freevecf4(&f4, status);
	return status;
}

int ship_vnormal_strips(lulog *log, double step,
		luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts) {
	int status = LU_OK;
	float points[][4] = {{-1,-3,0,1},{-1,3,0,1},{5,0,0,1},{0,0,1,1}};
	uint32_t index[] = {1,0,2,3,1,0};
	size_t np = sizeof(points) / sizeof(points[0]), ni = sizeof(index) / sizeof(index[0]);
	luary_vecf4 *f4 = NULL;
	luary_uint32 *ioffsets = NULL, *indices = NULL;
	try(luary_mkvecf4(log, &f4, np));
	memcpy(f4->v, points, sizeof(points)); f4->mem.used = np;
	for (size_t i = 0; i < f4->mem.used; ++i) {
		luglv_scale_inplace(step, &f4->v[i]);
	}
	try(luary_mkuint32(log, &indices, ni));
	memcpy(indices->i, index, sizeof(index)); indices->mem.used = ni;
	try(luary_mkuint32(log, &ioffsets, 1));
	try(luary_pushuint32(log, ioffsets, 0));
	try(luary_mkuint32(log, counts, 1));
	try(luary_pushuint32(log, *counts, ni));
	try(normals(log, indices, ioffsets, *counts, f4, vertices));
	try(uint2int(log, ioffsets, offsets));
	luary_dumpvnorm(log, *vertices, "Ship vnorms", (*vertices)->mem.used);
	finally:
	status = luary_freevecf4(&f4, status);
	status = luary_freeuint32(&ioffsets, status);
	status = luary_freeuint32(&indices, status);
	return status;
}

