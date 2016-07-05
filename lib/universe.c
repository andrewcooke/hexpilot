
#include <stdlib.h>

#include "lu/status.h"

#include "universe.h"


int mkuniverse(lulog *log, universe **universe, size_t programs_size) {
    LU_STATUS
    LU_ALLOC(log, *universe, 1)
    LU_ALLOC_SIZE(log, (*universe)->programs, programs_size)
    (*universe)->log = log;
    LU_NO_CLEANUP
}

int free_universe(universe **universe, int prev) {
    LU_STATUS
    if (universe && *universe) {
    	status = free_world(&(*universe)->flight, status);
    	free((*universe)->programs);
    	free(*universe);
        *universe = NULL;
    }
LU_CLEANUP
    LU_RETURN2(prev)
}
