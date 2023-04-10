/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  btdamo.c
 *    Description:  从NV获取BT地址，在设置BT的MAC. 
 *                 
 *        Version:  1.0.0(2019年11月27日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2019年11月27日 18时43分59秒"
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cutils/properties.h>
#include <qmi-framework/qmi_client.h>
#include <qmi-framework/qmi_client_instance_defs.h>

#include "ql_mcm_gps.h"
#include "mcm_loc_v01.h"

static qmi_client_type loc_qmi_client;
static QL_LOC_LOCATION_INFO_T t_loc_info;

void qmi_loc_ind_cb(qmi_client_type user_handle, unsigned int msg_id,
		void *ind_buf, unsigned int ind_buf_len, void *resp_cb_data)
{
	int ret = 0;
	void *ind_data = NULL;
	int ind_data_len = 0;

	if (!user_handle || !ind_buf) {
		printf ("qmi_loc_ind_cb bad param(s)\n");
		return;
	}

	switch(msg_id) {
	// LOC
	case MCM_LOC_LOCATION_INFO_IND_V01:
		ind_data_len = sizeof(mcm_loc_location_info_ind_msg_v01);
		break;
	case MCM_LOC_STATUS_INFO_IND_V01:
		ind_data_len = sizeof(mcm_loc_status_info_ind_msg_v01);
		break;
	case MCM_LOC_SV_INFO_IND_V01:
		ind_data_len = sizeof(mcm_loc_sv_info_ind_msg_v01);
		break;
	case MCM_LOC_NMEA_INFO_IND_V01:
		ind_data_len = sizeof(mcm_loc_nmea_info_ind_msg_v01);
		break;
	case MCM_LOC_CAPABILITIES_INFO_IND_V01:
		ind_data_len = sizeof(mcm_loc_capabilities_info_ind_msg_v01);
		break;
	case MCM_LOC_UTC_TIME_REQ_IND_V01:
		ind_data_len = sizeof(mcm_loc_utc_time_req_ind_msg_v01);
		break;
	case MCM_LOC_XTRA_DATA_REQ_IND_V01:
		ind_data_len = sizeof(mcm_loc_xtra_data_req_ind_msg_v01);
		break;
	case MCM_LOC_AGPS_STATUS_IND_V01:
		ind_data_len = sizeof(mcm_loc_agps_status_ind_msg_v01);
		break;
	case MCM_LOC_NI_NOTIFICATION_IND_V01:
		ind_data_len = sizeof(mcm_loc_ni_notification_ind_msg_v01);
		break;
	case MCM_LOC_XTRA_REPORT_SERVER_IND_V01:
		ind_data_len = sizeof(mcm_loc_xtra_report_server_ind_msg_v01);
		break;
	default:
		ret = MCM_ERROR_GENERIC_V01;
		break;
	}

	if (ret != 0) {
		printf ("msg_id is unknown\n");
		return;
	}

	if (ind_data_len > 0) {
		//printf ("fulinux 1 ind_data_len = %d\n", ind_data_len);
		ind_data = (void *)malloc(ind_data_len);
	}

	ret = qmi_client_message_decode(user_handle,
			QMI_IDL_INDICATION,
			msg_id,
			ind_buf,
			ind_buf_len,
			ind_data,
			ind_data_len);
	if (ret != 0) {
		printf ("qmi_client_message_decode error\n");
		return;
	}

	switch(msg_id) {
	case MCM_LOC_STATUS_INFO_IND_V01:
	{
		mcm_loc_status_info_ind_msg_v01 *status_info = (mcm_loc_status_info_ind_msg_v01*)ind_data;
		printf ("qmi_loc_ind_cb loc status = %d\n", status_info->status.status);
		break;
	}
	case MCM_LOC_LOCATION_INFO_IND_V01://0x30E
	{
		mcm_gps_location_t_v01  *pt_location;
		mcm_loc_location_info_ind_msg_v01 *ind = (mcm_loc_location_info_ind_msg_v01*)ind_data;

		pt_location = (mcm_gps_location_t_v01*)&ind->location;

		t_loc_info.size = pt_location->size;
		t_loc_info.flags = pt_location->flags;
		t_loc_info.position_source = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_SOURCE_INFO_V01)? pt_location->position_source:0;
		t_loc_info.latitude = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_LAT_LONG_V01)? pt_location->latitude:0;
		t_loc_info.longitude = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_LAT_LONG_V01)? pt_location->longitude:0;
		t_loc_info.altitude = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_ALTITUDE_V01)? pt_location->altitude:0;
		t_loc_info.speed = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_SPEED_V01)? pt_location->speed:0;
		t_loc_info.bearing = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_BEARING_V01)? pt_location->bearing:0;
		t_loc_info.accuracy = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_ACCURACY_V01)? pt_location->accuracy:0;
		t_loc_info.timestamp = pt_location->timestamp;
		t_loc_info.is_indoor = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_IS_INDOOR_V01)? pt_location->is_indoor:0;
		t_loc_info.floor_number = (pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_FLOOR_NUMBE_V01)? pt_location->floor_number:0;
		t_loc_info.raw_data_len = pt_location->raw_data_len;

		memcpy(t_loc_info.raw_data, pt_location->raw_data, pt_location->raw_data_len);
		if(pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_MAP_URL_V01) {
			memcpy(t_loc_info.map_url, pt_location->map_url, QL_LOC_GPS_LOCATION_MAP_URL_SIZE);
		}
		if(pt_location->flags & MCM_LOC_GPS_LOCATION_HAS_MAP_INDEX_V01) {
			memcpy(t_loc_info.map_index, pt_location->map_index, QL_LOC_GPS_LOCATION_MAP_IDX_SIZE);
		}

		printf("**** Latitude = %lf, Longitude=%lf, altitude=%lf, accuracy = %f ****\n",
				t_loc_info.latitude, t_loc_info.longitude, t_loc_info.altitude, t_loc_info.accuracy);
		break;
	}
	case MCM_LOC_SV_INFO_IND_V01:
	{
		mcm_loc_sv_info_ind_msg_v01 *ind = (mcm_loc_sv_info_ind_msg_v01*)ind_data;
		QL_LOC_SV_STATUS_T *pt_sv_status = (QL_LOC_SV_STATUS_T*)malloc(sizeof(QL_LOC_SV_STATUS_T));

		if(pt_sv_status == NULL) {
			printf("Allocate mem fail!\n");
			break;
		}
		memset(pt_sv_status, 0, sizeof(QL_LOC_SV_STATUS_T));

		pt_sv_status->size              = ind->sv_info.size;
		pt_sv_status->num_svs           = ind->sv_info.num_svs;
		memcpy(pt_sv_status->sv_list, ind->sv_info.sv_list, sizeof(QL_LOC_SV_INFO_T)*QL_LOC_GPS_SUPPORT_SVS_MAX);
		pt_sv_status->ephemeris_mask    = ind->sv_info.ephemeris_mask;
		pt_sv_status->almanac_mask		= ind->sv_info.almanac_mask;
		pt_sv_status->used_in_fix_mask	= ind->sv_info.used_in_fix_mask;

		free(pt_sv_status);
		pt_sv_status = NULL;
		break;
	}
	case MCM_LOC_NMEA_INFO_IND_V01:
	{
		mcm_loc_nmea_info_ind_msg_v01 *ind = (mcm_loc_nmea_info_ind_msg_v01*)ind_data;
		QL_LOC_NMEA_INFO_T t_nmea = {0};

		t_nmea.timestamp = ind->timestamp;
		t_nmea.length = ind->length;
		memcpy(t_nmea.nmea, ind->nmea, ind->length);

		break;
	}
	case MCM_LOC_CAPABILITIES_INFO_IND_V01:
	{
		E_QL_LOC_CAPABILITIES_T e_cap;
		mcm_loc_capabilities_info_ind_msg_v01 *ind = (mcm_loc_capabilities_info_ind_msg_v01*)ind_data;

		e_cap = (E_QL_LOC_CAPABILITIES_T)ind->capabilities;
		break;
	}
	case MCM_LOC_AGPS_STATUS_IND_V01:
	{
		mcm_loc_agps_status_ind_msg_v01 *ind = (mcm_loc_agps_status_ind_msg_v01*)ind_data;
		QL_LOC_AGPS_STATUS_T t_agps = {0};

		t_agps.size = ind->status.size;
		t_agps.type = ind->status.type;
		t_agps.status = ind->status.status;
		t_agps.ipv4_addr= ind->status.ipv4_addr;
		memcpy(t_agps.ipv6_addr, ind->status.ipv6_addr, QL_LOC_IPV6_ADDR_LEN);
		memcpy(t_agps.ssid, ind->status.ssid, QL_LOC_GPS_SSID_BUF_SIZE);
		memcpy(t_agps.password, ind->status.password, QL_LOC_GPS_SSID_BUF_SIZE);
		break;
	}
	case MCM_LOC_NI_NOTIFICATION_IND_V01:
	{
		mcm_loc_ni_notification_ind_msg_v01 *ind = (mcm_loc_ni_notification_ind_msg_v01*)ind_data;
		QL_LOC_NI_NOTIFICATION_INTO_T *pt_ni_nfy = NULL;

		pt_ni_nfy = (QL_LOC_NI_NOTIFICATION_INTO_T*)malloc(sizeof(QL_LOC_NI_NOTIFICATION_INTO_T));
		if(pt_ni_nfy == NULL) {
			printf("Allocate mem fail!\n");
			break;
		}

		pt_ni_nfy->size             = ind->notification.size;
		pt_ni_nfy->notification_id  = ind->notification.notification_id;
		pt_ni_nfy->ni_type          = (E_QL_LOC_NI_TYPE_T)ind->notification.ni_type;
		pt_ni_nfy->notify_flags     = (E_QL_LOC_NI_NOTIFY_FLAGS_T)ind->notification.notify_flags;
		pt_ni_nfy->timeout          = ind->notification.timeout;
		pt_ni_nfy->default_response = (E_QL_LOC_NI_USER_RESPONSE_TYPE_T)ind->notification.default_response;
		pt_ni_nfy->requestor_id_encoding = (E_QL_LOC_NI_ENC_TYPE_T)ind->notification.requestor_id_encoding;
		pt_ni_nfy->text_encoding    = (E_QL_LOC_NI_ENC_TYPE_T)ind->notification.text_encoding;
		memcpy(pt_ni_nfy->requestor_id, ind->notification.requestor_id, QL_LOC_NI_SHORT_STRING_MAXLEN);
		memcpy(pt_ni_nfy->text, ind->notification.text, QL_LOC_NI_LONG_STRING_MAXLEN);
		memcpy(pt_ni_nfy->extras, ind->notification.extras, QL_LOC_NI_LONG_STRING_MAXLEN);

		free(pt_ni_nfy);
		pt_ni_nfy = NULL;
		break;
	}
	case MCM_LOC_XTRA_REPORT_SERVER_IND_V01:
	{
		mcm_loc_xtra_report_server_ind_msg_v01 *ind = (mcm_loc_xtra_report_server_ind_msg_v01*)ind_data;
		QL_LOC_XTRA_REPORT_SERVER_INTO_T *pt_xtra_rptsrv = NULL;

		pt_xtra_rptsrv = (QL_LOC_XTRA_REPORT_SERVER_INTO_T*)malloc(sizeof(QL_LOC_XTRA_REPORT_SERVER_INTO_T));
		if(pt_xtra_rptsrv == NULL) {
			printf("Allocate mem fail!\n");
			break;
		}
		memcpy(pt_xtra_rptsrv->server1, ind->server1, QL_LOC_MAX_SEVER_ADDR_LENGTH);
		memcpy(pt_xtra_rptsrv->server2, ind->server2, QL_LOC_MAX_SEVER_ADDR_LENGTH);
		memcpy(pt_xtra_rptsrv->server3, ind->server3, QL_LOC_MAX_SEVER_ADDR_LENGTH);

		free(pt_xtra_rptsrv);
		pt_xtra_rptsrv = NULL;
		break;
	}
	case MCM_LOC_UTC_TIME_REQ_IND_V01:
	case MCM_LOC_XTRA_DATA_REQ_IND_V01:
	//empty msg, no need to handle
	default:
		printf ("qmi_loc_ind_cb receive msg_id = 0x%X\n", msg_id);
		break;
	}

	if (ind_data) {
		free(ind_data);
		ind_data = NULL;
	}

	return;
}

