/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH Germany. 
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
 *    Pascal Rieux - Please refer to git log
 *    
 *******************************************************************************/

/*
 *  This Connectivity Monitoring object is optional and has a single instance
 * 
 *  Resources:
 *
 *          Name             | ID | Oper. | Inst. | Mand.|  Type   | Range | Units |
 *  Network Bearer           |  0 |  R    | Single|  Yes | Integer |       |       |
 *  Available Network Bearer |  1 |  R    | Multi |  Yes | Integer |       |       |
 *  Radio Signal Strength    |  2 |  R    | Single|  Yes | Integer |       | dBm   |
 *  Link Quality             |  3 |  R    | Single|  No  | Integer | 0-100 |   %   |
 *  IP Addresses             |  4 |  R    | Multi |  Yes | String  |       |       |
 *  Router IP Addresses      |  5 |  R    | Multi |  No  | String  |       |       |
 *  Link Utilization         |  6 |  R    | Single|  No  | Integer | 0-100 |   %   |
 *  APN                      |  7 |  R    | Multi |  No  | String  |       |       |
 *  Cell ID                  |  8 |  R    | Single|  No  | Integer |       |       |
 *  SMNC                     |  9 |  R    | Single|  No  | Integer | 0-999 |   %   |
 *  SMCC                     | 10 |  R    | Single|  No  | Integer | 0-999 |       |
 *
 */

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lwm2m_configure.h"
#include "quectel_getinfo.h"
#include <fcntl.h>
#include "internals.h"
#include "lwm2mclient.h"
// Resource Id's:
#define RES_M_NETWORK_BEARER            0
#define RES_M_AVL_NETWORK_BEARER        1
#define RES_M_RADIO_SIGNAL_STRENGTH     2
#define RES_O_LINK_QUALITY              3
#define RES_M_IP_ADDRESSES              4
#define RES_O_ROUTER_IP_ADDRESS         5
#define RES_O_LINK_UTILIZATION          6
#define RES_O_APN                       7
#define RES_O_CELL_ID                   8
#define RES_O_SMNC                      9
#define RES_O_SMCC                      10
#define RES_O_APN_30000                 30000
#define RES_O_APN_CLASS2                 0
#define RES_O_APN_CLASS3                 1
#define RES_O_APN_CLASS6                 2
#define RES_O_APN_CLASS7                 3
//add by joe end
#define VALUE_NETWORK_BEARER_INVALID    -1
#define VALUE_NETWORK_BEARER_GSM        0   //GSM see 
#define VALUE_NETWORK_BEARER_TDSCDMA    1
#define VALUE_NETWORK_BEARER_WCDMA      2
#define VALUE_NETWORK_BEARER_CDMA2000   3
#define VALUE_NETWORK_BEARER_WIMAX      4
#define VALUE_NETWORK_BEARER_LTE_TDD    5
#define VALUE_NETWORK_BEARER_LTE_FDD    6

#define VALUE_AVL_NETWORK_BEARER_1  0   //GSM
#define VALUE_AVL_NETWORK_BEARER_2  21  //WLAN
#define VALUE_AVL_NETWORK_BEARER_3  41  //Ethernet
#define VALUE_AVL_NETWORK_BEARER_4  42  //DSL
#define VALUE_AVL_NETWORK_BEARER_5  43  //PLC
#define VALUE_IP_ADDRESS_1              "192.168.178.101"
#define VALUE_IP_ADDRESS_2              "192.168.178.102"
#define VALUE_ROUTER_IP_ADDRESS_1       "192.168.178.001"
#define VALUE_ROUTER_IP_ADDRESS_2       "192.168.178.002"
//#define VALUE_APN_1                     "web.vodafone.de"
//#define VALUE_APN_2                     "cda.vodafone.de"
#define VALUE_CELL_ID                   69696969
#define VALUE_RADIO_SIGNAL_STRENGTH     80                  //dBm
#define VALUE_LINK_QUALITY              98     
#define VALUE_LINK_UTILIZATION          80
#define VALUE_SMNC                      33
#define VALUE_SMCC                      44
#define MAX_AVAIL_BEARER	15


bool conn_mon_created = false;
static int apn_list_count = 0;
char class2_apn[CARRIER_APN_LEN];
char class3_apn[CARRIER_APN_LEN];
char class6_apn[CARRIER_APN_LEN];
char class7_apn[CARRIER_APN_LEN];


typedef struct conn_mon_persistent_info_
{
  char class2_apn[CARRIER_APN_LEN];
  char class3_apn[CARRIER_APN_LEN];
  char class6_apn[CARRIER_APN_LEN];
  char class7_apn[CARRIER_APN_LEN];
} conn_mon_persistent_info_t;

static void prv_get_device_apn(char *apn);

typedef struct
{
    char ipAddresses[2][16];        // limited to 2!
    char routerIpAddresses[2][16];  // limited to 2!
    long cellId;
    int signalStrength;
    int linkQuality;
    int linkUtilization;
    int32_t mcc;
    int32_t mnc;
    uint16_t networkBearer;
    uint8_t availNetworkBearerLength;
    uint16_t availNetworkBearer[MAX_AVAIL_BEARER];
    resource_instance_string_list_t*    apnList;
    //add by joe end
} conn_m_data_t;

/*	For m2m_get_connectivity_info*/
#define MASK_SIGNAL_STRENGTH    0x01
#define MASK_LINK_QUALITY       0x02
#define MASK_CELL_ID            0x04
#define MASK_MCC                0x08
#define MASK_MNC                0x0A

//extern static vzw_apn_id s_vzw_apnid ;
//add by joe end

bool sendATcmdSyncAPN(){
    char atc_cmd_req[100] = {0};
    char atc_cmd_resp[512] = {0};
    int ret = 0;
    int ret_ext = false;
    //AT+QMBNCFG="autosel",1
    char *cmd = "AT+QMBNCFG=\"autosel\",1";
    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
    strcpy(atc_cmd_req,cmd);
    int cmd_len = strlen(atc_cmd_req);
    LOG_ARG("send at comand sync apn modify cmd_len=%d\n",cmd_len);
    atc_cmd_req[cmd_len]='\0';
    for (int i = 0; i < 1; i++)//only try once now
    {
        ret = send_AT_Command(atc_cmd_req,&atc_cmd_resp);
        if(ret < 0){
            LOG("send at comand sync apn modify failed\n");
            continue;
        }
        if(strlen(atc_cmd_resp) == 0){
            LOG("send at comand sync apn modify no rsp\n");
            continue;
        }
        if(strstr(atc_cmd_resp, "OK") != NULL){
            LOG("send at comand sync apn modify success!\n");
            break;
        }
    }
    return ret_ext;
}

