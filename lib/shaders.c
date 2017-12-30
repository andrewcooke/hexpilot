
#include <status_codes.h>
#include <stdlib.h>
#include <string.h>

#include "lu/status.h"
#include "lu/files.h"

#include "hexpilot.h"
#include "shaders.h"


const char *shader_type_str(lulog *log, GLenum shader_type) {
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

int compile_shader_from_file(lulog *log, GLenum shader_type, const char *filename,
		luary_uint32 **shaders) {
	int status = LU_OK;
	lustr source = {};
	try(lufle_find_and_read_config(log, hp_xstr(DATADIR), HEXPILOT, filename,
			HEXPILOT_DATA, &source))
	try(compile_shader_from_string(log, shader_type, source.c, shaders))
exit:
	status = lustr_free(&source, status);
	return status;
}

int compile_shader_from_string(lulog *log, GLenum shader_type, const char *source, luary_uint32 **shaders) {
    int status = LU_OK;
    ludebug(log, "Compiling %s shader:", shader_type_str(log, shader_type));
    lulog_lines(log, lulog_level_debug, source);
    GL_CHECK(GLuint shader = glCreateShader(shader_type))
    GL_CHECK(glShaderSource(shader, 1, &source, NULL))
    GL_CHECK(glCompileShader(shader))
    GLint compile_status;
    GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status))
    if (!compile_status) {
        luerror(log, "Failed to compile %s shader", shader_type_str(log, shader_type));
        GLint log_length;
        GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        GL_CHECK(glGetShaderInfoLog(shader, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    if (!*shaders) {
        try(luary_mkuint32(log, shaders, 1))
    }
    try(luary_pushuint32(log, *shaders, shader))
    luinfo(log, "Compiled %s shader", shader_type_str(log, shader_type));
    exit:return status;
}

int link_program(lulog *log, luary_uint32 *shaders, GLuint *program) {
    int status = LU_OK;
    GL_CHECK(*program = glCreateProgram())
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        GL_CHECK(glAttachShader(*program, shaders->i[i]))
    }
    GL_CHECK(glLinkProgram(*program))
    GLint link_status;
    GL_CHECK(glGetProgramiv(*program, GL_LINK_STATUS, &link_status))
    if (!link_status) {
        luerror(log, "Failed to link program");
        GLint log_length;
        GL_CHECK(glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        GL_CHECK(glGetProgramInfoLog(*program, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    for (size_t i = 0; i < shaders->mem.used; ++i) {
        GL_CHECK(glDetachShader(*program, shaders->i[i]))
    }
    luinfo(log, "Linked program with %zu shaders", shaders->mem.used);
    exit:return status;
}

int free_shaders(lulog *log, luary_uint32 **shaders, int prev_status) {
    int status = LU_OK;
    if (shaders && *shaders) {
        for (size_t i = 0; i < (*shaders)->mem.used; ++i) {
            GL_CHECK(glDeleteShader((*shaders)->i[i]));
        }
    }
exit:
    status = luary_freeuint32(shaders, status);
    return prev_status ? prev_status : status;
}


int set_uniform(lulog *log, GLuint program, const char *name, GLuint *uniform, GLuint index) {
    int status = LU_OK;
    GL_CHECK(glUseProgram(program))
    GL_CHECK(GLuint locn = glGetUniformLocation(program, name))
    GL_CHECK(glUniform1i(locn, index))
    *uniform = index;
exit:
    GL_CHECK(glUseProgram(0))
    return status;
}

int use_uniform_texture(lulog *log, GLuint uniform, GLuint texture) {
    int status = LU_OK;
    glActiveTexture(GL_TEXTURE0 + uniform);
    glBindTexture(GL_TEXTURE_2D, texture);
    exit:return status;
}


int interleaved_vnorm_vao(lulog *log, buffer *buffer, GLuint *vao) {
    int status = LU_OK;
    GL_CHECK(glGenVertexArrays(1, vao))
    GL_CHECK(glBindVertexArray(*vao))
    try(bind_buffer(log, buffer))
    GL_CHECK(glEnableVertexAttribArray(0))
    GL_CHECK(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, 0))
    GL_CHECK(glEnableVertexAttribArray(1))
    GL_CHECK(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (void*)16))
exit:
    GL_CLEAN(glBindVertexArray(0))
    LU_CLEAN(unbind_buffer(log, buffer))
    return status;
}


