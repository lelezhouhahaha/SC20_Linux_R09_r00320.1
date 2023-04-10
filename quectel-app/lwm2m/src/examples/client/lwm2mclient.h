/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH, Germany.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/
/*
 * lwm2mclient.h
 *
 *  General functions of lwm2m test client.
 *
 *  Created on: 22.01.2015
 *  Author: Achim Kraus
 *  Copyright (c) 2015 Bosch Software Innovations GmbH, Germany. All rights reserved.
 */

#ifndef LWM2MCLIENT_H_
#define LWM2MCLIENT_H_

#include "liblwm2m.h"

extern int g_reboot;

typedef enum
{
	LWM2M_CLIENT_OK = 0,
	LWM2M_CLIENT_ERR_INVALID_PARAM = -1,
	LWM2M_CLIENT_ERR_WODBLOCK = -2,
	LWM2M_CLIENT_ERR_NET_FAIL = -3,
	LWM2M_CLIENT_ERR_DNS_FAIL = -4,
	LWM2M_CLIENT_ERR_NET_ACTIVING = -5,
	LWM2M_CLIENT_ERR_OUT_MEM = -6,
	LWM2M_CLIENT_ERR_SOCK_BIND_FAIL = -7,
	LWM2M_CLIENT_ERR_SOCK_CONN_FAIL = -8,
	LWM2M_CLIENT_ERR_REG_FAIL = -9,
	LWM2M_CLIENT_ERR_UPDATE_FAIL = -10,
	LWM2M_CLIENT_ERR_DEREG_FAIL = -11,
	LWM2M_CLIENT_ERR_DTLS_FAIL = -12,
	LWM2M_CLIENT_ERR_NOT_SUPPORT = -13,
	LWM2M_CLIENT_ERR_OPEN_FAIL = -14,
	LWM2M_CLIENT_ERR_CLOSE_FAIL = -15,
	LWM2M_CLIENT_ERR_BOOTSTRAP_FAIL = -16,
}lwm2m_client_error_code;

typedef enum
{
	LWM2M_CLIENT_OPT_PDPCID = 1,
	LWM2M_CLIENT_OPT_EPNAME_MODE,
	LWM2M_CLIENT_OPT_IMSI,
	LWM2M_CLIENT_OPT_IMEI,
	LWM2M_CLIENT_OPT_MSISDN,
#if defined(CHINA_MOBILE_LWM2M_CLIENT)
	LWM2M_CLIENT_OPT_APPKEY,
	LWM2M_CLIENT_OPT_APPPWD,
#endif
	LWM2M_CLIENT_OPT_OBJLIST,
	LWM2M_CLIENT_OPT_LOCAL_PORT,
	LWM2M_CLIENT_OPT_FW_DLOAD_STATE,
	LWM2M_CLIENT_OPT_UPDATE_RSRQ,
	LWM2M_CLIENT_OPT_UPDATE_RSRP,
}lwm2m_client_option_type;

typedef enum
{
	LWM2M_CLIENT_INIT = 0,
	LWM2M_CLIENT_PDP_ACTIVE,
	LWM2M_CLIENT_PDP_DEACTIVE,
	LWM2M_CLIENT_DNS_QUERY,
	LWM2M_CLIENT_SOC_NEW,
    LWM2M_CLIENT_DTLS,
    LWM2M_CLIENT_BOOTSTRAPING,
	LWM2M_CLIENT_BOOTSTRAP,
	LWM2M_CLIENT_REGISTERING,
	LWM2M_CLIENT_READY,
	LWM2M_CLIENT_DEREG ,
}lwm2m_client_stat;


typedef enum
{
	LWM2M_CLIENT_STATE_CHG_IND = 1,
	LWM2M_CLIENT_OPT_OBJ_IND = 2,
	LWM2M_CLIENT_FW_DLOAD_IND = 3,
	LWM2M_CLIENT_FW_UPDATE_IND = 4,
	LWM2M_CLIENT_REBOOT_IND = 5,
	LWM2M_CLIENT_UPDATE_IND = 6,
	LWM2M_CLIENT_FW_DATA_IND = 7,
}lwm2m_client_event_type;
//add by joe start
typedef struct{
	int 	result;
	int 	state;
	int 	short_server_id;
	char    apn[256];
}lwm2m_state_event_info;

typedef struct{
	int    opt_code;
	int    obj_id;
	int    instance_id;
	int    resource_id;
}lwm2m_obj_opt_event_info;

