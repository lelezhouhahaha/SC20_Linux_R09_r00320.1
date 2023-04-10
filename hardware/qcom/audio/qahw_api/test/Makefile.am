lib_LTLIBRARIES = libql_audio_api.la

PLAY_CFLAGS = -Wno-sign-compare -Werror
PLAY_CFLAGS += -D__unused=__attribute__\(\(__unused__\)\)
PLAY_INCLUDES = -I $(top_srcdir)/qahw_api/inc
PLAY_INCLUDES += -I $(top_srcdir)/qahw/inc

libql_audio_api_la_SOURCES = qahw_playback_test.c \
                        qahw_effect_test.c\
                        qahw_multi_record_test.c

libql_audio_api_la_LIBADD = -lutils ../libqahw.la
AM_CPPFLAGS = -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qahw/inc

library_include_HEADERS = qlaudio_api.h
library_includedir = $(pkgincludedir)

if QAP
AM_CPPFLAGS += -DQAP
AM_CPPFLAGS += -I ${WORKSPACE}/audio/mm-audio/qap_wrapper/inc/
libql_audio_api_la_SOURCES += qap_wrapper_extn.c
libql_audio_api_la_LIBADD += -lqap_wrapper
endif
AM_CPPFLAGS = -D__unused=__attribute__\(\(__unused__\)\)
libql_audio_api_la_CFLAGS = $(PLAY_CFLAGS) $(PLAY_INCLUDES)
libql_audio_api_la_CFLAGS += $(AM_CFLAGS) -DLINUX_ENABLED

libql_audio_api_la_CFLAGS += -Dstrlcat=g_strlcat $(GLIB_CFLAGS) -include glib.h
libql_audio_api_la_CFLAGS += $(REC_CFLAGS) $(REC_INCLUDES)
libql_audio_api_la_CFLAGS += $(AM_CFLAGS)
libql_audio_api_la_LIBADD += -lutils ../libqahw.la $(GLIB_LIBS)
libql_audio_api_la_CPPFLAGS := $(AM_CPPFLAGS)
libql_audio_api_la_CPPFLAGS =  -DHAVE_PTHREADS -DHAVE_ANDROID_OS
libql_audio_api_la_CPPFLAGS += -DDEBUG_REFS_CALLSTACK_ENABLED=0
libql_audio_api_la_CPPFLAGS += -DNDEBUG

libql_audio_api_la_LDFLAGS   = -ltinyalsa -lhardware -lexpat -lcutils -llog -ldl -shared -avoid-version -lpthread -lutils
if QTI_AUDIO_SERVER_ENABLED
AM_CPPFLAGS = -DQTI_AUDIO_SERVER_ENABLED
libql_audio_api_la_LDFLAGS +=  -lqtiaudioserver -lbinder
endif



#bin_PROGRAMS = trans_loopback_test

#trans_loopback_test_INCLUDES = -I $(top_srcdir)/qahw_api/inc

#trans_loopback_test_SOURCES = trans_loopback_test.c

#trans_loopback_test_CFLAGS  = $(AM_CFLAGS) -Wno-sign-compare -Werror
#trans_loopback_test_CFLAGS  += $(trans_loopback_test_INCLUDES)
#trans_loopback_test_LDADD = -llog  -lutils ../libqahw.la -lcutils -lm libql_audio_api.la
