/******************************************************************************

  @file    quectel_getinfo.c
  @brief   File contains the implementation for get device info
  @author  Jambo.liu@quectel.com
  ---------------------------------------------------------------------------

  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

 *****************************************************************************/
#include "quectel_getinfo.h"
#include "internals.h"

#ifdef LWM2M_SUPPORT_QLRIL_API
#include <telephony/ril.h>
#include <ql_misc.h>

#define PRV_PRODUCTNAME      "SC20-AL"
#define PRV_PRODUCTNAME_ALD      "SC20-ALD"
#define PRV_HARDWARE_VER  "R1.0"
#define PRV_MANUFACTURER_1      "Quectel"
#define PRV_FIRMWARE_VERSION      "SC20ALSAR09A05"
//static nw_client_handle_type g_h_nw = 0;
static int    h_sim   = 0;
static int    h_loc   = 0;
//static dm_client_handle_type    h_dm     = 0;

void QL_LWM2M_SwithRadioPower(){
    int on = 1;
    int timeout = 10;
    int ret = 0;
    int radio_switch_retry = 3;
    /* for "airplane" mode */
    LOG("QL_LWM2M_SwithRadioPower start\n.");
    int radioOn_index = 0;
    for (radioOn_index = 0; radioOn_index < radio_switch_retry; radioOn_index++)     
    {
        ret = QLRIL_SetRadioPower(&g_qlril_handle, 0, timeout);
    }
    
    int radioOff_index = 0;
    for (radioOff_index = 0; radioOff_index < radio_switch_retry; ++radioOff_index)
    {
        ret = QLRIL_SetRadioPower(&g_qlril_handle, 1, timeout);
    }
    LOG("QL_LWM2M_SwithRadioPower end\n.");
}

int QL_LWM2M_GetNetworkType()
{
    int ret = 0;
    int type = 0;

    RIL_SignalStrength_v10 *ss = NULL;
    ret = QLRIL_GetSignalStrength(&g_qlril_handle, (void **)&ss);
    if (ret < 0 || ss == NULL) {
        LOG_ARG("QLRIL_GetSignalStrength failed with return:%d\n", ret);
        return;
    }
    if (ss->GW_SignalStrength.signalStrength != 99) {
        type = 0;
    }
    if (ss->CDMA_SignalStrength.dbm > 0 || ss->CDMA_SignalStrength.ecio > 0) {
        type = 3;
    }
    if (ss->EVDO_SignalStrength.dbm > 0 || ss->EVDO_SignalStrength.ecio > 0 ||
            ss->EVDO_SignalStrength.signalNoiseRatio > 0) {
        type = 2;
    }
    if (ss->LTE_SignalStrength.signalStrength != 99 || ss->LTE_SignalStrength.rsrp != INT_MAX) {
        type = 6;
    }
    if (ss->TD_SCDMA_SignalStrength.rscp > 0 && ss->TD_SCDMA_SignalStrength.rscp != INT_MAX) {
        type = 1;
    }
    free(ss);
    ss = NULL;

        //LOG_ARG("QL_LWM2M_GetNetworkType  =%d\n", type);
    return type;
}

int QL_LWM2M_GetDataRegistrationState()
{
    int ret = -1;
    int slotId = 1;
    uint32_t radioTechnology = 0;
	char **resp = NULL;
    ret = QLRIL_GetDataRegistrationState(&g_qlril_handle, &resp);
    if (ret < 0) {
        LOG_ARG("QLRIL_GetDataRegistrationState failed with return:%d\n",ret);
        return -1;
    }else{
       // LOG("QLRIL_GetDataRegistrationState  successful\n");
		if (resp != NULL && resp[3] != NULL){
		    radioTechnology = atoi(resp[3]);
			
         //   LOG_ARG("QLRIL_GetDataRegistrationState  radioTechnology=%d\n",radioTechnology);
            if (resp) {
                free(resp);
                resp = NULL;
            }
		    return radioTechnology;
		}
        return -1;
    }

}

uint32_t QL_LWM2M_GetCellInfo()
{
    int ret = 0;
    int slotId = 1;
    uint32_t cellid = 0;
	char **resp = NULL;
    ret = QLRIL_GetDataRegistrationState(&g_qlril_handle, &resp);
    if (ret < 0) {
        LOG_ARG("QLRIL_GetDataRegistrationState failed with return:%d\n",ret);
        return cellid;
    }else{
        //LOG("QLRIL_GetDataRegistrationState  successful\n");
		if (resp != NULL && resp[8] != NULL){
		    cellid = atoi(resp[8]);
			
            //LOG_ARG("QLRIL_GetDataRegistrationState  cellid=%d\n",cellid);
		}
        if (resp) {
            free(resp);
            resp = NULL;
        }
    }

    return cellid;
}


