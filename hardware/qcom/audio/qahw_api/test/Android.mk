LOCAL_PATH := $(call my-dir)

# 
# ==============================================================================
include $(CLEAR_VARS)
LOCAL_SRC_FILES := qahw_playback_test.c \
                   qahw_effect_test.c\
                    qahw_multi_record_test.c
LOCAL_MODULE := libql_audio_api

hal-play-inc     = $(TARGET_OUT_HEADERS)/mm-audio/qahw_api/inc
hal-play-inc    += $(TARGET_OUT_HEADERS)/mm-audio/qahw/inc
hal-play-inc    += external/tinyalsa/include

LOCAL_CFLAGS += -Wall -Werror -Wno-sign-compare

LOCAL_SHARED_LIBRARIES := \
    libaudioutils\
    libqahw \
    libqahwwrapper \
    libutils \
    libcutils

LOCAL_32_BIT_ONLY := true

LOCAL_C_INCLUDES += $(hal-play-inc)

LOCAL_PRELINK_MODULE    := false
LOCAL_VENDOR_MODULE     := true

include $(BUILD_SHARED_LIBRARY)