bool sendATcmdRenameAPN(int profileid, char *apn){
    char atc_cmd_req[100] = {0};
    char atc_cmd_resp[512] = {0};
    int ret = 0;
    int ret_ext = false;
    //AT+cgdcont=1,"IPV4V6","attm2mglobal","0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0",0,0,0,0

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
    sprintf(atc_cmd_req,"AT+cgdcont=%d,\"IPV4V6\",\"%s\",\"0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0\",0,0,0,0", profileid, apn);
    int cmd_len = strlen(atc_cmd_req);
    LOG_ARG("send at comand rename apn cmd_len=%d\n",cmd_len);
    atc_cmd_req[cmd_len]='\0';
    for (int i = 0; i < 1; i++)//try once
    {
        ret = send_AT_Command(atc_cmd_req,&atc_cmd_resp);
        if(ret < 0){
            LOG("send at comand rename apn failed\n");
            continue;
        }
        if(strlen(atc_cmd_resp) == 0){
            LOG("send at comand rename apn no rsp\n");
            continue;
        }
        if(strstr(atc_cmd_resp, "OK") != NULL){
            LOG("send at comand rename apn success!\n");
            ret_ext = true;
            g_atc_apn_changed_time = lwm2m_gettime();
            sendATcmdSyncAPN();
            break;
        }
    }
    return ret_ext;
}

void quec_AT_cmd_modify_class3_apn_work_thread(char *apn_name){
    vzw_apn_id *vzw_apn_id = get_vzw_apnid();
    if(apn_list_count > 1){
        LOG("quec_AT_cmd_modify_class3_apn_work_thread wait 6 secs\n");
        sleep(6);
    }else{
        LOG("quec_AT_cmd_modify_class3_apn_work_thread wait 2 secs\n");
        sleep(2);
    }
    bool ret_apn3 = sendATcmdRenameAPN(vzw_apn_id->vzwinternet_apn3.profileid,apn_name);

    #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
    quec_check_class2_apn_network_available();
    #endif
    if(strcmp(apn_name,"VZWTEST") != 0 && strcmp(apn_name,"VZWINTERNETTEST") != 0 ){
        if(ret_apn3 == true){
            strlcpy( vzw_apn_id->vzwinternet_apn3.apn_name, apn_name, CARRIER_APN_LEN);
            LOG_ARG("set vzw_apn_id class3 apn_name=%s\n",vzw_apn_id->vzwinternet_apn3.apn_name);
        }
        enableApnNetworkByprofile(true,vzw_apn_id->vzwinternet_apn3.profileid,apn_name);
    }

}

void quec_AT_cmd_modify_class6_apn_work_thread(char *apn_name){
    vzw_apn_id *vzw_apn_id = get_vzw_apnid();
    sleep(2);
    bool ret_apn6 = sendATcmdRenameAPN(vzw_apn_id->vzwclass6_apn6.profileid,apn_name);

    #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
    quec_check_class2_apn_network_available();
    #endif
    if(strcmp(apn_name,"VZWTEST") != 0 && strcmp(apn_name,"VZWCLASS6TEST") != 0 ){
        if(ret_apn6 == true){
            strlcpy( vzw_apn_id->vzwclass6_apn6.apn_name, apn_name, CARRIER_APN_LEN);
            LOG_ARG("set vzw_apn_id class6 apn_name=%s\n",vzw_apn_id->vzwclass6_apn6.apn_name);
        }
        //enableApnNetworkByprofile(true,vzw_apn_id->vzwclass6_apn6.profileid,apn_name);
    }


}