int QL_LWM2M_GetConnInfo(QL_LWM2M_CONN_INFO_T * conn_info){
    int ret = 0;
    int signal = 0;
    int rsrq = 0;
    int csq_value = 0;
    
    RIL_SignalStrength_v10 *ss = NULL;
    //LOG("QL_LWM2M_GetConnInfo enter\n");
    int radio_tech = RADIO_TECH_LTE;
    char **voiceRegistrationState = NULL;
    ret = QLRIL_GetVoiceRegistrationState(&g_qlril_handle, 1, &voiceRegistrationState);
    if (ret > 0 ){
        if(voiceRegistrationState != NULL && voiceRegistrationState[3] != NULL){
           radio_tech = atoi(voiceRegistrationState[3]);
          // radio_tech = voiceRegistrationState[3];
           //LOG_ARG("QLRIL_GetVoiceRegistrationState radio_tech=:%d\n", radio_tech);
        }
		if (voiceRegistrationState) {
			free(voiceRegistrationState);
			voiceRegistrationState = NULL;
		}        
    }

    
    ret = QLRIL_GetSignalStrength(&g_qlril_handle, (void **)&ss);

    if (ret < 0){
        LOG("QLRIL_GetSignalStrength fail\n");
    }else if (ss != NULL) {

        switch (radio_tech){
            case RADIO_TECH_HSPAP:
            case RADIO_TECH_HSPA:
            case RADIO_TECH_HSUPA:
            case RADIO_TECH_HSDPA:
            case RADIO_TECH_UMTS:
                // LOG("EVDO_SignalStrength dbm\n");
                signal = ss->EVDO_SignalStrength.dbm;

            break;
            case RADIO_TECH_EHRPD:
            case RADIO_TECH_EVDO_B:
            case RADIO_TECH_EVDO_A:
            case RADIO_TECH_EVDO_0:
            case RADIO_TECH_1xRTT:
            case RADIO_TECH_IS95B:
            case RADIO_TECH_IS95A:
                 //LOG("CDMA_SignalStrength dbm\n");
               signal = ss->CDMA_SignalStrength.dbm;
            break;

            case RADIO_TECH_GSM:
            case RADIO_TECH_EDGE:
            case RADIO_TECH_GPRS:
                 //LOG("GW_SignalStrength signalStrength\n");
               signal = ss->GW_SignalStrength.signalStrength;
            break;

            case RADIO_TECH_LTE:
                 //LOG("LTE_SignalStrength rsrp\n");
                signal = ss->LTE_SignalStrength.rsrp;
                rsrq = ss->LTE_SignalStrength.rsrq;
            break;

            default:

            break;
        }
        free(ss);
        ss = NULL;
    }
    //LOG_ARG("QL_LWM2M_GetConnInfo  signalStrength =%d\n", signal);
    //LOG_ARG("QL_LWM2M_GetConnInfo  rsrq =%d\n", rsrq);
    
    conn_info->signalStrength = signal;
    conn_info->linkQuality = rsrq;
    conn_info->linkUtilization = 80;
    return 1;
}


int QL_LWM2M_GetSignalStrength(){
    int ret = 0;
    int signal = 0;

    RIL_SignalStrength_v10 *ss = NULL;
    ret = QLRIL_GetSignalStrength(&g_qlril_handle, (void **)&ss);
    if (ret < 0 || ss == NULL) {
        LOG_ARG("QLRIL_GetSignalStrength failed with return:%d\n", ret);
        return;
    }
    if (ss->GW_SignalStrength.signalStrength != 99) {
        signal = ss->GW_SignalStrength.signalStrength;
    }

    if (ss->CDMA_SignalStrength.dbm > 0 || ss->CDMA_SignalStrength.ecio > 0) {
        signal = ss->CDMA_SignalStrength.dbm;
    }

    if (ss->EVDO_SignalStrength.dbm > 0 || ss->EVDO_SignalStrength.ecio > 0 ||
            ss->EVDO_SignalStrength.signalNoiseRatio > 0) {
        signal = ss->EVDO_SignalStrength.dbm;
    }

    if (ss->LTE_SignalStrength.signalStrength != 99 || ss->LTE_SignalStrength.rsrp != INT_MAX) {
        signal = ss->LTE_SignalStrength.rsrp;
    }

    if (ss->TD_SCDMA_SignalStrength.rscp > 0 && ss->TD_SCDMA_SignalStrength.rscp != INT_MAX) {
        signal = ss->TD_SCDMA_SignalStrength.rscp;
    }
    free(ss);
    ss = NULL;
    //LOG_ARG("QL_LWM2M_GetSignalStrength  =%d\n", signal);
    return signal;

}

int QL_LWM2M_GetRsRq(){
    int ret = 0;
    int signal = 0;
    
    RIL_SignalStrength_v10 *ss = NULL;
    int radio_tech = QL_LWM2M_GetNetworkType();
    ret = QLRIL_GetSignalStrength(&g_qlril_handle, (void **)&ss);

    if (ret < 0){
        LOG("QLRIL_GetSignalStrength fail\n");
    }else if (ss != NULL) {

        switch (radio_tech){
            case RADIO_TECH_HSPAP:
            case RADIO_TECH_HSPA:
            case RADIO_TECH_HSUPA:
            case RADIO_TECH_HSDPA:
            case RADIO_TECH_UMTS:

                signal = ss->EVDO_SignalStrength.ecio;

            break;
            case RADIO_TECH_EHRPD:
            case RADIO_TECH_EVDO_B:
            case RADIO_TECH_EVDO_A:
            case RADIO_TECH_EVDO_0:
            case RADIO_TECH_1xRTT:
            case RADIO_TECH_IS95B:
            case RADIO_TECH_IS95A:

               signal = ss->CDMA_SignalStrength.ecio;
            break;

            case RADIO_TECH_GSM:
            case RADIO_TECH_EDGE:
            case RADIO_TECH_GPRS:

               signal = ss->GW_SignalStrength.bitErrorRate;
            break;

            case RADIO_TECH_LTE:
                signal = ss->LTE_SignalStrength.rsrq;
            break;

            default:

            break;
        }
		free(ss);
		ss = NULL;        
    }
    //LOG_ARG("QL_LWM2M_GetRsRq  =%d\n", signal);
    return signal;
}

