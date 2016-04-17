
import sys
from PySide import QtCore, QtGui, QtOpenGL
from OpenGL.GL import *
from OpenGL.GL.shaders import compileShader, compileProgram
from numpy import *
from ctypes import c_void_p

null = c_void_p(0)


class HelloTriangle(QtOpenGL.QGLWidget):

    def __init__(self):
        super().__init__()
        self.vertices = array([0.75, 0.75, 0.0, 1.0,
                               0.75, -0.75, 0.0, 1.0,
                               -0.75, -0.75, 0.0, 1.0], dtype=float32)
        self.program = None

    def sizeHint(self):
        return QtCore.QSize(300, 200)

    def resizeGL(self, w, h):
        print("new size %dx%d" % (w, h))
        glViewport(0, 0, w, h)

    def initializeProgram(self):
        print('initializeProgram')
        self.program = compileProgram(
            compileShader('''
#version 330
layout(location = 0) in vec4 position;
void main() {
   gl_Position = position;
}''', GL_VERTEX_SHADER),
            compileShader('''
#version 330
out vec4 outputColor;
void main() {
    outputColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}''', GL_FRAGMENT_SHADER))

    def initializeVertexBuffer(self):
        print('initializeVertexBuffer')
        self.buffer = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.buffer)
        glBufferData(GL_ARRAY_BUFFER, self.vertices, GL_STATIC_DRAW)
        glBindBuffer(GL_ARRAY_BUFFER, 0)

    def initializeGL(self):
        print('initializeGL')
        self.initializeVertexBuffer()
        self.initializeProgram()
        glBindVertexArray(glGenVertexArrays(1))

    def paintGL(self):
        print('paintGL')
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClear(GL_COLOR_BUFFER_BIT)
        glUseProgram(self.program)
        glBindBuffer(GL_ARRAY_BUFFER, self.buffer)
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, null)
        glDrawArrays(GL_TRIANGLES, 0, len(self.vertices) // 4)
        glDisableVertexAttribArray(0)
        glUseProgram(0)


app = QtGui.QApplication(sys.argv)
widget = HelloTriangle()
widget.show()
app.exec_()
