
#include "lu/status.h"
#include "lu/arrays.h"
#include "lu/error_codes.h"

#include "shaders.h"
#include "error_codes.h"

#include "programs.h"


/**
 * edges using geometry shader.
 */
int build_triangle_edges(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "flat_model_g.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_GEOMETRY_SHADER, "edge_lines.geom", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

/**
 * copy a frame by rendering a texture directly (needs a quad to select the area).
 */
int build_direct_texture(lulog *log, direct_texture *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_texture.frag", &shaders))
    LU_CHECK(link_program(log, shaders, &program->name))
    LU_CHECK(set_uniform(log, program->name, "frame", &program->frame, 0))
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

/**
 * merge two frames via textures.
 */
int build_merge_frames(lulog *log, merge_frames *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "merge_frames.frag", &shaders))
    LU_CHECK(link_program(log, shaders, &program->name))
    LU_CHECK(set_uniform(log, program->name, "frame1", &program->frame1, 0))
    LU_CHECK(set_uniform(log, program->name, "frame2", &program->frame2, 1))
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

/**
 * blur a frame (roughly uniform over 5 pixels radius).
 */
int build_blur(lulog *log, blur *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "blur.frag", &shaders))
    LU_CHECK(link_program(log, shaders, &program->name))
    LU_CHECK(set_uniform(log, program->name, "frame", &program->frame, 0))
    LU_CHECK(set_uniform(log, program->name, "horizontal", &program->horizontal, 1))
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}


int draw_triangle_edges(lulog *log, model *model, programs *programs) {
    LU_STATUS
    GL_CHECK(glBindVertexArray(model->vao))
//    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE))
    GL_CHECK(glUseProgram(programs->triangle_edges))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
//    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))
LU_CLEANUP
    GL_CLEAN(glBindVertexArray(0))
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}


