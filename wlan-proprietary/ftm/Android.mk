ifeq ($(call is-vendor-board-platform,QCOM),true)

# Build only if board has BT/FM/WLAN
ifeq ($(findstring true, $(BOARD_HAVE_QCOM_FM) $(BOARD_HAVE_BLUETOOTH) $(BOARD_HAS_ATH_WLAN_AR6320)),true)

LOCAL_PATH:= $(call my-dir)

BDROID_DIR:= system/bt
ifeq ($(TARGET_SUPPORTS_WEARABLES),true)
QCOM_DIR := hardware/qcom/bt/msm8909/libbt-vendor
else
QCOM_DIR := hardware/qcom/bt/libbt-vendor
endif


include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(TARGET_OUT_HEADERS)/diag/include \
LOCAL_C_INCLUDES += vendor/qcom/proprietary/diag/src \
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc \
LOCAL_C_INCLUDES += vendor/qcom/proprietary/bt/hci_qcomm_init \
LOCAL_C_INCLUDES += vendor/qcom/opensource/fm/helium \
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
LOCAL_C_INCLUDES += $(BDROID_DIR)/hci/include \
LOCAL_C_INCLUDES += $(QCOM_DIR)/include \

LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS:= \
              -DANDROID \
              -DDEBUG

#LOCAL_CFLAGS += -include bionic/libc/include/sys/socket.h
#LOCAL_CFLAGS += -include bionic/libc/include/netinet/in.h

LOCAL_CFLAGS +=  -DCONFIG_FTM_BT
ifeq ($(BOARD_HAVE_QCOM_FM),true)
LOCAL_CFLAGS +=  -DCONFIG_FTM_FM
endif

ifeq ($(BOARD_HAS_QCA_FM_SOC), "cherokee")
LOCAL_CFLAGS += -DFM_SOC_TYPE_CHEROKEE
endif

ifneq ($(BOARD_ANT_WIRELESS_DEVICE), )
LOCAL_CFLAGS +=  -DCONFIG_FTM_ANT
endif
LOCAL_CFLAGS +=  -DCONFIG_FTM_NFC
LOCAL_CFLAGS +=  -DCONFIG_FTM_NTAG

ifeq ($(BOARD_HAVE_BLUETOOTH_BLUEZ), true)
    LOCAL_CFLAGS += -DHAS_BLUEZ_BUILDCFG
endif # BOARD_HAVE_BLUETOOTH_BLUEZ

LOCAL_SRC_FILES:= \
    ftm_main.c \
    ftm_bt.c \
    ftm_bt_power_pfal_linux.c \
    ftm_bt_hci_pfal_linux.c \
    ftm_bt_persist.cpp \
    ftm_nfc.c \
    ftm_nfcnq.c \
    ftm_nfcqti.c \
    ftm_nfcnq_fwdl.c \
    ftm_ntag.c

ifeq ($(call is-platform-sdk-version-at-least,23),true)
LOCAL_CFLAGS += -DANDROID_M
endif

ifeq ($(BOARD_HAVE_QCOM_FM),true)
ifeq ($(BOARD_HAS_QCA_FM_SOC), "cherokee")
LOCAL_SRC_FILES += ftm_fm.c ftm_fm_pfal_linux_3990.c
else
LOCAL_SRC_FILES += ftm_fm.c ftm_fm_pfal_linux.c
endif
endif

ifneq ($(BOARD_ANT_WIRELESS_DEVICE), )
LOCAL_SRC_FILES += ftm_ant.c
endif

ifeq ($(findstring true, $(BOARD_HAS_ATH_WLAN) $(BOARD_HAS_ATH_WLAN_AR6320)),true)
LOCAL_CFLAGS += -DBOARD_HAS_ATH_WLAN_AR6320
LOCAL_CFLAGS +=  -DCONFIG_FTM_WLAN
LOCAL_CFLAGS +=  -DCONFIG_FTM_WLAN_AUTOLOAD
LOCAL_STATIC_LIBRARIES += libtcmd
LOCAL_SHARED_LIBRARIES += libnl
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/libtcmd
LOCAL_SRC_FILES  += ftm_wlan.c
endif

LOCAL_SHARED_LIBRARIES += libdl
LOCAL_MODULE:= ftmdaemon
LOCAL_CLANG := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES  += libdiag
LOCAL_SHARED_LIBRARIES  += libbtnv libcutils libhardware

# By default NV persist gets used
LOCAL_CFLAGS += -DBT_NV_SUPPORT

LDFLAGS += -ldl

include $(BUILD_EXECUTABLE)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif # filter
endif # is-vendor-board-platform