static uint8_t prv_set_value(lwm2m_data_t * dataP,
                             conn_m_data_t * connDataP)
{
    switch (dataP->id)
    {
    case RES_M_NETWORK_BEARER:
    {
        #if 0
        int64_t bearer_type = QL_LWM2M_GetNetworkType();
        if(bearer_type == VALUE_NETWORK_BEARER_INVALID)
        {
            return COAP_405_METHOD_NOT_ALLOWED;
        }
        #endif
        lwm2m_data_encode_int(connDataP->networkBearer, dataP);


        return COAP_205_CONTENT;
    }
    case RES_M_AVL_NETWORK_BEARER:
    {
        int riCnt = 1;   // reduced to 1 instance to fit in one block size
        lwm2m_data_t * subTlvP;

        int64_t bearer_type = QL_LWM2M_GetNetworkType();
        if(bearer_type == VALUE_NETWORK_BEARER_INVALID)
        {
            bearer_type = VALUE_AVL_NETWORK_BEARER_1;
        }
        subTlvP = lwm2m_data_new(riCnt);
        subTlvP[0].id    = 0;
        lwm2m_data_encode_int(bearer_type, subTlvP);
        lwm2m_data_encode_instances(subTlvP, riCnt, dataP);
        return COAP_205_CONTENT ;
    }

    case RES_M_RADIO_SIGNAL_STRENGTH: //s-int
        //connDataP->signalStrength = QL_LWM2M_GetSignalStrength();
        lwm2m_data_encode_int(connDataP->signalStrength, dataP);
        return COAP_205_CONTENT;

    case RES_O_LINK_QUALITY: //s-int
        lwm2m_data_encode_int(connDataP->linkQuality, dataP);
        return COAP_205_CONTENT ;

    case RES_M_IP_ADDRESSES:
    {
        int ri, riCnt = 1;   // reduced to 1 instance to fit in one block size
        if(lwm2m_check_is_vzw_netwrok()){
            riCnt = 2;//Joe modify for VZW diag test
        }
        lwm2m_data_t* subTlvP = lwm2m_data_new(riCnt);
        for (ri = 0; ri < riCnt; ri++)
        {
            subTlvP[ri].id = ri;
            if(lwm2m_check_is_vzw_netwrok()){
                char ipaddr[16]={0};
                if(ri == 0 )
                    quec_lwm2m_get_class2_ip_by_ifname(ipaddr);
                else
                    quec_lwm2m_get_ip_by_ifname(ipaddr);
                strcpy(connDataP->ipAddresses[ri], ipaddr);
            }else{
                strcpy(connDataP->ipAddresses[ri], get_ipaddr());
            }
            lwm2m_data_encode_string(connDataP->ipAddresses[ri], subTlvP + ri);
        }
        lwm2m_data_encode_instances(subTlvP, riCnt, dataP);
        return COAP_205_CONTENT ;
    }
        break;

    case RES_O_ROUTER_IP_ADDRESS:
    {
        int ri, riCnt = 1;   // reduced to 1 instance to fit in one block size
        lwm2m_data_t* subTlvP = lwm2m_data_new(riCnt);
        for (ri=0; ri<riCnt; ri++)
        {
            subTlvP[ri].id = ri;
            lwm2m_data_encode_string(connDataP->routerIpAddresses[ri], subTlvP + ri);
        }
        lwm2m_data_encode_instances(subTlvP, riCnt, dataP);
        return COAP_205_CONTENT ;
    }
        break;

    case RES_O_LINK_UTILIZATION:
        lwm2m_data_encode_int(connDataP->linkUtilization, dataP);
        return COAP_205_CONTENT;

    case RES_O_APN:
        if(lwm2m_check_is_vzw_netwrok()){
            int riCnt2 = 1;   // reduced to 1 instance to fit in one block size
            lwm2m_data_t * subTlvP2;
            subTlvP2 = lwm2m_data_new(riCnt2);
            subTlvP2[0].id     = 0;

            //char apn2[256] = {0};
            //prv_get_device_apn(&apn2);
            char *apn2 = lwm2m_malloc(20);
            memset(apn2, 0, sizeof(20));
            get_vzw_apn2_name(apn2);
            lwm2m_data_encode_string(apn2, dataP);

            return COAP_205_CONTENT;
        }else{
            int riCnt = 1;   // reduced to 1 instance to fit in one block size
            lwm2m_data_t * subTlvP;
            subTlvP = lwm2m_data_new(riCnt);
            subTlvP[0].id     = 0;
            char apn[256] = {0};
            prv_get_device_apn(&apn);
            // lwm2m_data_encode_string(apn, dataP);
            lwm2m_data_encode_string(apn, subTlvP);
            lwm2m_data_encode_instances(subTlvP, riCnt, dataP);
            return COAP_205_CONTENT;
        }
    case RES_O_APN_30000: //add by joe start
        if(lwm2m_check_is_vzw_netwrok()){
        
          int ri_apn_7 = 0, ri_apn_30000 = 0;
          lwm2m_data_t* subTlvP = NULL;
          resource_instance_string_list_t* apnList = NULL;
          for (apnList = connDataP->apnList;apnList!=NULL;apnList = apnList->next)
          {
              if(apnList->resInstId == RES_O_APN_CLASS2)
                 ri_apn_7++;
              else
                 ri_apn_30000++;
          }
        
          if ((ri_apn_7==0) && (ri_apn_30000 == 0))  // no values!
          {
            LOG("(ri_apn_7==0) && (ri_apn_30000 == 0) return");
             return COAP_404_NOT_FOUND;
          }
          if(dataP->id == RES_O_APN)
             subTlvP = lwm2m_data_new(ri_apn_7);
          else if(dataP->id == RES_O_APN_30000)
             subTlvP = lwm2m_data_new(ri_apn_30000);
          if (NULL == subTlvP) 
             return COAP_500_INTERNAL_SERVER_ERROR;
          ri_apn_7 = 0;
          ri_apn_30000 = 0;
          for (apnList = connDataP->apnList; apnList!= NULL;apnList = apnList->next)
          {
             if((dataP->id == RES_O_APN) && (apnList->resInstId == RES_O_APN_CLASS2))
             {
                 subTlvP[ri_apn_7].id = (uint16_t)apnList->resInstId;
                 lwm2m_data_encode_string(apnList->InstValue, &subTlvP[ri_apn_7]);
                 ri_apn_7++;
             }
             else if ((dataP->id == RES_O_APN_30000) && (apnList->resInstId != RES_O_APN_CLASS2))
             {
               subTlvP[ri_apn_30000].id = (uint16_t)apnList->resInstId;
               lwm2m_data_encode_string(apnList->InstValue, &subTlvP[ri_apn_30000]);
               ri_apn_30000++;
             }
          }
          if(dataP->id == RES_O_APN_30000)
           lwm2m_data_encode_instances(subTlvP, ri_apn_30000, dataP);
          else
           lwm2m_data_encode_instances(subTlvP, ri_apn_7, dataP);
          return COAP_205_CONTENT;

        }

    case RES_O_CELL_ID:
        lwm2m_data_encode_int(connDataP->cellId, dataP);
        return COAP_205_CONTENT ;

    case RES_O_SMNC:
    {
        int mnc = QL_LWM2M_GetMnc();
        lwm2m_data_encode_int(mnc, dataP);
    }
        return COAP_205_CONTENT ;

    case RES_O_SMCC:
        {
            int mcc = QL_LWM2M_GetMcc();
            lwm2m_data_encode_int(mcc, dataP);
        }
        return COAP_205_CONTENT ;
    default:
        return COAP_404_NOT_FOUND ;
    }
}

static uint8_t prv_read(uint16_t instanceId,
                        int * numDataP,
                        lwm2m_data_t ** dataArrayP,
                        lwm2m_object_t * objectP)
{
    uint8_t result;
    int i;

    // this is a single instance object
    //if (instanceId != 0)
    //{
    //    return COAP_404_NOT_FOUND ;
    //}

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_M_NETWORK_BEARER,
                RES_M_AVL_NETWORK_BEARER,
                RES_M_RADIO_SIGNAL_STRENGTH,
                RES_O_LINK_QUALITY,
                RES_M_IP_ADDRESSES,
                RES_O_ROUTER_IP_ADDRESS,
                RES_O_LINK_UTILIZATION,
                RES_O_APN,
                RES_O_APN_30000,//add by joe
                RES_O_CELL_ID,
                RES_O_SMNC,
                RES_O_SMCC
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR ;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        result = prv_set_value((*dataArrayP) + i, (conn_m_data_t*) (objectP->userData));
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT );

    return result;
}
//add by joe start
void conn_monitoring_update_ip_addresses(conn_m_data_t* connDataP)
{
#if 0

     int ri = 0;
     resource_instance_string_list_t *apnLst;
     apptcpip_ip_addr_struct local_addr;
     apptcpip_ip_addr_struct local_addr_v6;
     struct ps_in_addr  addr;
     struct ps_in6_addr addr_v6;
     char ip_addr_str[48] = {0}, router_ip_addr_str[48] = {0};
     char ip_addr_str_v6[64] = {0}, router_ip_addr_str_v6[64] = {0};
     int ipCnt = 0;
     resource_instance_string_list_t* ipListSave;
    

     memset(&local_addr, 0, sizeof(apptcpip_ip_addr_struct));
     if (NULL != connDataP)
     {
        apnLst = connDataP->apnList;
        ipListSave = connDataP->ipAddresses;
        connDataP->ipAddresses = NULL;
        if(apptcpip_netif_get_local_ip_address_extend(connDataP->pdp_cid, &local_addr,APPTCPIP_AF_INET)== APPTCPIP_OK)
        {
            memset(&addr, 0,sizeof(struct ps_in_addr));
            addr.ps_s_addr = dss_ntohl(local_addr.apptcpip_ipv4_addr.ip_addr);

            dss_inet_ntoa(addr,(uint8 *)ip_addr_str, 48);
            prv_write_string_resource_inst_val(&(connDataP->ipAddresses),ri, ip_addr_str);
            dss_inet_ntoa(addr,(uint8 *)router_ip_addr_str, 48);
            prv_write_string_resource_inst_val(&(connDataP->routerIpAddresses),ri, router_ip_addr_str);
        }
        if(apptcpip_netif_get_local_ip_address_extend(connDataP->pdp_cid, &local_addr_v6,APPTCPIP_AF_INET6)== APPTCPIP_OK)
        {
            int16    dss_errno;
            memset(&addr_v6, 0,sizeof(struct ps_in6_addr));
            addr_v6.in6_u.u6_addr64[0] = dss_ntohll(local_addr.apptcpip_ipv6_addr.ip_addr[0]);
            addr_v6.in6_u.u6_addr64[1] = dss_ntohll(local_addr.apptcpip_ipv6_addr.ip_addr[1]);

            (void) dss_inet_ntop((char*)&addr_v6,
                         DSS_AF_INET6,
                         (void*)ip_addr_str_v6,
                         64,
                         &dss_errno);
             prv_write_string_resource_inst_val(&(connDataP->ipAddresses),ri, ip_addr_str_v6);
             (void) dss_inet_ntop((char*)&addr_v6,
                         DSS_AF_INET6,
                         (void*)router_ip_addr_str_v6,
                         64,
                         &dss_errno);
             prv_write_string_resource_inst_val(&(connDataP->routerIpAddresses),ri, router_ip_addr_str_v6);
        }

     }
#endif
}

