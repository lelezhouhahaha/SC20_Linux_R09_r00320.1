/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <stdio.h>
#include "sensor_lib.h"
#include "camera_dbg.h"
//#include <utils/Log.h>

#define SENSOR_MODEL_NO_GC2385 "gc2385"
#define GC2385_LOAD_CHROMATIX(n) \
  "libchromatix_"SENSOR_MODEL_NO_GC2385"_"#n".so"

#define IMAGE_NORMAL_MIRROR
//#define IMAGE_H_MIRROR
//#define IMAGE_V_MIRROR
//#define IMAGE_HV_MIRROR

#ifdef IMAGE_NORMAL_MIRROR
#define MIRROR 0xd4
#define STARTY 0x03
#define STARTX 0x05
#define BLK_Select_H 0x3c
#define BLK_Select_L 0x00
#endif

#ifdef IMAGE_H_MIRROR
#define MIRROR 0xd5
#define STARTY 0x03
#define STARTX 0x06
#define BLK_Select_H 0x3c
#define BLK_Select_L 0x00
#endif

#ifdef IMAGE_V_MIRROR
#define MIRROR 0xd6
#define STARTY 0x04
#define STARTX 0x05
#define BLK_Select_H 0x00
#define BLK_Select_L 0x3c
#endif

#ifdef IMAGE_HV_MIRROR
#define MIRROR 0xd7
#define STARTY 0x04
#define STARTX 0x06
#define BLK_Select_H 0x00
#define BLK_Select_L 0x3c
#endif

#define ANALOG_GAIN_1 64  // 1.00x
#define ANALOG_GAIN_2 92  // 1.43x
#define ANALOG_GAIN_3 127 // 1.99x
#define ANALOG_GAIN_4 183 // 2.86x
#define ANALOG_GAIN_5 257 // 4.01x
#define ANALOG_GAIN_6 369 // 5.76x
#define ANALOG_GAIN_7 531 // 8.30x
#define ANALOG_GAIN_8 750 // 11.72x
#define ANALOG_GAIN_9 1092 // 17.06x

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
	   .delay = 5,
	 },
	 {
	   .seq_type = SENSOR_VREG,
	   .seq_val = CAM_VANA,
	   .config_val = 0,
	   .delay = 5,
	 },
	 {
	   .seq_type = SENSOR_CLK,
	   .seq_val = SENSOR_CAM_MCLK,
	   .config_val = 24000000,
	   .delay = 5,
	 },
	 {
	   .seq_type = SENSOR_GPIO,
	   .seq_val = SENSOR_GPIO_STANDBY,
	   .config_val = GPIO_OUT_HIGH,
	   .delay = 10,
	 },
	 {
	   .seq_type = SENSOR_GPIO,
	   .seq_val = SENSOR_GPIO_RESET,
	   .config_val = GPIO_OUT_HIGH,
	   .delay = 2,
	 },
	 {
	   .seq_type = SENSOR_I2C_MUX,
	   .seq_val = 0,
	   .config_val = 0,
	   .delay = 1,
	 },
};

static struct msm_sensor_power_setting power_down_setting[] = {
 	 {
	   .seq_type = SENSOR_I2C_MUX,
	   .seq_val = 0,
	   .config_val = 0,
	   .delay = 1,
	 },
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
	   .seq_type = SENSOR_CLK,
	   .seq_val = SENSOR_CAM_MCLK,
	   .config_val = 24000000,
	   .delay = 1,
	 },
	 {
	   .seq_type = SENSOR_VREG,
	   .seq_val = CAM_VANA,
	   .config_val = 0,
	   .delay = 5,
	 },
	 {
	   .seq_type = SENSOR_VREG,
	   .seq_val = CAM_VIO,
	   .config_val = 0,
	   .delay = 1,
	 },
};
static struct msm_camera_sensor_slave_info sensor_slave_info = {
  /* Camera slot where this camera is mounted */
  .camera_id = CAMERA_1,
  /* sensor slave address */
  .slave_addr = 0x6e,
  /*sensor i2c frequency*///FAST=400K,STANDARD=100K
  .i2c_freq_mode = I2C_FAST_MODE,
  /* sensor address type */
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  /* sensor id info*/
  .sensor_id_info = {
  /* sensor id register address */
  .sensor_id_reg_addr = 0xf0,
  /* sensor id */
  .sensor_id = 0x2385,
  },
  /* power up / down setting */
  .power_setting_array = {
    .power_setting = power_setting,
    .size = ARRAY_SIZE(power_setting),
    .power_down_setting = power_down_setting,
    .size_down = ARRAY_SIZE(power_down_setting),
  },
};

