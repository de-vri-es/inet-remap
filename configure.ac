AC_PREREQ([2.69])
AC_INIT([inet_remap], [0.1], [Maarten de Vries <maarten@de-vri.es>])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/inet_remap.cpp])

AC_PROG_CXX
AM_PROG_AR

LT_INIT([disable-static])
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h sys/socket.h])
AC_SEARCH_LIBS([dlsym], [dl])
AC_CONFIG_HEADERS([src/config.h])
AX_CXX_COMPILE_STDCXX_11([noext], [mandatory])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