void conn_mon_write_object_teardown_network(char* ifname){

    //teardown current network
    char *args[10] = {0};
    args[0] = "ifconfig";
    args[1] = ifname;
    args[2] = "down";
    quec_mcm_system_call("ifconfig", args);
}

//add by joe end
/* @fn		static uint8_t prv_discover()
 * @brief	This function is used to discover resources of a connectivity monitoring object
 * @param	instanceId - instance id of object 
 *			numdataP   - pointer to store number of data array elements 
			dataArrayP - pointer to store data 
			objectP	- pointer to lwm2m_object 
 * @return	on success - LWM2M_404_NOT_FOUND
    		on error	- LWM2M_404_NOT_FOUND
*/   
static uint8_t prv_discover(uint16_t instanceId,
						int * numDataP,
						lwm2m_data_t ** dataArrayP,
						lwm2m_object_t * objectP)
//add by joe start
{
#if 1
     {
	 uint8_t result = 0;
	 int i = 0, ri_apn_7 = 0, ri_apn_30000 = 0, ri_ip = 0, ri_rip = 0;
//	 resource_instance_string_list_t *apn, *ip, *router_ip;
	 conn_m_data_t * targetP;
						
						  // this is a single instance object
	 if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
	 {
		return COAP_500_INTERNAL_SERVER_ERROR;
     }

	 if (instanceId != 0)
	 {
		return COAP_404_NOT_FOUND;
	 }
						
	 result = COAP_205_CONTENT;
	 targetP = (conn_m_data_t*) (objectP->userData);
//	 conn_monitoring_update_ip_addresses(targetP);
	return result;
}

#else
{
	 uint8_t result = 0;
	 int i = 0, ri_apn_7 = 0, ri_apn_30000 = 0, ri_ip = 0, ri_rip = 0;
	 resource_instance_string_list_t *apn, *ip, *router_ip;
	 conn_m_data_t * targetP;
						
						  // this is a single instance object
	// if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
	 //{
	//	return COAP_500_INTERNAL_SERVER_ERROR;
    // }

	 if (instanceId != 0)
	 {
		return COAP_404_NOT_FOUND;
	 }
						
	 result = COAP_205_CONTENT;
	 targetP = (conn_m_data_t*) (objectP->userData);
	 conn_monitoring_update_ip_addresses(targetP);
	 if(targetP != NULL)
	 {
		 for (apn = targetP->apnList;apn!=NULL;apn = apn->next)
		 {
			if(apn->resInstId == RES_O_APN_CLASS2)
				  ri_apn_7++;
			else
				  ri_apn_30000++;
		 }
				
		 for (ip = targetP->ipAddresses, ri_ip=0; ip!=NULL; ip = ip->next, ri_ip++);
						
		 for (router_ip= targetP->routerIpAddresses, ri_rip=0; router_ip!=NULL; router_ip = router_ip->next, ri_rip++);
	}
	// is the server asking for the full object ?
	if (*numDataP == 0)
	{
		uint16_t resList[] = {
			  RES_M_NETWORK_BEARER,
			  RES_M_AVL_NETWORK_BEARER,
			  RES_M_RADIO_SIGNAL_STRENGTH,
			  RES_O_LINK_QUALITY,
			  RES_M_IP_ADDRESSES,
			  RES_O_ROUTER_IP_ADDRESS,
			 // As interface is not available , so disabling , once interface available need to enable 
#if 0
			  // VZ_REQ_OTADMLWM2M_41068 Requirement 
			  RES_O_LINK_UTILIZATION,
#endif            
			  RES_O_APN,
			  RES_O_APN_30000,
			  RES_O_CELL_ID,
			  RES_O_SMNC,
			  RES_O_SMCC ,
			};
		int nbRes = sizeof(resList) / sizeof(uint16_t);
	//	int temp = nbRes;
						
		*dataArrayP = lwm2m_data_new(nbRes);
		
		if (*dataArrayP == NULL) 
			return COAP_500_INTERNAL_SERVER_ERROR;
						
		*numDataP = nbRes;
						
		for (i = 0; i < nbRes; i++)
		{
			(*dataArrayP)[i].id = resList[i];
			if((*dataArrayP)[i].id == RES_M_AVL_NETWORK_BEARER )
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				if (targetP != NULL)
				{
					  (*dataArrayP)[i].value.asChildren.count = targetP->availNetworkBearerLength;
				}
			}
		 	else if ((*dataArrayP)[i].id == RES_M_IP_ADDRESSES )
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = ri_ip;
			}
			else if((*dataArrayP)[i].id == RES_O_ROUTER_IP_ADDRESS)
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = ri_rip;
			} 
			else if((*dataArrayP)[i].id == RES_O_APN)
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = ri_apn_7;
			} 
			else if((*dataArrayP)[i].id == RES_O_APN_30000)
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = ri_apn_30000;
			} 
		}

	}
	else
	{
		for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
		{
		 	switch ((*dataArrayP)[i].id)
			{ 			   
			case RES_M_AVL_NETWORK_BEARER:
				{
				  (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				  if (targetP != NULL)
				  {
						(*dataArrayP)[i].value.asChildren.count = targetP->availNetworkBearerLength;
				  }
				  break;
				}
			case RES_M_IP_ADDRESSES:
				{
				  (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				  (*dataArrayP)[i].value.asChildren.count = ri_ip;
				  break;
				}
			case RES_O_ROUTER_IP_ADDRESS:
				{
				  (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				  (*dataArrayP)[i].value.asChildren.count = ri_rip;
				  break;
				}
			case RES_O_APN:
			    {
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
					(*dataArrayP)[i].value.asChildren.count = ri_apn_7;
					 break;
				}
			case RES_O_APN_30000:
			    {
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
					(*dataArrayP)[i].value.asChildren.count = ri_apn_30000;
					break;
				}
			case RES_M_NETWORK_BEARER:
			case RES_M_RADIO_SIGNAL_STRENGTH:
			case RES_O_LINK_QUALITY:			 
			  // As interface is not available , so disabling , once interface available need to enable 
#if 0  
				//  VZ_REQ_OTADMLWM2M_41068 Requirement			
			case RES_O_LINK_UTILIZATION:
#endif
			case RES_O_CELL_ID:
			case RES_O_SMNC:
			case RES_O_SMCC:
				break;
						
			default:
				result = COAP_404_NOT_FOUND;
			}
		}
	}
	return result;
}
#endif
}

/**
* @fn static uint8_t prv_write_conn_m()
*
* @brief This fucntion implements the write operation performed 
*		  on Connectivity Monitoring object
*
* @param instanceId Instance ID of the object to be read
* @param numData number of resources in the array
* @param dataArray resource value array
* @param objectP Object to be read
*
* @return LWM2M response type
*/
static uint8_t prv_write_conn_m
(uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP)

{
    int i = 0;
	uint8_t result = 0;

    if( dataArray == NULL || objectP == NULL ) 
	{
	 	return COAP_500_INTERNAL_SERVER_ERROR;
	}
						
	// this is a single instance object
	if (instanceId != 0)
	{
		return COAP_404_NOT_FOUND;
	}
						
	i = 0;
						
	do
	{
	    switch (dataArray[i].id)
	    {
		case RES_O_APN_30000:
			{
				int ri = 0;
				lwm2m_data_t* subTlvArray = NULL;
				char apn_name[CARRIER_APN_LEN];
                #if 0 //do not support in Lwm2m1.0
				apptcpip_pdp_profile_param_struct pdp_profile;
				if(resInstId != INVALID_RES_INST_ID)
				{
					strlcpy(apn_name, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length + 1);
					if(resInstId == RES_O_APN_CLASS2)
					{
						return COAP_404_NOT_FOUND;
					}
					if (!prv_write_string_resource_inst_val(&(((conn_m_data_t*)(objectP->userData))->apnList),resInstId, apn_name))
					{
						result = COAP_500_INTERNAL_SERVER_ERROR;
						break;
					}
					else
					{
						if(resInstId == RES_O_APN_CLASS3)
						{							
						 	 LWM2M_STRCPY( class3_apn, (const char *)dataArray[i].value.asBuffer.buffer, CARRIER_APN_LEN);
						 	 LWM2M_STRCPY( class3_apn, apn_name, CARRIER_APN_LEN);

							 memset(&pdp_profile, 0, sizeof(apptcpip_pdp_profile_param_struct));
							 apptcpip_netif_get_pdp_profile(3,&pdp_profile);
							 memset(pdp_profile.apn, 0, 129);
							 LWM2M_STRCPY( (char *)pdp_profile.apn, apn_name, CARRIER_APN_LEN);
							 apptcpip_netif_set_pdp_profile(3,&pdp_profile);
						}
						if(resInstId == RES_O_APN_CLASS6)
						{
							  LWM2M_STRCPY( class6_apn, (const char *)dataArray[i].value.asBuffer.buffer, CARRIER_APN_LEN);
							  LWM2M_STRCPY( class6_apn, apn_name, CARRIER_APN_LEN);
							  memset(&pdp_profile, 0, sizeof(apptcpip_pdp_profile_param_struct));
							  apptcpip_netif_get_pdp_profile(6,&pdp_profile);
							  memset(pdp_profile.apn, 0, 129);
							  LWM2M_STRCPY( (char *)pdp_profile.apn, apn_name, CARRIER_APN_LEN);
							 apptcpip_netif_set_pdp_profile(6,&pdp_profile);
						}
						if(resInstId == RES_O_APN_CLASS7)
						{
							  LWM2M_STRCPY( class7_apn, (const char *)dataArray[i].value.asBuffer.buffer, CARRIER_APN_LEN);
							  LWM2M_STRCPY( class7_apn, apn_name, CARRIER_APN_LEN);
							  memset(&pdp_profile, 0, sizeof(apptcpip_pdp_profile_param_struct));
							  apptcpip_netif_get_pdp_profile(7,&pdp_profile);
							  memset(pdp_profile.apn, 0, 129);
							  LWM2M_STRCPY( (char *)pdp_profile.apn, apn_name, CARRIER_APN_LEN);
							  apptcpip_netif_set_pdp_profile(7,&pdp_profile);
						}
					
						result = COAP_204_CHANGED;
					
					}
				}
				else
                #endif
				{
					  resource_instance_string_list_t* apnListSave = ((conn_m_data_t*)(objectP->userData))->apnList;
					  if(dataArray[i].type != LWM2M_TYPE_MULTIPLE_RESOURCE)
		   				 return COAP_400_BAD_REQUEST;
						#if 0
					  if(write_method == COAP_PUT)
						  ((conn_m_data_t*)(objectP->userData))->apnList = NULL;
						#endif
					  subTlvArray = dataArray[i].value.asChildren.array;
                      LOG_ARG("apnlist dataArray[i].value.asChildren.count=%d\n",dataArray[i].value.asChildren.count);
                      apn_list_count = dataArray[i].value.asChildren.count;
					  if (dataArray[i].value.asChildren.count == 0)
					  {
					  	 result = COAP_400_BAD_REQUEST;
					  }
					  else if (subTlvArray==NULL)
					  {
						result = COAP_400_BAD_REQUEST;
					  }
					  else
					  {
					  	 for (ri=0; ri < dataArray[i].value.asChildren.count; ri++)
						 {
						     strlcpy(apn_name, (char *)subTlvArray[ri].value.asBuffer.buffer, subTlvArray[ri].value.asBuffer.length + 1);
							 if(subTlvArray[ri].id == RES_O_APN_CLASS2)
							 {
								return COAP_405_METHOD_NOT_ALLOWED;
							 }
							 if (!prv_write_string_resource_inst_val(&(((conn_m_data_t*)(objectP->userData))->apnList),subTlvArray[ri].id, apn_name))
							 {
								result = COAP_500_INTERNAL_SERVER_ERROR;
								break;
							 }
							 else
							 {
                            
                                vzw_apn_id *vzw_apn_id = get_vzw_apnid();
                                char profileid[10] = {0};
								if(subTlvArray[ri].id == RES_O_APN_CLASS3)
								{
								    LOG_ARG("class3_apn=%s apn_name=%s\n",class3_apn,apn_name);
                                    if(strcmp(class3_apn,apn_name) == 0){
                                        LOG("skip the same apn name change\n");
                                        return COAP_204_CHANGED;//no change ,skip this
                                    }
								   reset_vzw_apn3_callid();//shutdown old apn3 network
								   sprintf(profileid,"%d",vzw_apn_id->vzwinternet_apn3.profileid);//transmit profildid to char
								   strlcpy( class3_apn, apn_name, CARRIER_APN_LEN);

                                   pthread_t atcmd3_thread_id;
                                   pthread_create(&atcmd3_thread_id, NULL, quec_AT_cmd_modify_class3_apn_work_thread, class3_apn);

								}
								if(subTlvArray[ri].id == RES_O_APN_CLASS6)
								{
								    LOG_ARG("class6_apn=%s apn_name=%s\n",class6_apn,apn_name);
                                    if(strcmp(class6_apn,(const char *)subTlvArray[ri].value.asBuffer.buffer) == 0){
                                        return COAP_204_CHANGED;//no change ,skip this
                                    }
                                  sprintf(profileid,"%d",vzw_apn_id->vzwclass6_apn6.profileid);
								  strlcpy( class6_apn, apn_name, CARRIER_APN_LEN);

                                  pthread_t atcmd6_thread_id;
                                  pthread_create(&atcmd6_thread_id, NULL, quec_AT_cmd_modify_class6_apn_work_thread, class6_apn);

								}
								if(subTlvArray[ri].id == RES_O_APN_CLASS7)
								{
                                    if(strcmp(class7_apn,(const char *)subTlvArray[ri].value.asBuffer.buffer) == 0){
                                        return COAP_204_CHANGED;//no change ,skip this
                                    }
                                  sprintf(profileid,"%d",7);
								  strlcpy( class7_apn, apn_name, CARRIER_APN_LEN);
                                  sendATcmdRenameAPN(7,apn_name);
                                  #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
                                  quec_check_class2_apn_network_available();
                                  #endif
                                  enableApnNetworkByprofile(true,7,apn_name);
								}

								result = COAP_204_CHANGED;
							 }
						 }
					 }
                     #if 0
					 if (result != COAP_204_CHANGED)
					 {
						// free pot. partial created new ones
						if(write_method == COAP_PUT)
							  LWM2M_LIST_FREE(((conn_m_data_t*)(objectP->userData))->apnList);
							// restore old values:
								((conn_m_data_t*)(objectP->userData))->apnList = apnListSave;
					 }
					 else
					 {
						// final free saved value list
						if(write_method == COAP_PUT)
							  LWM2M_LIST_FREE(apnListSave);
					  }
                     #endif
				  }				
			    }
				break;
		   case RES_M_NETWORK_BEARER:
		   case RES_M_AVL_NETWORK_BEARER:
		   case RES_M_RADIO_SIGNAL_STRENGTH:
		   case RES_O_LINK_QUALITY:
		   case RES_M_IP_ADDRESSES:
		   case RES_O_CELL_ID:
		   case RES_O_SMNC:
		   case RES_O_SMCC:
		   case RES_O_APN:
		   case RES_O_ROUTER_IP_ADDRESS:
				{
				  /*If create/write have one more one resource to write we write on reablable resource*/
			   	   if(numData > 1)
					  result = COAP_204_CHANGED;
				   else
					  result = COAP_405_METHOD_NOT_ALLOWED;
				   break;
				}
		   default:
				/*If create/write have one more one resource to write we write on unknown/notsupported resource*/
					if(numData > 1)
					  result = COAP_204_CHANGED;
					else
					  result = COAP_404_NOT_FOUND;
	    	}
			
			i++;
	  } while (i < numData && result == COAP_204_CHANGED);
     return result;  
}

/**
 * @fn static uint8_t prv_conn_mon_create()
 *
 * @brief This fucntion is used to create Connectivity Monitoring object
 *
 * @param instanceId Instance ID of the object to be created
 * @param numData number of resources in the array
 * @param dataArray resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
*/
static uint8_t prv_conn_mon_create
(
void *ctx_p,
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP)
{
  conn_m_data_t *connObj = NULL;
  unsigned int ret_status = 0;
  uint8_t result = 0;
						
  if( dataArray == NULL || objectP == NULL ) 
  {
	return COAP_500_INTERNAL_SERVER_ERROR;
  }
						
   /*check if already one instance is created */
  if(objectP->instanceList != NULL)
  {
	return COAP_400_BAD_REQUEST;
  }
						
  connObj = (conn_m_data_t*) lwm2m_malloc(sizeof(conn_m_data_t));
  if( NULL == connObj)
  {
	 return COAP_500_INTERNAL_SERVER_ERROR;
  }
						
  memset(connObj, 0, sizeof(conn_m_data_t));
  objectP->userData = connObj;
						
  get_connectivity_info(connObj, &ret_status);
						
  if(ret_status & MASK_SIGNAL_STRENGTH)
  {
	connObj->signalStrength  = 0;
  }
						
  if(ret_status & MASK_LINK_QUALITY)
  {
	connObj->linkQuality	 = 0;
  }
						
  if(ret_status & MASK_CELL_ID)
  { 	
	connObj->cellId = 0;
  }
	
  if(ret_status & MASK_MCC)
  {
	connObj->mcc = 0;
  }
						
  if(ret_status & MASK_MNC)
  {
	connObj->mnc = 0;
  } 
  prv_write_string_resource_inst_val(&(connObj->apnList),RES_O_APN_CLASS2, CLASS2_APN);
  prv_write_string_resource_inst_val(&(connObj->apnList),RES_O_APN_CLASS3, CLASS3_APN);
  prv_write_string_resource_inst_val(&(connObj->apnList),RES_O_APN_CLASS6, CLASS6_APN);
  prv_write_string_resource_inst_val(&(connObj->apnList),RES_O_APN_CLASS7, CLASS7_APN);
						  
  result = prv_write_conn_m(instanceId, numData, dataArray, objectP);
  if (result != COAP_204_CHANGED)
  {
	// Issue in creating object instanace so retuning Bad request error.
	result = COAP_400_BAD_REQUEST;
	objectP->userData = NULL;
	lwm2m_free(connObj);
	return result;
  }

						
  objectP->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
  if (NULL != objectP->instanceList)
  {
	memset(objectP->instanceList, 0x00, sizeof(lwm2m_list_t));
  }
						
  unlink(LWM2M_CONN_MON_OBJ_PERSISTENCE_PATH);
  return COAP_201_CREATED;
						
}
/**
* @fn static uint8_t prv_conn_mon_delete()
*
* @brief This fucntion is used to delete Connectivity Monitoring object
*
* @param id Instance ID of the object to be deleted
* @param objectP Object to be deleted
*
* @return LWM2M response type
*/
static uint8_t prv_conn_mon_delete(uint16_t id,
                                   lwm2m_object_t * objectP)
{
   int fd = 0;
   conn_m_data_t * connObj = NULL;
						
   if( objectP == NULL ) 
   {
		return COAP_500_INTERNAL_SERVER_ERROR;
   }
						
						
   objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&connObj);
   if (NULL == connObj) 
   	  return COAP_404_NOT_FOUND;
						
   lwm2m_free(connObj);
   connObj = NULL;
						
	/*  Update single instance object persistence */
						
	fd = open( LWM2M_CONN_MON_OBJ_PERSISTENCE_PATH, O_CREAT, 00744);
	if( EFAILURE == fd)
	{
		return COAP_500_INTERNAL_SERVER_ERROR;
	}

	close(fd);
						
	//conn_mon_created = TRUE;
	free_object_conn_m(objectP);
						
	return COAP_202_DELETED;
						
}
void get_connectivity_info(conn_m_data_t *conn_obj, unsigned int *ret_status)
{
#if 0
	*ret_status = 0;
	conn_obj->signalStrength = prv_get_device_rsrp();
	if(conn_obj->signalStrength != 65535)
		*ret_status |= MASK_SIGNAL_STRENGTH;

	conn_obj->linkQuality = prv_get_device_rx_quality();
	if(conn_obj->linkQuality != 0xFFFFFFFF)
		*ret_status |= MASK_LINK_QUALITY;

	conn_obj->cellId = prv_get_device_cellid();
	if(conn_obj->cellId != 65535)
		*ret_status |= MASK_CELL_ID;

	conn_obj->mcc =	prv_get_current_mcc();
	if(conn_obj->mcc != 0xFFFF)
		*ret_status |= MASK_MCC;

	conn_obj->mnc =	prv_get_current_mnc();
	if(conn_obj->mnc != 0xFFFF)
		*ret_status |= MASK_MNC;
#endif
}
/** 
 * @fn	   m2m_set_network_bearer_to_conn_m_obj()
 * @brief  This function is used to fetch network bearer and, available network bearer
 *		   values from QMI
 * @param  context	  - lwm2m context handle 
 * @return on success - ESUCCESS
 *		   on error   - EFAILURE
*/
int set_network_bearer_to_conn_m_obj(lwm2m_context_t * context)
{
#if 0
  lwm2m_object_t * targetP = NULL; 
					
  if (context == NULL)
  {
	return EFAILURE;
  }
  targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_CONN_MONITOR_OBJECT_ID);
  if (targetP == NULL) 
  {
	return EFAILURE;
  }
  if (NULL != targetP->userData)
  {
	 ((conn_m_data_t*) (targetP->userData))->networkBearer = prv_get_current_bearer_type();
  }
