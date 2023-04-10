/*============================================================================

  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

============================================================================*/

//=========================================================================
// Chromatix header version info (MUST BE THE FIRST PARAMETER)
//=========================================================================

/* Header Version Info */
{
   0x0310, /* Chromatix Version */
   0, /* Revision */
   /* Chromatix App Version */
   {
      6, /* Major */
      13, /* Minor */
      1, /* Revision */
      0, /* Build */
   },
   HEADER_3A, /* Header Type */
   0, /* Is Compressed */
   0, /* Is Mono */
   0, /* Is Video */
   0, /* Reserved Align */
   MODE_UNKNOWN, /* Chromatix Mode */
   0, /* Target ID */
   0, /* Chromatix ID */
   /* Reserved */
   {
      0, 0, 0, 0
   }
},

{
    0,
    0,
},
//=========================================================================
// SHDR mode
//=========================================================================
{
/*shdr_aec_tuning_type*/
  {
  1,         /*shdr_enable*/
  8,         /*num_lut_entries*/
  6,         /*num_lowlight_lut_entries*/
  0,         /*pre_Bhist_inverse_map*/
  0.02f,     /*temporal_filter1*/
  1.0f,      /*temporal_filter2*/
  /*exp_ratio, gtm, exp_up, exp_low, non_linear_low, non_linear_high, reserved[5] */
  {   {64.0f, 0.550f, 1.50f, 0.87f, 4, 200,{0.0f}},
      {48.0f, 0.570f, 1.40f, 0.87f, 4, 200,{0.0f}},
      {32.0f, 0.595f, 1.40f, 0.87f, 4, 200,{0.0f}},
      {24.0f, 0.615f, 1.40f, 0.90f, 4, 200,{0.0f}},
      {16.0f, 0.650f, 1.35f, 0.90f, 4, 200,{0.0f}},
      {12.0f, 0.670f, 1.35f, 0.90f, 4, 200,{0.0f}},
      { 8.0f, 0.710f, 1.35f, 0.90f, 4, 200,{0.0f}},
      { 4.0f, 0.790f, 1.35f, 0.90f, 4, 200,{0.0f}},
  },
  /*lux_index, k, exp_ratio_up_limit, exp_ratio_low_light, reserved[5] */
  {   {122,0.750f,64.0f,4.0f,{0.0f}},
      {168,0.750f,64.0f,4.0f,{0.0f}},
      {192,0.750f,64.0f,4.0f,{0.0f}},
      {215,0.500f,16.0f,4.0f,{0.0f}},
      {239,0.500f, 8.0f,4.0f,{0.0f}},
      {262,0.500f, 8.0f,4.0f,{0.0f}},
  },
  {0.0f}, /* reserved */
  },

/*shdr_bayer_proc_tuning_type*/
  {
  10,            /* adc_bit_depth */
  12,            /* tm_out_bit_depth */
  0.65,          /* bayer_gtm_gamma */
  500,           /* hdr_dark_n1 */
  500.0,         /* hdr_dark_n2_minus_n1_normalization_factor */
  4,             /* hdr_max_weight */
  1.6,           /* tm_gain */
  16.0,          /* hdr_ratio */
  1,             /* perf_hint */
  1,             /* num_gpu_passes */
  },

/*shdr_ltm_tuning_type*/
  {
  1,            /* auto_ltm_enable */
  0,            /* halo_score_target */
  128,          /* brightness_target */
  20.0f,        /* contrast_target */
  7.9f,         /* ltm_gain_limit */
  20,           /* low_code_tone_end */
  20,           /* mid_tone_start */
  60,           /* mid_tone_end */
  0.078125f,    /* smear_prev_low_limit */
  0.234375f     /* smear_prev_high_limit */
  },


/*reserved[200]*/
  {
  0.0f,
  },

},

//=========================================================================
// SHDR mode
//=========================================================================
{
0,
},

