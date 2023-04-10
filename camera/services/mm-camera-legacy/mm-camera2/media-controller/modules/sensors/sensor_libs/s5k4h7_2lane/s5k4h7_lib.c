/*============================================================================

  Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"
#include <utils/Log.h>
#define SENSOR_MODEL_NAME "s5k4h7"
#define S5K4H7_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NAME"_"#n".so"

#define FEATURE_CAPTURE_PREVIEW_RAW_IMAGE        0
#define HorizontalMirrorAndVerticalFlip 1

/* S5K4H7 CONSTANTS */
#define S5K4H7_MAX_INTEGRATION_MARGIN   8

#define S5K4H7_DATA_PEDESTAL            0x40 /* 10bit */

#define S5K4H7_MIN_AGAIN_REG_VAL        32  /* 1.0x */
#define S5K4H7_MAX_AGAIN_REG_VAL        512 /* 16.0x */

#define S5K4H7_MIN_DGAIN_REG_VAL        256 /* 1.0x */
#define S5K4H7_MAX_DGAIN_REG_VAL        256 /* 1.0x */

/* S5K4H7 FORMULAS */
#define S5K4H7_MIN_AGAIN    (S5K4H7_MIN_AGAIN_REG_VAL / 32)
#define S5K4H7_MAX_AGAIN    (S5K4H7_MAX_AGAIN_REG_VAL / 32)

#define S5K4H7_MIN_DGAIN    (S5K4H7_MIN_DGAIN_REG_VAL / 256)
#define S5K4H7_MAX_DGAIN    (S5K4H7_MAX_DGAIN_REG_VAL / 256)

#define S5K4H7_MIN_GAIN     S5K4H7_MIN_AGAIN * S5K4H7_MIN_DGAIN
#define S5K4H7_MAX_GAIN     S5K4H7_MAX_AGAIN * S5K4H7_MAX_DGAIN

static sensor_lib_t sensor_lib_ptr;

static struct msm_sensor_power_setting power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VIO,
		.config_val = 0,
		.delay = 1,
	},
    /*
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VIO,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
    */
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VANA,
		.config_val = 0,
		.delay = 1,
	},
    /*
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
    */
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
    /*
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VDIG,
		.config_val = 0,
		.delay = 1,
	},
    */
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 24000000,
		.delay = 1,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VAF,
		.config_val = GPIO_OUT_HIGH,
		.delay = 1,
	},
    /*
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 1,
	},
    */

};

static struct msm_sensor_power_setting power_down_setting[] = {
	  {
	    .seq_type = SENSOR_I2C_MUX,
	    .seq_val = 0,
	    .config_val = 0,
	    .delay = 0,
	  },
	  {
	    .seq_type = SENSOR_GPIO,
	    .seq_val = SENSOR_GPIO_RESET,
	    .config_val = GPIO_OUT_LOW,
	    .delay = 1,
	  },
	  {
	    .seq_type = SENSOR_GPIO,
	    .seq_val = SENSOR_GPIO_STANDBY,
	    .config_val = GPIO_OUT_LOW,
	    .delay = 1,
	  },
	  {
	    .seq_type = SENSOR_CLK,
	    .seq_val = SENSOR_CAM_MCLK,
	    .config_val = 24000000,
	    .delay = 5,
	  },
	  {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	  },
      /*
	  {
	    .seq_type = SENSOR_VREG,
	    .seq_val = CAM_VDIG,
	    .config_val = 0,
	    .delay = 1,
	  },
      */
	  {
	    .seq_type = SENSOR_VREG,
	    .seq_val = CAM_VANA,
	    .config_val = 0,
	    .delay = 1,
	  },
      /*
	  {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	  },
      */
	  {
	    .seq_type = SENSOR_VREG,
	    .seq_val = CAM_VIO,
	    .config_val = 0,
	    .delay = 1,
	  },
      /*
	  {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VIO,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	  },
      */
	  {
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VAF,
		.config_val = GPIO_OUT_LOW,
		.delay = 1,
	  },
    /*
	{
		.seq_type = SENSOR_VREG,
		.seq_val = CAM_VAF,
		.config_val = 0,
		.delay = 1,
	},
    */
};