int QL_LWM2M_GetMcc()
    {
        int ret = 0;
        char **opt = NULL;
        char mcc[10]={0};
        ret = QLRIL_GetOperator(&g_qlril_handle, &opt);
        LOG_ARG("QLRIL_GetOperator MNC ret=%d\n", ret);
        if (ret > 0) {
            if (opt[2]){
                LOG_ARG("QLRIL_GetOperator =%s\n", opt[2]);
                strncpy(mcc,opt[2],3);
                LOG_ARG("QLRIL_GetOperator mcc=%s\n", mcc);
            }
            if (opt) {
                free(opt);
                opt = NULL;
            }
        }
        return atoi(mcc);
    }

int QL_LWM2M_GetMnc()
{
    int ret = 0;
    char **opt = NULL;
    char mnc[10]={0};
    ret = QLRIL_GetOperator(&g_qlril_handle, &opt);
    LOG_ARG("QLRIL_GetOperator MNC ret=%d\n", ret);
    if (ret > 0) {
        if (opt[2]){
            LOG_ARG("QLRIL_GetOperator =%s\n", opt[2]);
            strncpy(mnc,opt[2]+3,strlen(opt[2])-3);
            LOG_ARG("QLRIL_GetOperator mnc=%s\n", mnc);
        }
        if (opt) {
            free(opt);
            opt = NULL;
        }
    }

    return atoi(mnc);
}


int QL_LWM2M_GetIccid(char * iccid)
{
    int    ret     = 0;
    char aid[40] = {0};
    char    iccid_buf[40] = {0};

    //get aid first
    RIL_CardStatus_v6 *cardStatus = NULL;
    ret = QLRIL_GetIccCardStatus(&g_qlril_handle, (void **)&cardStatus);
    if (ret == 0){
        if (cardStatus == NULL)
            return -1;
        if (cardStatus->applications[0].aid_ptr) {
            LOG_ARG ("aid_ptr: %s", cardStatus->applications[0].aid_ptr);
            strcpy(aid,cardStatus->applications[0].aid_ptr);
        LOG_ARG("QLRIL_GetIccCardStatus aid:%s\n", aid);
        }

    }else{
        LOG_ARG("QLRIL_GetIccCardStatus failed with return:%d\n", ret);
        return -1;
    }
    if (cardStatus)
    free(cardStatus);
    cardStatus = NULL; 
    
    ret = QLRIL_GetSimIccId(&g_qlril_handle, aid, iccid_buf, sizeof(iccid_buf));
    //snprintf(iccid, 64, "%s", iccid_buf);
    if (ret < 0)
        LOG_ARG("QLRIL_GetSimIccId failed with return:%d\n", ret);
    else {
        strcpy(iccid,iccid_buf);
        LOG_ARG("SIM card iccid_buf:%s\n", iccid_buf);
        LOG_ARG("SIM card ICCID:%s\n", iccid);
    }
   
    return ret;
}

#if 0
int QL_LWM2M_GetImsi(char * imsi)
{

    char imsi_buf[20] = {0};
    int    ret = 0;
    char aid[40] = {0};

    //get aid first
    RIL_CardStatus_v6 *cardStatus = NULL;
    ret = QLRIL_GetIccCardStatus(&g_qlril_handle, (void **)&cardStatus);
    if (ret == 0){
        if (cardStatus == NULL)
            return -1;
        if (cardStatus->applications[0].aid_ptr) {
            LOG_ARG ("aid_ptr: %s", cardStatus->applications[0].aid_ptr);
            strcpy(aid,cardStatus->applications[0].aid_ptr);
            LOG_ARG("QLRIL_GetIccCardStatus aid:%s\n", aid);
        }

    }else{
        LOG_ARG("QLRIL_GetIccCardStatus failed with return:%d\n", ret);
        return -1;
    }

    
    ret = QLRIL_GetIMSIForApp(&g_qlril_handle, aid, imsi_buf, sizeof(imsi_buf));
    if (ret >= 0){
        LOG_ARG("QLRIL_GetIMSI  return:%s\n", imsi_buf);
    
       strcpy(imsi,imsi_buf);
        return 0;
    }else {
        LOG_ARG("QLRIL_GetIMSI failed with return:%d\n", ret);
    }

    return ret;
}
#endif
int QL_LWM2M_GetImsi(char * imsi)
{
	int ret = 0;
	char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
	char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    char *mIMSI = NULL;

	memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
	strcpy(atc_cmd_req,"AT+CIMI");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
	LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

    if (ret <= 0){
        LOG("get cimi imsi fail\n");
	    ret = 1;
    }else{
    	//parse atc_cmd_resp
        mIMSI = strtok(atc_cmd_resp, "\r\n");
        while(mIMSI)
        {
            LOG_ARG ("temp is =%s\n", mIMSI);
            if(strstr(mIMSI, "CIMI") != NULL){
                mIMSI = strtok(NULL,"\r\n");
                break;
            }else if(strlen(mIMSI) > 10){
                break;
            }

        }	

    	LOG_ARG("get:mIMSI = %s \n", mIMSI);
		strcpy(imsi,mIMSI);
		
		ret = 0;
    }

	return ret;
}

