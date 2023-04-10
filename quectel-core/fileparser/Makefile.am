EXTRA_DIST = README.md
CLEANFILES =

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

#pkgconfigdir = $(libdir)/pkgconfig
#pkgconfig_DATA = libbinaryfile.pc
#EXTRA_DIST += libbinaryfile.pc.in
#CLEANFILES += libbinaryfile.pc

SUBDIRS = src util

CLEANFILES += *~

clean-local:
	rm -rf autom4te.cache
