
#include <stdlib.h>
#include <string.h>

#include "lu/status.h"

#include "shaders.h"
#include "error_codes.h"


const char *shader_type_str(const lulog *log, GLenum shader_type) {
    switch (shader_type) {
    case GL_COMPUTE_SHADER: return "compute";
    case GL_VERTEX_SHADER: return "vertex";
    case GL_TESS_CONTROL_SHADER: return "tess control";
    case GL_TESS_EVALUATION_SHADER: return "tess evaluation";
    case GL_GEOMETRY_SHADER: return "geometry";
    case GL_FRAGMENT_SHADER: return "fragment";
    default:
        luwarn(log, "Unexpected shader type: %x", shader_type);
        return "unknown";
    }
}

int compile_shader(const lulog *log, GLenum shader_type, const char *source, luarray_uint *shaders) {
    LU_STATUS
    ludebug(log, "Compiling %s shader:", shader_type_str(log, shader_type));
    lulog_lines(log, lulog_level_debug, source);
    HP_GLCHECK(GLuint shader = glCreateShader(shader_type))
    HP_GLCHECK(glShaderSource(shader, 1, &source, NULL))
    HP_GLCHECK(glCompileShader(shader))
    GLint compile_status;
    HP_GLCHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status))
    if (!compile_status) {
        luerror(log, "Failed to compile %s shader", shader_type_str(log, shader_type));
        GLint log_length;
        HP_GLCHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        HP_GLCHECK(glGetShaderInfoLog(shader, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    LU_CHECK(luarray_pushuint(log, shaders, shader))
    luinfo(log, "Compiled %s shader", shader_type_str(log, shader_type));
    LU_NO_CLEANUP
}

int link_program(const lulog *log, luarray_uint *shaders, GLuint *program) {
    LU_STATUS
    HP_GLCHECK(*program = glCreateProgram())
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        HP_GLCHECK(glAttachShader(*program, shaders->i[i]))
    }
    HP_GLCHECK(glLinkProgram(*program))
    GLint link_status;
    HP_GLCHECK(glGetProgramiv(*program, GL_LINK_STATUS, &link_status))
    if (!link_status) {
        luerror(log, "Failed to link program");
        GLint log_length;
        HP_GLCHECK(glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        HP_GLCHECK(glGetProgramInfoLog(*program, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        HP_GLCHECK(glDetachShader(*program, shaders->i[i]))
    }
    luinfo(log, "Linked program with %zu shaders", shaders->mem.used);
    LU_NO_CLEANUP
}