#if 0
int QL_LWM2M_GetNumber(char * num){
    int    ret     = E_QL_OK;
    int app_type = 1;
    QL_SIM_APP_ID_INFO_T    t_info;
    char    num_buf[64] = {0};

    ret = QL_MCM_SIM_Client_Init(&h_sim);
    LOG_ARG("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", ret, h_sim);
    // printf("please input AppType(0: unknown, 1: 3GPP, 2: 3GPP2, 3: ISIM): \n");
    // scanf("%d", &app_type);
    // if(app_type > 3)
    // {
        // LOG("Invalid app type !\n");
        // break;
    // }
    memset(num_buf, 0, 64);
    t_info.e_slot_id    = E_QL_MCM_SIM_SLOT_ID_1;
    t_info.e_app        = (E_QL_MCM_SIM_APP_TYPE_T)(app_type + 0xB00);
    ret = QL_MCM_SIM_GetPhoneNumber(h_sim, &t_info, num_buf, 64);
    LOG_ARG("QL_MCM_SIM_GetPhoneNumber ret = %d, phoneNum: %s\n", ret, num_buf);

    if(strncasecmp(num_buf,"1", 1) == 0){
        snprintf(num, 64, "%s", num_buf + 1);
    }else if(strncasecmp(num_buf,"86", 2) == 0){
        snprintf(num, 64, "%s", num_buf + 2);
    }
    //strcpy(num,num_buf);
    return ret;
}
#endif
int QL_LWM2M_GetImei(char * imei){
    int ret =0;
    char IMEI[40] = {0};
    ret = QLRIL_GetIMEI(&g_qlril_handle, IMEI, sizeof(IMEI));
    if (ret < 0)
        LOG_ARG("QLRIL_GetIMEI failed with return:%d\n", ret);
    else {
        LOG_ARG("QLRIL_GetIMEI with return:%s\n", IMEI);
        strcpy(imei,IMEI);
    }

    return ret;
}

char* QL_LWM2M_GetManufacture(){
    return PRV_MANUFACTURER_1;
}

void QL_LWM2M_str_chart(char *src, char *dest, char c)
{
	int m_start_index = 0;
	int m_end_index = 0;
	int flag = 0;
	size_t length = 0;
    int i = 0;
	length = strlen(src);
	printf("Conway:size : %d , c = %c \n", length, c);
	for(i = 0; i < length; i++)
	{
		printf("Conway:src[%d]:%c \n", i, *(src + i));
		if(*(src + i) == c)
		{
			if(0 == flag)
			{				
				m_start_index = i;
				flag = flag + 1;
				printf("Conway:start_i = %d \n", i);
			} 
			else if (1 == flag)
			{
				m_end_index = i;
				printf("Conway:m_end_index = %d \n", i);
				break;
			}
		}
	}
	printf("Conway:start_index = %d, end_index = %d \n", m_start_index, m_end_index);
	printf("Conway:target str = %s \n", (src + m_start_index));
	strncpy(dest, (src + m_start_index + 1), (m_end_index - m_start_index -1));

}

int QL_LWM2M_GetFirmwareVersion(char *version){
	int ret = 0;
#if 0
   
    char version_buf[50] = {0};
    char version_buf_sub[50] = {0};
    ret = QLRIL_GetBasebandVersion(&g_qlril_handle, version_buf, sizeof(version_buf));
    if (ret < 0)
        LOG_ARG("Firmware version failed ret = %d \n", ret);
    else {
        LOG_ARG("Firmware version_buf= %s \n", version_buf);
        sscanf(version_buf, "%[^(]", version_buf_sub);
        //sscanf(version_buf, "%[^)]", version_buf_sub);
        //sscanf(version_buf, "%*[^(]&%[^)]", version_buf_sub);
        LOG_ARG("Firmware version_buf_sub = %s \n", version_buf_sub);
        strcpy(version,version_buf_sub);
    }
    

#else
    static int flag = 10;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    char *temp = NULL;

	if (flag > 0) 
	{
		flag--;
		memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

	    strcpy((char *)atc_cmd_req, "ATI");
        ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
	    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
	    if (ret < 0) {
			LOG("failed to get Firmware verion.");
	    }

        temp = strtok(atc_cmd_resp, "\r\n");
        while(temp)
        {
            LOG_ARG ("temp is =%s\n", temp);
            if(strstr(temp, "Revision") != NULL){
                sscanf(temp, "%*s%s", version);
                break;
            }
            temp = strtok(NULL,"\n");
            LOG ("get ati loop run\n");
        }		
		LOG_ARG("Firmware version = %s \n", version);
	}
#endif
    //return PRV_FIRMWARE_VERSION;
    
}

