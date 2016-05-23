
#ifndef HP_ERROR_CODES_H
#define HP_ERROR_CODES_H

#define HP_OFFSET 0x100
#define HP_ERR_GLFW (HP_OFFSET | 1)
#define HP_ERR_GLAD (HP_OFFSET | 2)
#define HP_ERR_OPENGL (HP_OFFSET | 3)

// warning - this generates two statements.
// (chosen to allow declarations in operation that remain in-scope)
#define HP_GLCHECK(operation) operation; {\
    GLenum err = glGetError();\
    if (err != GL_NO_ERROR) {\
        luerror(log, "OpenGL Error %x in %s (%s:%d)", err, __func__, __FILE__, __LINE__);\
        status = HP_ERR_OPENGL;\
        goto exit;\
    }}

#endif
