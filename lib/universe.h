
#ifndef HP_UNIVERSE_H
#define HP_UNIVERSE_H

#include "worlds.h"


typedef struct universe {
    lulog *log;
    void *programs;
    world *flight;  // TODO - should be more general
} universe;

int universe_mk(lulog *log, universe **universe, size_t programs_size);
int universe_free(universe **universe, int status);

#endif
