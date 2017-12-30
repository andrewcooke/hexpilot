
#include <stdlib.h>

#include "lu/status.h"
#include "lu/status_codes.h"

#include "universe.h"


int mkuniverse(lulog *log, universe **universe, size_t programs_size) {
    int status = LU_OK;
    LU_ALLOC(log, *universe, 1);
    LU_ALLOC_SIZE(log, (*universe)->programs, programs_size)
    (*universe)->log = log;
    exit:
	return status;
}

int free_universe(universe **universe, int prev) {
    int status = LU_OK;
    if (universe && *universe) {
    	status = free_world(&(*universe)->flight, status);
    	free((*universe)->programs);
    	free(*universe);
        *universe = NULL;
    }
exit:
    return prev ? prev : status;
}