int format_svn_info(char* source, char* buf)
{
    int i=0,j=0,flag=0;
    while(source[i]!='\0')
    {
        if(flag==0 && source[i]=='\"')
        {
            flag=1;
        }
        else if(flag==1 && source[i]=='\"')
        {
            buf[j]='\0';
            return 1;
        }
        else if(flag==1 && source[i]!='\"')
        {
            buf[j++]=source[i];
        }
        i++;
    }
    return 0;
}

int QL_LWM2M_GetSvnVersion(char *version){
	int ret = 0;

    static int flag = 10;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    char *temp = NULL;
    char svn_info[64];

	if (flag > 0) 
	{
		flag--;
		memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

	    strcpy((char *)atc_cmd_req, "AT+EGMR=0,9");
        ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
	    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
	    if (ret < 0) {
			LOG("failed to get SVN verion.");
	    }

        temp = strtok(atc_cmd_resp, "\r\n");
        while(temp)
        {
            LOG_ARG ("temp is =%s\n", temp);
            if(strstr(temp, "+EGMR:") != NULL){
                sscanf(temp, "%*s%s", svn_info);
                break;
            }
            temp = strtok(NULL,"\n");
            LOG ("get EGMR loop run\n");
        }		
		LOG_ARG("svn version info svn_info = %s \n", svn_info);
        format_svn_info(svn_info,version);
        LOG_ARG("svn version info version= %s \n", version);
	}
    
}
char* QL_LWM2M_GetProductname(){
    char *svn = GetSoftwareVersion();
    if (strstr(svn, "SC20ALD") != NULL){
        return PRV_PRODUCTNAME_ALD;
    }else{
        return PRV_PRODUCTNAME;
    }
}

char* QL_LWM2M_GetHardversion(){
    return PRV_HARDWARE_VER;
}

int QL_LWM2M_GetGps(location_data_t* data){
    int ret = 0;
    int timeout = 10;//timeout sec
    double longitude = 0, latitude = 0, altitude = 0;
    float accuracy = 0;
   
    ret = QLRIL_GNSS_GetLocation(&g_qlril_handle, &longitude, &latitude, &altitude, &accuracy, timeout);
    if (ret == 0) {
        LOG("QLRIL_GNSS_GetLocation successful \n");
        printf ("Longitude:%lf\n", longitude);
        printf ("Latitude:%lf\n", latitude);
        printf ("Altitude:%lf\n", altitude);
        printf ("Accuracy:%lf\n", accuracy);
        data->latitude    = latitude;
        data->longitude   = longitude;
        data->altitude    = altitude;
        //data->radius      = accuracy;
    } else {
        LOG("QLRIL_GNSS_GetLocation failed \n");
    }

    return ret;
}


#else
#include <ql-mcm-api/ql_mcm_nw.h>
#include <ql-mcm-api/ql_mcm_dm.h>
#include <ql-mcm-api/ql_mcm_gps.h>
#define PRV_PRODUCTNAME      "SC20-AL"
#define PRV_HARDWARE_VER  "R1.0"
static nw_client_handle_type g_h_nw = 0;
static int    h_sim   = 0;
static int    h_loc   = 0;
static dm_client_handle_type    h_dm     = 0;
/**
 * 
 * 
 */

int QL_LWM2M_GetNetworkType()
{
    int radio_tech = 0, ret;
    QL_MCM_NW_REG_STATUS_INFO_T t_info;

    if(g_h_nw == 0)
    {
        ret = QL_MCM_NW_Client_Init(&g_h_nw);
        if(ret != MCM_SUCCESS_V01)
        {
            LOG("QL_MCM_NW_Client_Init failed.");
            return NULL;
        }
    }
    LOG_ARG("g_h_nw: %d", g_h_nw);
    ret = QL_MCM_NW_GetRegStatus(g_h_nw, &t_info);
    if(ret != MCM_SUCCESS_V01)
    {
        LOG("QL_MCM_NW_GetRegStatus failed, release handle.");
        QL_MCM_NW_Client_Deinit(g_h_nw);
        g_h_nw = 0;
        return NULL;
    }

    if(t_info.voice_registration_valid && t_info.voice_registration.registration_state != E_QL_MCM_NW_SERVICE_NONE)
    {
        radio_tech = t_info.voice_registration.radio_tech;
        LOG_ARG("voice radio_tech: %d", radio_tech);
    }
    else if(t_info.data_registration_valid && t_info.data_registration.registration_state != E_QL_MCM_NW_SERVICE_NONE)
    {
        radio_tech = t_info.data_registration.radio_tech;
        LOG_ARG("data radio_tech: %d", radio_tech);
    }

    switch (radio_tech)
    {
        case E_QL_MCM_NW_RADIO_TECH_HSPAP:
        case E_QL_MCM_NW_RADIO_TECH_HSPA:
        case E_QL_MCM_NW_RADIO_TECH_HSUPA:
        case E_QL_MCM_NW_RADIO_TECH_HSDPA:
        case E_QL_MCM_NW_RADIO_TECH_UMTS:
        {
            return LW_NETWORK_MODE_WCDMA;
        }
        break;
        case E_QL_MCM_NW_RADIO_TECH_EHRPD:
        case E_QL_MCM_NW_RADIO_TECH_EVDO_B:
        case E_QL_MCM_NW_RADIO_TECH_EVDO_A:
        case E_QL_MCM_NW_RADIO_TECH_EVDO_0:
        case E_QL_MCM_NW_RADIO_TECH_1xRTT:
        case E_QL_MCM_NW_RADIO_TECH_IS95B:
        case E_QL_MCM_NW_RADIO_TECH_IS95A:
        {
            return LW_NETWORK_MODE_CDMA;
        }
        break;
        case E_QL_MCM_NW_RADIO_TECH_GSM:
        case E_QL_MCM_NW_RADIO_TECH_EDGE:
        case E_QL_MCM_NW_RADIO_TECH_GPRS:
        {
            return LW_NETWORK_MODE_GSM;
        }
        break;
        case E_QL_MCM_NW_RADIO_TECH_LTE:
        {
            return LW_NETWORK_MODE_LTE;
        }
    }
    return LW_NETWORK_MODE_UNKNOWN;
}

