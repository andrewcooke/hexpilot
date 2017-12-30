
#include <status_codes.h>
#include <string.h>

#include "lu/log.h"
#include "lu/status.h"
#include "lu/array_macros.h"
#include "lu/dynamic_memory.h"

#include "models.h"
#include "universe.h"


LUARY_MKBASE(model, model*, m)

int luary_pushmodel(lulog *log, luary_model *models, model *model) {
    int status = LU_OK;
    try(luary_reservemodel(log, models, 1))
    models->m[models->mem.used++] = model;
    finally:return status;
}

int mkmodel(lulog *log, model **model, send *send, draw *draw) {
    int status = LU_OK;
    LU_ALLOC(log, *model, 1)
    (*model)->draw = draw;
    (*model)->send = send;
    finally:return status;
}

