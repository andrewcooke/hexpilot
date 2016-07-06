
#ifndef HP_UNIVERSE_H
#define HP_UNIVERSE_H

#include "worlds.h"


typedef struct universe {
    lulog *log;
    void * programs;
    world *flight;  // TODO - should be more general
} universe;

int mkuniverse(lulog *log, universe **universe, size_t programs_size);
int free_universe(universe **universe, int status);

#endif
