#======================================================================
#makefile for    -lmmcamera2_isp2_bf_stats47.so form mm-camera2
#======================================================================
EXTRA_CFLAGS =
local_ldflags = $(mmcamera_debug_lflags)
AM_CFLAGS =  -DAMSS_VERSION=$(AMSS_VERSION) \
  $(mmcamera_debug_defines) \
  $(mmcamera_debug_cflags)
AM_CFLAGS  += -Werror
mmcamerapath = $(srcdir)/../../../../../../../
EXTRA_CFLAGS += -I$(srcdir)
EXTRA_CFLAGS += -I$(mmcamerapath)/includes/
EXTRA_CFLAGS += -I$(mmcamerapath)/server-tuning/tuning/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/includes/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/bus/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/controller/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/event/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/module/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/object/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/pipeline/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/port/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/stream/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/mct/tools/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/includes/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/common/
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/sub_module
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler
EXTRA_CFLAGS += -I$(CAMERAHAL_PATH)/QCamera2/stack/common
if ENABLE_msm8996
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp47
endif
if ENABLE_msm8952
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp41
endif
if ENABLE_msm8937
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include42
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp42
endif
if ENABLE_msm8953
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include42
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include41
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp42
endif
if ENABLE_msmcobalt
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include48
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp48
endif
if ENABLE_sdm660
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include48
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp48
endif
if ENABLE_msm8998
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_gamma/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_down_scaler/include47
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/hw/stats/bf_stats/bf_stats47/include48
EXTRA_CFLAGS += -I$(mmcamerapath)/media-controller/modules/isp2/module/isp48
endif
local_src_files        = module_bf_stats47.c bf_stats47.c
if ENABLE_msm8952
local_src_files        += bf_stats_ext47.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_msm8937
local_src_files        += bf_stats_ext47.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_msm8953
local_src_files        += bf_stats_ext47.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_msm8996
local_src_files        += bf_stats_ext47.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_msmcobalt
local_src_files        += include48/bf_stats_ext48.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_sdm660
local_src_files        += include48/bf_stats_ext48.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
if ENABLE_msm8998
local_src_files        += include48/bf_stats_ext48.c ../bf_gamma/bf_gamma.c ../bf_down_scaler/bf_down_scaler.c
endif
local_libadd =    -lcutils -lmmcamera_dbg #\
    #-lmmcamera2_mct \
    #-lmmcamera_isp_sub_module \
    #-lmmcamera_dbg \
    #-lmmcamera2_isp_modules
if ENABLE_MM_DEBUG
local_libadd +=    -llog
endif
final_sources=
final_libadd=
if ENABLE_msm8952
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_msm8937
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_msm8953
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_msm8996
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_msmcobalt
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_sdm660
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
if ENABLE_msm8998
final_sources+=$(local_src_files)
final_libadd+=$(local_libadd)
endif
libsdir = $(prefix)/lib
libs_LTLIBRARIES  =libmmcamera_isp_bf_stats47.la
libmmcamera_isp_bf_stats47_la_SOURCES = $(final_sources)
libmmcamera_isp_bf_stats47_la_LIBADD = $(final_libadd)
libmmcamera_isp_bf_stats47_la_LDFLAGS = -shared ../../../../../../mct/libmmcamera2_mct.la ../../../../module/libmmcamera2_isp_modules.la ../../../sub_module/libmmcamera_isp_sub_module.la
libmmcamera_isp_bf_stats47_la_CFLAGS = ${AM_CFLAGS} ${EXTRA_CFLAGS}
