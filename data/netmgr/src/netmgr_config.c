/*!
  @file
  netmgr_config.c

  @brief
  This file provides implementation of netmgr configration using configdb.
*/

/*===========================================================================
  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#include "netmgr_config.h"
#include "netmgr_util.h"

#ifndef FEATURE_EXTERNAL_AP
#include "configdb.h"
#endif

#include "ds_util.h"

#include "netmgr_cb.h"
#include "netmgr_main_cb.h"
#include "qmi_platform_config.h"

#define NETMGR_CONFIG_CB_MODNAME "CONFIG"

#ifdef FEATURE_DATA_IWLAN
/*===========================================================================
  FUNCTION:  netmgr_process_iwlan_config
===========================================================================*/
/*!
    @brief
    This function processes the iwlan related config items

    @param[in] target: configuration to be used within the XML file.

    @return  0 on error.
    @return -1 on failure.
*/
/*=========================================================================*/
LOCAL
int netmgr_process_iwlan_config
(
  const char* target
)
{
  int result = 0;
  int ret;
  int i,j, rev_min;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];
  int32 prop_val_int;
  int32 rev_control_ports_len = 0;
  int32 rev_data_ports_len = 0;

  rev_min = 0;
  /* iwlan_enable */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".iwlan_enable");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  switch(prop_val_int)
  {
    case 0:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     break;

    case 1:
     netmgr_main_cfg.iwlan_enabled = IWLAN_ENABLE;
     break;

    case 2:
     netmgr_main_cfg.iwlan_enabled = NSWO_ONLY;
     break;

    default:
     netmgr_main_cfg.iwlan_enabled = IWLAN_DISABLE;
     netmgr_log_err("Error reading property [%s] property value[%d]", prop_name, prop_val_int);
  }

  /* The reverse ports starting and ending index depends on the
   * modem being used. Query the correct starting and ending indexes
   */
  if (netmgr_main_cfg.modem_enable[0])
  {
    rev_min = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MSM);
  }
  else if(netmgr_main_cfg.modem_enable[1])
  {
    rev_min = NETMGR_MAIN_GET_INST_MIN_REV(NETMGR_MODEM_MDM);
  }

  /* rev_control_ports_len and rev_control_ports */
  j = 0;
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_control_ports_len, sizeof(rev_control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }

  for (i = rev_min; i < (rev_min + rev_control_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_control_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    netmgr_ctl_port_array[i].enabled = TRUE;
    j++;
  }

  /* rev_data_ports_len and rev_data_ports */
  j = 0;
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rev_data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &rev_data_ports_len, sizeof(rev_data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto err;
  }
  for (i = rev_min; i < (rev_min + rev_data_ports_len); i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".rev_data_ports.",j);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto err;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    netmgr_ctl_port_array[i].enabled = TRUE;
    j++;
  }

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MSM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* Change prefix for reverse links */
      netmgr_log_high("%s", "Changing MDM rev prefixes");
      strlcpy(NETMGR_MAIN_GET_DEV_REV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_REV_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

err:
  return result;
}
#endif /* FEATURE_DATA_IWLAN */

/*===========================================================================
  FUNCTION:  netmgr_process_cmd_logging_config
===========================================================================*/
/*!
    @brief
    This function processes the ds_exec option overrides

    @return  nothing
*/
/*=========================================================================*/
#ifndef FEATURE_EXTERNAL_AP
LOCAL
void netmgr_process_cmd_logging_config
(
  void
)
{
  int result = 0;
  int ret;
  char *prop_name;
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];
  int32 prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.cmd_debug";
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Couldn't read property [%s] Err[%d], defaulting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.exec_opts.cmd_debug = prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.log_options";
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Couldn't read property [%s] Err[%d], defaulting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.exec_opts.log_options = prop_val_int;

  prop_name = "netmgr_config.cmd_logging_config.outfile";
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    memset(prop_val_str, 0, sizeof(prop_val_str));
  }
  strlcpy(netmgr_main_cfg.exec_opts.outfile, prop_val_str, sizeof(netmgr_main_cfg.exec_opts.outfile));
}

