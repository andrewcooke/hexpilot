
#include "glad.h"
#include <GLFW/glfw3.h>

#include "error_codes.h"
#include "lu/log.h"
#include "lu/status.h"


static int display(const lulog *log) {
    LU_STATUS
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f( 0.5f, -0.5f);
    glVertex2f( 0.5f,  0.5f);
    glVertex2f(-0.5f,  0.5f);
    glEnd();
    glFlush();
    LU_NO_CLEANUP
}

static const char* vertex_shader =
        "#version 330\n"
        "layout(location = 0) in vec4 position;\n"
        "void main(){\n"
        "  gl_Position = position;\n"
        "}\n";

static const char* fragment_shader =
        "#version 330\n"
        "out vec4 outputColor;\n"
        "void main(){\n"
        "  outputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n";

static const char *shader_type_str(const lulog *log, GLenum shader_type) {
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

static int compile_shader(const lulog *log, GLenum shader_type, const char *source, GLuint *shader) {
    LU_STATUS
    ludebug(log, "Compiling %s shader:", shader_type_str(log, shader_type));
    lulog_lines(log, lulog_level_debug, source);
    HP_GLCHECK(*shader = glCreateShader(shader_type))
    HP_GLCHECK(glShaderSource(*shader, 1, &source, NULL))
    HP_GLCHECK(glCompileShader(*shader))
    GLint compile_status;
    HP_GLCHECK(glGetShaderiv(*shader, GL_COMPILE_STATUS, &compile_status))
    if (!compile_status) {
        luerror(log, "Failed to compile %s shader", shader_type_str(log, shader_type));
        GLint log_length;
        HP_GLCHECK(glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        HP_GLCHECK(glGetShaderInfoLog(*shader, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    luinfo(log, "Compiled %s shader", shader_type_str(log, shader_type));
    LU_NO_CLEANUP
}

static int link_program(const lulog *log, const GLuint *shaders, size_t n_shaders) {
    LU_STATUS
    HP_GLCHECK(GLuint program = glCreateProgram())
    for (size_t i = 0; i < n_shaders; ++i) {
        HP_GLCHECK(glAttachShader(program, shaders[i]))
    }
    HP_GLCHECK(glLinkProgram(program))
    GLint link_status;
    HP_GLCHECK(glGetProgramiv(program, GL_LINK_STATUS, &link_status))
    if (!link_status) {
        luerror(log, "Failed to link program");
        GLint log_length;
        HP_GLCHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        HP_GLCHECK(glGetProgramInfoLog(program, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
        status = HP_ERR_OPENGL;
        goto exit;
    }
    for (size_t i = 0; i < n_shaders; ++i) {
        HP_GLCHECK(glDetachShader(program, shaders[i]))
    }
    luinfo(log, "Linked program with %zu shaders", n_shaders);
    LU_NO_CLEANUP
}

static int init(const lulog *log) {
    LU_STATUS
    GLuint shaders[2];
    LU_CHECK(compile_shader(log, GL_VERTEX_SHADER, vertex_shader, &shaders[0]))
    LU_CHECK(compile_shader(log, GL_FRAGMENT_SHADER, fragment_shader, &shaders[1]))
    LU_CHECK(link_program(log, shaders, 2));
    for (size_t i = 0; i < 2; ++i) {
        HP_GLCHECK(glDeleteShader(shaders[i]))
    }
    LU_NO_CLEANUP
}

static int main_loop(const lulog *log) {

    LU_STATUS
    GLFWwindow *window = NULL;

    // not clear to me to what extent these duplicate or conflict with glad
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    LU_ASSERT(window = glfwCreateWindow(320, 320, "hexpilot", NULL, NULL),
            HP_ERR_GLFW, log, "Could not create window")

    glfwMakeContextCurrent(window);
    LU_ASSERT(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress),
            HP_ERR_GLAD, log, "Could not load OpenGL via glad")
    LU_ASSERT(GLVersion.major > 1, HP_ERR_OPENGL, log,
            "Bad OpenGL version: %d.%d", GLVersion.major, GLVersion.minor)
    luinfo(log, "OpenGL %s, GLSL %s",
            glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    LU_CHECK(init(log))
    while (!glfwWindowShouldClose(window)) {
        LU_CHECK(display(log))
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

LU_CLEANUP
    glfwTerminate();
    LU_RETURN
}


static lulog *LOG = NULL;

static void on_error(int error, const char *message) {
    luerror(LOG, "%d: %s", error, message);
}

int main(int argc, char** argv) {
    LU_STATUS
    lulog_mkstderr(&LOG, lulog_level_debug);
    glfwSetErrorCallback(on_error);
    LU_ASSERT(glfwInit(), HP_ERR_GLFW, LOG, "Could not start GLFW")
    LU_CHECK(main_loop(LOG))
LU_CLEANUP
    return status ? 1 : 0;
}