static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_0,
  /* sensor slave address */
  .slave_addr = 0x5a,
  /* sensor i2c frequency*/
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
    /* sensor id register address */
    .sensor_id_reg_addr = 0x0000,
    /* sensor id */
    .sensor_id = 0x487B,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
    .power_down_setting = power_down_setting,
    .size_down = ARRAY_SIZE(power_down_setting),
  },
  //.is_flash_supported = SENSOR_FLASH_SUPPORTED,
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = CAMERA_MODE_2D_B,
  .position = BACK_CAMERA_B,
  .sensor_mount_angle = SENSOR_MOUNTANGLE_90,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_10_BIT_DIRECT,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0x034C,
  .y_output = 0x034E,
  .line_length_pclk = 0x0342,
  .frame_length_lines = 0x0340,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x0202,
  .global_gain_addr = 0x0204,
  .vert_offset = 8,
};

static sensor_aec_data_t aec_info = {
  //.min_gain = 1.0,
  .max_gain = 16.0,
  .max_linecount = 65527,
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 2.94,
  .pix_size = 1.12,
  .f_number = 2.2,
  .total_f_dist = 1.97,
  .hor_view_angle = 75,
  .ver_view_angle = 58,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x07,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x4320,
  .csi_lane_mask = 0x07,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
   {0x0100, 0x00},
   {0x0101, 0x03},
   {0x0000, 0x12},
   {0x0000, 0x48},
   {0x0A02, 0x15},
   {0x0B05, 0x01},
   {0x3074, 0x06},
   {0x3075, 0x2F},
   {0x308A, 0x20},
   {0x308B, 0x08},
   {0x308C, 0x0B},
   {0x3081, 0x07},
   {0x307B, 0x85},
   {0x307A, 0x0A},
   {0x3079, 0x0A},
   {0x306E, 0x71},
   {0x306F, 0x28},
   {0x301F, 0x20},
   {0x306B, 0x9A},
   {0x3091, 0x1F},
   {0x30C4, 0x06},
   {0x3200, 0x09},
   {0x306A, 0x79},
   {0x30B0, 0xFF},
   {0x306D, 0x08},
   {0x3080, 0x00},
   {0x3929, 0x3F},
   {0x3084, 0x16},
   {0x3070, 0x0F},
   {0x3B45, 0x01},
   {0x30C2, 0x05},
   {0x3069, 0x87},
   {0x3924, 0x7F},
   {0x3925, 0xFD},
   {0x3C08, 0xFF},
   {0x3C09, 0xFF},
   {0x3C31, 0xFF},
   {0x3C32, 0xFF},
   {0x0b00, 0x01},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 1,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 1,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0x0100, 0x01},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0x0100, 0x00},
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupon_reg_array[] = {
  {0x0104, 0x01},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0x0104, 0x00},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg s5k4h7_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
};

