
#ifndef HP_PROGRAMS_H
#define HP_PROGRAMS_H

#include "lu/log.h"

#include "glad.h"

#include "models.h"


typedef struct programs {
    GLuint lit_per_vertex;
    GLuint black;
} programs;

int build_lit_per_vertex(lulog *log, GLuint *program);
int build_black(lulog *log, GLuint *program);

int draw_filled_triangles(lulog *log, model *model, programs *programs);
int draw_lines_and_triangles(lulog *log, model *model, programs *programs);

#endif
