
#include <math.h>

#include "lu/status.h"
#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/structs.h"
#include "lu/vectors.h"

#include "tiles.h"
#include "vertices.h"
#include "error_codes.h"


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

static int mkindex(lulog *log, luary_ijz *ijz, ludta_ij bl, ludta_ij tr,
        size_t **index) {
    LU_STATUS
    size_t nx = tr.i - bl.i + 1, ny = tr.j - bl.j + 1;
    LU_ALLOC(log, *index, nx*ny)
    for (size_t i = 0; i < ijz->mem.used; ++i) {
        // note that we add 1 - zero means missing
        (*index)[ijz2index(ijz->ijz[i], bl, tr)] = i+1;
    }
    LU_NO_CLEANUP
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
    LU_STATUS
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
                LU_ASSERT(ijeq(right(ijz->ijz[*current]), ijz2ij(ijz->ijz[*current+1])), LU_ERR,
                        log, "Unsorted points?  Current at (%d,%d), next at (%d,%d), prev at (%d,%d), downright at (%d,%d)",
                        ijz->ijz[*current].i, ijz->ijz[*current].j,
                        ijz->ijz[*current+1].i, ijz->ijz[*current+1].j,
                        pprev->i, pprev->j, pnext->i, pnext->j)
                *current = *current + 1;
            }
            LU_CHECK(luary_pushuint32(log, indices, next-1)) // correct for 0/NULL
            counts->i[counts->mem.used-1]++;
            pprev = pnext; nextisup = !nextisup;
        } else {
            *current = *current + 1;
            goto exit;
        }
    } while (*current < ijz->mem.used);
    LU_NO_CLEANUP
}

static int addstrip(lulog *log, luary_ijz *ijz, size_t *current, size_t *index,
        ludta_ij bl, ludta_ij tr,
		luary_uint32 *indices, luary_uint32 *offsets, luary_uint32 *counts) {
    LU_STATUS
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
        LU_CHECK(luary_pushuint32(log, offsets, indices->mem.used))
        LU_CHECK(luary_pushuint32(log, indices, i0-1)) // correct for 0/NULL
        LU_CHECK(luary_pushuint32(log, indices, i0-1)) // correct for 0/NULL
        LU_CHECK(luary_pushuint32(log, indices, *current))
        LU_CHECK(luary_pushuint32(log, counts, 3))
        LU_CHECK(addpoints(log, ijz, current, p1, 1, index, bl, tr, indices, counts));
    } else {
        // here the initial triangle looks like /\ and (l-r) is CW.
        size_t i3 = index[ij2index(right(*p1), bl, tr)];
        ludta_ijz *p3 = i3 ? &ijz->ijz[i3-1] : NULL;
        if (p3) {
            LU_ASSERT(*current + 1 < ijz->mem.used, LU_ERR, log,
                    "Unsorted points?  No data at %zu", *current + 1);
            LU_ASSERT(p3 == &ijz->ijz[*current + 1], LU_ERR, log,
                    "Unsorted points?  p3=(%d,%d), p1=(%d,%d), next=(%d,%d)",
                    p3->i, p3->j, p1->i, p1->j, ijz->ijz[*current + 1].i, ijz->ijz[*current + 1].j)
        }
        if (p2 && p3) {
            LU_CHECK(luary_pushuint32(log, offsets, indices->mem.used))
            LU_CHECK(luary_pushuint32(log, indices, *current))
            LU_CHECK(luary_pushuint32(log, indices, i2 - 1)) // correct for 0/NULL
            LU_CHECK(luary_pushuint32(log, counts, 2))
            LU_CHECK(addpoints(log, ijz, current, p2, 0, index, bl, tr, indices, counts));
        } else {
            *current = *current+1;
        }
    }
    LU_NO_CLEANUP
}