static struct msm_sensor_init_params sensor_init_params = {
  .modes_supported = 1,
  .position = 1,
  .sensor_mount_angle = 270,
};

static sensor_output_t sensor_output = {
  .output_format = SENSOR_BAYER,
  .connection_mode = SENSOR_MIPI_CSI,
  .raw_output = SENSOR_10_BIT_DIRECT,
};

static struct msm_sensor_output_reg_addr_t output_reg_addr = {
  .x_output = 0xff,
  .y_output = 0xff,
  .line_length_pclk = 0xff,
  .frame_length_lines = 0xff,
};

static struct msm_sensor_exp_gain_info_t exp_gain_info = {
  .coarse_int_time_addr = 0x03,
  .global_gain_addr = 0xb2,
  .vert_offset = 16,
};

static sensor_aec_data_t aec_info = {
  .max_gain = 8.0,
  .max_linecount = 9407, //9423-16
};

static sensor_lens_info_t default_lens_info = {
  .focal_length = 3.15,
  .pix_size = 1.75,
  .f_number = 2.8,
  .total_f_dist = 1.2,
  .hor_view_angle = 54.8,
  .ver_view_angle = 42.5,
};

#ifndef VFE_40
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x0004,
  .csi_lane_mask = 0x18,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#else
static struct csi_lane_params_t csi_lane_params = {
  .csi_lane_assign = 0x0004,
  .csi_lane_mask = 0x18,
  .csi_if = 1,
  .csid_core = {0},
  .csi_phy_sel = 0,
};
#endif

static struct msm_camera_i2c_reg_array init_reg_array0[] = {
  /*system*/
  {0xfe,0x00},
  {0xfe,0x00},
  {0xfe,0x00},
  {0xf2,0x02},
  {0xf4,0x03},
  {0xf7,0x01},
  {0xf8,0x28},
  {0xf9,0x02},
  {0xfa,0x08},
  {0xfc,0x8e},
  {0xe7,0xcc},
  {0x88,0x03},
  /*analog*/
  {0x03,0x04},
  {0x04,0x80},
  {0x05,0x02},
  {0x06,0x86},
  {0x07,0x00},
  {0x08,0x20},
  {0x09,0x00},
  {0x0a,0x04},
  {0x0b,0x00},
  {0x0c,0x02},
  {0x17,MIRROR},
  {0x18,0x02},
  {0x19,0x17},
  {0x1c,0x18},
  {0x20,0x73},
  {0x22,0xa2},
  {0x21,0x38},
  {0x29,0x20},
  {0x2f,0x14},
  {0x3f,0x40},
  {0xcd,0x94},
  {0xce,0x45},
  {0xd1,0x0c},
  {0xd7,0x9b},
  {0xd8,0x99},
  {0xda,0x3b},
  {0xd9,0xb5},
  {0xdb,0x75},
  {0xe3,0x1b},
  {0xe4,0xf8},
  /*BLk*/
  {0x40,0x22},
  {0x43,0x07},
  {0x4e,BLK_Select_H},
  {0x4f,BLK_Select_L},
  {0x68,0x00},
  /*gain*/
  {0xb0,0x46},
  {0xb1,0x01},
  {0xb2,0x00},
  {0xb6,0x00},
  /*out crop*/
  {0x90,0x01},
  {0x92,STARTY},
  {0x94,STARTX},
  {0x95,0x04},
  {0x96,0xb0},
  {0x97,0x06},
  {0x98,0x40},
  /*mipi set*/
  {0xfe,0x00},
  {0xed,0x00},
  {0xfe,0x03},
  {0x01,0x03},
  {0x02,0x82},
  {0x03,0xd0},
  {0x04,0x04},
  {0x05,0x00},
  {0x15,0x02},
  {0x06,0x80},
  {0x11,0x2b},
  {0x12,0xd0},
  {0x13,0x07},
  {0x15,0x00},
  {0x1b,0x10},
  {0x1c,0x10},
  {0x21,0x08},
  {0x22,0x05},
  {0x23,0x13},
  {0x24,0x02},
  {0x25,0x13},
  {0x26,0x06},
  {0x29,0x06},
  {0x2a,0x08},
  {0x2b,0x06},
  {0xfe,0x00},
};