uint32_t QL_LWM2M_GetCellInfo()
{
    int ret;
    QL_MCM_NW_CELL_INFO_T cell_info;
    
    if(g_h_nw == 0)
    {
        ret = QL_MCM_NW_Client_Init(&g_h_nw);
        if(ret != MCM_SUCCESS_V01)
        {
            QL_API_LOGE("QL_MCM_NW_Client_Init failed.");
            return -1;
        }
    }

    memset(&cell_info, 0, sizeof(QL_MCM_NW_CELL_INFO_T));

    ret = QL_MCM_NW_GetCellInfo(g_h_nw, &cell_info);
    if(0 != ret) {
        QL_API_LOGE("get cell info failed, ret is %d, release handle.", ret);
        QL_MCM_NW_Client_Deinit(g_h_nw);
        g_h_nw = 0;
        return -1;
    }

    QL_MCM_NW_Client_Deinit(g_h_nw);

    if(cell_info.gsm_info_valid)
    {
        return cell_info.gsm_info[0].cid;
    }
    else if(cell_info.umts_info_valid)
    {
        return cell_info.umts_info[0].cid;
    }
    else if(cell_info.lte_info_valid)
    {
        return cell_info.lte_info[0].cid;
    }
}


int QL_LWM2M_GetConnInfo(QL_LWM2M_CONN_INFO_T * conn_info){
    int ret = 0;
    int signal = 0;
    int rsrq = 0;
    int csq_value = 0;
    QL_MCM_NW_SIGNAL_STRENGTH_INFO_T t_info;
    memset(&t_info, 0, sizeof(QL_MCM_NW_SIGNAL_STRENGTH_INFO_T));

    if(g_h_nw == 0)
    {
        ret = QL_MCM_NW_Client_Init(&g_h_nw);
        if(ret != MCM_SUCCESS_V01)
        {
            LOG("QL_MCM_NW_Client_Init failed.");
            return -1;
        }
    }

    ret = QL_MCM_NW_GetSignalStrength(g_h_nw, &t_info);
    if(ret != MCM_SUCCESS_V01)
    {
        LOG("QL_MCM_NW_GetRegStatus failed, release handle.");
        QL_MCM_NW_Client_Deinit(g_h_nw);
        g_h_nw = 0;
        return -1;
    }

    if(t_info.gsm_sig_info_valid) {
        signal = t_info.gsm_sig_info.rssi;
    }
    if(t_info.wcdma_sig_info_valid) {
        signal = t_info.wcdma_sig_info.rssi;
    }
    if(t_info.tdscdma_sig_info_valid) {
        signal = t_info.tdscdma_sig_info.rssi;
    }
    if(t_info.lte_sig_info_valid) {
        signal = t_info.lte_sig_info.rsrp;
        rsrq = t_info.lte_sig_info.rsrq;
    }
    if(t_info.cdma_sig_info_valid) {
        signal = t_info.cdma_sig_info.rssi;
    }
    if(t_info.hdr_sig_info_valid) {
        signal = t_info.hdr_sig_info.rssi;
    }
    conn_info->signalStrength = signal;
    conn_info->linkQuality = rsrq;
    conn_info->linkUtilization = 80;
    return 1;
}


