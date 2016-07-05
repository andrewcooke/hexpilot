
#include "lu/status.h"
#include "lu/arrays.h"
#include "lu/error_codes.h"

#include "shaders.h"

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
