
#include "../status.h"
#include "uint32.h"


LUARY_MKBASE(uint32, uint32_t, i)

int luary_uint32_push(lulog *log, luary_uint32 *i, uint32_t j) {
    int status = LU_OK;
    try(luary_uint32_res(log, i, 1))
    i->i[i->mem.used++] = j;
    finally:
	return status;
}

LUARY_MKDUMP(luary_uint32_dump, luary_uint32, "%u", ptr->i[i])