/*===========================================================================
  FUNCTION:  netmgr_config_load
===========================================================================*/
/*!
    @brief
    This function loads the configuration based on the config file and
    "target" property specified.

    @param[in] xml_file: XML file location string.
    @param[in] target: configuration to be used within the XML file.

    @return  0 if configuration file is loaded properly.
    @return -1 if configuration file file load/parse fails.
*/
/*=========================================================================*/
int netmgr_config_load
(
  const char *xml_file,
  const char *target
)
{
  int result = 0;
  int32 ret;
  int i;
  char prop_name[NETMGR_CFGDB_STRVAL_MAX];
  int32 num_modems = 0;
  int32 control_ports_len = 0;
  int32 data_ports_len = 0;
  int32 prop_val_int;
  char prop_val_str[NETMGR_CFGDB_STRVAL_MAX];

  netmgr_log_med("netmgr_config: Configuring using file %s, target %s\n", xml_file, target);
  ret = configdb_init(CFGDB_OPMODE_CACHED, xml_file);
  if (CFGDB_SUCCESS != ret)
  {
    netmgr_log_err("Unable to open/parse config file [%s] Err [%d]", xml_file, ret);
    result = -1;
    goto bail;
  }

  /* qmi_dpm_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qmi_dpm_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.dpm_enabled = (prop_val_int ? TRUE : FALSE);


  /* qos_via_idl */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".qos_via_idl");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    netmgr_main_cfg.qos_via_idl = FALSE;
  }
  else
  {
    netmgr_main_cfg.qos_via_idl = (prop_val_int ? TRUE : FALSE);
  }

  /* use_qmuxd */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".use_qmuxd");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d] Default[%d]", prop_name, ret, TRUE);
    prop_val_int = TRUE;
  }
  netmgr_main_cfg.use_qmuxd = (prop_val_int ? TRUE : FALSE);

  /* dpm_retry_timeout value */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dpm_retry_timeout");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 10000;
    netmgr_log_err("Error reading property [%s] Err[%d] Default [%d]",
                   prop_name, ret,prop_val_int);
  }
  netmgr_main_cfg.dpm_retry_timeout = prop_val_int;

  /* wda_data_format_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".wda_data_format_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.wda_data_format = (prop_val_int ? TRUE : FALSE);

  /* ep_type */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".ep_type");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]. Default[%d]",
                   prop_name, ret, DS_EP_TYPE_EMBEDDED);
    prop_val_int = DS_EP_TYPE_EMBEDDED;
  }
  netmgr_main_cfg.ep_type = prop_val_int;

  /* single_qmux_ch_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.single_qmux_ch_enabled = (prop_val_int ? TRUE : FALSE);

  /* single_qmux_ch_conn_id_str */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_conn_id_str");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.qmux_ch_name, prop_val_str, sizeof(netmgr_main_cfg.qmux_ch_name));

  /* single_qmux_ch_name */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".single_qmux_ch_name");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  strlcpy(netmgr_main_cfg.smd_ch_name, prop_val_str, sizeof(netmgr_main_cfg.smd_ch_name));

  /* rmnet_data_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".rmnet_data_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.rmnet_data_enabled = (prop_val_int ? TRUE : FALSE);

  /* dataformat_agg_dl_pkt */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_pkt");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_cnt = (int)prop_val_int;

  /* dataformat_agg_dl_size */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_dl_size");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_agg_size = (int)prop_val_int;

  /* dataformat_agg_ul_pkt */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_pkt");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Warning: Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.data_format.ul_agg_cnt = 0;
  }
  netmgr_main_cfg.data_format.ul_agg_cnt = prop_val_int;

  /* dataformat_agg_ul_size */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_agg_ul_size");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Warning: reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.data_format.ul_agg_size = 0;
  }
  netmgr_main_cfg.data_format.ul_agg_size = prop_val_int;

  /* dataformat_dl_data_aggregation_protocol */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_dl_data_aggregation_protocol");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.dl_data_aggregation_protocol = (int)prop_val_int;

  /* dataformat_ul_data_aggregation_protocol */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_ul_data_aggregation_protocol");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.ul_data_aggregation_protocol = (int)prop_val_int;

  /* dataformat_dl_gro_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_dl_gro_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.data_format.dl_gro_enabled = (int)prop_val_int;

  /* dataformat_ul_gso_enabled */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dataformat_ul_gso_enabled");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.data_format.ul_gso_enabled = (int)prop_val_int;

  /* phys_net_dev */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".phys_net_dev");
  memset(prop_val_str, 0, sizeof(prop_val_str));
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, prop_val_str, sizeof(prop_val_str));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

  strlcpy(netmgr_main_cfg.phys_net_dev, prop_val_str, sizeof(netmgr_main_cfg.phys_net_dev));

  /* pnd_rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".pnd_rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.pnd_rps_mask = (int)prop_val_int;

  /* vnd_rps_mask */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".vnd_rps_mask");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.vnd_rps_mask = (int)prop_val_int;

 if (DS_TARGET_SOC_ID_MSM8996_SGAU == ds_get_soc_id())
 {
    netmgr_main_cfg.vnd_rps_mask = 13;
 }
  /* netdev_budget */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_budget");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.netdev_budget = (int)prop_val_int;

