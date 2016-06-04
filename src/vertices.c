
#include <string.h>

#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/structs.h"
#include "lu/log.h"

#include "vertices.h"

LUARY_MKBASE(vnorm, luary_vnorm, vnorm, v)

int luary_pushvnorm(lulog *log, luary_vnorm *vn, ludta_fxyzw v, ludta_fxyzw n) {
    LU_STATUS
    LU_CHECK(luary_reservevnorm(log, vn, 1))
    vn->v[vn->mem.used++] = (vnorm){v, n};
    LU_NO_CLEANUP
}

LUARY_MKDUMP(luary_dumpvnorm, luary_vnorm, "{%g,%g,%g,%g}/{%g,%g,%g,%g}",
        ptr->v[i].v.x, ptr->v[i].v.y, ptr->v[i].v.z, ptr->v[i].v.w,
        ptr->v[i].n.x, ptr->v[i].n.y, ptr->v[i].n.z, ptr->v[i].n.w)
