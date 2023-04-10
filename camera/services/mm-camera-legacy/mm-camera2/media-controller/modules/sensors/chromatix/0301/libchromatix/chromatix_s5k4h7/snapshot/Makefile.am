CURR_PATH = $(top_srcdir)/mm-camera2/media-controller/modules/sensors
MM2_PATH = $(top_srcdir)/mm-camera2

lib_LTLIBRARIES = libchromatix_s5k4h7_snapshot.la

libchromatix_s5k4h7_snapshot_la_CFLAGS = -DAMSS_VERSION=$(AMSS_VERSION)
#libchromatix_s5k4h7_snapshot_la_CFLAGS += -fPIC

libchromatix_s5k4h7_snapshot_la_CFLAGS += -I $(CURR_PATH)/chromatix/0301
libchromatix_s5k4h7_snapshot_la_CFLAGS += -I $(CURR_PATH)/module
libchromatix_s5k4h7_snapshot_la_CFLAGS += -I $(MM2_PATH)/../common
libchromatix_s5k4h7_snapshot_la_CFLAGS += -I $(CURR_PATH)/chromatix/0301/chromatix_s5k4h7/snapshot

libchromatix_s5k4h7_snapshot_la_SOURCES = chromatix_s5k4h7_snapshot.c

libchromatix_s5k4h7_snapshot_la_LDFLAGS = -shared
libchromatix_s5k4h7_snapshot_la_LIBADD = -ldl
