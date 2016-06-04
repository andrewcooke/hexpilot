
#include <string.h>

#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/structs.h"
#include "lu/log.h"

#include "vertices.h"

LUARY_MKBASE(vnorm, luary_vnorm, vnorm, vn)

int luary_pushvnorm(lulog *log, luary_vnorm *vn, luvec_f4 *v, luvec_f4 *n) {
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
        luvec_strf4(&ptr->vn[i][0], 100, bv), luvec_strf4(&ptr->vn[i][1], 100, bn))
