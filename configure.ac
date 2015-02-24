AC_PREREQ([2.69])
AC_INIT([inet_remap], [0.1], [maarten@de-vri.es])
AC_CONFIG_MACRO_DIRS([m4])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_SRCDIR([src/inet_remap.cpp])

AC_PROG_CXX
AM_PROG_AR

LT_INIT()
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h sys/socket.h])
AC_SEARCH_LIBS([dlsym], [dl])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
