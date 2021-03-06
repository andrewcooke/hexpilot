
#include <math.h>
#include <status.h>
#include <string.h>

#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/gl.h"
#include "lu/arrays.h"
#include "lu/log.h"

#include "vertices.h"


LUARY_MKBASE(vnorm, vnorm, vn)

int luary_vnorm_push(lulog *log, luary_vnorm *vn, luglv *v, luglv *n) {
	int status = LU_OK;
	try(luary_vnorm_res(log, vn, 1));
	for (size_t i = 0; i < 4; ++i) {
		vn->vn[vn->mem.used][0][i] = (*v)[i];
		vn->vn[vn->mem.used][1][i] = (*n)[i];
	}
	vn->mem.used++;
	finally:
	return status;
}

static char bv[100];
static char bn[100];
LUARY_MKDUMP(luary_vnorm_dump, luary_vnorm, "%s/%s",
		luglv_str(&ptr->vn[i][0], 100, bv), luglv_str(&ptr->vn[i][1], 100, bn))

static int ijeq(ludta_ij ij1, ludta_ij ij2) {
	return ij1.i == ij2.i && ij1.j == ij2.j;
}

static inline ludta_ij ijz2ij(ludta_ijz ijz) {
	return (ludta_ij){ijz.i, ijz.j};
}

static inline int ij2index(ludta_ij ij, ludta_ij bl, ludta_ij tr) {
	return ij.i - bl.i + (ij.j - bl.j) * (tr.i - bl.i + 1);
}

static inline int ijz2index(ludta_ijz ijz, ludta_ij bl, ludta_ij tr) {
	return ij2index(ijz2ij(ijz), bl, tr);
}

static int index_mk(lulog *log, luary_ijz *ijz, ludta_ij bl, ludta_ij tr,
		size_t **index) {
	int status = LU_OK;
	size_t nx = tr.i - bl.i + 1, ny = tr.j - bl.j + 1;
	LU_ALLOC(log, *index, nx*ny);
	for (size_t i = 0; i < ijz->mem.used; ++i) {
		// note that we add 1 - zero means missing
		(*index)[ijz2index(ijz->ijz[i], bl, tr)] = i+1;
	}
	finally:return status;
}

static inline ludta_ij upleft(ludta_ijz p) {
	return (ludta_ij){p.i-1, p.j+1};
}

static inline ludta_ij upright(ludta_ijz p) {
	return (ludta_ij){p.i, p.j+1};
}

static inline ludta_ij right(ludta_ijz p) {
	return (ludta_ij){p.i+1, p.j};
}

static inline ludta_ij downright(ludta_ijz p) {
	return (ludta_ij){p.i+1, p.j-1};
}

static int addpoints(lulog *log, luary_ijz *ijz, size_t *current,
		ludta_ijz *pprev, int nextisup,
		size_t *index, ludta_ij bl, ludta_ij tr,
		luary_uint32 *indices, luary_uint32 *counts) {
	int status = LU_OK;
	do {
		size_t next = index[ij2index((nextisup ? upright : downright)(*pprev), bl, tr)];
		ludta_ijz *pnext = next ? &ijz->ijz[next-1] : NULL;
		if (pnext) {
			if (!nextisup) {
				// we're walking geometrically (in i,j space) across the
				// points, but we need to keep the traversal of ijz (via
				// the current index) in step so we can restart correctly
				// for the next strip.  this is only possible if the points
				// are sorted (which enumerate should guarantee).
				assert(ijeq(right(ijz->ijz[*current]), ijz2ij(ijz->ijz[*current+1])), LU_ERR,
						log, "Unsorted points?  Current at (%d,%d), next at (%d,%d), prev at (%d,%d), downright at (%d,%d)",
						ijz->ijz[*current].i, ijz->ijz[*current].j,
						ijz->ijz[*current+1].i, ijz->ijz[*current+1].j,
						pprev->i, pprev->j, pnext->i, pnext->j)
                														*current = *current + 1;
			}
			try(luary_uint32_push(log, indices, next-1)); // correct for 0/NULL
			counts->i[counts->mem.used-1]++;
			pprev = pnext; nextisup = !nextisup;
		} else {
			*current = *current + 1;
			goto finally;
		}
	} while (*current < ijz->mem.used);
	finally:
	return status;
}

