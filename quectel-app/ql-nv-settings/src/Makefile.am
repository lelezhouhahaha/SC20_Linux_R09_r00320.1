AM_CFLAGS = -Wundef \
			-Wstrict-prototypes \
			-Wno-trigraphs \
			-g -O0 \
			-fno-inline \
			-fno-short-enums \
			-fpic \
			-fpie -pie \
			-Dstrlcpy=g_strlcpy \
			-Dstrlcat=g_strlcat
AM_CXXFLAGS = \
			  $(AM_CFLAGS) \
			  -fpermissive

AM_CPPFLAGS = -D__packed__= \
			-fpie -pie \
			-Dstrlcpy=g_strlcpy \
			-Dstrlcat=g_strlcat


#######################################
requiredlibs =  -lrt -lql_misc

library_includedir = $(pkgincludedir)

ql_nv_settings_sources = main.c

bin_PROGRAMS = ql-nv-settings 
ql_nv_settings_CC = @CC@
ql_nv_settings_SOURCES = $(ql_nv_settings_sources)
ql_nv_settings_CFLAGS = -DUSE_GLIB $(AM_CFLAGS)
ql_nv_settings_LDFLAGS = -lpthread
ql_nv_settings_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS)
ql_nv_settings_LDADD = $(requiredlibs)
