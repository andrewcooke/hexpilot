
#include "glad.h"
#include <GLFW/glfw3.h>

#include "error_codes.h"
#include "lu/log.h"
#include "lu/status.h"


static int display(lulog *log) {
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

int createShader(lulog *log, GLenum shaderType, const char *source, GLuint *shader) {
    LU_STATUS
    HP_GLCHECK(*shader = glCreateShader(shaderType))
    HP_GLCHECK(glShaderSource(*shader, 1, &source, NULL))
    HP_GLCHECK(glCompileShader(*shader))
    GLint compile_status;
    HP_GLCHECK(glGetShaderiv(*shader, GL_COMPILE_STATUS, &compile_status))
    if (!compile_status) {
        GLint log_length;
        HP_GLCHECK(glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &log_length))
        GLchar log_text[log_length];
        HP_GLCHECK(glGetShaderInfoLog(*shader, log_length, NULL, log_text))
        lulog_lines(log, lulog_level_debug, log_text);
    }
    LU_NO_CLEANUP
}

int init(lulog *log) {
    LU_STATUS
    GLuint shaders[2];
    LU_CHECK(createShader(log, GL_VERTEX_SHADER, vertex_shader, &shaders[0]))
    LU_CHECK(createShader(log, GL_FRAGMENT_SHADER, fragment_shader, &shaders[1]))
    LU_NO_CLEANUP
}

int main_loop(lulog *log) {

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

void on_error(int error, const char *message) {
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
