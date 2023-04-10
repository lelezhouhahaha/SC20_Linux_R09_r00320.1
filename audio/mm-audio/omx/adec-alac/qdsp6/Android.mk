ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE)),true)
ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

# ---------------------------------------------------------------------------------
#                 Common definitons
# ---------------------------------------------------------------------------------

libOmxAlacDec-def := -g -O3
libOmxAlacDec-def += -DQC_MODIFIED
libOmxAlacDec-def += -D_ANDROID_
libOmxAlacDec-def += -D_ENABLE_QC_MSG_LOG_
libOmxAlacDec-def += -DAUDIOV2
libOmxAlacDec-def += -Wconversion
ifeq ($(call is-board-platform-in-list,msm8996 msm8937 msm8953 msm8998 apq8098_latv sdm660 sdm845 sdm710 qcs605 msmnile $(MSMSTEPPE)),true)
libOmxAlacDec-def += -DQCOM_AUDIO_USE_SYSTEM_HEAP_ID
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_NT_PAUSE_TIMEOUT)),true)
libOmxAlacDec-def += -DNT_PAUSE_TIMEOUT_ENABLED
endif

# ---------------------------------------------------------------------------------
#             Make the Shared library (libOmxAlacDec)
# ---------------------------------------------------------------------------------

include $(CLEAR_VARS)

libOmxAlacDec-inc        := $(LOCAL_PATH)/inc
ifeq ($(call is-board-platform-in-list,msmnile $(MSMSTEPPE)),true)
libOmxAlacDec-inc        += system/core/libion/include
libOmxAlacDec-inc        += system/core/libion/kernel-headers
endif
libOmxAlacDec-inc        += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
libOmxAlacDec-inc	 += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
libOmxAlacDec-inc        += $(TARGET_OUT_HEADERS)/mm-core/omxcore
libOmxAlacDec-inc        += $(TARGET_OUT_HEADERS)/common/inc

LOCAL_ADDITIONAL_DEPENDENCIES  := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
  libOmxAlacDec-inc += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
  LOCAL_ADDITIONAL_DEPENDENCIES += $(BOARD_VENDOR_KERNEL_MODULES)
endif

LOCAL_MODULE            := libOmxAlacDec
LOCAL_MODULE_TAGS       := optional
LOCAL_CFLAGS            := $(libOmxAlacDec-def)
LOCAL_C_INCLUDES        := $(libOmxAlacDec-inc)
LOCAL_PRELINK_MODULE    := false
LOCAL_SHARED_LIBRARIES  := libutils liblog
ifeq ($(call is-board-platform-in-list,msmnile $(MSMSTEPPE)),true)
LOCAL_SHARED_LIBRARIES  += libion
endif

LOCAL_SRC_FILES         := src/adec_svr.c
LOCAL_SRC_FILES         += src/omx_alac_adec.cpp

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
endif #TARGET_BOARD_PLATFORM
# ---------------------------------------------------------------------------------
#                     END
# ---------------------------------------------------------------------------------

