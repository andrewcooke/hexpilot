
#ifndef HP_INIT_H
#define HP_INIT_H

#include "glad.h"
#include <GLFW/glfw3.h>

typedef struct luarray_gluint {
    GLuint *i;
    lumem mem;
} luarray_gluint;

int luarray_mkgluintn(const lulog *log, luarray_gluint **i, int n);
int luarray_freegluint(luarray_gluint **i, int prev_status);
int luarray_reservegluint(const lulog *log, luarray_gluint *i, int n);
int luarray_pushgluint(const lulog *log, luarray_gluint *i, GLuint u);

int create_glfw_context(const lulog *log, GLFWwindow **window);
int load_opengl_functions(const lulog *log);
const char *shader_type_str(const lulog *log, GLenum shader_type);
int compile_shader(const lulog *log, GLenum shader_type, const char *source, luarray_gluint *shaders);
int link_program(const lulog *log, luarray_gluint *shaders, GLuint *program);

#endif
