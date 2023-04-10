/*********************************************************************************
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *
 *       Filename:  ql_supl.c
 *                 
 *        Version:  1.0.0(2020年04月14日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2020年04月14日 09时48分07秒"
 *                  参考qcom-opensource/location 
 ********************************************************************************/
#include <stdio.h>   /*  Standard input/output definitions */
#include <string.h>  /*  String function definitions */
#include <unistd.h>  /*  UNIX standard function definitions */
#include <fcntl.h>   /*  File control definitions */
#include <errno.h>   /*  Error number definitions */
#include <termios.h> /*  POSIX terminal control definitions */
#include <limits.h>
#include <errno.h>
#include <pthread.h>

#include <loc-hal/loc_api_v02_client.h>
#include <loc-hal/loc_api_sync_req.h>
#include <ql-mcm-api/ql_in.h>

#define SUPL_TEST_CONFIG_FILE   "/systemrw/supl_test.conf"

static voice_client_handle_type h_voice = 0;
static nw_client_handle_type h_nw = 0;
static int voice_call_id = 0;
static int e911_state = 0;

static int load_pos_cont = 4; 
static int load_pos_interval = 2; 
static char supl_urladdr[50] = {0};
static int supl_server_type = 3;
static int del_flag = 1;
static int server_conn_handle = 0;
static int server_conn_req_type = 1;
static int server_conn_status_type = 1;
static int server_conn_pdn_type = 1;
static char server_conn_apn_name[50] = {0};
static int disconnect_pdn_sw = 1;
static int disconnect_pdn_time = 300;

locClientHandleType handle;

static void ql_voice_call_cb_ind(unsigned int ind_id, void *ind_data, uint32_t ind_data_len)
{
    ql_mcm_voice_e911_state_ind *p;

    switch(ind_id) {
    case E_QL_MCM_VOICE_E911_STATE_IND:
        p = (ql_mcm_voice_e911_state_ind *)ind_data;
        printf("Peeta E911 State = %d\n", p->state);
        if (p->state == 1)
            e911_state = 1;
        else if (p->state == 0)
            e911_state = 2;
        else
            e911_state = 0;
        break;
    default:
        printf("Peeta ql_voice_call_cb_ind ind_id = %d\n", ind_id);
        break;
    }
}

int disconnect_APN_QMI(void)
{
    int ret = 0;

    ret = QL_MCM_NW_SetSysSelEmergencyMode(h_nw, 0);
    if (ret != 0)
        printf("QL_MCM_NW_SetSysSelEmergencyMode  return err=%d\n", ret);

    return 0;
}

int locInformNI(void)
{
    locClientReqUnionType req_union;
    locClientStatusEnumType result = eLOC_CLIENT_FAILURE_UNSUPPORTED;

    qmiLocNiUserRespReqMsgT_v02 niUserResp_req;
    qmiLocNiUserRespIndMsgT_v02 niUserResp_ind;

    memset(&niUserResp_req, 0, sizeof(niUserResp_req));
    memset(&niUserResp_ind, 0, sizeof(niUserResp_ind));

    req_union.pNiUserRespReq = &niUserResp_req;

    result = loc_sync_send_req(handle,
            QMI_LOC_NI_USER_RESPONSE_REQ_V02,
            req_union, LOC_ENGINE_SYNC_REQUEST_TIMEOUT,
            QMI_LOC_NI_USER_RESPONSE_IND_V02,
            &niUserResp_ind);

    if (result != eLOC_CLIENT_SUCCESS ||
            eQMI_LOC_SUCCESS_V02 != niUserResp_ind.status) {
        printf("response QMI_LOC_NI_USER_RESPONSE_REQ_V02 result = %d, \n", result);
    }

    return 0;
}