static struct msm_camera_i2c_reg_setting init_reg_setting[] = {
  {
    .reg_setting = init_reg_array0,
    .size = ARRAY_SIZE(init_reg_array0),
    .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 1,//50,
  },
};

static struct sensor_lib_reg_settings_array init_settings_array = {
  .reg_settings = init_reg_setting,
  .size = 1,
};

static struct msm_camera_i2c_reg_array start_reg_array[] = {
  {0xfe,0x00},
  {0xed,0x90},
  {0xfe,0x00},
};

static  struct msm_camera_i2c_reg_setting start_settings = {
  .reg_setting = start_reg_array,
  .size = ARRAY_SIZE(start_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 1, //10,
};

static struct msm_camera_i2c_reg_array stop_reg_array[] = {
  {0xfe,0x00},
  {0xed,0x00},
  {0xfe,0x00},
};

static struct msm_camera_i2c_reg_setting stop_settings = {
  .reg_setting = stop_reg_array,
  .size = ARRAY_SIZE(stop_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 1, //10,
};

static struct    msm_camera_i2c_reg_array groupon_reg_array[] = {
  {0xfe, 0x00},
};

static struct msm_camera_i2c_reg_setting groupon_settings = {
  .reg_setting = groupon_reg_array,
  .size = ARRAY_SIZE(groupon_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_i2c_reg_array groupoff_reg_array[] = {
  {0xfe, 0x00},
};

static struct msm_camera_i2c_reg_setting groupoff_settings = {
  .reg_setting = groupoff_reg_array,
  .size = ARRAY_SIZE(groupoff_reg_array),
  .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
  .data_type = MSM_CAMERA_I2C_BYTE_DATA,
  .delay = 0,
};

static struct msm_camera_csid_vc_cfg gc2385_cid_cfg[] = {
  {0, CSI_RAW10, CSI_DECODE_10BIT},
  {1, CSI_EMBED_DATA, CSI_DECODE_10BIT},
};

static struct msm_camera_csi2_params gc2385_csi_params = {
  .csid_params = {
    .lane_cnt = 1,
    .lut_params = {
      .num_cid = ARRAY_SIZE(gc2385_cid_cfg),
      .vc_cfg = {
         &gc2385_cid_cfg[0],
         &gc2385_cid_cfg[1],
      },
    },
  },
  .csiphy_params = {
    .lane_cnt = 1,
    .settle_cnt = 0x14,//120ns
#ifndef VFE_40
    .combo_mode = 1,
#endif
  },
};

struct sensor_pix_fmt_info_t rgb10[] =
{  //only a simbol rgb10
  {V4L2_PIX_FMT_SBGGR10},
};

struct sensor_pix_fmt_info_t meta[] =
{//only a simbol meta
  {MSM_V4L2_PIX_FMT_META},
};

static sensor_stream_info_t gc2385_stream_info[] = {
  {1, &gc2385_cid_cfg[0], rgb10},
  {1, &gc2385_cid_cfg[1], meta},
};

static sensor_stream_info_array_t gc2385_stream_info_array = {
  .sensor_stream_info = gc2385_stream_info,
  .size = ARRAY_SIZE(gc2385_stream_info),
};

static struct msm_camera_i2c_reg_array res0_reg_array[] = {
/* lane snap */
  {0xfe,0x00},
};

static struct msm_camera_i2c_reg_array res1_reg_array[] = {
/*  preveiw */
  {0xfe,0x00},
};

static struct msm_camera_i2c_reg_setting res_settings[] = {
{  //capture
    .reg_setting = res0_reg_array,
    .size = ARRAY_SIZE(res0_reg_array),
    .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
  {//preview
    .reg_setting = res1_reg_array,
    .size = ARRAY_SIZE(res1_reg_array),
    .addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
    .data_type = MSM_CAMERA_I2C_BYTE_DATA,
    .delay = 0,
  },
};

static struct sensor_lib_reg_settings_array res_settings_array = {
  .reg_settings = res_settings,
  .size = ARRAY_SIZE(res_settings),
};

static struct msm_camera_csi2_params *csi_params[] = {
  &gc2385_csi_params, /* RES 0*/
  &gc2385_csi_params, /* RES 1*/
};

static struct sensor_lib_csi_params_array csi_params_array = {
  .csi2_params = &csi_params[0],
  .size = ARRAY_SIZE(csi_params),
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
  {
/* For SNAPSHOT */
    .x_output = 1600,
    .y_output = 1200,
    .line_length_pclk =   1079,
    .frame_length_lines = 1264,
    .vt_pixel_clk = 41000000,
    .op_pixel_clk = 82000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 8,
    .mode = SENSOR_DEFAULT_MODE,
  },
/* For PREVIEW */
  {
    .x_output = 1600,
    .y_output = 1200,
    .line_length_pclk =   1079,
    .frame_length_lines = 1264,
    .vt_pixel_clk = 41000000,
    .op_pixel_clk = 82000000,
    .binning_factor = 1,
    .max_fps = 30.0,
    .min_fps = 8,
    .mode = SENSOR_DEFAULT_MODE,
  },
};

static struct sensor_lib_out_info_array out_info_array = {
  .out_info = sensor_out_info,
  .size = ARRAY_SIZE(sensor_out_info),
};

static sensor_res_cfg_type_t gc2385_res_cfg[] = {
  SENSOR_SET_STOP_STREAM,
  SENSOR_SET_NEW_RESOLUTION, /* set stream config */
  SENSOR_SET_CSIPHY_CFG,
  SENSOR_SET_CSID_CFG,
  SENSOR_LOAD_CHROMATIX, /* set chromatix prt */
  SENSOR_SEND_EVENT, /* send event */
  SENSOR_SET_START_STREAM,
};

static struct sensor_res_cfg_table_t gc2385_res_table = {
  .res_cfg_type = gc2385_res_cfg,
  .size = ARRAY_SIZE(gc2385_res_cfg),
};

static struct sensor_lib_chromatix_t gc2385_chromatix[] = {
  {
    .common_chromatix = GC2385_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
  },
  {
    .common_chromatix = GC2385_LOAD_CHROMATIX(common),
    .camera_preview_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
    .camera_snapshot_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
    .camcorder_chromatix = GC2385_LOAD_CHROMATIX(preview), /* RES0 */
  },
};

static struct sensor_lib_chromatix_array gc2385_lib_chromatix_array = {
  .sensor_lib_chromatix = gc2385_chromatix,
  .size = ARRAY_SIZE(gc2385_chromatix),
};

/*===========================================================================
 * FUNCTION    - gc2385_real_to_register_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint16_t gc2385_real_to_register_gain(float gain)
{
    uint16_t reg_gain;
    if (gain < 1.0)
        gain = 1.0;
    if (gain > 8.0)
        gain = 8.0;
//    ALOGE("gc2385_PETER,real_gain=%f" , gain);
    reg_gain = (uint16_t)(gain * 64.0f);
    return reg_gain;

}

/*===========================================================================
 * FUNCTION    - gc2385_register_to_real_gain -
 *
 * DESCRIPTION:
 *==========================================================================*/
static float gc2385_register_to_real_gain(uint16_t reg_gain)
{
    float gain;
    if (reg_gain > 0x0200)
        reg_gain = 0x0200;
//    ALOGE("gc2385_PETER register_gain=%u" , reg_gain);
    gain = (float)(reg_gain/64.0f);
    return gain;

}

/*===========================================================================
 * FUNCTION    - gc2385_calculate_exposure -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int32_t gc2385_calculate_exposure(float real_gain,
  uint16_t line_count, sensor_exposure_info_t *exp_info)
{
  if (!exp_info) {
    return -1;
  }
  exp_info->reg_gain = gc2385_real_to_register_gain(real_gain);
  float sensor_real_gain = gc2385_register_to_real_gain(exp_info->reg_gain);
  exp_info->digital_gain = real_gain / sensor_real_gain;
  exp_info->line_count = line_count;
  return 0;
}

/*===========================================================================
 * FUNCTION    - gc2385_fill_exposure_array -
 *
 * DESCRIPTION:
 *==========================================================================*/

static int32_t gc2385_fill_exposure_array(uint16_t gain, uint32_t line,
  uint32_t fl_lines,int32_t luma_avg, uint32_t fgain,
   struct msm_camera_i2c_reg_setting *reg_setting)
{
  int32_t rc = 0;

  uint16_t reg_count = 0;
  uint16_t gain_b6,gain_b1,gain_b2;
  uint16_t iReg,temp,line_temp;
  int32_t i;

  //ALOGE("GC2385,fl_lines=%d,gain=%d, line=%d\n",fl_lines,gain,line);

  if (!reg_setting) {
    return -1;
  }

  reg_setting->reg_setting[reg_count].reg_addr =0xfe;
  reg_setting->reg_setting[reg_count].reg_data =0x00;
  reg_count++;

  iReg = gain;
  if(iReg < 0x40)
    iReg = 0x40;
  if((ANALOG_GAIN_1<= iReg)&&(iReg < ANALOG_GAIN_2))  //ANALOG_GAIN_1 == 64
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x73;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa2;
    reg_count++;

    //analog gain
    gain_b6 = 0x00;
    temp = 256*iReg/ANALOG_GAIN_1;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_2<= iReg)&&(iReg < ANALOG_GAIN_3))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x73;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa2;
    reg_count++;

    //analog gain
    gain_b6 = 0x01;
    temp = 256*iReg/ANALOG_GAIN_2;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_3<= iReg)&&(iReg < ANALOG_GAIN_4))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x73;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa2;
    reg_count++;

