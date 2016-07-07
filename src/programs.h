
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
    GLuint frame1;
    GLuint frame2;
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
int build_merge_frames(lulog *log, merge_frames *program);

int draw_filled_triangles(lulog *log, model *model, programs *programs);
int draw_lines_and_triangles(lulog *log, model *model, programs *programs);

#endif
