
#include <math.h>

#include "lu/status.h"
#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/structs.h"

#include "tiles.h"
#include "vertices.h"
#include "error_codes.h"


static int ijeq(ludata_ij ij1, ludata_ij ij2) {
    return ij1.i == ij2.i && ij1.j == ij2.j;
}

static inline ludata_ij ijz2ij(ludata_ijz ijz) {
    return (ludata_ij){ijz.i, ijz.j};
}

static inline int ij2index(ludata_ij ij, ludata_ij bl, ludata_ij tr) {
    return ij.i - bl.i + (ij.j - bl.j) * (tr.i - bl.i + 1);
}

static inline int ijz2index(ludata_ijz ijz, ludata_ij bl, ludata_ij tr) {
    return ij2index(ijz2ij(ijz), bl, tr);
}

static int mkindex(lulog *log, luarray_ijz *ijz, ludata_ij bl, ludata_ij tr,
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

static inline ludata_ij upleft(ludata_ijz p) {
    return (ludata_ij){p.i-1, p.j+1};
}

static inline ludata_ij upright(ludata_ijz p) {
    return (ludata_ij){p.i, p.j+1};
}

static inline ludata_ij right(ludata_ijz p) {
    return (ludata_ij){p.i+1, p.j};
}

static inline ludata_ij downright(ludata_ijz p) {
    return (ludata_ij){p.i+1, p.j-1};
}

static int addpoints(lulog *log, luarray_ijz *ijz, size_t *current,
        ludata_ijz *pprev, int nextisup,
        size_t *index, ludata_ij bl, ludata_ij tr,
        luarray_uint32 *indices, luarray_uint32 *counts) {
    LU_STATUS
    do {
        size_t next = index[ij2index((nextisup ? upright : downright)(*pprev), bl, tr)];
        ludata_ijz *pnext = next ? &ijz->ijz[next-1] : NULL;
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
            LU_CHECK(luarray_pushuint32(log, indices, next-1)) // correct for 0/NULL
            counts->i[counts->mem.used-1]++;
            pprev = pnext; nextisup = !nextisup;
        } else {
            *current = *current + 1;
            goto exit;
        }
    } while (*current < ijz->mem.used);
    LU_NO_CLEANUP
}

static int addstrip(lulog *log, luarray_ijz *ijz, size_t *current, size_t *index,
        ludata_ij bl, ludata_ij tr,
		luarray_uint32 *indices, luarray_uint32 *offsets, luarray_uint32 *counts) {
    LU_STATUS
    ludata_ijz *p1 = &ijz->ijz[*current];
    size_t i0 = index[ij2index(upleft(*p1), bl, tr)];
    ludata_ijz *p0 = i0 ? &ijz->ijz[i0-1] : NULL;
    size_t i2 = index[ij2index(upright(*p1), bl, tr)];
    ludata_ijz *p2 = i2 ? &ijz->ijz[i2-1] : NULL;
    // if at least three points exist, add the first two and then add the rest
    if (p0 && p2) {
        LU_CHECK(luarray_pushuint32(log, offsets, indices->mem.used))
        LU_CHECK(luarray_pushuint32(log, indices, i0-1)) // correct for 0/NULL
        LU_CHECK(luarray_pushuint32(log, indices, *current))
        LU_CHECK(luarray_pushuint32(log, counts, 2))
        LU_CHECK(addpoints(log, ijz, current, p1, 1, index, bl, tr, indices, counts));
    } else {
        size_t i3 = index[ij2index(right(*p1), bl, tr)];
        ludata_ijz *p3 = i3 ? &ijz->ijz[i3-1] : NULL;
        if (p3) {
            LU_ASSERT(*current + 1 < ijz->mem.used, LU_ERR, log,
                    "Unsorted points?  No data at %zu", *current + 1);
            LU_ASSERT(p3 == &ijz->ijz[*current + 1], LU_ERR, log,
                    "Unsorted points?  p3=(%d,%d), p1=(%d,%d), next=(%d,%d)",
                    p3->i, p3->j, p1->i, p1->j, ijz->ijz[*current + 1].i, ijz->ijz[*current + 1].j)
        }
        if (p2 && p3) {
            LU_CHECK(luarray_pushuint32(log, offsets, indices->mem.used))
            LU_CHECK(luarray_pushuint32(log, indices, *current))
            LU_CHECK(luarray_pushuint32(log, indices, i2 - 1)) // correct for 0/NULL
            LU_CHECK(luarray_pushuint32(log, counts, 2))
            LU_CHECK(addpoints(log, ijz, current, p2, 0, index, bl, tr, indices, counts));
        } else {
            *current = *current+1;
        }
    }
    LU_NO_CLEANUP
}

static int strips(lulog *log, luarray_ijz *ijz,
		luarray_uint32 **indices, luarray_uint32 **offsets, luarray_uint32 **counts){
    LU_STATUS
    ludata_ij bl, tr;
    size_t *index = NULL;
    LU_CHECK(lutile_range(log, ijz, &bl, &tr, NULL))
    bl.i--; bl.j--; tr.i++; tr.j++;  // add border for failed lookups
    LU_CHECK(mkindex(log, ijz, bl, tr, &index))
    LU_CHECK(luarray_mkuint32n(log, indices, 4 * ijz->mem.used))  // guess some overhead
    LU_CHECK(luarray_mkuint32n(log, offsets, tr.j - bl.j + 1))  // optimistic?
    LU_CHECK(luarray_mkuint32n(log, counts, tr.j - bl.j + 1))  // optimistic?
    size_t current = 0;
    while (current < ijz->mem.used) {
        LU_CHECK(addstrip(log, ijz, &current, index, bl, tr, *indices, *offsets, *counts))
    }
    luinfo(log, "Generated %zu triangle strips", (*offsets)->mem.used - 1);
LU_CLEANUP
    free(index);
    LU_RETURN
}