int atlOpenStatus(void)
{
    locClientReqUnionType req_union;
    locClientStatusEnumType result = eLOC_CLIENT_FAILURE_UNSUPPORTED;

    qmiLocInformLocationServerConnStatusReqMsgT_v02 conn_status_req;
    qmiLocInformLocationServerConnStatusIndMsgT_v02 conn_status_ind;

    memset(&conn_status_req, 0, sizeof(conn_status_req));
    memset(&conn_status_ind, 0, sizeof(conn_status_ind));

    conn_status_req.connHandle = server_conn_handle;
    if (server_conn_req_type == 1)
        conn_status_req.requestType = eQMI_LOC_SERVER_REQUEST_OPEN_V02;
    else
        conn_status_req.requestType = eQMI_LOC_SERVER_REQUEST_CLOSE_V02;

    if (server_conn_status_type == 1)
        conn_status_req.statusType = eQMI_LOC_SERVER_REQ_STATUS_SUCCESS_V02;
    else
        conn_status_req.statusType = eQMI_LOC_SERVER_REQ_STATUS_FAILURE_V02;

    conn_status_req.apnProfile_valid = 1;
    if (server_conn_pdn_type == 1)
        conn_status_req.apnProfile.pdnType = eQMI_LOC_APN_PROFILE_PDN_TYPE_IPV4_V02;
    else
        conn_status_req.apnProfile.pdnType = server_conn_pdn_type;

    strlcpy(conn_status_req.apnProfile.apnName, server_conn_apn_name, strlen(server_conn_apn_name) + 1);

    req_union.pInformLocationServerConnStatusReq = &conn_status_req;

    result = loc_sync_send_req(handle,
            QMI_LOC_INFORM_LOCATION_SERVER_CONN_STATUS_REQ_V02,
            req_union, LOC_ENGINE_SYNC_REQUEST_TIMEOUT,
            QMI_LOC_INFORM_LOCATION_SERVER_CONN_STATUS_IND_V02,
            &conn_status_ind);

    if (result != eLOC_CLIENT_SUCCESS ||
            eQMI_LOC_SUCCESS_V02 != conn_status_ind.status) {
        printf("response QMI_LOC_INFORM_LOCATION_SERVER_CONN_STATUS_REQ_V02 result = %d, \n", result);
    }

    return 0;
}

int delete9x07AidingData(void)
{
    locClientReqUnionType req_union;
    locClientStatusEnumType status = eLOC_CLIENT_FAILURE_UNSUPPORTED;

    qmiLocDeleteAssistDataReqMsgT_v02 delete_req;
    qmiLocDeleteAssistDataIndMsgT_v02 delete_resp;

    memset(&delete_req, 0, sizeof(delete_req));
    memset(&delete_resp, 0, sizeof(delete_resp));

    delete_req.deleteAllFlag = (uint8_t)del_flag;

    req_union.pDeleteAssistDataReq = &delete_req;

    status = loc_sync_send_req(handle,
                               QMI_LOC_DELETE_ASSIST_DATA_REQ_V02,
                               req_union, LOC_ENGINE_SYNC_REQUEST_TIMEOUT,
                               QMI_LOC_DELETE_ASSIST_DATA_IND_V02,
                               &delete_resp);

    if (status != eLOC_CLIENT_SUCCESS ||
          eQMI_LOC_SUCCESS_V02 != delete_resp.status) {
        printf("response QMI_LOC_DELETE_ASSIST_DATA_REQ_V02 status = %d, \n",status);
    }

    return 0;
}

int locSetServer(const char *url, int len)
{
    int err = 0;

    locClientReqUnionType req_union;
    locClientStatusEnumType status = eLOC_CLIENT_SUCCESS;

    qmiLocSetServerReqMsgT_v02 set_server_req;
    qmiLocSetServerIndMsgT_v02 set_server_ind;

    if (len < 0 || (size_t)len > sizeof(set_server_req.urlAddr)) {
        printf("len = %d greater than max allowed url length\n", len);

        return -1;
    }

    memset(&set_server_req, 0, sizeof(set_server_req));
    memset(&set_server_ind, 0, sizeof(set_server_ind));

    printf("url = (%s), len = %d\n", url, len);

    if (supl_server_type == 3)
        set_server_req.serverType = eQMI_LOC_SERVER_TYPE_UMTS_SLP_V02;
    else
        set_server_req.serverType = supl_server_type;

    set_server_req.urlAddr_valid = 1;

    strlcpy(set_server_req.urlAddr, url, sizeof(set_server_req.urlAddr));

    req_union.pSetServerReq = &set_server_req;

    status = loc_sync_send_req(handle,
            QMI_LOC_SET_SERVER_REQ_V02,
            req_union, LOC_ENGINE_SYNC_REQUEST_TIMEOUT + 2000,
            QMI_LOC_SET_SERVER_IND_V02,
            &set_server_ind);

    if (status != eLOC_CLIENT_SUCCESS ||
            eQMI_LOC_SUCCESS_V02 != set_server_ind.status) {
        printf("error status = %s, set_server_ind.status = %s\n",
                loc_get_v02_client_status_name(status),
                loc_get_v02_qmi_status_name(set_server_ind.status));
        err = -2;
    }

    return err;
}