#endif  
  return ESUCCESS;
}

static int32_t prv_conn_mon_setvalue 
(
void *ctx_p,
lwm2m_object_data_t * lwm2m_data,
lwm2m_object_t * targetP,
lwm2m_uri_t *uriP
)
{
	int result = 0;
	conn_m_data_t* connDataP = NULL;
	
	if (lwm2m_data == NULL || targetP == NULL)
	{
		return 0;
	}
	connDataP = targetP->userData;
	
	if (NULL == connDataP)
	{
		return 0;
    }
	
	switch (lwm2m_data->instance_info->resource_info->resource_ID) 
	{
		case RES_M_RADIO_SIGNAL_STRENGTH://rsrp
		  {
		    if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				connDataP->signalStrength = lwm2m_data->instance_info->resource_info->value.asInteger;
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				//uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		  }
		  break;
	
		case RES_O_LINK_QUALITY: //rsrq
		  {
		 	 if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				connDataP->linkQuality = lwm2m_data->instance_info->resource_info->value.asInteger;
				uriP->objectId   = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				//uriP->flag	   = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID ;
		
				result = 1;
		 	}
		  }
		  break;
		default :
		  result = 0;
		  break;
	  }
	
	  return result;

}

bool checkifApn3unAvailable(){
    //return true;
#ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
    if(get_vzw_apn3_callid() == -1 || get_vzw_apn3_callid() == -2/*|| get_vzw_apn3_callid() == 0*/){
        return true;
    }
    return false;

#else
    return false;
#endif
}
//add by joe end
lwm2m_object_t * get_object_conn_m(void)
{
    /*
     * The get_object_conn_m() function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * connObj;

    connObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != connObj)
    {
        memset(connObj, 0, sizeof(lwm2m_object_t));

        /*
         * It assigns his unique ID
         */
        connObj->objID = LWM2M_CONN_MONITOR_OBJECT_ID;
        
        /*
         * and its unique instance
         *
         */
        connObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != connObj->instanceList)
        {
            memset(connObj->instanceList, 0, sizeof(lwm2m_list_t));
        }
        else
        {
            lwm2m_free(connObj);
            return NULL;
        }
        
        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        connObj->readFunc = prv_read;
        connObj->discoverFunc = prv_discover;
        connObj->executeFunc = NULL;
		connObj->deleteFunc = prv_conn_mon_delete;
		connObj->writeFunc = prv_write_conn_m;
        connObj->userData = lwm2m_malloc(sizeof(conn_m_data_t));

        /*
         * Also some user data can be stored in the object with a private structure containing the needed variables
         */
        if (NULL != connObj->userData)
        {
            memset(connObj->userData, 0, sizeof(conn_m_data_t));
            QL_LWM2M_CONN_INFO_T conn_info;
            QL_LWM2M_GetConnInfo(&conn_info);
            conn_m_data_t *myData = (conn_m_data_t*) connObj->userData;
            myData->cellId          = QL_LWM2M_GetCellInfo();
            // myData->signalStrength  = QL_LWM2M_GetSignalStrength();
            // myData->linkQuality     = QL_LWM2M_GetRsRq();
            // myData->linkUtilization = VALUE_LINK_UTILIZATION;
            myData->signalStrength  = conn_info.signalStrength;
            myData->linkQuality     = conn_info.linkQuality;
            myData->linkUtilization = conn_info.linkUtilization;
            if(lwm2m_check_is_vzw_netwrok()){
                char ipaddr[16]={0};
                quec_lwm2m_get_ip_by_ifname(ipaddr);
                strcpy(myData->ipAddresses[0], ipaddr);
            }else{
                strcpy(myData->ipAddresses[0], get_ipaddr());
            }
            strcpy(myData->ipAddresses[1],       VALUE_IP_ADDRESS_2);
            strcpy(myData->routerIpAddresses[0], VALUE_ROUTER_IP_ADDRESS_1);
            strcpy(myData->routerIpAddresses[1], VALUE_ROUTER_IP_ADDRESS_2);

            
            //init apn list
            if(lwm2m_check_is_vzw_netwrok()){
                vzw_apn_id *vzw_apn_id = get_vzw_apnid();
                //VZW class2 apn
                if(vzw_apn_id->vzwadmin_apn2.apn_name != NULL && strlen(vzw_apn_id->vzwadmin_apn2.apn_name) != 0){
                    LOG_ARG("get class2 apn from mbn = %s\n",vzw_apn_id->vzwadmin_apn2.apn_name);
                    strlcpy( class2_apn, vzw_apn_id->vzwadmin_apn2.apn_name, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS2, vzw_apn_id->vzwadmin_apn2.apn_name);
                }else{
                    strlcpy( class2_apn, CLASS2_APN, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS2, CLASS2_APN);
                }
                //VZW class3 apn
                if(vzw_apn_id->vzwinternet_apn3.apn_name != NULL && strlen(vzw_apn_id->vzwinternet_apn3.apn_name) != 0){
                    LOG_ARG("get class3 apn from mbn = %s\n",vzw_apn_id->vzwinternet_apn3.apn_name);
                    strlcpy( class3_apn, vzw_apn_id->vzwinternet_apn3.apn_name, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS3, vzw_apn_id->vzwinternet_apn3.apn_name);
                }else{
                    strlcpy( class3_apn, CLASS3_APN, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS3, CLASS3_APN);
                }
                //VZW class6 apn
                if(vzw_apn_id->vzwclass6_apn6.apn_name != NULL && strlen(vzw_apn_id->vzwclass6_apn6.apn_name) != 0){
                    LOG_ARG("get class6 apn from mbn = %s\n",vzw_apn_id->vzwclass6_apn6.apn_name);
                    strlcpy( class6_apn, vzw_apn_id->vzwclass6_apn6.apn_name, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS6, vzw_apn_id->vzwclass6_apn6.apn_name);
                }else{
                    strlcpy( class6_apn, CLASS6_APN, CARRIER_APN_LEN);
                    prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS6, CLASS6_APN);
                }

            }else{
                strlcpy( class2_apn, CLASS2_APN, CARRIER_APN_LEN);
                prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS2, CLASS2_APN);
                strlcpy( class3_apn, CLASS3_APN, CARRIER_APN_LEN);
                prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS3, CLASS3_APN);
                strlcpy( class6_apn, CLASS6_APN, CARRIER_APN_LEN);
                prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS6, CLASS6_APN);
            }
            
            strlcpy( class7_apn, CLASS7_APN, CARRIER_APN_LEN);
            prv_write_string_resource_inst_val(&(myData->apnList),RES_O_APN_CLASS7, CLASS7_APN);
        }
        else
        {
            lwm2m_free(connObj);
            connObj = NULL;
        }
    }
    return connObj;
}

