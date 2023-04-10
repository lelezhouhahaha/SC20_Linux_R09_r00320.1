HI843_8909_CHROMATIX_PREVIEW_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_HI843_8909_preview)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(HI843_8909_CHROMATIX_PREVIEW_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS:= -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../../common/
LOCAL_C_INCLUDES += chromatix_hi843_8909_preview.h

LOCAL_SRC_FILES:= chromatix_hi843_8909_preview.c

LOCAL_MODULE           := libchromatix_hi843_8909_preview
LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)
