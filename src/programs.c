
#include <status_codes.h>
#include "lu/status.h"
#include "lu/arrays.h"
#include "lu/status_codes.h"

#include "shaders.h"
#include "programs.h"


/**
 * edges using geometry shader.
 */
int build_triangle_edges(lulog *log, GLuint *program) {
    int status = LU_OK;
    luary_uint32 *shaders = NULL;
    try(compile_shader_from_file(log, GL_VERTEX_SHADER, "flat_model_g.vert", &shaders))
    try(compile_shader_from_file(log, GL_GEOMETRY_SHADER, "edge_lines.geom", &shaders))
    try(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    try(link_program(log, shaders, program));
finally:
    status = free_shaders(log, &shaders, status);
    return status;
}

/**
 * copy a frame by rendering a texture directly (needs a quad to select the area).
 */
int build_direct_texture(lulog *log, direct_texture *program) {
    int status = LU_OK;
    luary_uint32 *shaders = NULL;
    try(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    try(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_texture.frag", &shaders))
    try(link_program(log, shaders, &program->name))
    try(set_uniform(log, program->name, "frame", &program->frame, 0))
finally:
    status = free_shaders(log, &shaders, status);
    return status;
}

/**
 * merge two frames via textures.
 */
int build_merge_frames(lulog *log, merge_frames *program) {
    int status = LU_OK;
    luary_uint32 *shaders = NULL;
    try(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    try(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "merge_frames.frag", &shaders))
    try(link_program(log, shaders, &program->name))
    try(set_uniform(log, program->name, "frame1", &program->frame1, 0))
    try(set_uniform(log, program->name, "frame2", &program->frame2, 1))
finally:
    status = free_shaders(log, &shaders, status);
    return status;
}

/**
 * blur a frame (roughly uniform over 5 pixels radius).
 */
int build_blur(lulog *log, blur *program) {
    int status = LU_OK;
    luary_uint32 *shaders = NULL;
    try(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    try(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "blur.frag", &shaders))
    try(link_program(log, shaders, &program->name))
    try(set_uniform(log, program->name, "frame", &program->frame, 0))
    try(set_uniform(log, program->name, "horizontal", &program->horizontal, 1))
finally:
    status = free_shaders(log, &shaders, status);
    return status;
}


int draw_triangle_edges(lulog *log, model *model, programs *programs) {
    int status = LU_OK;
    GL_CHECK(glBindVertexArray(model->vao))
//    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE))
    GL_CHECK(glUseProgram(programs->triangle_edges))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
//    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))
finally:
    GL_CLEAN(glBindVertexArray(0))
    GL_CLEAN(glUseProgram(0))
    return status;
}