static int ijz2fxyzw(lulog *log, luarray_ijz *ijz, float step, luarray_fxyzw **fxyzw) {
    LU_STATUS
    LU_CHECK(luarray_mkfxyzwn(log, fxyzw, ijz->mem.used))
    for (size_t i = 0; i < ijz->mem.used; ++i) {
        ludata_ijz *p = &ijz->ijz[i];
        float x = (p->i + p->j * cos(M_PI/3)) * step;
        float y = p->j * sin(M_PI/3) * step;
        float z = p->z;
        LU_CHECK(luarray_pushfxyzw(log, *fxyzw, x, y, z, 1.0f))
    }
    LU_NO_CLEANUP
}

static int offsets2void(lulog *log, luarray_uint32 *in, size_t chunk, luarray_void **out) {
    LU_STATUS
    LU_CHECK(luarray_mkvoidn(log, out, in->mem.used))
    for (size_t i = 0; i < in->mem.used; ++i) {
        LU_CHECK(luarray_pushvoid(log, *out, (void*)(chunk * in->i[i])))
    }
    LU_NO_CLEANUP
}


static int hexagon_common(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_ijz **vertices, luarray_uint32 **indices,
		luarray_uint32 **offsets, luarray_uint32 **counts) {
    LU_STATUS
    lutile_config *config = NULL;
    lutile_tile *hexagon = NULL;
    LU_CHECK(lutile_defaultconfig(log, &config, seed))
    LU_CHECK(lutile_mkhexagon(log, &hexagon, side, subsamples, octweight))
    LU_CHECK(hexagon->enumerate(hexagon, log, config, -1, vertices))
    LU_CHECK(strips(log, *vertices, indices, offsets, counts))
LU_CLEANUP
    status = lutile_freeconfig(&config, status);
    if (hexagon) status = hexagon->free(&hexagon, status);
    LU_RETURN
}


int hexagon_vertex_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_fxyzw **vertices, luarray_uint32 **indices,
        luarray_void **offsets, luarray_uint32 **counts) {
    LU_STATUS
    luarray_ijz *ijz = NULL;
    luarray_uint32 *ioffsets = NULL;
    LU_CHECK(hexagon_common(log, seed, side, subsamples, step, octweight,
            &ijz, indices, &ioffsets, counts))
    LU_CHECK(ijz2fxyzw(log, ijz, step, vertices))
    LU_CHECK(offsets2void(log, ioffsets, sizeof(*(*indices)->i), offsets))
LU_CLEANUP
    status = luarray_freeuint32(&ioffsets, status);
    status = luarray_freeijz(&ijz, status);
    LU_RETURN
}

static int uniquify(lulog *log, luarray_uint32 *indices, luarray_uint32 *offsets,
        luarray_uint32 *counts, luarray_ijz *vertices) {
    LU_STATUS
    for (size_t i = 0; i < offsets->mem.used; ++i) {
        size_t large = max(indices->i[offsets->i[i]], indices->i[offsets->i[i]+1]);
        for (size_t j = 2; j < counts->i[i]; ++j) {
            size_t k = offsets->i[i] + j;
            if (indices->i[k] > large) {
                indices->i[k] = vertices->mem.used;
                ludata_ijz v = vertices->ijz[indices->i[k]];
                LU_CHECK(luarray_pushijz(log, vertices, v.i, v.j, v.z))
            }
        }
    }
    LU_NO_CLEANUP
}

static int normals(lulog *log, luarray_uint32 *indices, luarray_uint32 *offsets,
        luarray_uint32 *counts, luarray_fxyzw *vertices, luarray_vnorm **vnorms) {
    LU_STATUS
    LU_CHECK(luarray_mkvnormn(log, vnorms, vertices->mem.used))
    for (size_t i = 0; i < offsets->mem.used; ++i) {
        for (size_t j = 0; j < counts->i[i]; ++j) {
            size_t k = offsets->i[i] + j;
            ludata_fxyzw n = {};
            if (j > 1) {
                // calculate normal
            }
            LU_CHECK(luarray_pushvnorm(log, *vnorms, vertices->fxyzw[k], n))
            LU_ASSERT(k == (*vnorms)->mem.used, HP_ERR, log, "Vertex gap")
        }
    }
    LU_NO_CLEANUP
}

int hexagon_vnormal_strips(lulog *log, uint64_t seed,
        size_t side, size_t subsamples, double step, double octweight,
        luarray_vnorm **vertices, luarray_uint32 **indices,
        luarray_void **offsets, luarray_uint32 **counts) {
    LU_STATUS
    luarray_ijz *ijz = NULL;
    luarray_uint32 *ioffsets = NULL;
    luarray_fxyzw *fxyzw = NULL;
    LU_CHECK(hexagon_common(log, seed, side, subsamples, step, octweight,
            &ijz, indices, &ioffsets, counts))
    LU_CHECK(uniquify(log, *indices, ioffsets, *counts, ijz))
    LU_CHECK(ijz2fxyzw(log, ijz, step, &fxyzw))
    LU_CHECK(normals(log, *indices, ioffsets, *counts, fxyzw, vertices))
    LU_CHECK(offsets2void(log, ioffsets, sizeof(*(*indices)->i), offsets))
LU_CLEANUP
    status = luarray_freeuint32(&ioffsets, status);
    status = luarray_freeijz(&ijz, status);
    status = luarray_freefxyzw(&fxyzw, status);
    LU_RETURN
}


