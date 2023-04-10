AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
        -fpie -pie \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat \
		-I$(WORKSPACE)/vendor/qcom/opensource/bluetooth/hal/include \
		-I$(WORKSPACE)/quectel-core/ql-bluetooth/inc

AM_CXXFLAGS = \
        -fpermissive

AM_CPPFLAGS = -fpie -pie \
        -Dstrlcpy=g_strlcpy \
        -Dstrlcat=g_strlcat

requiredlibs = -lgthread-2.0 \
		 -lglib-2.0 -lpthread

library_includedir = $(pkgincludedir)

BT_TEST_sources = main.c test_adapter.c test_gatt.c

bin_PROGRAMS = BT_TEST
BT_TEST_CC = @CC@
BT_TEST_SOURCES = $(BT_TEST_sources)
BT_TEST_CFLAGS = $(AM_CFLAGS) -fPIC
BT_TEST_LDFLAGS = -lpthread -lql_bluetooth 
BT_TEST_CPPFLAGS = -fPIC $(AM_CPPFLAGS) 
BT_TEST_LDADD = $(requiredlibs)