    //analog gain
    gain_b6 = 0x02;
    temp = 256*iReg/ANALOG_GAIN_3;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_4<= iReg)&&(iReg < ANALOG_GAIN_5))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x73;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa2;
    reg_count++;

    //analog gain
    gain_b6 = 0x03;
    temp = 256*iReg/ANALOG_GAIN_4;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_5<= iReg)&&(iReg < ANALOG_GAIN_6))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x73;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa3;
    reg_count++;

    //analog gain
    gain_b6 = 0x04;
    temp = 256*iReg/ANALOG_GAIN_5;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_6<= iReg)&&(iReg<ANALOG_GAIN_7))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x74;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa3;
    reg_count++;

    //analog gain
    gain_b6 = 0x05;
    temp = 256*iReg/ANALOG_GAIN_6;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_7<= iReg)&&(iReg<ANALOG_GAIN_8))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x74;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa3;
    reg_count++;

    //analog gain
    gain_b6 = 0x06;
    temp = 256*iReg/ANALOG_GAIN_7;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else if((ANALOG_GAIN_8<= iReg)&&(iReg<ANALOG_GAIN_9))
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x74;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa3;
    reg_count++;

    //analog gain
    gain_b6 = 0x07;
    temp = 256*iReg/ANALOG_GAIN_8;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }
  else
  {
    reg_setting->reg_setting[reg_count].reg_addr =0x20;
    reg_setting->reg_setting[reg_count].reg_data =0x75;
    reg_count++;
    reg_setting->reg_setting[reg_count].reg_addr =0x22;
    reg_setting->reg_setting[reg_count].reg_data =0xa4;
    reg_count++;

    //analog gain
    gain_b6 = 0x08;
    temp = 256*iReg/ANALOG_GAIN_9;
    gain_b1 = temp>>8;
    gain_b2 = temp&0xff;
  }

