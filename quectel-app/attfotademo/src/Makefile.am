#SUBDIRS=client core shared
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

AM_CPPFLAGS = -fpie -pie \
		-Dstrlcpy=g_strlcpy \
		-Dstrlcat=g_strlcat \
		-I$(WORKSPACE)/quectel-core/ql-mcm-api/inc \
		-I$(WORKSPACE)/quectel-core/ql-ota/include
        
#AM_CPPFLAGS = -I$(WORKSPACE)/quectel-core/ql-mbedtls/library \
#              -I$(WORKSPACE)/quectel-core/ql-mbedtls/include/
#-lmbedtls -lmbedx509 -lmbedcrypto 

requiredlibs = -lgthread-2.0 \
		 -lglib-2.0 -lpthread \
		 -lql_power \
		 -lql_factory_reset\
		 $(QLMCMAPI_LIBS) \
		 -lql_ota \
		 -lcurl \
		 -lql_mcm_api

library_include_INCLUDES = $(QLMCMAPI_CFLAGS) 
library_includedir = $(pkgincludedir)


attfotademo_sources =core/attfotademo.c
	
#QCMAP_CLI_h_sources =

bin_PROGRAMS = attfotademo 
attfotademo_CC = @CC@
attfotademo_SOURCES = $(attfotademo_sources)
#if USE_GLIB
attfotademo_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) #@GLIB_CFLAGS@
attfotademo_LDFLAGS = -lpthread $(XMLLIB_LIBS) #@GLIB_LIBS@
attfotademo_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) #@GLIB_CFLAGS@
#else
attfotademo_CFLAGS = $(AM_CFLAGS) -fPIC
attfotademo_LDFLAGS = -lpthread $(XMLLIB_LIBS) 
attfotademo_CPPFLAGS = $(AM_CPPFLAGS)
#endif
attfotademo_LDADD = $(requiredlibs)
