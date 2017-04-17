
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

typedef struct blur {
    GLuint name;
    GLuint frame;
    GLuint horizontal;
} blur;

typedef struct programs {
    GLuint triangle_edges;
    direct_texture copy_frame;
    merge_frames merge_frames;
    blur blur;
} programs;

int build_triangle_edges(lulog *log, GLuint *program);
int build_direct_texture(lulog *log, direct_texture *program);
int build_merge_frames(lulog *log, merge_frames *program);
int build_blur(lulog *log, blur *program);

int draw_triangle_edges(lulog *log, model *model, programs *programs);

#endif