/***********************Shutter Start***************************************/
  if(line < 6) line = 6; /*anti color deviation on shot expoure*/
/*  if(line == (sensor_out_info[0].frame_length_lines-1))//add 20160527
  line += 1;
*/
    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->coarse_int_time_addr;
    reg_setting->reg_setting[reg_count].reg_data = (line & 0x3F00) >> 8;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->coarse_int_time_addr + 1;
    reg_setting->reg_setting[reg_count].reg_data = line & 0xFF;
    reg_count++;

  /***********************Shutter End***************************************/

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->global_gain_addr + 4;
    reg_setting->reg_setting[reg_count].reg_data = gain_b6; //0xb6
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
      sensor_lib_ptr.exp_gain_info->global_gain_addr - 1;  //0xb1
    reg_setting->reg_setting[reg_count].reg_data = gain_b1;
    reg_count++;

    reg_setting->reg_setting[reg_count].reg_addr =
        sensor_lib_ptr.exp_gain_info->global_gain_addr;
    reg_setting->reg_setting[reg_count].reg_data = gain_b2;  //0xb2
    reg_count++;

    reg_setting->size = reg_count;
    reg_setting->addr_type = MSM_CAMERA_I2C_BYTE_ADDR;
    reg_setting->data_type = MSM_CAMERA_I2C_BYTE_DATA;
    reg_setting->delay = 0;

  return rc;
}

