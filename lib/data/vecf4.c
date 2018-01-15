
#include "../status.h"
#include "vecf4.h"


LUARY_MKBASE(vecf4, luglv, v)

int luary_vecf4_push(lulog *log, luary_vecf4 *v, float x, float y, float z, float w) {
    int status = LU_OK;
    try(luary_vecf4_res(log, v, 1));
    v->v[v->mem.used][0] = x;
    v->v[v->mem.used][1] = y;
    v->v[v->mem.used][2] = z;
    v->v[v->mem.used][3] = w;
    v->mem.used++;
    finally:
	return status;
}
