AC_INIT([nbfc-linux], [0.3.20], [braph93@gmx.de])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB([m], [roundf])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h unistd.h sys/socket.h])

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
AC_CHECK_FUNCS([atexit memset mkdir realpath setlocale socket strchr strrchr strstr strcspn strdup strerror strtol strtoull])
AC_CHECK_LIB([curl], [curl_easy_init], [], [AC_MSG_ERROR([libcurl not found])])
AC_CHECK_LIB([crypto], [SHA1], [], [AC_MSG_ERROR([libcrypto (OpenSSL) not found])])

# =============================================================================
# Init-System
# =============================================================================

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

# =============================================================================
# Syslog
# =============================================================================
AC_ARG_ENABLE([syslog],
  AS_HELP_STRING([--enable-syslog],
    [Enable logging through the syslog facility (default: enabled)]),
    [enable_syslog=$enableval], [enable_syslog=yes])

case "$enable_syslog" in
  yes)
    AC_DEFINE([ENABLE_SYSLOG], [1], [Define to 1 to enable logging through syslog()])
    ;;
  *)
    AC_DEFINE([ENABLE_SYSLOG], [0], [Define to 1 to enable logging through syslog()])
    ;;
esac

# =============================================================================
# Embedded Controller Components
# =============================================================================

# ENABLE_EC_DEBUG -- Enable debugging of the embedded controller
AC_ARG_ENABLE([ec-debug],
  AS_HELP_STRING([--enable-ec-debug],
    [Enable debugging of the embedded controller (default: enabled)]),
    [enable_ec_debug=$enableval], [enable_ec_debug=yes])

case "$enable_ec_debug" in
  yes)
    AC_DEFINE([ENABLE_EC_DEBUG], [1], [Define to 1 to enable debugging of the embedded controller])
    ;;
  *)
    AC_DEFINE([ENABLE_EC_DEBUG], [0], [Define to 1 to enable debugging of the embedded controller])
    ;;
esac

# ENABLE_EC_DUMMY - Enable the 'dummy' embedded controller
AC_ARG_ENABLE([ec-dummy],
  AS_HELP_STRING([--enable-ec-dummy],
    [Enable the dummy embedded controller (default: enabled)]),
    [enable_ec_dummy=$enableval], [enable_ec_dummy=yes])

case "$enable_ec_dummy" in
  yes)
    AC_DEFINE([ENABLE_EC_DUMMY], [1], [Define to 1 to enable the 'dummy' embedded controller])
    ;;
  *)
    AC_DEFINE([ENABLE_EC_DUMMY], [0], [Define to 1 to enable the 'dummy' embedded controller])
    ;;
esac

# ENABLE_EC_DEV_PORT - Enable the '/dev/port' embedded controller
AC_ARG_ENABLE([ec-dev-port],
  AS_HELP_STRING([--enable-ec-dev-port],
    [Enable the /dev/port embedded controller (default: enabled)]),
    [enable_ec_dev_port=$enableval], [enable_ec_dev_port=yes])

case "$enable_ec_dev_port" in
  yes)
    AC_DEFINE([ENABLE_EC_DEV_PORT], [1], [Define to 1 to enable the '/dev/port' embedded controller])
    ;;
  *)
    AC_DEFINE([ENABLE_EC_DEV_PORT], [0], [Define to 1 to enable the '/dev/port' embedded controller])
    ;;
esac

# ENABLE_EC_SYS - Enable the '/sys/kernel/debug/ec/ec0/io' embedded controller
AC_ARG_ENABLE([ec-sys],
  AS_HELP_STRING([--enable-ec-sys],
    [Enable the /sys/kernel/debug/ec/ec0/io embedded controller (default: enabled)]),
    [enable_ec_sys=$enableval], [enable_ec_sys=yes])

case "$enable_ec_sys" in
  yes)
    AC_DEFINE([ENABLE_EC_SYS], [1], [Define to 1 to enable the '/sys/kernel/debug/ec/ec0/io' embedded controller])
    ;;
  *)
    AC_DEFINE([ENABLE_EC_SYS], [0], [Define to 1 to enable the '/sys/kernel/debug/ec/ec0/io' embedded controller])
    ;;
esac

# ENABLE_EC_ACPI - Enable the '/dev/ec' embedded controller
AC_ARG_ENABLE([ec-acpi],
  AS_HELP_STRING([--enable-ec-acpi],
    [Enable the /dev/ec embedded controller (default: enabled)]),
    [enable_ec_acpi=$enableval], [enable_ec_acpi=yes])

case "$enable_ec_acpi" in
  yes)
    AC_DEFINE([ENABLE_EC_ACPI], [1], [Define to 1 to enable the '/dev/ec' embedded controller])
    ;;
  *)
    AC_DEFINE([ENABLE_EC_ACPI], [0], [Define to 1 to enable the '/dev/ec' embedded controller])
    ;;
esac

AC_SUBST([INIT_SYSTEM], [$init_system])
AC_SUBST([PACKAGE_VERSION])

AH_TOP([
#ifndef CONFIG_H_
#define CONFIG_H_
])

AH_BOTTOM([
#endif
])

AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
