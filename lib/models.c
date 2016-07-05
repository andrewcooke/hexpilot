
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/dynamic_memory.h"

#include "models.h"
#include "universe.h"
#include "error_codes.h"


LUARY_MKBASE(model, luary_model, model*, m)

int luary_pushmodel(lulog *log, luary_model *models, model *model) {
    LU_STATUS
    LU_CHECK(luary_reservemodel(log, models, 1))
    models->m[models->mem.used++] = model;
    LU_NO_CLEANUP
}

int mkmodel(lulog *log, model **model, send *send, draw *draw) {
    LU_STATUS
    LU_ALLOC(log, *model, 1)
    (*model)->draw = draw;
    (*model)->send = send;
    LU_NO_CLEANUP
}

