noinst_PROGRAMS = \
				  hextest \
				  iniexample \
				  parse

common_ldflags = \
				 ${top_builddir}/src/libfileparser.la

hextest_SOURCE = hextest.c
hextest_LDADD = $(common_ldflags)

iniexample_SOURCE = iniexample.c
iniexample_LDADD = $(common_ldflags)

parse_SOURCE = parse.c
parse_LDADD = $(common_ldflags)

AM_CPPFLAGS = \
			  -D_FILE_OFFSET_BITS=64 \
			  -DSO_RXQ_OVFL=40 \
			  -include $(top_builddir)/config.h \
			  -DSYSCONFDIR=\""$(sysconfdir)"\" \
			  -DLIBEXECDIR=\""$(libexecdir)"\" \
			  -I${top_srcdir}/src \
			  -I${top_builddir}/src

AM_CFLAGS = ${my_CFLAGS}
AM_CFLAGS += --static
AM_LDFLAGS = -static

CLEANFILES = *~
