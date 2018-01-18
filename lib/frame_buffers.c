
#include <frame_buffers.h>
#include <status.h>
#include "lu/internal.h"

#include "glad.h"


int frame_buffer_mk(lulog *log, frame_buffer *frame, GLFWwindow *window, int multisample_aa, int depth) {
	int status = LU_OK;

	// http://learnopengl.com/#!Advanced-OpenGL/Framebuffers
	// http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing

	frame->multisample_aa = multisample_aa;
	gl_try(glGenFramebuffers(1, &frame->name))
	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, frame->name))
	gl_try(glGenTextures(1, &frame->texture))
	gl_try(glBindTexture(multisample_aa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, frame->texture))
	glfwGetFramebufferSize(window, &frame->width, &frame->height);

	if (multisample_aa) {
		gl_try(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, frame->width, frame->height, GL_TRUE))
	} else {
		gl_try(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL))
	}
	gl_try(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl_try(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	if (multisample_aa) {
		gl_try(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frame->texture, 0))
	} else {
		gl_try(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame->texture, 0))
	}

	if (depth) {
		gl_try(glGenRenderbuffers(1, &frame->depth));
		gl_try(glBindRenderbuffer(GL_RENDERBUFFER, frame->depth));
		if (multisample_aa) {
			gl_try(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, frame->width, frame->height));
		} else {
			gl_try(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frame->width, frame->height));
		}
		gl_try(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frame->depth));
	}

	gl_try(glBindFramebuffer(GL_FRAMEBUFFER, frame->name));
	gl_try(glClear(GL_COLOR_BUFFER_BIT | (depth ? GL_DEPTH_BUFFER_BIT : 0)));

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
			LU_ERR, log, "Frame buffer incomplete");

	finally:
	gl_try(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0));
	GL_CLEAN(glBindFramebuffer(GL_FRAMEBUFFER, 0))
	return status;
}

static int free_contents(lulog *log, frame_buffer *frame) {
	int status = LU_OK;
	finally:
	if (frame->depth) {
		GL_CLEAN(glDeleteRenderbuffers(1, &frame->depth))
	}
	GL_CLEAN(glDeleteTextures(1, &frame->texture))
	GL_CLEAN(glDeleteFramebuffers(1, &frame->name))
	return status;
}

int frame_buffer_rescale(lulog *log, frame_buffer *frame, GLFWwindow *window) {
	int status = LU_OK;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	if (width != frame->width || height != frame->height) {
		try(free_contents(log, frame));
		try(frame_buffer_mk(log, frame, window, frame->multisample_aa, frame->depth));
	}
	finally:
	return status;
}