static int strips(lulog *log, luary_ijz *ijz,
		luary_uint32 **indices, luary_uint32 **offsets, luary_uint32 **counts){
    LU_STATUS
    ludta_ij bl, tr;
    size_t *index = NULL;
    LU_CHECK(lutle_range(log, ijz, &bl, &tr, NULL))
    bl.i--; bl.j--; tr.i++; tr.j++;  // add border for failed lookups
    LU_CHECK(mkindex(log, ijz, bl, tr, &index))
    LU_CHECK(luary_mkuint32n(log, indices, 4 * ijz->mem.used))  // guess some overhead
    LU_CHECK(luary_mkuint32n(log, offsets, tr.j - bl.j + 1))  // optimistic?
    LU_CHECK(luary_mkuint32n(log, counts, tr.j - bl.j + 1))  // optimistic?
    size_t current = 0;
    while (current < ijz->mem.used) {
        LU_CHECK(addstrip(log, ijz, &current, index, bl, tr, *indices, *offsets, *counts))
    }
    luinfo(log, "Generated %zu triangle strips", (*offsets)->mem.used);
LU_CLEANUP
    free(index);
    LU_RETURN
}

static int ijz2fxyzw(lulog *log, luary_ijz *ijz, float step, luary_fxyzw **fxyzw) {
    LU_STATUS
    LU_CHECK(luary_mkfxyzwn(log, fxyzw, ijz->mem.used))
    for (size_t i = 0; i < ijz->mem.used; ++i) {
        ludta_ijz *p = &ijz->ijz[i];
        float x = (p->i + p->j * cos(M_PI/3)) * step;
        float y = p->j * sin(M_PI/3) * step;
        float z = p->z;
        LU_CHECK(luary_pushfxyzw(log, *fxyzw, x, y, z, 1.0f))
    }
    LU_NO_CLEANUP
}

static int ijz2vecf4(lulog *log, luary_ijz *ijz, float step, luary_vecf4 **f4) {
    LU_STATUS
    LU_CHECK(luary_mkvecf4n(log, f4, ijz->mem.used))
    float lo[] = {0, 0, ijz->ijz[0].z}, hi[] = {0, 0, ijz->ijz[0].z};
    for (size_t i = 0; i < ijz->mem.used; ++i) {
        ludta_ijz *p = &ijz->ijz[i];
        float x = (p->i + p->j * cos(M_PI/3)) * step;
        hi[0] = max(hi[0], x); lo[0] = min(lo[0], x);
        float y = p->j * sin(M_PI/3) * step;
        hi[1] = max(hi[1], y); lo[1] = min(lo[1], y);
        float z = p->z;
        hi[2] = max(hi[2], z); lo[2] = min(lo[2], z);
        LU_CHECK(luary_pushvecf4(log, *f4, x, y, z, 1.0f))
    }
    ludebug(log, "Data cover range %0.2f - %0.2f, %0.2f - %0.2f, %0.2f - %0.2f",
            lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]);
    LU_NO_CLEANUP
}

static int offsets2void(lulog *log, luary_uint32 *in, size_t chunk, luary_void **out) {
    LU_STATUS
    LU_CHECK(luary_mkvoidn(log, out, in->mem.used))
    for (size_t i = 0; i < in->mem.used; ++i) {
        LU_CHECK(luary_pushvoid(log, *out, (void*)(chunk * in->i[i])))
    }
    LU_NO_CLEANUP
}

static int fixz(lulog *log, luary_ijz *vertices) {
    LU_STATUS
    float zmax = vertices->ijz[0].z, zmin = zmax;
    for (size_t i = 0; i < vertices->mem.used; ++i) {
        zmax = max(zmax, vertices->ijz[i].z);
        zmin = min(zmin, vertices->ijz[i].z);
    }
    if (zmax > zmin) {
        for (size_t i = 0; i < vertices->mem.used; ++i) {
            // -1 to 1
            vertices->ijz[i].z = 2 * ((vertices->ijz[i].z - zmin) / (zmax - zmin) - 0.5);
        }
    } else {
        luwarn(log, "Cannot normalize z data: constant values (setting to zero)");
        for (size_t i = 0; i < vertices->mem.used; ++i) {
            vertices->ijz[i].z = 0.0;
        }
    }
    LU_NO_CLEANUP
}


