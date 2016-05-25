
#ifndef HP_SHADERS_H
#define HP_SHADERS_H

#include "glad.h"
#include <GLFW/glfw3.h>

#include "lu/log.h"
#include "lu/arrays.h"


const char *shader_type_str(const lulog *log, GLenum shader_type);
int compile_shader(const lulog *log, GLenum shader_type, const char *source, luarray_uint *shaders);
int link_program(const lulog *log, luarray_uint *shaders, GLuint *program);

#endif
