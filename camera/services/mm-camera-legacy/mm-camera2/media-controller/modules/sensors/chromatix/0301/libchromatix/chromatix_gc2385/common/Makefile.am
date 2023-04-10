# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_gc2385_common)
# ---------------------------------------------------------------------------
#libchromatix_gc2385_common

CURR_PATH = $(top_srcdir)/mm-camera2/media-controller/modules/sensors
MM2_PATH = $(top_srcdir)/mm-camera2

lib_LTLIBRARIES = libchromatix_gc2385_common.la

libchromatix_gc2385_common_la_CFLAGS = -DAMSS_VERSION=$(AMSS_VERSION)
#libchromatix_gc2385_common_la_CFLAGS += -fPIC

libchromatix_gc2385_common_la_CFLAGS += -I $(CURR_PATH)/chromatix/0301/
libchromatix_gc2385_common_la_CFLAGS += -I $(CURR_PATH)/module
libchromatix_gc2385_common_la_CFLAGS += -I $(MM2_PATH)/../common
libchromatix_gc2385_common_la_CFLAGS += -I $(CURR_PATH)/chromatix/0301/chromatix_gc2385/common

#TODO:
#LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
#LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

libchromatix_gc2385_common_la_SOURCES = chromatix_gc2385_common.c

#LOCAL_SHARED_LIBRARIES := libcutils
libchromatix_gc2385_common_la_LDFLAGS = -shared
libchromatix_gc2385_common_la_LIBADD = -ldl -lcutils