static int addstrip(lulog *log, luary_ijz *ijz, size_t *current, size_t *index,
		ludta_ij bl, ludta_ij tr,
		luary_uint32 *indices, luary_uint32 *offsets, luary_uint32 *counts) {
	int status = LU_OK;
	ludta_ijz *p1 = &ijz->ijz[*current];
	size_t i0 = index[ij2index(upleft(*p1), bl, tr)];
	ludta_ijz *p0 = i0 ? &ijz->ijz[i0-1] : NULL;
	size_t i2 = index[ij2index(upright(*p1), bl, tr)];
	ludta_ijz *p2 = i2 ? &ijz->ijz[i2-1] : NULL;
	// if at least three points exist, add the first two and then add the rest
	if (p0 && p2) {
		// here the initial triangle looks like \/ and (l-r) is CCW.
		// if we want CW front face then we need to add an extra point
		// at the start (a degenerate triangle).
		try(luary_uint32_push(log, offsets, indices->mem.used));
		try(luary_uint32_push(log, indices, i0-1)); // correct for 0/NULL
		try(luary_uint32_push(log, indices, i0-1)); // correct for 0/NULL
		try(luary_uint32_push(log, indices, *current));
		try(luary_uint32_push(log, counts, 3));
		try(addpoints(log, ijz, current, p1, 1, index, bl, tr, indices, counts));
	} else {
		// here the initial triangle looks like /\ and (l-r) is CW.
		size_t i3 = index[ij2index(right(*p1), bl, tr)];
		ludta_ijz *p3 = i3 ? &ijz->ijz[i3-1] : NULL;
		if (p3) {
			assert(*current + 1 < ijz->mem.used, LU_ERR, log,
					"Unsorted points?  No data at %zu", *current + 1);
			assert(p3 == &ijz->ijz[*current + 1], LU_ERR, log,
					"Unsorted points?  p3=(%d,%d), p1=(%d,%d), next=(%d,%d)",
					p3->i, p3->j, p1->i, p1->j, ijz->ijz[*current + 1].i, ijz->ijz[*current + 1].j);
		}
		if (p2 && p3) {
			try(luary_uint32_push(log, offsets, indices->mem.used));
			try(luary_uint32_push(log, indices, *current));
			try(luary_uint32_push(log, indices, i2 - 1)); // correct for 0/NULL
			try(luary_uint32_push(log, counts, 2));
			try(addpoints(log, ijz, current, p2, 0, index, bl, tr, indices, counts));
		} else {
			*current = *current+1;
		}
	}
	finally:
	return status;
}

int strips(lulog *log, luary_ijz *ijz,
		luary_uint32 **indices, luary_uint32 **offsets, luary_uint32 **counts){
	int status = LU_OK;
	ludta_ij bl, tr;
	size_t *index = NULL;
	try(lutle_range(log, ijz, &bl, &tr, NULL));
	bl.i--; bl.j--; tr.i++; tr.j++;  // add border for failed lookups
	try(index_mk(log, ijz, bl, tr, &index));
	try(luary_uint32_mk(log, indices, 4 * ijz->mem.used));  // guess some overhead
	try(luary_uint32_mk(log, offsets, tr.j - bl.j + 1));  // optimistic?
	try(luary_uint32_mk(log, counts, tr.j - bl.j + 1));  // optimistic?
	size_t current = 0;
	while (current < ijz->mem.used) {
		try(addstrip(log, ijz, &current, index, bl, tr, *indices, *offsets, *counts));
	}
	luinfo(log, "Generated %zu triangle strips", (*offsets)->mem.used);
	finally:
	free(index);
	return status;
}

int ijz2fxyzw(lulog *log, luary_ijz *ijz, float step, luary_fxyzw **fxyzw) {
	int status = LU_OK;
	try(luary_fxyzw_mk(log, fxyzw, ijz->mem.used));
	for (size_t i = 0; i < ijz->mem.used; ++i) {
		ludta_ijz *p = &ijz->ijz[i];
		float x = (p->i + p->j * cos(M_PI/3)) * step;
		float y = p->j * sin(M_PI/3) * step;
		float z = p->z;
		try(luary_fxyzw_push(log, *fxyzw, x, y, z, 1.0f));
	}
	finally:
	return status;
}

