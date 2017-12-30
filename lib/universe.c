
#include <stdlib.h>

#include "lu/internal.h"

#include "status.h"
#include "universe.h"


int mkuniverse(lulog *log, universe **universe, size_t programs_size) {
	int status = LU_OK;
	LU_ALLOC(log, *universe, 1);
	LU_ALLOC_SIZE(log, (*universe)->programs, programs_size);
	(*universe)->log = log;
	finally:
	return status;
}

int free_universe(universe **universe, int prev_status) {
	int status = LU_OK;
	if (universe && *universe) {
		status = free_world(&(*universe)->flight, status);
		free((*universe)->programs);
		free(*universe);
		*universe = NULL;
	}
	finally:
	return lu_both(prev_status, status);
}
