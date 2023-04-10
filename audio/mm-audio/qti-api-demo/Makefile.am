pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qti-api-demo.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CPPFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
	    -Wno-sign-compare \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qti-audio-server
AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qahw/inc
AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qahw_api/inc
AM_CPPFLAGS += -I ${WORKSPACE}/audio/mm-audio/qap_wrapper/inc/
AM_CPPFLAGS += -I $(WORKSPACE)/hardware/qcom/audio/qahw/inc
AM_CPPFLAGS += -I $(WORKSPACE)/hardware/qcom/audio/qahw_api/inc

bin_PROGRAMS = audio_test 

audio_test_SOURCES = audio_test.c
audio_test_LDADD = -ltinyalsa -lhardware -lexpat -lcutils -ldl 
audio_test_CFLAGS = -Dstrlcat=g_strlcat 
audio_test_CPPFLAGS = $(AM_CPPFLAGS)
audio_test_CPPFLAGS += -DNDEBUG
audio_test_CPPFLAGS += -std=c++11 -DHAVE_PTHREADS -DHAVE_ANDROID_OS
audio_test_CPPFLAGS += -DDEBUG_REFS_CALLSTACK_ENABLED=0
audio_test_LDFLAGS = -shared -module -avoid-version -lpthread -lutils
audio_test_LDADD += -llog -lutils -lm -lql_audio_api -lpthread -lqahw
AM_CPPFLAGS += -DQTI_AUDIO_SERVER_ENABLED
 
