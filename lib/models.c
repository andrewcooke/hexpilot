
#include <status.h>
#include <string.h>

#include "lu/log.h"
#include "lu/arrays.h"
#include "lu/memory.h"

#include "models.h"
#include "universe.h"


LUARY_MKBASE(model, model*, m)

int luary_model_push(lulog *log, luary_model *models, model *model) {
    int status = LU_OK;
    try(luary_model_res(log, models, 1));
    models->m[models->mem.used++] = model;
    finally:
	return status;
}

int model_mk(lulog *log, model **model, send *send, draw *draw) {
    int status = LU_OK;
    LU_ALLOC(log, *model, 1);
    (*model)->draw = draw;
    (*model)->send = send;
    finally:
	return status;
}