//=========================================================================
// IR Mode
//=========================================================================
{
	//IR_CHRO
	/* ir_mode_enable */
	0,

	/* ir_led_adj */
	{
	  /* ir_led_enable */
	  1,
	  /* ir_led_intensity_unit */
	  1.0,
	  /* ir_led_intensity_idx_max */
	  90,
	  /* ir_led_intensity_idx_min */
	  1,
	},
	/* ir_als_adjust_type */
	{
	  /* ir_lightsensor_enable */
	  0,
	  /* day2ir_switch_pt */
	  1000.0,
	  /* ir2day_switch_pt */
	  10000.0,
	},
	 /* ir_center_weight */
	 {
		  {
			 0.101600f, 0.103330f, 0.108200f, 0.115420f, 0.123690f, 0.131600f, 0.137780f, 0.141160f, 0.141160f, 0.137780f, 0.131600f, 0.123690f, 0.115420f, 0.108200f, 0.103330f, 0.101600f
		  },
		  {
			 0.103330f, 0.106930f, 0.117100f, 0.132080f, 0.149290f, 0.165750f, 0.178610f, 0.185650f, 0.185650f, 0.178610f, 0.165750f, 0.149290f, 0.132080f, 0.117100f, 0.106930f, 0.103330f
		  },
		  {
			 0.108200f, 0.117100f, 0.142000f, 0.179190f, 0.221670f, 0.262290f, 0.294040f, 0.311410f, 0.311410f, 0.294030f, 0.262290f, 0.221670f, 0.179190f, 0.142000f, 0.117100f, 0.108200f
		  },
		  {
			 0.115420f, 0.132080f, 0.179190f, 0.248600f, 0.328310f, 0.404540f, 0.464100f, 0.496700f, 0.496700f, 0.464100f, 0.404540f, 0.328310f, 0.248600f, 0.179190f, 0.132080f, 0.115420f
		  },
		  {
			 0.123690f, 0.149290f, 0.221670f, 0.328310f, 0.450780f, 0.567890f, 0.659400f, 0.709490f, 0.709490f, 0.659400f, 0.567890f, 0.450770f, 0.328310f, 0.221670f, 0.149290f, 0.123690f
		  },
		  {
			 0.131600f, 0.165750f, 0.262290f, 0.404540f, 0.567890f, 0.724100f, 0.846170f, 0.912970f, 0.912970f, 0.846160f, 0.724100f, 0.567890f, 0.404530f, 0.262290f, 0.165750f, 0.131600f
		  },
		  {
			 0.137780f, 0.178610f, 0.294040f, 0.464100f, 0.659400f, 0.846170f, 0.992100f, 1.000000f, 1.000000f, 0.992100f, 0.846160f, 0.659390f, 0.464100f, 0.294030f, 0.178610f, 0.137780f
		  },
		  {
			 0.141160f, 0.185650f, 0.311410f, 0.496700f, 0.709490f, 0.912970f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 0.912960f, 0.709480f, 0.496690f, 0.311400f, 0.185640f, 0.141160f
		  },
		  {
			 0.141160f, 0.185650f, 0.311410f, 0.496700f, 0.709490f, 0.912970f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 0.912960f, 0.709480f, 0.496690f, 0.311400f, 0.185640f, 0.141160f
		  },
		  {
			 0.137780f, 0.178610f, 0.294030f, 0.464100f, 0.659400f, 0.846160f, 0.992100f, 1.000000f, 1.000000f, 0.992090f, 0.846160f, 0.659390f, 0.464090f, 0.294030f, 0.178610f, 0.137780f
		  },
		  {
			 0.131600f, 0.165750f, 0.262290f, 0.404540f, 0.567890f, 0.724100f, 0.846160f, 0.912960f, 0.912960f, 0.846160f, 0.724090f, 0.567880f, 0.404530f, 0.262290f, 0.165750f, 0.131600f
		  },
		  {
			 0.123690f, 0.149290f, 0.221670f, 0.328310f, 0.450770f, 0.567890f, 0.659390f, 0.709480f, 0.709480f, 0.659390f, 0.567880f, 0.450770f, 0.328310f, 0.221670f, 0.149290f, 0.123690f
		  },
		  {
			 0.115420f, 0.132080f, 0.179190f, 0.248600f, 0.328310f, 0.404530f, 0.464100f, 0.496690f, 0.496690f, 0.464090f, 0.404530f, 0.328310f, 0.248600f, 0.179190f, 0.132080f, 0.115420f
		  },
		  {
			 0.108200f, 0.117100f, 0.142000f, 0.179190f, 0.221670f, 0.262290f, 0.294030f, 0.311400f, 0.311400f, 0.294030f, 0.262290f, 0.221670f, 0.179190f, 0.142000f, 0.117100f, 0.108200f
		  },
		  {
			 0.103330f, 0.106930f, 0.117100f, 0.132080f, 0.149290f, 0.165750f, 0.178610f, 0.185640f, 0.185640f, 0.178610f, 0.165750f, 0.149290f, 0.132080f, 0.117100f, 0.106930f, 0.103330f
		  },
		  {
			 0.101600f, 0.103330f, 0.108200f, 0.115420f, 0.123690f, 0.131600f, 0.137780f, 0.141160f, 0.141160f, 0.137780f, 0.131600f, 0.123690f, 0.115420f, 0.108200f, 0.103330f, 0.101600f
		  },
	},
	/* ir_highlight_suppress_weight */
	{
	   0.200000f, 0.200000f, 0.200000f, 0.200000f, 0.200000f, 0.200000f, 0.200000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f,
	   0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.250000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f,
	   0.500000f, 0.500000f, 0.500000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 0.750000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f,
	   1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 1.000000f,
	},

	/* ir_mode_conv_speed */
	0.8,
	/* day2ir_exp_index */
	300,
	/* ir2day_exp_index */
	250,
	/* ir_mode_day2ir_check_cnt */
	2,
	/* ir_mode_ir2day_check_cnt */
	2,
	/* ir_mode_skip_check_cnt */
	5,
	/* ir_mode_aec_skip_cnt; */
	3,

	/* ir_daylight_distribution_adjust_type */
	{
	  /* normal_light_cnt_thres */
	  0.3,
	  /* normal_light_target_thres */
	  0.5,
	  /* normal_light_GB_ratio_max */
	  107,
	  /* normal_light_GB_ratio_min */
	  93,
	  /* normal_light_GR_ratio_max */
	  107,
	  /* normal_light_GR_ratio_min */
	  93,
	  /* normal_light_RB_ratio_max */
	  110,
	  /* normal_light_RB_ratio_min */
	  90,
	  /* ir_distribution_weight */
	  {
		  {
			 0.101600f, 0.103330f, 0.108200f, 0.115420f, 0.123690f, 0.131600f, 0.137780f, 0.141160f, 0.141160f, 0.137780f, 0.131600f, 0.123690f, 0.115420f, 0.108200f, 0.103330f, 0.101600f
		  },
		  {
			 0.103330f, 0.106930f, 0.117100f, 0.132080f, 0.149290f, 0.165750f, 0.178610f, 0.185650f, 0.185650f, 0.178610f, 0.165750f, 0.149290f, 0.132080f, 0.117100f, 0.106930f, 0.103330f
		  },
		  {
			 0.108200f, 0.117100f, 0.142000f, 0.179190f, 0.221670f, 0.262290f, 0.294040f, 0.311410f, 0.311410f, 0.294030f, 0.262290f, 0.221670f, 0.179190f, 0.142000f, 0.117100f, 0.108200f
		  },
		  {
			 0.115420f, 0.132080f, 0.179190f, 0.248600f, 0.328310f, 0.404540f, 0.464100f, 0.496700f, 0.496700f, 0.464100f, 0.404540f, 0.328310f, 0.248600f, 0.179190f, 0.132080f, 0.115420f
		  },
		  {
			 0.123690f, 0.149290f, 0.221670f, 0.328310f, 0.450780f, 0.567890f, 0.659400f, 0.709490f, 0.709490f, 0.659400f, 0.567890f, 0.450770f, 0.328310f, 0.221670f, 0.149290f, 0.123690f
		  },
		  {
			 0.131600f, 0.165750f, 0.262290f, 0.404540f, 0.567890f, 0.724100f, 0.846170f, 0.912970f, 0.912970f, 0.846160f, 0.724100f, 0.567890f, 0.404530f, 0.262290f, 0.165750f, 0.131600f
		  },
		  {
			 0.137780f, 0.178610f, 0.294040f, 0.464100f, 0.659400f, 0.846170f, 0.992100f, 1.000000f, 1.000000f, 0.992100f, 0.846160f, 0.659390f, 0.464100f, 0.294030f, 0.178610f, 0.137780f
		  },
		  {
			 0.141160f, 0.185650f, 0.311410f, 0.496700f, 0.709490f, 0.912970f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 0.912960f, 0.709480f, 0.496690f, 0.311400f, 0.185640f, 0.141160f
		  },
		  {
			 0.141160f, 0.185650f, 0.311410f, 0.496700f, 0.709490f, 0.912970f, 1.000000f, 1.000000f, 1.000000f, 1.000000f, 0.912960f, 0.709480f, 0.496690f, 0.311400f, 0.185640f, 0.141160f
		  },
		  {
			 0.137780f, 0.178610f, 0.294030f, 0.464100f, 0.659400f, 0.846160f, 0.992100f, 1.000000f, 1.000000f, 0.992090f, 0.846160f, 0.659390f, 0.464090f, 0.294030f, 0.178610f, 0.137780f
		  },
		  {
			 0.131600f, 0.165750f, 0.262290f, 0.404540f, 0.567890f, 0.724100f, 0.846160f, 0.912960f, 0.912960f, 0.846160f, 0.724090f, 0.567880f, 0.404530f, 0.262290f, 0.165750f, 0.131600f
		  },
		  {
			 0.123690f, 0.149290f, 0.221670f, 0.328310f, 0.450770f, 0.567890f, 0.659390f, 0.709480f, 0.709480f, 0.659390f, 0.567880f, 0.450770f, 0.328310f, 0.221670f, 0.149290f, 0.123690f
		  },
		  {
			 0.115420f, 0.132080f, 0.179190f, 0.248600f, 0.328310f, 0.404530f, 0.464100f, 0.496690f, 0.496690f, 0.464090f, 0.404530f, 0.328310f, 0.248600f, 0.179190f, 0.132080f, 0.115420f
		  },
		  {
			 0.108200f, 0.117100f, 0.142000f, 0.179190f, 0.221670f, 0.262290f, 0.294030f, 0.311400f, 0.311400f, 0.294030f, 0.262290f, 0.221670f, 0.179190f, 0.142000f, 0.117100f, 0.108200f
		  },
		  {
			 0.103330f, 0.106930f, 0.117100f, 0.132080f, 0.149290f, 0.165750f, 0.178610f, 0.185640f, 0.185640f, 0.178610f, 0.165750f, 0.149290f, 0.132080f, 0.117100f, 0.106930f, 0.103330f
		  },
		  {
			 0.101600f, 0.103330f, 0.108200f, 0.115420f, 0.123690f, 0.131600f, 0.137780f, 0.141160f, 0.141160f, 0.137780f, 0.131600f, 0.123690f, 0.115420f, 0.108200f, 0.103330f, 0.101600f
		  },
	  },
       },
},
//=========================================================================
// LCAC Data
//=========================================================================
{
  1,  /*enable*/
  2,  /*peak_filter_kernel_size*/
  5,  /*local_chromaticity_threshold*/
  180,  /*saturated_luma*/
  200,  /*saturated_contrast*/
  8,  /*peak_threshold*/
  5,  /*bleeding_threshold*/
  {   /*cac_correction_kernel*/
    2, 2, 3, 3, 4, 4, 5, 5
  },
  {   /*color_weight*/
    {
      { 0,0,0,1,1,1,1,1 },
      { 0,0,1,1,1,1,1,1 },
      { 0,1,1,1,1,1,1,1 },
      { 1,1,1,1,1,1,1,0 },
      { 1,1,1,0,0,0,0,0 },
      { 1,1,1,0,0,0,0,0 },
      { 1,1,1,0,0,0,0,0 },
      { 0,0,0,0,0,0,0,0 },
    },
    {
      { 1,1,1,1,1,1,1,1 },
      { 1,1,1,1,1,1,1,1 },
      { 1,1,1,1,1,1,1,1 },
      { 1,1,1,1,1,1,1,0 },
      { 1,1,1,1,1,1,0,0 },
      { 1,1,1,1,0,0,0,0 },
      { 1,1,1,0,0,0,0,0 },
      { 1,0,0,0,0,0,0,0 },
    },
  },
  { 50,50,40,40,40,40,30,20 }, /*contrast_threshold*/
  { 64,64,64,64,64,64,64,40 }, /*luma_threshold*/
},
//=========================================================================
// IOT Reserve
//=========================================================================
{
    {
      0,
    },
    {
      0,
    },
}
