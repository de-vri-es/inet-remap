ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST=src/key.hpp src/parse.hpp

bin_PROGRAMS = inet-remap
inet_remap_SOURCES = src/inet_remap.cpp src/parse.cpp
inet_remap_CXXFLAGS = $(AM_CXXFLAGS)
inet_remap_CPPFLAGS = ${AM_CPPFLAGS} -DPRELOAD_PATH=\"${libdir}/libinet-remap-preload.so\"

lib_LTLIBRARIES = libinet-remap-preload.la
libinet_remap_preload_la_SOURCES = src/inet_remap_preload.cpp src/parse.cpp
libinet_remap_preload_la_LDFLAGS = -shared -version-info 1:0:0

.PHONY: arch-dist arch-native arch-lib32
arch-dist: arch-native arch-lib32

arch-native: dist archlinux/native/PKGBUILD
	ln -sf ../../inet_remap-@VERSION@.tar.gz 'archlinux/native/inet_remap-@VERSION@.tar.gz'
	cd 'archlinux/native' && makepkg -f

arch-lib32: dist archlinux/lib32/PKGBUILD
	ln -sf ../../inet_remap-@VERSION@.tar.gz 'archlinux/lib32/inet_remap-@VERSION@.tar.gz'
	cd 'archlinux/lib32' && makepkg -f
