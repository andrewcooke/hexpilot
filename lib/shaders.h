
#ifndef HP_SHADERS_H
#define HP_SHADERS_H

#include "glad.h"
#include "buffers.h"

#include "lu/log.h"
#include "lu/arrays.h"


const char *shader_type_str(lulog *log, GLenum shader_type);
int compile_shader(lulog *log, GLenum shader_type, const char *source, luary_uint32 **shaders);
int link_program(lulog *log, luary_uint32 *shaders, GLuint *program);
int free_shaders(lulog *log, luary_uint32 **shaders, int status);

int interleaved_vnorm_vao(lulog *log, GLuint program, buffer *buffer, GLuint *vao);

#endif