static struct msm_camera_csi2_params s5k4h7_csi_params = {
    .csid_params = {
     .lane_cnt = 2,
     .lut_params = {
      .num_cid = 1,
      .vc_cfg = {
         &s5k4h7_cid_cfg[0],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 2,
    .settle_cnt = 0x14,
  },
};

static struct msm_camera_csi2_params *csi_params[] = {
  &s5k4h7_csi_params, /* RES 0*/
  &s5k4h7_csi_params, /* RES 1*/

};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
};

static struct sensor_pix_fmt_info_t s5k4h7_pix_fmt0_fourcc[] = {
  { V4L2_PIX_FMT_SGBRG10 },
 // { V4L2_PIX_FMT_SGBRG10 },
};

static struct sensor_pix_fmt_info_t s5k4h7_pix_fmt1_fourcc[] = {
  { MSM_V4L2_PIX_FMT_META },
};

static sensor_stream_info_t s5k4h7_stream_info[] = {
  {1, &s5k4h7_cid_cfg[0], s5k4h7_pix_fmt0_fourcc},
 // {1, &s5k4h7_cid_cfg[1], s5k4h7_pix_fmt1_fourcc},
};

static sensor_stream_info_array_t s5k4h7_stream_info_array = {
  .sensor_stream_info = s5k4h7_stream_info,
  .size = ARRAY_SIZE(s5k4h7_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
   {0x0136, 0x18},
   {0x0137, 0x00},
   {0x0305, 0x06},
   {0x0306, 0x00},
   {0x0307, 0x8C},
   {0x030D, 0x06},
   {0x030E, 0x00},
   {0x030F, 0xE1},
   {0x3C1F, 0x00},
   {0x3C17, 0x00},
   {0x3C1C, 0x05},
   {0x3C1D, 0x15},
   {0x0301, 0x04},
   {0x0820, 0x03},
   {0x0821, 0x84},
   {0x0822, 0x00},
   {0x0823, 0x00},
   {0x0112, 0x0A},
   {0x0113, 0x0A},
   {0x0114, 0x01},
   {0x3906, 0x04},
   {0x0344, 0x00},
   {0x0345, 0x08},
   {0x0346, 0x00},
   {0x0347, 0x08},
   {0x0348, 0x0C},
   {0x0349, 0xC7},
   {0x034A, 0x09},
   {0x034B, 0x97},
   {0x034C, 0x0C},
   {0x034D, 0xC0},
   {0x034E, 0x09},
   {0x034F, 0x90},
   {0x0900, 0x00},
   {0x0901, 0x00},
   {0x0381, 0x01},
   {0x0383, 0x01},
   {0x0385, 0x01},
   {0x0387, 0x01},
   {0x0340, 0x09},
   {0x0341, 0xE4},
   {0x0342, 0x14},
   {0x0343, 0x8C},
   {0x0200, 0x13},
   {0x0201, 0xFC},
   {0x0202, 0x00},
   {0x0203, 0x02},
   {0x3400, 0x01},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
   {0x0100, 0x00},
   {0x0136, 0x18},
   {0x0137, 0x00},
   {0x0305, 0x06},
   {0x0306, 0x00},
   {0x0307, 0x8C},
   {0x030D, 0x06},
   {0x030E, 0x00},
   {0x030F, 0xE1},
   {0x3C1F, 0x00},
   {0x3C17, 0x00},
   {0x3C1C, 0x05},
   {0x3C1D, 0x15},
   {0x0301, 0x04},
   {0x0820, 0x03},
   {0x0821, 0x84},
   {0x0822, 0x00},
   {0x0823, 0x00},
   {0x0112, 0x0A},
   {0x0113, 0x0A},
   {0x0114, 0x01},
   {0x3906, 0x04},
   {0x0344, 0x02},
   {0x0345, 0xA8},
   {0x0346, 0x02},
   {0x0347, 0xB4},
   {0x0348, 0x0A},
   {0x0349, 0x27},
   {0x034A, 0x06},
   {0x034B, 0xEB},
   {0x034C, 0x07},
   {0x034D, 0x80},
   {0x034E, 0x04},
   {0x034F, 0x38},
   {0x0900, 0x00},
   {0x0901, 0x00},
   {0x0381, 0x01},
   {0x0383, 0x01},
   {0x0385, 0x01},
   {0x0387, 0x01},
   {0x0340, 0x04},
   {0x0341, 0xF0},
   {0x0342, 0x0E},
   {0x0343, 0x68},
   {0x0200, 0x0D},
   {0x0201, 0xD8},
   {0x0202, 0x02},
   {0x0203, 0x08},
   {0x3400, 0x01},
};



static struct msm_camera_i2c_reg_setting res_settings[] = {
  {
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },

  {
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_WORD_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },

};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct sensor_crop_parms_t crop_params[] = {
  {0, 0, 0, 0}, /* RES 0 */
  {0, 0, 0, 0}, /* RES 1 */

};

static struct sensor_lib_crop_params_array crop_params_array = {
  .crop_params = crop_params,
  .size = ARRAY_SIZE(crop_params),
};

static struct sensor_lib_out_info_t sensor_out_info[] = {
  { /* snapshot 20 fps full settings */
        .x_output = 3264,
        .y_output = 2448,
        .line_length_pclk = 5260,
        .frame_length_lines = 2532,
        .vt_pixel_clk = 400000000, //118400000, //236800000,
        .op_pixel_clk = 280000000,
        .binning_factor = 1,
        .min_fps = 7.5,
        .max_fps = 30,//actually only 21,for 2 lane mipi only support 21fps at fill size
        .mode = SENSOR_DEFAULT_MODE,
  },
#if 0
 { /* preview 30.0 fps qtr size settings*/
        .x_output = 1920,
        .y_output = 1080,
        .line_length_pclk = 3688,
        .frame_length_lines = 1264,
        .vt_pixel_clk = 400000000,
        .op_pixel_clk = 280000000,
        .binning_factor = 1,
        .min_fps = 7.5,
        .max_fps = 60.06,
        .mode = SENSOR_HFR_MODE,
  },
#endif
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t s5k4h7_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t s5k4h7_res_table = {
  .res_cfg_type = s5k4h7_res_cfg,
  .size = ARRAY_SIZE(s5k4h7_res_cfg),
};

static struct sensor_lib_chromatix_t s5k4h7_chromatix[] = {
  {
    .common_chromatix = S5K4H7_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K4H7_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = S5K4H7_LOAD_CHROMATIX(snapshot), /* RES0 */
    .camcorder_chromatix = S5K4H7_LOAD_CHROMATIX(default_video), /* RES0 */
    .liveshot_chromatix = S5K4H7_LOAD_CHROMATIX(preview), /* RES0 */
  },
#if 0
  {
    .common_chromatix = S5K4H7_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = S5K4H7_LOAD_CHROMATIX(preview), /* RES2 */
    .camera_snapshot_chromatix = S5K4H7_LOAD_CHROMATIX(snapshot), /* RES2 */
    .camcorder_chromatix = S5K4H7_LOAD_CHROMATIX(snapshot), /* RES2 */
    .liveshot_chromatix = S5K4H7_LOAD_CHROMATIX(preview), /* RES2 */
  },
#endif
};

static struct sensor_lib_chromatix_array s5k4h7_lib_chromatix_array = {
  .sensor_lib_chromatix = s5k4h7_chromatix,
  .size = ARRAY_SIZE(s5k4h7_chromatix),
};

/*===========================================================================
 * FUNCTION    - s5k4h7_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t s5k4h7_real_to_register_gain(float real_gain)
{
    unsigned int reg_gain = 0;

    if (real_gain < S5K4H7_MIN_AGAIN)
    {
        real_gain = S5K4H7_MIN_AGAIN;
    }
    else if (real_gain > S5K4H7_MAX_AGAIN)
    {
        real_gain = S5K4H7_MAX_AGAIN;
    }

    reg_gain = (unsigned int)(real_gain * 32.0);

    return reg_gain;
}

/*===========================================================================
 * FUNCTION    - s5k4h7_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float s5k4h7_register_to_real_gain(uint16_t reg_gain)
{
    float real_gain;

    if (reg_gain > S5K4H7_MAX_AGAIN_REG_VAL)
    {
        reg_gain = S5K4H7_MAX_AGAIN_REG_VAL;
    }

    real_gain = (float)reg_gain / 32.0;

    return real_gain;
}

static unsigned int s5k4h7_digital_gain_calc(float real_gain, float sensor_real_gain)
{
    unsigned int reg_dig_gain = S5K4H7_MIN_DGAIN_REG_VAL;
    float real_dig_gain = S5K4H7_MIN_DGAIN;

    if(real_gain > S5K4H7_MAX_AGAIN)
    {
        real_dig_gain = real_gain / sensor_real_gain;
    }
    else
    {
        real_dig_gain = S5K4H7_MIN_DGAIN;
    }

    if(real_dig_gain > S5K4H7_MAX_DGAIN)
    {
        real_dig_gain = S5K4H7_MAX_DGAIN;
    }

    reg_dig_gain = (unsigned int)(real_dig_gain * 256);

    return reg_dig_gain;
}

/*===========================================================================
 * FUNCTION    - s5k4h7_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k4h7_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
    if (!exp_info)
    {
      return -1;
    }

    exp_info->reg_gain = s5k4h7_real_to_register_gain(real_gain);
     exp_info->sensor_real_gain = s5k4h7_register_to_real_gain(exp_info->reg_gain);
 //    float sensor_real_gain = s5k4h7_register_to_real_gain(exp_info->reg_gain);
/*   exp_info->sensor_digital_gain =
      s5k4h7_digital_gain_calc(real_gain, exp_info->sensor_real_gain);
    exp_info->sensor_real_dig_gain = exp_info->sensor_digital_gain / 256;
    exp_info->digital_gain = real_gain/
      (exp_info->sensor_real_gain * exp_info->sensor_real_dig_gain);*/
    exp_info->digital_gain = real_gain / exp_info->sensor_real_gain;
    exp_info->line_count = line_count;
    exp_info->sensor_digital_gain = 0x1;
    return 0;
}

/*===========================================================================
 * FUNCTION    - s5k4h7_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t s5k4h7_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines, int32_t luma_avg, uint32_t fgain,
        struct msm_camera_i2c_reg_setting *reg_setting)
{
    uint16_t i = 0;
    uint16_t reg_count = 0;

    if (!reg_setting)
    {
      return -1;
    }

    for (i = 0; i < sensor_lib_ptr.groupon_settings->size; i++)
    {
        reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupon_settings->reg_setting[i].reg_addr;
        reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupon_settings->reg_setting[i].reg_data;
        reg_count = reg_count + 1;
    }

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.output_reg_addr->frame_length_lines + 1;
    reg_setting->reg_setting[reg_count].reg_data = (fl_lines & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x0000FF00) >> 8;
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    reg_setting->reg_setting[reg_count].reg_addr =
    sensor_lib_ptr.exp_gain_info->global_gain_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = (gain & 0x000000FF);
    reg_setting->reg_setting[reg_count].delay = 0;
    reg_count = reg_count + 1;

    for (i = 0; i < sensor_lib_ptr.groupoff_settings->size; i++)
    {
        reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_addr;
        reg_setting->reg_setting[reg_count].reg_data =
        sensor_lib_ptr.groupoff_settings->reg_setting[i].reg_data;
        reg_count = reg_count + 1;
    }

    reg_setting->size = reg_count;
    reg_setting->addr_type = MSM_CAMERA_I2C_WORD_ADDR;
    reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
    reg_setting->delay = 0;

    return 0;
}

static sensor_exposure_table_t s5k4h7_expsoure_tbl = {
  .sensor_calculate_exposure = s5k4h7_calculate_exposure,
  .sensor_fill_exposure_array = s5k4h7_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
   /* sensor eeprom name */

  /* sensor actuator name */
   .actuator_name = "dw9714",
   //.eeprom_name = "s5k4h7",
 /* sensor output settings */
  .sensor_output = &sensor_output,
  /* sensor output register address */
  .output_reg_addr = &output_reg_addr,
  /* sensor exposure gain register address */
  .exp_gain_info = &exp_gain_info,
  /* sensor aec info */
  .aec_info = &aec_info,
  /* sensor snapshot exposure wait frames info */
  .snapshot_exp_wait_frames = 1,
  /* number of frames to skip after start stream */
  .sensor_num_frame_skip = 1, //1,
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2,
  /* sensor exposure table size */
  .exposure_table_size = 9,// 8,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = s5k4h7_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(s5k4h7_cid_cfg),
  /* init settings */
  .init_settings_array = &init_settings_array,
  /* start settings */
  .start_settings = &start_settings,
  /* stop settings */
  .stop_settings = &stop_settings,
  /* group on settings */
  .groupon_settings = &groupon_settings,
  /* group off settings */
  .groupoff_settings = &groupoff_settings,
  /* resolution cfg table */
  .sensor_res_cfg_table = &s5k4h7_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &s5k4h7_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &s5k4h7_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &s5k4h7_lib_chromatix_array,
  //.use_otp = 1,
  //.multi_module = 1,
};

/*===========================================================================
 * FUNCTION    - s5k4h7_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *s5k4h7_open_lib(void)
{
  return &sensor_lib_ptr;
}

