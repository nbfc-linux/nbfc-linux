AC_INIT([nbfc-linux], [0.2.5], [braph93@gmx.de])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([m], [roundf])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_TYPE_MODE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([atexit memset mkdir realpath setlocale strchr strrchr strcspn strdup strerror strtol strtoull])

# Init-System
AC_ARG_WITH([init-system],
  [AS_HELP_STRING([--with-init-system=SYSTEM], [Sets the Init-System (systemd, systemv, openrc)])],
  [init_system="$withval"],
  [init_system="systemd"])

case "$init_system" in
  systemd|systemv|openrc) ;;
  *) AC_MSG_ERROR([Invalid value for --with-init-system. Use systemd, systemv or openrc.]);;
esac

AC_MSG_CHECKING([which Init-System should be used])
AC_MSG_RESULT([$with_init_system])

AC_SUBST([INIT_SYSTEM], [$init_system])
AC_SUBST([PACKAGE_VERSION])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