void free_object_conn_m(lwm2m_object_t * objectP)
{
    lwm2m_free(objectP->userData);
    lwm2m_list_free(objectP->instanceList);
    lwm2m_free(objectP);
}

uint8_t connectivity_moni_change(lwm2m_data_t * dataArray,
                                 lwm2m_object_t * objectP)
{
    int64_t value;
    uint8_t result;
    conn_m_data_t * data;

    data = (conn_m_data_t*) (objectP->userData);

    switch (dataArray->id)
    {
    case RES_M_NETWORK_BEARER://add by joe
        if (1 == lwm2m_data_decode_int(dataArray, &value))
        {
            data->networkBearer = value;
            result = COAP_204_CHANGED;
        }
        else
        {
            result = COAP_400_BAD_REQUEST;
        }
        break;

    case RES_M_RADIO_SIGNAL_STRENGTH:
        if (1 == lwm2m_data_decode_int(dataArray, &value))
        {
            data->signalStrength = value;
            result = COAP_204_CHANGED;
        }
        else
        {
            result = COAP_400_BAD_REQUEST;
        }
        break;

    case RES_O_LINK_QUALITY:
        if (1 == lwm2m_data_decode_int(dataArray, &value))
        {
            data->linkQuality = value;
            result = COAP_204_CHANGED;
        }
        else
        {
            result = COAP_400_BAD_REQUEST;
        }
        break;

    case RES_M_IP_ADDRESSES:
        if (sizeof(data->ipAddresses[0]) <= dataArray->value.asBuffer.length)
        {
            result = COAP_400_BAD_REQUEST;
        }
        else
        {
            memset(data->ipAddresses[0], 0, sizeof(data->ipAddresses[0]));
            memcpy(data->ipAddresses[0], dataArray->value.asBuffer.buffer, dataArray->value.asBuffer.length);
            data->ipAddresses[0][dataArray->value.asBuffer.length] = 0;
            result = COAP_204_CHANGED;
        }
        break;

    case RES_O_ROUTER_IP_ADDRESS:
        if (sizeof(data->routerIpAddresses[0]) <= dataArray->value.asBuffer.length)
        {
            result = COAP_400_BAD_REQUEST;
        }
        else
        {
            memset(data->routerIpAddresses[0], 0, sizeof(data->routerIpAddresses[0]));
            memcpy(data->routerIpAddresses[0], dataArray->value.asBuffer.buffer, dataArray->value.asBuffer.length);
            data->routerIpAddresses[0][dataArray->value.asBuffer.length] = 0;
            result = COAP_204_CHANGED;
        }
        break;

    case RES_O_CELL_ID:
        if (1 == lwm2m_data_decode_int(dataArray, &value))
        {
            data->cellId = value;
            result = COAP_204_CHANGED;
        }
        else
        {
            result = COAP_400_BAD_REQUEST;
        }
        break;

    default:
        result = COAP_405_METHOD_NOT_ALLOWED;
    }

    return result;
}

