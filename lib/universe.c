
#include <stdlib.h>

#include "lu/status.h"

#include "universe.h"


int mkuniverse(lulog *log, universe **universe) {
    LU_STATUS
    LU_ALLOC(log, *universe, 1)
    (*universe)->log = log;
    LU_NO_CLEANUP
}

int free_universe(universe **universe, int prev) {
    LU_STATUS
    if (universe && *universe) {
    	status = free_world(&(*universe)->flight, status);
    	free(*universe);
        *universe = NULL;
    }
LU_CLEANUP
    LU_RETURN2(prev)
}
