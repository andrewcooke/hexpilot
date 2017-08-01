
#include <status_codes.h>
#include "lu/status.h"

#include "glad.h"
#include "frames.h"


int init_frame(lulog *log, GLFWwindow *window, frame *frame, int msaa, int depth) {
    LU_STATUS

    // http://learnopengl.com/#!Advanced-OpenGL/Framebuffers
    // http://www.learnopengl.com/#!Advanced-OpenGL/Anti-Aliasing

    frame->msaa = msaa;
    GL_CHECK(glGenFramebuffers(1, &frame->render))
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame->render))
    GL_CHECK(glGenTextures(1, &frame->texture))
    GL_CHECK(glBindTexture(msaa ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, frame->texture))
    glfwGetFramebufferSize(window, &frame->width, &frame->height);

    if (msaa) {
        GL_CHECK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, frame->width, frame->height, GL_TRUE))
    } else {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL))
    }
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
    if (msaa) {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, frame->texture, 0))
    } else {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frame->texture, 0))
    }

    if (depth) {
        GL_CHECK(glGenRenderbuffers(1, &frame->depth))
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, frame->depth))
        if (msaa) {
            GL_CHECK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, frame->width, frame->height))
        } else {
            GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, frame->width, frame->height))
        }
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, frame->depth))
    }

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame->render))
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | (depth ? GL_DEPTH_BUFFER_BIT : 0)))

    LU_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
            LU_ERR, log, "Frame buffer incomplete")

LU_CLEANUP
    GL_CHECK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0))
    GL_CLEAN(glBindFramebuffer(GL_FRAMEBUFFER, 0))
    LU_RETURN
}

int free_frame_contents(lulog *log, frame *frame) {
    LU_STATUS
LU_CLEANUP
    if (frame->depth) {
        GL_CLEAN(glDeleteRenderbuffers(1, &frame->depth))
    }
    GL_CLEAN(glDeleteTextures(1, &frame->texture))
    GL_CLEAN(glDeleteFramebuffers(1, &frame->render))
    LU_RETURN
}

int rescale_frame(lulog *log, GLFWwindow *window, frame *frame) {
    LU_STATUS
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    if (width != frame->width || height != frame->height) {
        LU_CHECK(free_frame_contents(log, frame))
        LU_CHECK(init_frame(log, window, frame, frame->msaa, frame->depth))
    }
    LU_NO_CLEANUP
}