static int hexagon_common(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_ijz **vertices, luary_uint32 **indices,
		luary_uint32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    lutle_config *config = NULL;
    lutle_tile *hexagon = NULL;
    LU_CHECK(lutle_defaultconfig(log, &config, seed))
    LU_CHECK(lutle_mkhexagon(log, &hexagon, side, subsamples, octweight))
    LU_CHECK(hexagon->enumerate(hexagon, log, config, -1, vertices))
    LU_CHECK(fixz(log, *vertices));
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
    LU_CHECK(hexagon_common(log, seed, side, subsamples, step, octweight,
            &ijz, indices, &ioffsets, counts))
    LU_CHECK(ijz2fxyzw(log, ijz, step, vertices))
    LU_CHECK(offsets2void(log, ioffsets, sizeof(*(*indices)->i), offsets))
LU_CLEANUP
    status = luary_freeuint32(&ioffsets, status);
    status = luary_freeijz(&ijz, status);
    LU_RETURN
}

// this duplicates points so that none are shared between two triangle
// strips.  this is necessary so that we can have a single normal for
// each triangle (with no interpolation).
static int uniquify(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
        luary_uint32 *counts, luary_ijz *vertices) {
    LU_STATUS
    size_t before = vertices->mem.used;
    for (size_t i = 0; i < offsets->mem.used; ++i) {
        size_t large = max(indices->i[offsets->i[i]], indices->i[offsets->i[i]+1]);
        for (size_t j = 0; j < counts->i[i]; ++j) {
            size_t k = offsets->i[i] + j;
            if (indices->i[k] >= large) {
                ludta_ijz v = vertices->ijz[indices->i[k]];
                indices->i[k] = vertices->mem.used;
                LU_CHECK(luary_pushijz(log, vertices, v.i, v.j, v.z))
            }
        }
    }
    ludebug(log, "Uniquify increased vertex count from %zu to %zu (%.0f%%)",
            before, vertices->mem.used, 100.0 * (vertices->mem.used - before) / before);
    LU_NO_CLEANUP
}

static int normals(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
        luary_uint32 *counts, luary_vecf4 *vertices, luary_vnorm **vnorms) {
    LU_STATUS
    LU_CHECK(luary_mkvnormn(log, vnorms, vertices->mem.used))
    for (size_t i = 0; i < offsets->mem.used; ++i) {
        for (size_t j = 0; j < counts->i[i]; ++j) {
            size_t k = offsets->i[i] + j;
            luvec_f4 n = {};
            luvec_f4 *p0 = &vertices->v[indices->i[k]];
            if (j > 1) {
                luvec_f4 *p1 = &vertices->v[indices->i[k-1]];
                luvec_f4 *p2 = &vertices->v[indices->i[k-2]];
                if ((*p1)[1] < (*p2)[1]) {luvec_f4 *tmp = p1; p1 = p2; p2 = tmp;}
                luvec_f4 e1 = {}, e2 = {};
                luvec_subf4_3(p1, p0, &e1);
                luvec_subf4_3(p2, p0, &e2);
                luvec_crsf4_3(&e1, &e2, &n);
                n[3] = 0;
            }
            LU_ASSERT(k == (*vnorms)->mem.used, HP_ERR, log, "Vertex gap (%zu/%zu)", k, (*vnorms)->mem.used)
            LU_CHECK(luary_pushvnorm(log, *vnorms, p0, &n))
        }
    }
    LU_NO_CLEANUP
}


static int uint2int(lulog *log, luary_uint32 *in, luary_int32 **out) {
    LU_STATUS
    LU_CHECK(luary_mkint32n(log, out, in->mem.used))
    for (size_t i = 0; i < in->mem.used; ++i) {
        LU_CHECK(luary_pushint32(log, *out, in->i[i]))
    }
    LU_NO_CLEANUP
}

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luary_vnorm **vertices, luary_int32 **offsets, luary_uint32 **counts) {
    LU_STATUS
    luary_ijz *ijz = NULL;
    luary_uint32 *ioffsets = NULL, *indices = NULL;
    luary_vecf4 *f4 = NULL;
    LU_CHECK(hexagon_common(log, seed, side, subsamples, step, octweight,
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