int QL_LWM2M_GetSignalStrength(){
    int ret = 0;
    int signal = 0;
    int csq_value = 0;
    QL_MCM_NW_SIGNAL_STRENGTH_INFO_T t_info;
    memset(&t_info, 0, sizeof(QL_MCM_NW_SIGNAL_STRENGTH_INFO_T));

    if(g_h_nw == 0)
    {
        ret = QL_MCM_NW_Client_Init(&g_h_nw);
        if(ret != MCM_SUCCESS_V01)
        {
            LOG("QL_MCM_NW_Client_Init failed.");
            return -1;
        }
    }

    ret = QL_MCM_NW_GetSignalStrength(g_h_nw, &t_info);
    if(ret != MCM_SUCCESS_V01)
    {
        LOG("QL_MCM_NW_GetRegStatus failed, release handle.");
        QL_MCM_NW_Client_Deinit(g_h_nw);
        g_h_nw = 0;
        return -1;
    }

    if(t_info.gsm_sig_info_valid) {
        signal = t_info.gsm_sig_info.rssi;
    } 
    if(t_info.wcdma_sig_info_valid) {
        signal = t_info.wcdma_sig_info.rssi;
    } 
    if(t_info.tdscdma_sig_info_valid) {
        signal = t_info.tdscdma_sig_info.rssi;
    } 
    if(t_info.lte_sig_info_valid) {
        signal = t_info.lte_sig_info.rsrp;
    } 
    if(t_info.cdma_sig_info_valid) {
        signal = t_info.cdma_sig_info.rssi;
    } 
    if(t_info.hdr_sig_info_valid) {
        signal = t_info.hdr_sig_info.rssi;
    }
    csq_value = (signal + 113)/2;
    return signal;
}

int QL_LWM2M_GetRsRq(){
    int ret = 0;
    int signal = 0;
    QL_MCM_NW_SIGNAL_STRENGTH_INFO_T t_info;
    memset(&t_info, 0, sizeof(QL_MCM_NW_SIGNAL_STRENGTH_INFO_T));

    if(g_h_nw == 0)
    {
        ret = QL_MCM_NW_Client_Init(&g_h_nw);
        if(ret != MCM_SUCCESS_V01)
        {
            LOG("QL_MCM_NW_Client_Init failed.");
            return -1;
        }
    }

    ret = QL_MCM_NW_GetSignalStrength(g_h_nw, &t_info);
    if(ret != MCM_SUCCESS_V01)
    {
        LOG("QL_MCM_NW_GetRegStatus failed, release handle.");
        QL_MCM_NW_Client_Deinit(g_h_nw);
        g_h_nw = 0;
        return -1;
    }

    if(t_info.gsm_sig_info_valid) {
        signal = -1;
    } 
    if(t_info.wcdma_sig_info_valid) {
        signal = t_info.wcdma_sig_info.ecio;
    } 
    if(t_info.tdscdma_sig_info_valid) {
        signal = t_info.tdscdma_sig_info.ecio;
    } 
    if(t_info.lte_sig_info_valid) {
        signal = t_info.lte_sig_info.rsrq;
    } 
    if(t_info.cdma_sig_info_valid) {
        signal = t_info.cdma_sig_info.ecio;
    } 
    if(t_info.hdr_sig_info_valid) {
        signal = t_info.hdr_sig_info.ecio;
    }

    return signal;
}

int QL_LWM2M_GetMcc()
{
    QL_MCM_NW_OPERATOR_NAME_INFO_T  t_info;
    int ret = QL_MCM_NW_GetOperatorName(g_h_nw, &t_info);
    return atoi(t_info.mcc);
}
int QL_LWM2M_GetMnc()
{
    QL_MCM_NW_OPERATOR_NAME_INFO_T  t_info;
    int ret = QL_MCM_NW_GetOperatorName(g_h_nw, &t_info);
    return atoi(t_info.mnc);
}

int QL_LWM2M_GetIccid(char * iccid)
{
    int    ret     = E_QL_OK;
    char    iccid_buf[64] = {0};

    ret = QL_MCM_SIM_Client_Init(&h_sim);
    LOG_ARG("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", ret, h_sim);

    memset(iccid_buf, 0, 64);
    ret = QL_MCM_SIM_GetICCID(h_sim, E_QL_MCM_SIM_SLOT_ID_1, iccid_buf, 64);
    LOG_ARG("QL_MCM_SIM_GetICCID ret = %d, ICCID: %s\n", ret, iccid_buf);
    snprintf(iccid, 64, "%s", iccid_buf);
    //strcpy(iccid,iccid_buf);
    QL_MCM_SIM_Client_Deinit(h_sim);
    return ret;
}

int QL_LWM2M_GetImsi(char * imsi)
{
    int    ret     = E_QL_OK;
    char    imsi_buf[64] = {0};

    QL_SIM_APP_ID_INFO_T    t_info;

    ret = QL_MCM_SIM_Client_Init(&h_sim);
    LOG_ARG("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", ret, h_sim);
    memset(imsi_buf, 0, 64);
    t_info.e_slot_id    = E_QL_MCM_SIM_SLOT_ID_1;
    t_info.e_app        = E_QL_MCM_SIM_APP_TYPE_3GPP;
    ret = QL_MCM_SIM_GetIMSI(h_sim, &t_info, imsi_buf, 64);
    LOG_ARG(">>QL_MCM_SIM_GetIMSI ret = %d, IMSI: %s\n", ret, imsi_buf);
    strcpy(imsi,imsi_buf);
    //QL_MCM_SIM_Client_Deinit(h_sim);
    return ret;
}