void eventIndCb(
        locClientHandleType handle,
        uint32_t eventIndId,
        const locClientEventIndUnionType eventIndPayload,
        void *pClientCookie)
{
    int n;
    int *pIndex = pClientCookie;
    char buffer[201] = {0};

    switch(eventIndId) {
    case QMI_LOC_EVENT_NMEA_IND_V02:
        printf("eventIndCb: QMI_LOC_EVENT_NMEA_IND_V02\n");
        break;
    default:
        printf("Unsupported eventID %d\n", eventIndId);
        break;
    } 
}

void respIndCb(
        locClientHandleType handle,
        uint32_t respIndId,
        const locClientRespIndUnionType respIndPayload,
        uint32_t respIndPayloadSize,
        void *pClientCookie)
{
    int *pIndex = pClientCookie;

    loc_sync_process_ind(handle, respIndId,
            (void *)respIndPayload.pDeleteAssistDataInd, respIndPayloadSize);
}

static void errorCb(
        locClientHandleType handle,
        locClientErrorEnumType errorId,
        void *pClientCookie)
{
    int *pIndex = pClientCookie;

    printf("received errorId%d from handle %p"
            "with index %d\n", errorId,
            handle, *pIndex);
}

locClientCallbacksType globalCallbacks = {
    sizeof(locClientCallbacksType),
    eventIndCb,
    respIndCb,
    errorCb
};