int qmi_loc_init(void)
{
	printf ("%s: entry\n", __func__);
	qmi_client_error_type qmi_client_err = QMI_NO_ERR;
	qmi_idl_service_object_type loc_service;
	qmi_client_qmux_instance_type qmi_modem_port;
	int timeout = 3000;

	loc_service = mcm_loc_get_service_object_v01();
	if (!loc_service) {
		printf("%s: Not able to get loc service handle\n", __func__);
		return -1;
	}

	qmi_modem_port = QMI_CLIENT_INSTANCE_ANY;

	qmi_client_err = qmi_client_init_instance(loc_service, qmi_modem_port,
			qmi_loc_ind_cb, NULL, NULL, timeout, &loc_qmi_client);

	if (qmi_client_err != QMI_NO_ERR) {
		printf("%s: Error while initializing qmi_client_init_instance: %d\n",
				__func__, qmi_client_err);
		return -2;
	}

	//FIXME: qmi_client_register_error_cb

	printf ("%s: exit\n", __func__);

	return 0;
}

int qmi_loc_exit(void)
{
	qmi_client_error_type qmi_client_err = QMI_NO_ERR;

	qmi_client_err = qmi_client_release(loc_qmi_client);
	if(qmi_client_err != QMI_NO_ERR){
		printf("Error: while releasing qmi_client : %d\n", qmi_client_err);
		return -1;
	}

	loc_qmi_client = 0;

	return 0;
}

