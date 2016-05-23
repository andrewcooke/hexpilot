
#ifndef HP_INIT_H
#define HP_INIT_H

#include "glad.h"
#include <GLFW/glfw3.h>

int create_glfw_context(const lulog *log, GLFWwindow **window);
int load_opengl_functions(const lulog *log);
const char *shader_type_str(const lulog *log, GLenum shader_type);
int compile_shader(const lulog *log, GLenum shader_type, const char *source, GLuint *shader);
int link_program(const lulog *log, const GLuint *shaders, size_t n_shaders);

#endif
