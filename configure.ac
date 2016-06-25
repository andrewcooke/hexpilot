AC_INIT([hexpilot], [0.1], [andrew@acooke.org])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_PROG_AR()
LT_INIT
AC_PROG_CC_C99
dnl following required by check
AM_PROG_CC_C_O
PKG_CHECK_MODULES([CHECK], [check >= 0.9.0])
AX_CHECK_GL
AC_CHECK_LIB([glfw], [glfwInit], [], [AC_MSG_ERROR([No libGLFW (v3) found])])
AC_CHECK_LIB([lu], [ludebug], [], [AC_MSG_ERROR([No liblu found])])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile lib/Makefile src/Makefile tests/Makefile data/Makefile])
AC_OUTPUT
