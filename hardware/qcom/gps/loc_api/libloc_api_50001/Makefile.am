AM_CFLAGS = \
	 $(LOCCORE_CFLAGS) \
     -I../../utils \
     -I../../platform_lib_abstractions \
     -I$(WORKSPACE)/gps-noship/flp \
     -fno-short-enums \
      -D__func__=__PRETTY_FUNCTION__ \
     -DFEATURE_GNSS_BIT_API

libloc_adapter_so_la_SOURCES = loc_eng_log.cpp LocEngAdapter.cpp

if USE_GLIB
libloc_adapter_so_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libloc_adapter_so_la_LDFLAGS = -lstdc++ -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libloc_adapter_so_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libloc_adapter_so_la_CFLAGS = $(AM_CFLAGS)
libloc_adapter_so_la_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
libloc_adapter_so_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif
libloc_adapter_so_la_LIBADD = -lstdc++ -lcutils $(GPSUTILS_LIBS)


libloc_eng_so_la_SOURCES = \
    loc.cpp \
    loc_eng.cpp \
    loc_eng_agps.cpp \
    loc_eng_xtra.cpp \
    loc_eng_ni.cpp \
    loc_eng_log.cpp \
    loc_eng_dmn_conn.cpp \
    loc_eng_dmn_conn_handler.cpp \
    loc_eng_dmn_conn_thread_helper.c \
    loc_eng_dmn_conn_glue_msg.c \
    loc_eng_dmn_conn_glue_pipe.c


if USE_GLIB
libloc_eng_so_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
libloc_eng_so_la_LDFLAGS = -lstdc++ -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libloc_eng_so_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
libloc_eng_so_la_CFLAGS = $(AM_CFLAGS)
libloc_eng_so_la_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
libloc_eng_so_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

libloc_eng_so_la_LIBADD = -lstdc++ -lcutils -ldl $(GPSUTILS_LIBS) $(LOCCORE_LIBS) $(LOCSTUB_LIBS) libloc_adapter_so.la

library_include_HEADERS = \
   LocEngAdapter.h \
   loc.h \
   loc_eng.h \
   loc_eng_xtra.h \
   loc_eng_ni.h \
   loc_eng_agps.h \
   loc_eng_msg.h \
   loc_eng_log.h

library_includedir = $(pkgincludedir)/libloc_api_50001

#Create and Install libraries
lib_LTLIBRARIES = libloc_adapter_so.la libloc_eng_so.la
