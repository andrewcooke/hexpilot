
import sys
from PySide import QtCore, QtGui, QtOpenGL

class GLDemo(QtOpenGL.QGLWidget):

    def sizeHint(self):
        return QtCore.QSize(300, 200)

    def resizeGL(self, w, h):
        print("new size %dx%d" % (w, h))

app = QtGui.QApplication(sys.argv)
widget = GLDemo()
widget.show()
app.exec_()
