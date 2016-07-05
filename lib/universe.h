
#ifndef HP_UNIVERSE_H
#define HP_UNIVERSE_H

#include "worlds.h"


typedef struct programs {
	GLuint lit_per_vertex;
	GLuint black;
} programs;

typedef struct universe {
    lulog *log;
    programs programs;
    world *flight;
} universe;

int mkuniverse(lulog *log, universe **universe);
int free_universe(universe **universe, int status);

#endif