int loc_set_indications(void)
{
	qmi_client_error_type qmi_client_err;
	mcm_loc_set_indications_req_msg_v01 req;
	mcm_loc_set_indications_resp_msg_v01 resp;

	printf ("%s: entry\n", __func__);
	memset(&req,  0, sizeof(mcm_loc_set_indications_req_msg_v01));
	memset(&resp, 0, sizeof(mcm_loc_set_indications_resp_msg_v01));

	req.register_location_info_ind = 1;
	req.register_status_info_ind = 1;
	req.register_sv_info_ind = 1;
	req.register_nmea_info_ind = 1;
	req.register_capabilities_info_ind = 1;
	req.register_utc_time_req_ind = 1;
	req.register_xtra_data_req_ind = 1;
	req.register_agps_data_conn_cmd_req_ind = 1;
	req.register_ni_notify_user_response_req_ind = 1;

	qmi_client_err = qmi_client_send_msg_sync(loc_qmi_client,
			MCM_LOC_SET_INDICATIONS_REQ_V01, &req, sizeof(req), &resp,
			sizeof(resp), 3000);

	if (qmi_client_err != QMI_NO_ERR) {
		printf("%s: Error %d\n", __func__, qmi_client_err);
		return -1;
	}

	printf ("%s: exit\n", __func__);

	return 0;
}

