
#ifndef HP_DATA_UINT32_H
#define HP_DATA_UINT32_H

#include <stdint.h>

#include "lu/arrays.h"
#include "lu/log.h"


typedef struct luary_uint32 {
    uint32_t *i;
    lumem mem;
} luary_uint32;

int luary_uint32_mk(lulog *log, luary_uint32 **i, size_t n);
int luary_uint32_free(luary_uint32 **i, int prev_status);
int luary_uint32_res(lulog *log, luary_uint32 *i, size_t n);
int luary_uint32_push(lulog *log, luary_uint32 *i, unsigned int j);
int luary_uint32_dump(lulog *log, luary_uint32 *i, const char *name, size_t nlines);

#endif
