
#include <status.h>
#include <stdlib.h>
#include <string.h>

#include "lu/internal.h"
#include "lu/log.h"
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
			HEXPILOT_DATA, &source));
	try(compile_shader_from_string(log, shader_type, source.c, shaders));
	finally:
	status = lustr_free(&source, status);
	return status;
}

int compile_shader_from_string(lulog *log, GLenum shader_type, const char *source, luary_uint32 **shaders) {
	int status = LU_OK;
	ludebug(log, "Compiling %s shader:", shader_type_str(log, shader_type));
	lulog_lines(log, lulog_level_debug, source);
	gl_try(GLuint shader = glCreateShader(shader_type))
	gl_try(glShaderSource(shader, 1, &source, NULL))
	gl_try(glCompileShader(shader))
	GLint compile_status;
	gl_try(glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status))
	if (!compile_status) {
		luerror(log, "Failed to compile %s shader", shader_type_str(log, shader_type));
		GLint log_length;
		gl_try(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length))
		GLchar log_text[log_length];
		gl_try(glGetShaderInfoLog(shader, log_length, NULL, log_text))
		lulog_lines(log, lulog_level_debug, log_text);
		status = HP_ERR_OPENGL;
		goto finally;
	}
	if (!*shaders) {
		try(luary_uint32_mk(log, shaders, 1))
	}
	try(luary_uint32_push(log, *shaders, shader));
	luinfo(log, "Compiled %s shader", shader_type_str(log, shader_type));
	finally:
	return status;
}

int link_program(lulog *log, luary_uint32 *shaders, GLuint *program) {
	int status = LU_OK;
	gl_try(*program = glCreateProgram())
	for (size_t i = 0; i < shaders->mem.used; ++i) {
		gl_try(glAttachShader(*program, shaders->i[i]))
	}
	gl_try(glLinkProgram(*program))
	GLint link_status;
	gl_try(glGetProgramiv(*program, GL_LINK_STATUS, &link_status))
	if (!link_status) {
		luerror(log, "Failed to link program");
		GLint log_length;
		gl_try(glGetProgramiv(*program, GL_INFO_LOG_LENGTH, &log_length))
		GLchar log_text[log_length];
		gl_try(glGetProgramInfoLog(*program, log_length, NULL, log_text))
		lulog_lines(log, lulog_level_debug, log_text);
		status = HP_ERR_OPENGL;
		goto finally;
	}
	for (size_t i = 0; i < shaders->mem.used; ++i) {
		gl_try(glDetachShader(*program, shaders->i[i]))
	}
	luinfo(log, "Linked program with %zu shaders", shaders->mem.used);
	finally:return status;
}

int free_shaders(lulog *log, luary_uint32 **shaders, int prev_status) {
	int status = LU_OK;
	if (shaders && *shaders) {
		for (size_t i = 0; i < (*shaders)->mem.used; ++i) {
			gl_try(glDeleteShader((*shaders)->i[i]));
		}
	}
	finally:
	status = luary_uint32_free(shaders, status);
	return lu_both(prev_status, status);
}


int set_uniform(lulog *log, GLuint program, const char *name, GLuint *uniform, GLuint index) {
	int status = LU_OK;
	gl_try(glUseProgram(program))
	gl_try(GLuint locn = glGetUniformLocation(program, name))
	gl_try(glUniform1i(locn, index))
	*uniform = index;
	finally:
	gl_try(glUseProgram(0))
	return status;
}

int use_uniform_texture(lulog *log, GLuint uniform, GLuint texture) {
	int status = LU_OK;
	glActiveTexture(GL_TEXTURE0 + uniform);
	glBindTexture(GL_TEXTURE_2D, texture);
	finally:
	return status;
}


int interleaved_vnorm_vao(lulog *log, buffer *buffer, GLuint *vao) {
	int status = LU_OK;
	gl_try(glGenVertexArrays(1, vao))
	gl_try(glBindVertexArray(*vao))
	try(buffer_bind(log, buffer))
	gl_try(glEnableVertexAttribArray(0))
	gl_try(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 32, 0))
	gl_try(glEnableVertexAttribArray(1))
	gl_try(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 32, (void*)16))
	finally:
	GL_CLEAN(glBindVertexArray(0))
	status = lu_both(status, buffer_unbind(log, buffer));
	return status;
}