int QL_LWM2M_GetNumber(char * num){
    int    ret     = E_QL_OK;
    int app_type = 1;
    QL_SIM_APP_ID_INFO_T    t_info;
    char    num_buf[64] = {0};

    ret = QL_MCM_SIM_Client_Init(&h_sim);
    LOG_ARG("QL_MCM_SIM_Client_Init ret = %d with h_sim=%d\n", ret, h_sim);
    // printf("please input AppType(0: unknown, 1: 3GPP, 2: 3GPP2, 3: ISIM): \n");
    // scanf("%d", &app_type);
    // if(app_type > 3)
    // {
        // LOG("Invalid app type !\n");
        // break;
    // }
    memset(num_buf, 0, 64);
    t_info.e_slot_id    = E_QL_MCM_SIM_SLOT_ID_1;
    t_info.e_app        = (E_QL_MCM_SIM_APP_TYPE_T)(app_type + 0xB00);
    ret = QL_MCM_SIM_GetPhoneNumber(h_sim, &t_info, num_buf, 64);
    LOG_ARG("QL_MCM_SIM_GetPhoneNumber ret = %d, phoneNum: %s\n", ret, num_buf);

    if(strncasecmp(num_buf,"1", 1) == 0){
        snprintf(num, 64, "%s", num_buf + 1);
    }else if(strncasecmp(num_buf,"86", 2) == 0){
        snprintf(num, 64, "%s", num_buf + 2);
    }
    //strcpy(num,num_buf);
    return ret;
}

int QL_LWM2M_GetImei(char * imei){
    int    ret     = E_QL_OK;

    ret = QL_MCM_DM_Client_Init(&h_dm);
    ql_dm_device_serial_numbers_t dm_device_serial_numbers;
    memset(&dm_device_serial_numbers,0,sizeof(dm_device_serial_numbers));
    ret = QL_MCM_DM_GetSerialNumbers(h_dm,&dm_device_serial_numbers);
    LOG_ARG("QL_MCM_DM_GetSerialNumbers ret = %d\n", ret);
    if(ret < 0)
    {
        LOG_ARG("QL_MCM_DM_GetSerialNumbers fail ret = %d\n", ret);
    }
    else
    {
        strcpy(imei,dm_device_serial_numbers.imei);
        LOG_ARG("SerialNumbers  imei:%s      meid:%s\n",
            dm_device_serial_numbers.imei,dm_device_serial_numbers.meid);
    }
    QL_MCM_DM_Client_Deinit(h_dm);
}

int QL_LWM2M_GetAbout(ql_module_about_info_s * about){
    int    ret     = E_QL_OK;
    ret = QL_Module_About_Get(about);
    LOG_ARG("QL_Module_About_Get ret = %d\n", ret);
}

char * QL_LWM2M_GetProductname(){
    return PRV_PRODUCTNAME;
}

char * QL_LWM2M_GetHardversion(){
    return PRV_HARDWARE_VER;
}

int QL_LWM2M_GetGps(location_data_t* data){
    int    cmdIdx  = 0;
    int    ret     = E_QL_OK;
    QL_LOC_INJECT_TIME_INTO_T   t_info = {0};
    QL_LOC_LOCATION_INFO_T  t_loc_info  = {0};
    time_t timep;
    long long utime=0;
	int timeout_sec = 8;
    QL_LOC_POS_MODE_INFO_T  t_mode      = {0};

    //You need set it as you wish
    t_mode.mode                 = E_QL_LOC_POS_MODE_STANDALONE;
    //t_mode.recurrence           = E_QL_LOC_POS_RECURRENCE_PERIODIC;
    t_mode.min_interval         = 500;  //report nmea frequency 1Hz
    t_mode.preferred_accuracy   = 0;    // <50m
    //t_mode.preferred_time       = 90;    // 90s


    ret = QL_LOC_Client_Init(&h_loc);
    LOG_ARG("QL_LOC_Client_Init ret=%d\n", ret);

    // ret = QL_LOC_AddRxIndMsgHandler(ql_loc_rx_ind_msg_cb, (void*)h_loc);
    // LOG_ARG("QL_LOC_AddRxIndMsgHandler ret=%d\n", ret);

    ret = QL_LOC_Set_Indications(h_loc, 1);

    ret = QL_LOC_Set_Position_Mode(h_loc, &t_mode);
    LOG_ARG("QL_LOC_Set_Position_Mode ret %d\n", ret);

    ret = QL_LOC_Get_Current_Location(h_loc, &t_loc_info, timeout_sec);
    LOG_ARG(" QL_LOC_Get_Current_Location ret %d\n", ret);
    if(ret < 0)
    {
        if(ret == -2)
        {// -2: timeout, may need try again
            LOG("QL_LOC_Get_Current_Location timeout, try again!\n");
        }
        else
        {
            LOG_ARG("QL_LOC_Get_Current_Location Fail, ret %d\n", ret);
        }
    }else{
            data->latitude    = t_loc_info.latitude;// 27.986065;  // Mount Everest :)
            data->longitude   = t_loc_info.longitude;//86.922623;
            data->altitude    = t_loc_info.altitude;//8495.0000;
            data->radius      = t_loc_info.accuracy;
            //location_setVelocity(locationObj, 0, 0, 255); // 255: speedUncertainty not supported!
            data->timestamp   = t_loc_info.timestamp;//time(NULL);
            data->speed       = t_loc_info.speed;//0.0;
            data->bearing     = t_loc_info.bearing;
    }
	ret = QL_LOC_Stop_Navigation(h_loc);
    return ret;
}
#endif
