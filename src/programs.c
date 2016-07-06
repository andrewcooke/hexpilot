
#include "lu/status.h"
#include "lu/arrays.h"
#include "lu/error_codes.h"

#include "shaders.h"
#include "error_codes.h"

#include "programs.h"


int build_lit_per_vertex(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "lit_model.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

int build_black(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "black_model.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_colour.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

int build_direct_texture(lulog *log, GLuint *program) {
    LU_STATUS
    luary_uint32 *shaders = NULL;
    LU_CHECK(compile_shader_from_file(log, GL_VERTEX_SHADER, "direct_texture.vert", &shaders))
    LU_CHECK(compile_shader_from_file(log, GL_FRAGMENT_SHADER, "direct_texture.frag", &shaders))
    LU_CHECK(link_program(log, shaders, program));
LU_CLEANUP
    status = free_shaders(log, &shaders, status);
    LU_RETURN
}

int draw_filled_triangles(lulog *log, model *model, programs *programs) {
    LU_STATUS
    GL_CHECK(glUseProgram(programs->lit_per_vertex))
    GL_CHECK(glBindVertexArray(model->vao))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
LU_CLEANUP
    GL_CLEAN(glBindVertexArray(0))
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}

int draw_lines_and_triangles(lulog *log, model *model, programs *programs) {
    LU_STATUS
    GL_CHECK(glBindVertexArray(model->vao))
    GL_CHECK(glUseProgram(programs->black))
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
    GL_CHECK(glUseProgram(programs->lit_per_vertex))
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE))
    GL_CHECK(glMultiDrawArrays(GL_TRIANGLE_STRIP, model->offsets->i, model->counts->i, model->counts->mem.used));
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))
LU_CLEANUP
    GL_CLEAN(glBindVertexArray(0))
    GL_CLEAN(glUseProgram(0))
    LU_RETURN
}