int ijz2vecf4(lulog *log, luary_ijz *ijz, float step, luary_vecf4 **f4) {
	int status = LU_OK;
	try(luary_vecf4_mk(log, f4, ijz->mem.used));
	float lo[] = {0, 0, ijz->ijz[0].z}, hi[] = {0, 0, ijz->ijz[0].z};
	for (size_t i = 0; i < ijz->mem.used; ++i) {
		ludta_ijz *p = &ijz->ijz[i];
		float x = (p->i + p->j * cos(M_PI/3)) * step;
		hi[0] = max(hi[0], x); lo[0] = min(lo[0], x);
		float y = p->j * sin(M_PI/3) * step;
		hi[1] = max(hi[1], y); lo[1] = min(lo[1], y);
		float z = p->z;
		hi[2] = max(hi[2], z); lo[2] = min(lo[2], z);
		try(luary_vecf4_push(log, *f4, x, y, z, 1.0f));
	}
	ludebug(log, "Data cover range %0.2f - %0.2f, %0.2f - %0.2f, %0.2f - %0.2f",
			lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]);
	finally:
	return status;
}

int offsets2void(lulog *log, luary_uint32 *in, size_t chunk, luary_void **out) {
	int status = LU_OK;
	try(luary_void_mk(log, out, in->mem.used));
	for (size_t i = 0; i < in->mem.used; ++i) {
		try(luary_void_push(log, *out, (void*)(chunk * in->i[i])));
	}
	finally:return status;
}

int normalize_z(lulog *log, luary_ijz *vertices) {
	int status = LU_OK;
	float zmax = vertices->ijz[0].z, zmin = zmax;
	for (size_t i = 0; i < vertices->mem.used; ++i) {
		zmax = max(zmax, vertices->ijz[i].z);
		zmin = min(zmin, vertices->ijz[i].z);
	}
	if (zmax > zmin) {
		for (size_t i = 0; i < vertices->mem.used; ++i) {
			vertices->ijz[i].z = 2 * ((vertices->ijz[i].z - zmin) / (zmax - zmin) - 0.5);
		}
	} else {
		luwarn(log, "Cannot normalize z data: constant values (setting to zero)");
		for (size_t i = 0; i < vertices->mem.used; ++i) {
			vertices->ijz[i].z = 0.0;
		}
	}
	finally:return status;
}

// this duplicates points so that none are shared between two triangle
// strips.  this is necessary so that we can have a single normal for
// each triangle (with no interpolation).
int uniquify(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
		luary_uint32 *counts, luary_ijz *vertices) {
	int status = LU_OK;
	size_t before = vertices->mem.used;
	for (size_t i = 0; i < offsets->mem.used; ++i) {
		size_t large = max(indices->i[offsets->i[i]], indices->i[offsets->i[i]+1]);
		for (size_t j = 0; j < counts->i[i]; ++j) {
			size_t k = offsets->i[i] + j;
			if (indices->i[k] >= large) {
				ludta_ijz v = vertices->ijz[indices->i[k]];
				indices->i[k] = vertices->mem.used;
				try(luary_ijz_push(log, vertices, v.i, v.j, v.z));
			}
		}
	}
	ludebug(log, "Uniquify increased vertex count from %zu to %zu (%.0f%%)",
			before, vertices->mem.used, 100.0 * (vertices->mem.used - before) / before);
	finally:
	return status;
}

int normals(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
		luary_uint32 *counts, luary_vecf4 *vertices, luary_vnorm **vnorms) {
	int status = LU_OK;
	try(luary_vnorm_mk(log, vnorms, vertices->mem.used));
	for (size_t i = 0; i < offsets->mem.used; ++i) {
		size_t offset = offsets->i[i];
		for (size_t j = 0; j < counts->i[i]; ++j) {
			size_t k = offset + j;
			luglv n = {};
			luglv *p0 = &vertices->v[indices->i[k]];
			if (j > 1) {
				luglv *p1 = &vertices->v[indices->i[k-1]];
				luglv *p2 = &vertices->v[indices->i[k-2]];
				luglv e1 = {}, e2 = {};
				luglv_sub(p1, p0, &e1);
				luglv_sub(p2, p1, &e2);
				luglv_cross(&e1, &e2, &n);
				luglv_norm_inplace(&n);
				// alternate windows are reversed in strip
				if (j % 2) luglv_scale_inplace(-1, &n);
				n[3] = 0;
			}
			assert(k == (*vnorms)->mem.used, HP_ERR, log, "Vertex gap (%zu/%zu)", k, (*vnorms)->mem.used);
			try(luary_vnorm_push(log, *vnorms, p0, &n));
		}
	}
	finally:
	return status;
}


int uint2int(lulog *log, luary_uint32 *in, luary_int32 **out) {
	int status = LU_OK;
	try(luary_int32_mk(log, out, in->mem.used));
	for (size_t i = 0; i < in->mem.used; ++i) {
		try(luary_int32_push(log, *out, in->i[i]));
	}
	finally:
	return status;
}