typedef struct{
	uint8_t   start;
	uint8_t   end;
	uint32_t  length;
}lwm2m_fw_data_event;

typedef union{
	lwm2m_state_event_info     state_info;
	lwm2m_obj_opt_event_info   opt_info;
	lwm2m_fw_data_event        fw_data_info;
	char                       pkg_url[256];
}lwm2m_event_info;
//add by joe end
int lwm2m_client_write_object_resource(lwm2m_context_t * lwm2mH, int obj_id, int instance_id, int res_id, void *res_data);
/*
 * object_device.c
 */
lwm2m_object_t * get_object_device(void);
void free_object_device(lwm2m_object_t * objectP);
uint8_t device_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
void display_device_object(lwm2m_object_t * objectP);
/*
 * object_firmware.c
 */
lwm2m_object_t * get_object_firmware(void);
void free_object_firmware(lwm2m_object_t * objectP);
void display_firmware_object(lwm2m_object_t * objectP);
/*
 * object_location.c
 */
lwm2m_object_t * get_object_location(void);
void free_object_location(lwm2m_object_t * object);
void display_location_object(lwm2m_object_t * objectP);
/*
 * object_test.c
 */
#define TEST_OBJECT_ID 31024
lwm2m_object_t * get_test_object(void);
void free_test_object(lwm2m_object_t * object);
void display_test_object(lwm2m_object_t * objectP);
/*
 * object_server.c
 */
lwm2m_object_t * get_server_object(int serverId, const char* binding, int lifetime, bool storing, uint16_t instanceId);
void clean_server_object(lwm2m_object_t * object);
void display_server_object(lwm2m_object_t * objectP);
void copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_connectivity_moni.c
 */
lwm2m_object_t * get_object_conn_m(void);
void free_object_conn_m(lwm2m_object_t * objectP);
uint8_t connectivity_moni_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);

/*
 * object_connectivity_stat.c
 */
extern lwm2m_object_t * get_object_conn_s(void);
void free_object_conn_s(lwm2m_object_t * objectP);
extern void conn_s_updateTxStatistic(lwm2m_object_t * objectP, uint16_t txDataByte, bool smsBased);
extern void conn_s_updateRxStatistic(lwm2m_object_t * objectP, uint16_t rxDataByte, bool smsBased);

/*
 * object_access_control.c
 */
lwm2m_object_t* acc_ctrl_create_object(void);
void acl_ctrl_free_object(lwm2m_object_t * objectP);
bool  acc_ctrl_obj_add_inst (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t acObjectId, uint16_t acObjInstId, uint16_t acOwner);
bool  acc_ctrl_oi_add_ac_val(lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t aclResId, uint16_t acValue);
/*
 * lwm2mclient.c
 */
void handle_value_changed(lwm2m_context_t* lwm2mH, lwm2m_uri_t* uri, const char * value, size_t valueLength);
/*
 * system_api.c
 */
void init_value_change(lwm2m_context_t * lwm2m);
void system_reboot(void);

/*
 * object_security.c
 */
lwm2m_object_t * get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
void clean_security_object(lwm2m_object_t * objectP);
char * get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
void display_security_object(lwm2m_object_t * objectP);
void copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_host_device.c
 */
lwm2m_object_t * get_object_host_dev(void);
void free_object_host_device(lwm2m_object_t * object);

/*
 * object_apn_connection_profile.c
 */
lwm2m_object_t * get_object_apn_conn();
void free_object_apn_conn(lwm2m_object_t * objectP);

/*
 * object_connectivity_exte.c
 */
lwm2m_object_t * get_object_conn_ex();
void free_object_conn_ex(lwm2m_object_t * objectP);

#ifdef ENABLE_SOFTWARE_MGNT_OBJ
/*
 * object_software_mgnt.c
 */
lwm2m_object_t * get_object_software_mgnt();
void free_object_softwaremgnt(lwm2m_object_t * objectP);
#endif

//add by joe
bool lwm2m_check_is_vzw_netwrok();
bool lwm2m_check_is_att_netwrok();

bool lwm2m_client_handle_wap_sms(lwm2m_context_t *lwm2mH, uint8_t *data, uint16_t data_len);
void lwm2m_client_update_all_server_regist();
char* GetDeviceImei();
void GetDevicePhoneNumber(char* num);
void * pthread_init_device_info_and_network(void *arg);

char * get_ipaddr();
#endif /* LWM2MCLIENT_H_ */
