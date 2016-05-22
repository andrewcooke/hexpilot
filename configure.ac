AC_INIT([hexpilot], [0.1], [andrew@acooke.org])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_PROG_CC_C99
PKG_CHECK_MODULES([CHECK], [check >= 0.9.0])
AX_CHECK_GL
AC_CHECK_LIB([GLU], [gluPerspective], [], [AC_MSG_ERROR([No libGLU found])])
AC_CHECK_LIB([glfw], [glfwInit], [], [AC_MSG_ERROR([No libGLFW found])])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
