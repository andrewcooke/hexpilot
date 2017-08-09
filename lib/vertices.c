
#include <math.h>
#include <status_codes.h>
#include <string.h>

#include "lu/status.h"
#include "lu/tiles.h"
#include "lu/random.h"
#include "lu/arrays.h"
#include "lu/minmax.h"
#include "lu/structs.h"
#include "lu/gl.h"
#include "lu/array_macros.h"
#include "lu/log.h"

#include "vertices.h"


LUARY_MKBASE(vnorm, vnorm, vn)

int luary_pushvnorm(lulog *log, luary_vnorm *vn, luglv *v, luglv *n) {
    LU_STATUS
    LU_CHECK(luary_reservevnorm(log, vn, 1))
    for (size_t i = 0; i < 4; ++i) {
        vn->vn[vn->mem.used][0][i] = (*v)[i];
        vn->vn[vn->mem.used][1][i] = (*n)[i];
    }
    vn->mem.used++;
    LU_NO_CLEANUP
}

static char bv[100];
static char bn[100];
LUARY_MKDUMP(luary_dumpvnorm, luary_vnorm, "%s/%s",
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

int strips(lulog *log, luary_ijz *ijz,
        luary_uint32 **indices, luary_uint32 **offsets, luary_uint32 **counts){
    LU_STATUS
    ludta_ij bl, tr;
    size_t *index = NULL;
    LU_CHECK(lutle_range(log, ijz, &bl, &tr, NULL))
    bl.i--; bl.j--; tr.i++; tr.j++;  // add border for failed lookups
    LU_CHECK(mkindex(log, ijz, bl, tr, &index))
    LU_CHECK(luary_mkuint32(log, indices, 4 * ijz->mem.used))  // guess some overhead
    LU_CHECK(luary_mkuint32(log, offsets, tr.j - bl.j + 1))  // optimistic?
    LU_CHECK(luary_mkuint32(log, counts, tr.j - bl.j + 1))  // optimistic?
    size_t current = 0;
    while (current < ijz->mem.used) {
        LU_CHECK(addstrip(log, ijz, &current, index, bl, tr, *indices, *offsets, *counts))
    }
    luinfo(log, "Generated %zu triangle strips", (*offsets)->mem.used);
LU_CLEANUP
    free(index);
    LU_RETURN
}

int ijz2fxyzw(lulog *log, luary_ijz *ijz, float step, luary_fxyzw **fxyzw) {
    LU_STATUS
    LU_CHECK(luary_mkfxyzw(log, fxyzw, ijz->mem.used))
    for (size_t i = 0; i < ijz->mem.used; ++i) {
        ludta_ijz *p = &ijz->ijz[i];
        float x = (p->i + p->j * cos(M_PI/3)) * step;
        float y = p->j * sin(M_PI/3) * step;
        float z = p->z;
        LU_CHECK(luary_pushfxyzw(log, *fxyzw, x, y, z, 1.0f))
    }
    LU_NO_CLEANUP
}

int ijz2vecf4(lulog *log, luary_ijz *ijz, float step, luary_vecf4 **f4) {
    LU_STATUS
    LU_CHECK(luary_mkvecf4(log, f4, ijz->mem.used))
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

int offsets2void(lulog *log, luary_uint32 *in, size_t chunk, luary_void **out) {
    LU_STATUS
    LU_CHECK(luary_mkvoid(log, out, in->mem.used))
    for (size_t i = 0; i < in->mem.used; ++i) {
        LU_CHECK(luary_pushvoid(log, *out, (void*)(chunk * in->i[i])))
    }
    LU_NO_CLEANUP
}

int normalize_z(lulog *log, luary_ijz *vertices) {
    LU_STATUS
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
    LU_NO_CLEANUP
}

// this duplicates points so that none are shared between two triangle
// strips.  this is necessary so that we can have a single normal for
// each triangle (with no interpolation).
int uniquify(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
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

int normals(lulog *log, luary_uint32 *indices, luary_uint32 *offsets,
        luary_uint32 *counts, luary_vecf4 *vertices, luary_vnorm **vnorms) {
    LU_STATUS
    LU_CHECK(luary_mkvnorm(log, vnorms, vertices->mem.used))
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
            LU_ASSERT(k == (*vnorms)->mem.used, HP_ERR, log, "Vertex gap (%zu/%zu)", k, (*vnorms)->mem.used)
            LU_CHECK(luary_pushvnorm(log, *vnorms, p0, &n))
        }
    }
    LU_NO_CLEANUP
}


int uint2int(lulog *log, luary_uint32 *in, luary_int32 **out) {
    LU_STATUS
    LU_CHECK(luary_mkint32(log, out, in->mem.used))
    for (size_t i = 0; i < in->mem.used; ++i) {
        LU_CHECK(luary_pushint32(log, *out, in->i[i]))
    }
    LU_NO_CLEANUP
}

