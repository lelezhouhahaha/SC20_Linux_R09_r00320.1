/* af_haf_tuning_data_camcorder.h
 *
 * Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */


.enable = 1,

.algo_enable = {0,1,0,0},

.stats_enable = {1,1,1},

.lens_sag_comp_enable = 0,

.hysteresis_comp_enable = 0,

.stats_select = 0,

.fine_srch_drop_thres = 0.995f,

.fine_step_size = 4,

.max_move_step = 100,

.max_move_step_buffer = 30,

.base_frame_delay = 0,

.fine_srch_extension = {
    .max_fine_srch_extension_cnt = 3,
    .num_near_steps = 0,
    .num_far_steps = 4,
    .step_size = 12,
    .decrease_drop_ratio = 0.998
},

.pdaf = {
    /* Phase detection ROI configuration */
    .roi = {
      .num_entries = 1,
      .config = {
         /* { loc_y, loc_x, num_rows, num_cols} */
         { 0, 0, 1, 1 },
      },
    },
    /* Focus tracking table */
    /* Defines the minimum confidence needed for AF to utilize PD */
    .focus_tbl = {
      .num_entries = 5,
      .entries = {
        /* {defocus (logical steps), move_pcnt} */
        { 28,  0.35f },
        { 56,  0.25f },
        { 84,  0.35f },
        {112,  0.30f },
        {352,  0.25f },
      },
    },

    /* Phase detection noise table */
    /* Noise table controls PD noise scales with noise gain (dB) */
    /* Larger the multiplier -> greater the fine scan range */
    /* Larger the multipler -> less sensitive scene change detection is to PD changes */
    .noise_tbl = {
      .num_entries = 5,
      .entries = {
        /* {noise_gain (dB), multiplier (float)} */
        { 0.00f,  1.00f }, //sensor gain 1
        { 6.02f,  1.00f }, //sensor gain 2
        { 12.0f,  1.20f }, //sensor gain 4
        { 18.1f,  1.70f }, //sensor gain 8
        { 24.1f,  2.00f }, //sensor gain 16 (max)
      },
    },
    /* Phase detection confidence table */
    /* Defines the minimum confidence needed for AF to utilize PD */
    .conf_tbl = {
      .num_entries = 5,
      .entries = {
        /* {noise_gain (dB), min_conf} */
        { 0.00f,  200 },
        { 6.02f,  225 },
        { 12.0f,  250 },
        { 18.1f,  310 },
        { 24.1f,  330 },
      },
    },

    /* Depth stability requirement table */
    .stable_tbl = {
      .num_entries = 3,
      .entries = {
        /* {fps, min_stable_cnt} */
        { 10,  0 },
        { 14,  1 },
        { 24,  1 },
      },
    },
    /* Focus tracking and fine scan parameters */
    .focus_scan = {
      .focus_conv_frame_skip = 1,
      .enable_fine_scan     = 0,
      .min_fine_scan_range  = 1,
      .fine_scan_step_size  = 12,
      .focus_done_threshold  = 33,
    },
    /* Scene monitor tuning parameters */
    .scene_monitor = {
      .wait_after_focus_cnt    = 2,
      .wait_conf_recover_cnt   = 4,
      .defocus_threshold       = 40.0f,
      .depth_stable_threshold  = 45.0f, //0.8f,
    },
    /*reserve[200] */
    .reserve = {
          3,  /* Type */
          SINGLE_HYP_F_IDX, //SINGLE_30CM_IDX}, /* init pos */
          /*PDAF_CAF_DECISION_BY_MAJORITY*/
          /* This parameter is for deciding AF trigger type by PDAF/CAF according */
          /*to Conf numberor refer latest frame's Conf flag*/
          /* 1 by majority 0 by latest frame */
          0,
          /* PDAF_ENABLE_FACE_CAF_PRIORITY*/
          1,
          /* defocus_stable_filter_len */
          3,
          /* Enable soft_rgn_threshold*/
          0,
          /* defocus_dof_multiplier */
          1.2f,
          /* soft_rgn_thres_multiplier */
          0.6f,
          /*  fine_scan_range_enhance */
          0,
    },
},
.tof = {
    .sensitivity = 1.25,
    .outdoor_lux_idx = 160,
    .lens_laser_dist_comp_mm = 0,
    .actuator_shift_comp = 0,
    .actuator_hysteresis_comp = 3,
    .wait_frame_limt = 3,
    .max_tof_srch_cnt = 3,
    .jump_to_start_limit = 20,
    .num_near_steps_near_direction = 3,
    .num_far_steps_near_direction = 2,
    .num_near_steps_far_direction = 3,
    .num_far_steps_far_direction = 2,
    .tof_step_size = 6,
    .normal_light_cnt = 5,
    .lowlight_cnt = 3,
    .num_monitor_samples = 10,
    .scene_change_distance_thres = {20, 30, 50, 60, 80},
    .scene_change_distance_std_dev_thres = 30,
    .far_scene_coarse_srch_enable = 0,
    .af_sad = {
        .enable = 1,
        .gain_min = 2.0,
        .gain_max = 30.0,
        .ref_gain_min = 2.0f,
        .ref_gain_max = 30.0f,
        .threshold_min = 10,
        .threshold_max = 15,
        .ref_threshold_min = 12,
        .ref_threshold_max = 16,
        .frames_to_wait = 24,
    },
    .start_pos_for_tof = 80,
    .distance_region = {70, 200, 350, 500, 765},
    .jump_to_start_limit_low_light = 50,
    .tof_step_size_low_light = 8,
    .far_distance_fine_step_size = 5,
    .far_distance_unstable_cnt_factor = 2,
    .far_converge_point = 152,
},