#ifdef NETMGR_OFFTARGET
  netmgr_main_cfg.low_latency_filters = FALSE;
#else
  /* low_latency_filters */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".low_latency_filters");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.low_latency_filters = (prop_val_int ? TRUE : FALSE);
#endif

  /* tc_ul_baserate */

  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_baserate");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_baserate = 0;
  }
  else
  {
    /* TODO: configdb does not have a UL type; we are limited to ~2Gbps */
    netmgr_main_cfg.tc_ul_baserate = (unsigned long)prop_val_int;
  }

  /* dynamic_tc_ul_baserate */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".dynamic_tc_ul_baserate");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.dynamic_tc_ul_baserate = (int)prop_val_int;

  /* tc_ul_burst */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".tc_ul_burst");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_med("Error reading property [%s] Err[%d]; setting to 0", prop_name, ret);
    netmgr_main_cfg.tc_ul_burst = 0;
  }
  else
  {
    netmgr_main_cfg.tc_ul_burst = (unsigned long)prop_val_int;
  }

  /* netdev_max_backlog */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".netdev_max_backlog");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.netdev_max_backlog = (int)prop_val_int;

  /* frag_low_mem */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".frag_low_mem");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.frag_low_mem = (int)prop_val_int;

  /* frag_high_mem */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".frag_high_mem");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
    prop_val_int = 0;
  }
  netmgr_main_cfg.frag_high_mem = (int)prop_val_int;

#ifdef FEATURE_DATA_CV2X
  /* Max configurable MTU */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".max_mtu");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
  if (ret != CFGDB_SUCCESS)
  {
    prop_val_int = 0;
    netmgr_log_med("Error reading property [%s] Err[%d]; Setting to 0", prop_name, ret);
  }
  netmgr_main_cfg.max_mtu = (int) prop_val_int;
