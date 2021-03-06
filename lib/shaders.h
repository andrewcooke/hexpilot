
#ifndef HP_SHADERS_H
#define HP_SHADERS_H

#include "lu/log.h"
#include "lu/arrays.h"

#include "glad.h"
#include "data/uint32.h"
#include "data_buffers.h"


const char *shader_type_str(lulog *log, GLenum shader_type);
int compile_shader_from_file(lulog *log, GLenum shader_type, const char *filename,
		luary_uint32 **shaders);
int compile_shader_from_string(lulog *log, GLenum shader_type, const char *source,
		luary_uint32 **shaders);
int link_program(lulog *log, luary_uint32 *shaders, GLuint *program);
int free_shaders(lulog *log, luary_uint32 **shaders, int status);

int set_uniform(lulog *log, GLuint program, const char *name, GLuint *uniform, GLuint index);
int use_uniform_texture(lulog *log, GLuint uniform, GLuint texture);

int interleaved_vnorm_vao(lulog *log, data_buffer *buffer, GLuint *vao);

#endif
