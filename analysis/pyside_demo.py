
import sys
from PySide import QtCore, QtGui, QtOpenGL
from OpenGL.GL import *

class GLDemo(QtOpenGL.QGLWidget):

    def sizeHint(self):
        return QtCore.QSize(300, 200)

    def resizeGL(self, w, h):
        print("new size %dx%d" % (w, h))
        glViewport(0, 0, w, h)

    def initializeGL(self):
        pass

    def paintGL(self):
        pass


app = QtGui.QApplication(sys.argv)
widget = GLDemo()
widget.show()
app.exec_()
