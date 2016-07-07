
#ifndef HP_PROGRAMS_H
#define HP_PROGRAMS_H

#include "lu/log.h"

#include "glad.h"

#include "models.h"

typedef struct direct_texture {
    GLuint name;
    GLuint frame;
} direct_texture;

typedef struct merge_frames {
    GLuint name;
} merge_frames;

typedef struct programs {
    GLuint lit_per_vertex;
    GLuint black;
    direct_texture direct_texture;
    merge_frames merge_frames;
} programs;

int build_lit_per_vertex(lulog *log, GLuint *program);
int build_black(lulog *log, GLuint *program);
int build_direct_texture(lulog *log, direct_texture *program);

int draw_filled_triangles(lulog *log, model *model, programs *programs);
int draw_lines_and_triangles(lulog *log, model *model, programs *programs);

#endif
