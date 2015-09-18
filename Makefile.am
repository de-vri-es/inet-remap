ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = inet-remap
inet_remap_SOURCES = src/inet_remap.cpp src/parse.cpp
inet_remap_CXXFLAGS = $(AM_CXXFLAGS)
inet_remap_CPPFLAGS = ${AM_CPPFLAGS} -DPRELOAD_PATH=\"${libdir}/libinet-remap-preload.so\"

lib_LTLIBRARIES = libinet-remap-preload.la
libinet_remap_preload_la_SOURCES = src/inet_remap_preload.cpp src/parse.cpp
libinet_remap_preload_la_LDFLAGS = -shared
