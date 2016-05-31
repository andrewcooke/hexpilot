
#include <string.h>

#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/log.h"

#include "vertices.h"

LUARRAY_MKBASE(vnorm, luarray_vnorm, vnorm, v)

int luarray_pushvnorm(lulog *log, luarray_vnorm *vn, ludata_fxyzw v, ludata_fxyzw n) {
    LU_STATUS
    LU_CHECK(luarray_reservevnorm(log, vn, 1))
    vn->v[vn->mem.used++] = (vnorm){v, n};
    LU_NO_CLEANUP
}

LUARRAY_MKDUMP(luarray_dumpvnorm, luarray_vnorm, 1, "{%g,%g,%g,%g}/{%g,%g,%g,%g}",
        ptr->v[i].v.x, ptr->v[i].v.y, ptr->v[i].v.z, ptr->v[i].v.w,
        ptr->v[i].n.x, ptr->v[i].n.y, ptr->v[i].n.z, ptr->v[i].n.w)