#endif /* FEATURE_DATA_CV2X */

  /* num_modems and enable status */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".num_modems");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &num_modems, sizeof(num_modems));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  for (i = 0; i < num_modems; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".modems_enabled.",i);
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &prop_val_int, sizeof(prop_val_int));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    netmgr_main_cfg.modem_enable[i] = (prop_val_int ? TRUE : FALSE);
  }

  /* Disable all the links first, only enable ones present in config file */
  for (i = 0; i < NETMGR_MAX_LINK; i++)
  {
    netmgr_ctl_port_array[i].enabled = FALSE;
  }

  /* control_ports_len and control_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".control_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &control_ports_len, sizeof(control_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }

/* Updating data format acc to netmgr_config.xml */
  netmgr_main_cfg.data_format.num_mux_channels = control_ports_len;
  netmgr_main_cfg.data_format.num_iwlan_channels = control_ports_len + 1;

  for (i = 0; i < control_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".control_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, prop_val_str, sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

  /* data_ports_len and data_ports */
  snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s", "netmgr_config.", target, ".data_ports_len");
  ret = configdb_get_parameter(prop_name, CFGDB_TYPE_INT, &data_ports_len, sizeof(data_ports_len));
  if (ret != CFGDB_SUCCESS)
  {
    netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
    result = -1;
    goto bail;
  }
  netmgr_main_cfg.num_data_ports = data_ports_len;

  for (i = 0; i < data_ports_len; i++)
  {
    snprintf(prop_name, NETMGR_CFGDB_STRVAL_MAX, "%s%s%s%d", "netmgr_config.", target, ".data_ports.",i);
    memset(prop_val_str, 0, sizeof(prop_val_str));
    ret = configdb_get_parameter(prop_name, CFGDB_TYPE_STRING, &prop_val_str, sizeof(prop_val_str));
    if (ret != CFGDB_SUCCESS)
    {
      netmgr_log_err("Error reading property [%s] Err[%d]", prop_name, ret);
      result = -1;
      goto bail;
    }
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, prop_val_str, sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

#ifdef FEATURE_DATA_IWLAN
  /* Process iWLAN related configuration items */
  if (0 != netmgr_process_iwlan_config(target))
  {
    netmgr_log_err("%s", "Error processing iWLAN config items");
    result = -1;
    goto bail;
  }
#endif /* FEATURE_DATA_IWLAN */

  netmgr_process_cmd_logging_config();

  /* Remove prefix table as an enhancement later */
  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

bail:
  return result;
}

#else /* FEATURE_EXTERNAL_AP */
const char* netmgr_device_control_port_names [] =
{
  QMI_PORT_RMNET_DATA_0,
  QMI_PORT_RMNET_DATA_1,
  QMI_PORT_RMNET_DATA_2,
  QMI_PORT_RMNET_DATA_3,
  QMI_PORT_RMNET_DATA_4,
  QMI_PORT_RMNET_DATA_5,
  QMI_PORT_RMNET_DATA_6,
  QMI_PORT_RMNET_DATA_7
};

/*===========================================================================
  FUNCTION:  netmgr_config_load_ex_ap
===========================================================================*/
/*!
    @brief
    This function loads the configuration for external ap using hardcoded
    values in this function instead of a config xml file.

    @param[in] target: configuration to be used within the XML file.

    @return  0 if configuration file is loaded properly.
    @return -1 if configuration file file load/parse fails.
*/
/*=========================================================================*/
int netmgr_config_load_ex_ap
(
  const char *target
)
{
  int i;

  netmgr_log_med("netmgr_config: Configuring for target %s\n", target);

  netmgr_main_cfg.dpm_enabled = FALSE;
  netmgr_main_cfg.use_qmuxd = FALSE;
  netmgr_main_cfg.dpm_retry_timeout = 10000;
  netmgr_main_cfg.wda_data_format = TRUE;

  netmgr_main_cfg.single_qmux_ch_enabled = TRUE;

  netmgr_main_cfg.max_mtu = 9216;

  /*single_qmux_ch_conn_id_str*/
  strlcpy(netmgr_main_cfg.qmux_ch_name, "rmnet_mhi0", sizeof(netmgr_main_cfg.qmux_ch_name));

  /*single_qmux_ch_name*/
  strlcpy(netmgr_main_cfg.smd_ch_name, "MHICTL0", sizeof(netmgr_main_cfg.smd_ch_name));

  netmgr_main_cfg.rmnet_data_enabled = TRUE;
  netmgr_main_cfg.data_format.dl_agg_cnt = 20;
  netmgr_main_cfg.data_format.dl_agg_size = 16384;
  netmgr_main_cfg.data_format.ul_agg_cnt = 0;
  netmgr_main_cfg.data_format.ul_agg_size = 0;
  netmgr_main_cfg.data_format.dl_data_aggregation_protocol = 5;
  netmgr_main_cfg.data_format.ul_data_aggregation_protocol = 5;

  strlcpy(netmgr_main_cfg.phys_net_dev,"rmnet_mhi0", sizeof(netmgr_main_cfg.phys_net_dev));

  netmgr_main_cfg.data_format.dl_gro_enabled = 0;
  netmgr_main_cfg.data_format.ul_gso_enabled = 0;
  netmgr_main_cfg.qos_via_idl = FALSE;
  netmgr_main_cfg.tc_ul_baserate = 0;

  netmgr_main_cfg.low_latency_filters = FALSE;
  netmgr_main_cfg.frag_low_mem = 300000;
  netmgr_main_cfg.frag_high_mem = 400000 ;
  netmgr_main_cfg.modem_enable[0] = TRUE;
  netmgr_main_cfg.modem_enable[1] = FALSE;

  /* Disable all the links first, only enable ones present in config file */
  for (i = 0; i < NETMGR_MAX_LINK; i++)
  {
     netmgr_ctl_port_array[i].enabled = FALSE;
  }

  netmgr_main_cfg.data_format.num_mux_channels = 8;

  for(i = 0; i < netmgr_main_cfg.data_format.num_mux_channels; i++ )
  {
    strlcpy(netmgr_ctl_port_array[i].qmi_conn_id, netmgr_device_control_port_names[i], sizeof(netmgr_ctl_port_array[i].qmi_conn_id));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

  netmgr_main_cfg.num_data_ports = 8;
  for(i = 0; i < netmgr_main_cfg.num_data_ports; i++ )
  {
    strlcpy(netmgr_ctl_port_array[i].data_ctl_port, netmgr_device_control_port_names[i], sizeof(netmgr_ctl_port_array[i].data_ctl_port));
    netmgr_ctl_port_array[i].enabled = TRUE;
  }

  if (netmgr_main_cfg.rmnet_data_enabled)
  {
    if (netmgr_main_cfg.modem_enable[0])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MSM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
    else if(netmgr_main_cfg.modem_enable[1])
    {
      /* If rmnet_data is enabled, then update the dev-prefix for approriate modem */
      strlcpy(NETMGR_MAIN_GET_DEV_PREFIX(NETMGR_MODEM_MDM),
              NETMGR_MAIN_RMNET_DATA_PREFIX,
              NETMGR_IF_NAME_MAX_LEN);
    }
  }

  return NETMGR_SUCCESS;
}
#endif /* FEATURE_EXTERNAL_AP */

/*===========================================================================
  FUNCTION:  netmgr_config_print
===========================================================================*/
/*!
    @brief
    This function prints the configuration currently loaded for netmgr.

    @param None.
    @return  None.
*/
/*=========================================================================*/
void netmgr_config_print()
{
  int i;
  netmgr_log_med("==== Netmanager Configuration ====");
  netmgr_log_med("..qmi_dpm_enabled        : [%d]",     netmgr_main_cfg.dpm_enabled);
  netmgr_log_med("..dpm_retry_timeout      : [%d]",     netmgr_main_cfg.dpm_retry_timeout);
  netmgr_log_med("..wda_data_format_enabled: [%d]",     netmgr_main_cfg.wda_data_format);
  netmgr_log_med("..ep_type                : [%d]",     netmgr_main_cfg.ep_type);
  netmgr_log_med("..single_qmux_ch_enabled : [%d]",     netmgr_main_cfg.single_qmux_ch_enabled);
  netmgr_log_med("..single_qmux_ch_name    : [%s]",     netmgr_main_cfg.qmux_ch_name);
  netmgr_log_med("..single_qmux_ch_id      : [%s]",     netmgr_main_cfg.smd_ch_name);
  netmgr_log_med("..rmnet_data_enabled     : [%d]",     netmgr_main_cfg.rmnet_data_enabled);
  netmgr_log_med("..phys_net_dev           : [%s]",     netmgr_main_cfg.phys_net_dev);
  netmgr_log_med("..msm_modem_enabled      : [%d]",     netmgr_main_cfg.modem_enable[NETMGR_MODEM_MSM]);
  netmgr_log_med("..mdm_modem_enabled      : [%d]",     netmgr_main_cfg.modem_enable[NETMGR_MODEM_MDM]);
  netmgr_log_med("..modem_ssr_state        : [%d]",     netmgr_main_cfg.modem_ssr_state);
  netmgr_log_med("..df.dl_agg_size         : [%d]",     netmgr_main_cfg.data_format.dl_agg_size);
  netmgr_log_med("..df.dl_agg_cnt          : [%d]",     netmgr_main_cfg.data_format.dl_agg_cnt);
  netmgr_log_med("..df.dl_agg_mode         : [%d]",     netmgr_main_cfg.data_format.dl_data_aggregation_protocol);
  netmgr_log_med("..df.ul_agg_mode         : [%d]",     netmgr_main_cfg.data_format.ul_data_aggregation_protocol);
  netmgr_log_med("..df.dl_gro_enabled      : [%d]",     netmgr_main_cfg.data_format.dl_gro_enabled);
  netmgr_log_med("..exec_opts.log_options  : [0x%08X]", netmgr_main_cfg.exec_opts.log_options);
  netmgr_log_med("..exec_opts.outfile      : [%s]",     netmgr_main_cfg.exec_opts.outfile);
  netmgr_log_med("..df.ul_gso_enabled      : [%d]", netmgr_main_cfg.data_format.ul_gso_enabled);

  for (i = 0; i < NETMGR_MAX_LINK; i++)
  {
    netmgr_log_med("Link[%d] port[%s] qmi[%s] modem_wait[%d] enabled[%d]",
                    netmgr_ctl_port_array[i].link_id,
                    netmgr_ctl_port_array[i].data_ctl_port,
                    netmgr_ctl_port_array[i].qmi_conn_id,
                    netmgr_ctl_port_array[i].modem_wait,
                    netmgr_ctl_port_array[i].enabled);
  }
  netmgr_log_med("========");


  return;
}

/*===========================================================================
  FUNCTION:  netmgr_config_print_cb
===========================================================================*/
/*!
    @brief
    Wrapper around netmgr_config_print() to make it work with CB framework
*/
/*=========================================================================*/
LOCAL void *
netmgr_config_print_cb(void *data)
{
  netmgr_config_print();
  return 0;
}

/*===========================================================================
  FUNCTION:  netmgr_config_init
===========================================================================*/
/*!
    @brief
    Initialize the configration framework. Things that don't belong in actual
    configuration loading.
*/
/*=========================================================================*/
void
netmgr_config_init()
{
  /* Register callbacks with the MAIN table */
  netmgr_cb_insert_cb(NETMGR_MAIN_CB_TABLE,
                      NETMGR_MAIN_CB_SIGUSR1,
                      NETMGR_CONFIG_CB_MODNAME,
                      &netmgr_config_print_cb);
}