static sensor_exposure_table_t gc2385_expsoure_tbl = {
  .sensor_calculate_exposure = gc2385_calculate_exposure,
  .sensor_fill_exposure_array = gc2385_fill_exposure_array,
};

static sensor_lib_t sensor_lib_ptr = {
  /* sensor slave info */
  .sensor_slave_info = &sensor_slave_info,
  /* sensor init params */
  .sensor_init_params = &sensor_init_params,
  /* sensor actuator name */
  //.actuator_name = "dw9714",
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
  .sensor_num_frame_skip = 2,  // 1  peter
  /* number of frames to skip after start HDR stream */
  .sensor_num_HDR_frame_skip = 2, //add 20161228
  /* sensor exposure table size */
  .exposure_table_size = 20,
  /* sensor lens info */
  .default_lens_info = &default_lens_info,
  /* csi lane params */
  .csi_lane_params = &csi_lane_params,
  /* csi cid params */
  .csi_cid_params = gc2385_cid_cfg,
  /* csi csid params array size */
  .csi_cid_params_size = ARRAY_SIZE(gc2385_cid_cfg),
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
  .sensor_res_cfg_table = &gc2385_res_table,
  /* res settings */
  .res_settings_array = &res_settings_array,
  /* out info array */
  .out_info_array = &out_info_array,
  /* crop params array */
  .crop_params_array = &crop_params_array,
  /* csi params array */
  .csi_params_array = &csi_params_array,
  /* sensor port info array */
  .sensor_stream_info_array = &gc2385_stream_info_array,
  /* exposure funtion table */
  .exposure_func_table = &gc2385_expsoure_tbl,
  /* chromatix array */
  .chromatix_array = &gc2385_lib_chromatix_array,
};

/*===========================================================================
 * FUNCTION    - SKUAA_ST_gc2385_open_lib -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *gc2385_open_lib(void)
{
 // CDBG("gc2385_open_lib is called");
  return &sensor_lib_ptr;
}