// static int prv_get_current_mcc()
// {
    // char at[64];
    // char resp_mcc[64];
    // char mcc[16] = {0};
    // memset(resp_mcc, 0, 64);
    // memset(at, 0, 64);
    // memset(mcc, 0, 16);
    // char *temp = NULL;
    // sprintf(at, "AT+CIMI");
    // send_AT_Command(&at,&resp_mcc);
    // printf("mcc =%s \n",resp_mcc);
    // temp = strtok(resp_mcc, "\r\n");
    // while(temp)
    // {
        // if(strspn(temp, "0123456789") >= 1){
            // sscanf(temp, "%3s", mcc);
        // }
        // temp = strtok(NULL,"\n");
    // }
    // if(mcc == NULL){
        // return -1;
    // }
	// return atoi(mcc);
// }

// static int prv_get_current_mnc()
// {
    // char at[64];
    // char resp_mnc[64];
    // char mnc[64] = {0};
    // char real_mnc[64] = {0};
    // char *temp = NULL;

    // memset(resp_mnc, 0, 64);
    // memset(at, 0, 64);
    // sprintf(at, "AT+CIMI");
    // send_AT_Command(&at,&resp_mnc);
    // printf("mnc =%s \n",resp_mnc);
    // temp = strtok(resp_mnc, "\r\n");
    // while(temp)
    // {
        // if(strspn(temp, "0123456789") >= 1){
            // sscanf(temp, "%5s", mnc);
        // }
        // temp = strtok(NULL,"\n");
    // }
    // if(mnc != NULL){
        // snprintf(real_mnc, 64, "%s", mnc + 3);
    // }
	// return atoi(real_mnc);
// }

static void prv_get_device_apn(char *apn)
{
	att_apn_id *apn_id = lwm2m_get_apnid();
    if(apn_id->global_id != NULL){
        strcpy(apn, ATT_M2M_GLOBAL);
    }else{
        strcpy(apn, "m2m.com.attz");
    }
}
