//**************************************************************************************************
// Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//**************************************************************************************************

#include <cstdio>
#include <cstddef>
#include <dlfcn.h>
#include "msmgbm_adreno_utils.h"
#include "gbm_priv.h"
#include "msmgbm.h"


#define INT(exp) static_cast<int>(exp)
#define UINT(exp) static_cast<unsigned int>(exp)
#define ALIGN(x, align) (((x) + ((align)-1)) & ~((align)-1))

#ifdef GBM_DEBUG
    #define  GBM_DBG_LOG fprintf
#else
    #define  GBM_DBG_LOG(...) {}
#endif

namespace msm_gbm {

adreno_mem_info::adreno_mem_info() {
}

bool adreno_mem_info::init() {
  dlerror();   //Clear any errors

  libadreno_utils_ = ::dlopen("libadreno_utils.so", RTLD_NOW);
  if (libadreno_utils_) {
    *reinterpret_cast<void **>(&LINK_adreno_compute_fmt_aligned_width_and_height) =
        ::dlsym(libadreno_utils_, "compute_fmt_aligned_width_and_height");
    *reinterpret_cast<void **>(&LINK_adreno_compute_padding) =
        ::dlsym(libadreno_utils_, "compute_surface_padding");
    *reinterpret_cast<void **>(&LINK_adreno_is_mcro_tile_supportd_gpu) =
        ::dlsym(libadreno_utils_, "isMacroTilingSupportedByGpu");
    *reinterpret_cast<void **>(&LINK_adreno_compute_compressedfmt_aligned_width_and_height) =
        ::dlsym(libadreno_utils_, "compute_compressedfmt_aligned_width_and_height");
    *reinterpret_cast<void **>(&LINK_adreno_is_ubwc_supportd_gpu) =
        ::dlsym(libadreno_utils_, "isUBWCSupportedByGpu");
    *reinterpret_cast<void **>(&LINK_adreno_get_gpu_pixel_alignment) =
        ::dlsym(libadreno_utils_, "get_gpu_pixel_alignment");
  } else {

    char *error=NULL;
    if ((error= dlerror()) != NULL)
        LOG(LOG_ERR, "%s\n", error);
    LOG(LOG_ERR," Failed to load libadreno_utils.so\n");
    return false;
  }

  LOG(LOG_DBG,"adreno_mem_info Success\n");

  return true;
}

adreno_mem_info::~adreno_mem_info() {
  if (libadreno_utils_) {
    LOG(LOG_DBG,"Destroyer\n");
    ::dlclose(libadreno_utils_);
  }
}

bool adreno_mem_info::is_mcro_tiling_supported() {
  if (LINK_adreno_is_mcro_tile_supportd_gpu) {
    LOG(LOG_DBG," func access %p\n",LINK_adreno_is_mcro_tile_supportd_gpu);
    return LINK_adreno_is_mcro_tile_supportd_gpu();
  }

  return false;
}

void adreno_mem_info::get_aligned_wdth_hght_uncmprsd_rgb_fmt(int width,
                        int height, int format, int tile_enabled,unsigned int *aligned_w,
                                                                unsigned int *aligned_h) {

  *aligned_w = (unsigned int)ALIGN(width, 32);
  *aligned_h = (unsigned int)ALIGN(height, 32);

  // Don't add any additional padding if debug.gralloc.map_fb_memory
  // is enabled
  if (map_fb_) {
    return;
  }

  int bpp = 4;

  switch (format) {
    case GBM_FORMAT_RGB888:
      bpp = 3;
      break;
    case GBM_FORMAT_RGB565:
    case GBM_FORMAT_BGR565:
    case GBM_FORMAT_RGBA5551:
    case GBM_FORMAT_RGBA4444:
      bpp = 2;
      break;
    default:
      break;
  }

  int raster_mode = 0;          // Adreno unknown raster mode.
  int padding_threshold = 512;  // Threshold for padding surfaces.
  // the function below computes aligned width and aligned height
  // based on linear or macro tile mode selected.

  if (LINK_adreno_compute_fmt_aligned_width_and_height) {

    LOG(LOG_DBG,"Using LINK_adreno_compute_fmt_aligned_width_and_height func:= %p\n",
                                LINK_adreno_compute_fmt_aligned_width_and_height);
    ADRENOPIXELFORMAT gpu_pixel_format = get_gpu_pxl_fmt(format);
    LINK_adreno_compute_fmt_aligned_width_and_height(
        width, height, 0 /*plane_id*/, gpu_pixel_format, 1 /*num_samples */,
        tile_enabled, raster_mode, padding_threshold, reinterpret_cast<int *>(aligned_w),
        reinterpret_cast<int *>(aligned_h));
  } else if (LINK_adreno_compute_padding) {
    int surface_tile_height = 1;  // Linear surface
    LOG(LOG_DBG,"Using LINK_adreno_compute_padding func:= %p\n",LINK_adreno_compute_padding);
    *aligned_w = UINT(LINK_adreno_compute_padding(width, bpp, surface_tile_height,
                                                   raster_mode,padding_threshold));
    LOG(LOG_DBG,"Old GFX API is used to calculate stride\n");

  } else {
    LOG(LOG_ERR,"Symbols compute_surface_padding and "
        "compute_aligned_width_and_height not found\n");
  }
}

void adreno_mem_info::get_aligned_wdth_hght_cmprsd_rgb_fmt(
                        int width, int height, int format, unsigned int *aligned_w,
                                                            unsigned int *aligned_h) {
  if (LINK_adreno_compute_compressedfmt_aligned_width_and_height) {
    int bytesPerPixel = 0;
    int raster_mode = 0;          // Adreno unknown raster mode.
    int padding_threshold = 512;  // Threshold for padding
    // surfaces.

    LINK_adreno_compute_compressedfmt_aligned_width_and_height(
        width, height, format, 0, raster_mode, padding_threshold,
        reinterpret_cast<int *>(aligned_w), reinterpret_cast<int *>(aligned_h), &bytesPerPixel);
  } else {
    LOG(LOG_ERR,"compute_compressedfmt_aligned_width_and_height not found");

  }
}

bool adreno_mem_info::is_ubwc_supported_gpu(unsigned int format) {
  if (!gfx_ubwc_disable_ && LINK_adreno_is_ubwc_supportd_gpu) {
    ADRENOPIXELFORMAT gpu_format = get_gpu_pxl_fmt(format);
    return LINK_adreno_is_ubwc_supportd_gpu(gpu_format);
  }

  return false;
}

unsigned int  adreno_mem_info::get_gpu_pxl_align() {
  if (LINK_adreno_get_gpu_pixel_alignment) {
    return LINK_adreno_get_gpu_pixel_alignment();
  }

  return 1;
}

ADRENOPIXELFORMAT adreno_mem_info::get_gpu_pxl_fmt(unsigned int gbm_format) {

  switch (gbm_format) {
    case GBM_FORMAT_RGBA8888:
      return ADRENO_PIXELFORMAT_R8G8B8A8;
    case GBM_FORMAT_XBGR8888:
      return ADRENO_PIXELFORMAT_R8G8B8X8;
    case GBM_FORMAT_ABGR8888:
      return ADRENO_PIXELFORMAT_R8G8B8A8;
    case GBM_FORMAT_ABGR2101010:
      return ADRENO_PIXELFORMAT_R10G10B10A2_UNORM;
    case GBM_FORMAT_RGB565:
      return ADRENO_PIXELFORMAT_B5G6R5;
    case GBM_FORMAT_BGR565:
      return ADRENO_PIXELFORMAT_R5G6B5;
    case GBM_FORMAT_NV12_ENCODEABLE:
      return ADRENO_PIXELFORMAT_NV12;
    case GBM_FORMAT_YCbCr_420_SP_VENUS:
    case GBM_FORMAT_YCbCr_420_SP_VENUS_UBWC:
    case GBM_FORMAT_NV12:
      return ADRENO_PIXELFORMAT_NV12;
    case GBM_FORMAT_YCbCr_420_TP10_UBWC:
      return ADRENO_PIXELFORMAT_TP10;
    case GBM_FORMAT_YCbCr_420_P010_UBWC:
      return ADRENO_PIXELFORMAT_P010;
    default:
      LOG(LOG_ERR,":No map for format: 0x%x",gbm_format);
      break;
  }

  return ADRENO_PIXELFORMAT_UNKNOWN;
}

}  // namespace msm_gbm