int loc_set_position_mode(void)
{
	qmi_client_error_type qmi_client_err;
	mcm_loc_set_position_mode_req_msg_v01 set_position_req;
	mcm_loc_set_position_mode_resp_msg_v01 set_position_resp;

	printf ("%s: entry\n", __func__);

	set_position_req.mode = 0;
	set_position_req.recurrence = 0;
	set_position_req.min_interval = 1000;//E_QL_LOC_POS_RECURRENCE_PERIODIC
	set_position_req.preferred_accuracy = 0;// < 50m
	set_position_req.preferred_time = 5;//10s

	qmi_client_err = qmi_client_send_msg_sync(loc_qmi_client,
			MCM_LOC_SET_POSITION_MODE_REQ_V01, &set_position_req, sizeof(set_position_req), &set_position_resp,
			sizeof(set_position_resp), 3000);

	if (qmi_client_err != QMI_NO_ERR) {
		printf("%s: Error %d\n", __func__, qmi_client_err);
		return -1;
	}

	printf ("%s: exit\n", __func__);
	return 0;
}

int loc_start_navigation(void)
{
	qmi_client_error_type qmi_client_err;
	mcm_loc_start_nav_req_msg_v01 start_nav_req;
	mcm_loc_start_nav_resp_msg_v01 start_nav_resp;

	printf ("%s: entry\n", __func__);
	/* clear the request content */
	memset(&start_nav_req,  0, sizeof(mcm_loc_start_nav_req_msg_v01));
	memset(&start_nav_resp, 0, sizeof(mcm_loc_start_nav_resp_msg_v01));

	qmi_client_err = qmi_client_send_msg_sync(loc_qmi_client,
			MCM_LOC_START_NAV_REQ_V01, &start_nav_req, sizeof(start_nav_req), &start_nav_resp,
			sizeof(start_nav_resp), 3000);

	if (qmi_client_err != QMI_NO_ERR) {
		printf("%s: Error %d\n", __func__, qmi_client_err);
		return -1;
	}

	printf ("%s: exit\n", __func__);
	return 0;
}

int loc_stop_navigation(void)
{
	qmi_client_error_type qmi_client_err;

	mcm_loc_stop_nav_req_msg_v01 start_nav_req;
	mcm_loc_stop_nav_resp_msg_v01 start_nav_resp;

	printf ("%s: entry\n", __func__);
	/* clear the request content */
	memset(&start_nav_req,  0, sizeof(mcm_loc_start_nav_req_msg_v01));
	memset(&start_nav_resp, 0, sizeof(mcm_loc_start_nav_resp_msg_v01));

	qmi_client_err = qmi_client_send_msg_sync(loc_qmi_client,
			MCM_LOC_STOP_NAV_REQ_V01, &start_nav_req, sizeof(start_nav_req), &start_nav_resp,
			sizeof(start_nav_resp), 3000);

	if (qmi_client_err != QMI_NO_ERR) {
		printf("%s: Error %d\n", __func__, qmi_client_err);
		return -1;
	}

	printf ("%s: exit\n", __func__);
	return 0;
}

int loc_get_current_location()
{
}

int main (int argc, char **argv)
{
	int i;
	int ret = 0;

	ret = qmi_loc_init();
	if (ret < 0) {
		return -1;
	}

	ret = loc_set_indications();
	if (ret < 0) {
		return -1;
	}

	ret = loc_set_position_mode();
	if (ret < 0) {
		return -1;
	}

	loc_start_navigation();

	sleep(600);

	loc_stop_navigation();

	ret = qmi_loc_exit();

	return 0;
} /* ----- End of main() ----- */

