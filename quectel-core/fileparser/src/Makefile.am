EXTRA_DIST =

#pkginclude_HEADERS = hexparser.h iniparser.h dictionary.h version.h
lib_LTLIBRARIES = libfileparser.la

AM_CPPFLAGS = \
			  -D_FILE_OFFSET_BITS=64 \
			  -DSO_RXQ_OVFL=40 \
			  -include $(top_builddir)/config.h \
			  -DSYSCONFDIR=\""$(sysconfdir)"\" \
			  -DLIBEXECDIR=\""$(libexecdir)"\" \
			  -I${top_srcdir}/src

AM_CFLAGS = ${my_CFLAGS}

libfileparser_la_SOURCES = \
					hexparser.c \
					iniparser.c \
					dictionary.c \
					hexparser.h \
					iniparser.h \
					dictionary.h \
					version.h

libfileparser_la_LDFLAGS = -no-undefined

libfileparserincludedir = $(includedir)
libfileparserinclude_HEADERS = hexparser.h \
							   iniparser.h \
							   dictionary.h \
							   version.h

DISTCLEANFILES = version.h
EXTRA_DIST += version.h.in
CLEANFILES = *~