int ql_sync_supl_conf(void)
{
    int ret;
    int count = 0;
    FILE *fp;
    char *ptr;
    char str[50] = {0};
    char buf[100] = {0};
    static struct stat state_new, state_old;

    if (access(SUPL_TEST_CONFIG_FILE, F_OK)) {
        return -1;
    }

    ret =stat(SUPL_TEST_CONFIG_FILE, &state_new);
    if (state_new.st_mtime != state_old.st_mtime) {
        state_old.st_mtime = state_new.st_mtime;
        fp = fopen(SUPL_TEST_CONFIG_FILE, "r");

        if (fp == NULL) {
            perror("fopen ql_power_manager");
            return -2;
        }

        while(fgets(buf, sizeof(buf) - 1, fp) != NULL) {
            ptr = strchr(buf, '=');
            if (!ptr)
                continue;

            memset(str, 0, sizeof(str));

            if (!strncmp(buf, "SUPL_URLADDR", ptr - buf)) {
                sscanf(ptr + 1, "\"%[^\"]\"", &str);
                if (!strcmp(str, supl_urladdr))
                    continue;
                printf("SUPL_URLADDR=%s\n", str);
                strlcpy(supl_urladdr, str, sizeof(supl_urladdr));
                count++;
            }

            if (!strncmp(buf, "SUPL_SERVER_TYPE", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (supl_server_type == ret)
                    continue;
                printf("SUPL_SERVER_TYPE=%d\n", ret);
                supl_server_type = ret;
                count++;
            }

            if (!strncmp(buf, "DEL_FLAG", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (del_flag == ret)
                    continue;
                printf("DEL_FLAG=%d\n", ret);
                del_flag = ret;
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_HANDLE", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (server_conn_handle == ret)
                    continue;
                printf("SERVER_CONN_HANDLE=%d\n", ret);
                server_conn_handle = ret;
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_REQUEST_TYPE", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (server_conn_req_type == ret)
                    continue;
                printf("SERVER_CONN_REQUEST_TYPE=%d\n", ret);
                server_conn_req_type = ret;
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_STATUS_TYPE", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (server_conn_status_type == ret)
                    continue;
                printf("SERVER_CONN_STATUS_TYPE=%d\n", ret);
                server_conn_status_type = ret;
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_PDN_TYPE", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (server_conn_pdn_type == ret)
                    continue;
                printf("SERVER_CONN_PDN_TYPE=%d\n", ret);
                server_conn_pdn_type = ret;
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_APN_NAME", ptr - buf)) {
                sscanf(ptr + 1, "\"%[^\"]\"", &str);
                //printf("SERVER_CONN_APN_NAME=%s\n", str);
                if (!strcmp(str, server_conn_apn_name))
                    continue;
                printf("SERVER_CONN_APN_NAME=%s\n", str);
                strlcpy(server_conn_apn_name, str, sizeof(server_conn_apn_name));
                count++;
            }

            if (!strncmp(buf, "SERVER_CONN_APN_NAME_LEN", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                printf("SERVER_CONN_APN_NAME_LEN=%d\n", ret);
                count++;
            }

            if (!strncmp(buf, "LOAD_POS_CONT", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (load_pos_cont == ret)
                    continue;
                printf("LOAD_POS_CONT=%d\n", ret);
                load_pos_cont = ret;
                count++;
            }

            if (!strncmp(buf, "LOAD_POS_INTERVAL", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (load_pos_interval == ret)
                    continue;
                printf("LOAD_POS_INTERVAL=%d\n", ret);
                load_pos_interval = ret;
                count++;
                
            }

            if (!strncmp(buf, "DISCONNECT_PDN_SW", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (disconnect_pdn_sw == ret)
                    continue;
                printf("DISCONNECT_PDN_SW=%d\n", ret);
                disconnect_pdn_sw = ret;
                count++;
            }

            if (!strncmp(buf, "DISCONNECT_PDN_TIME", ptr - buf)) {
                sscanf(ptr + 1, "%d", &ret);
                if (disconnect_pdn_time == ret)
                    continue;
                printf("DISCONNECT_PDN_TIME=%d\n", ret);
                disconnect_pdn_time = ret;
                count++;
            }

            memset(buf, 0, sizeof(buf));
        }
    }

    return count;
}

static void *handle_cycle_to_send_qmi_thread(void)
{
    int times = 0;
    while (1) {
        if (e911_state == 0) {
            printf("Peeta +++++++++++++1 e911_state = %d\n", e911_state);
            sleep(1);
            continue;
        }

        if (e911_state == 2) {
            printf("Peeta +++++++++++++2 e911_state = %d\n", e911_state);
            e911_state = 0;
            if (disconnect_pdn_sw)
                sleep(disconnect_pdn_time);
            disconnect_APN_QMI();
            continue;
        }

        times = 0;
        while (times++ < load_pos_cont && e911_state == 1) {
            printf("Peeta +++++++++++++3 e911_state = %d\n", e911_state);

            locSetServer(supl_urladdr, strlen(supl_urladdr) + 1);

            delete9x07AidingData();

            atlOpenStatus();

            locInformNI();

            if (e911_state == 2)
                break;

            sleep(load_pos_interval);
        }

        while (e911_state == 1)
            sleep(1);
        printf("Peeta +++++++++++++4 e911_state = %d\n", e911_state);
    }
}

int main (int argc, char **argv)
{
    int ret;
    int id1 = 1;

    pthread_t tid;

    locClientStatusEnumType status;

    ret = QL_Voice_Call_Client_Init(&h_voice);
    printf("QL_Voice_Call_Client_Init ret = %d, with h_voice=%d\n", ret, h_voice);
    if (ret == 0 && h_voice > 0) {
        ret = QL_Voice_Call_AddCommonStateHandler(h_voice,
                (QL_VoiceCall_CommonStateHandlerFunc_t)ql_voice_call_cb_ind);
    }

    ret = QL_MCM_NW_Client_Init(&h_nw);
    printf("QL_MCM_NW_Client_Init ret = %d, with h_nw=%d\n", ret, h_nw);
    if (ret != 0 && h_voice <= 0) {
        printf("QL_MCM_NW_Client_Init error\n");
        return -1;
    }

    status = locClientOpen(QMI_LOC_EVENT_MASK_NMEA_V02,
            &globalCallbacks,
            &handle,
            (void *)&id1);
    if (eLOC_CLIENT_SUCCESS != status ||
            handle == LOC_CLIENT_INVALID_HANDLE_VALUE) {
        printf ("locClientOpen failed, status = %d\n", status);
        return -1;
    }

    printf ("\nlocClientOpen success\n");

    strcpy(supl_urladdr, "slp2048.rs.de:7275");
    strcpy(server_conn_apn_name, "vzwinternet");

    ql_sync_supl_conf();
    ret = pthread_create(&tid, NULL, handle_cycle_to_send_qmi_thread, NULL);

    while(1) {
        ql_sync_supl_conf();

        sleep(1);
    }

    return 0;
} /* ----- End of main() ----- */

