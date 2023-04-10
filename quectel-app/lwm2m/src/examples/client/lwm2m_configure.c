
/*===========================================================================
	Copyright(c) 2019 Quectel Wierless Solution,Co.,Ltd. All Rights Reserved.
	Quectel Wireless Solution Proprietary and Confidential.
============================================================================*/
/*===========================================================================
							EDIT HISTORY FOR MODULE
This section contains comments describing changees made to the module.
Notice that changes are listed in reverse chronological order.
WHEN		 WHO		WHAT,WHERE,WHY
----------	-----		----------------------------------------------------
10/05/2019	Herry	  	Modify file for lwm2m att server.
============================================================================*/


#include "comdef.h"
#include "customer.h"
//#include "fs_public.h"
//#include "quectel_apptcpip_util.h"
#include "lwm2m_configure.h"
#include "cJSON.h"
#include "lwm2mclient.h"
#include "liblwm2m.h"
#include "fcntl.h"
// #include "nv.h"
// #include "nv_items.h"
//#include "mmgsdilib_common.h"
//#include "dsat_v.h"
//#include "Quectel_func_sim_common.h"
#include "sys/stat.h"
//#include <quectel_at_handle.h>
#include "internals.h"
#include <unistd.h>
#include <sys/wait.h>
#include <ql-mcm-api/ql_in.h>
#include <stdio.h>
#include <stdlib.h>
#include "j_aes.h"
#include "quectel_getinfo.h"
#include <pthread.h>
#ifdef LWM2M_SUPPORT_QLRIL_API
#include <RilPublic.h>
#include <ql_misc.h>
#endif

//#define ATC_REQ_CMD_MAX_LEN     100
//#define ATC_RESP_CMD_MAX_LEN    2048
#define HCISMD_SET_FILE         "/sys/module/hci_smd/parameters/hcismd_set"
LOCAL cJSON *pServer_Obj = NULL;
LOCAL cJSON *pHostDev_info_Obj = NULL;

LOCAL lwm2m_server_info_t  server_list[LWM2M_MAX_SERVER_CNT];
static char s_global_id[MAX_ID_LENGTH] = {0};
static char s_attz_id[MAX_ID_LENGTH] = {0};
static att_apn_id s_apnid = {0};
static data_client_handle_type  h_data = 0;
static char ip_addr[32] = {0};
static char ip_addr_class2[32] = {0};
static int mobaq = 0;

//add by joe
static vzw_apn_id s_vzw_apnid = {0};
//extern rex_tcb_type *apptcpip_myself;

lwm2m_client_net_type dsat_lwm2m_get_lwm2m_net(void){
    if(lwm2m_check_is_att_netwrok()){
        return LWM2M_CLIENT_NET_ATT;
    }else if(lwm2m_check_is_vzw_netwrok()){
        return LWM2M_CLIENT_NET_VZW;
    }
};
    
bool FUNC_SIM_GetMsisdnGsmNum(char *pNum){
    char at1[64]={0};
    char at2[64]={0};
    char resp_num[64];
    char *tempnum = NULL;
    char gsmNum_real[64] = {0};
    int index = 0;

    sprintf(at1, "AT$QCPBMPREF=1");
    send_AT_Command(&at1,NULL);

    sprintf(at2, "AT+CNUM");
    send_AT_Command(&at2,&resp_num);

    tempnum = strtok(resp_num, ",");
    while(tempnum)
    {
     if (index = 1) {
         sscanf(tempnum, "\"%[^\"]\"", gsmNum_real);
     }
     index++;
     tempnum = strtok(NULL,"\n");
    }
    LOG_ARG("at get phone =%s \n",gsmNum_real);
    if(strncasecmp(gsmNum_real,"+1", 2) == 0){
     snprintf(pNum, 64, "%s", gsmNum_real + 2);
    }else if(strncasecmp(gsmNum_real,"+86", 3) == 0){
     snprintf(pNum, 64, "%s", gsmNum_real + 3);
    }else{
     snprintf(pNum, 64, "%s", gsmNum_real + 1);
    }
    LOG_ARG("at get pNum =%s \n",pNum);
    // strcpy(pNum,phoneNumber);
    // return true;
}

int dsatutil_atoi
(
  unsigned int *val_arg_ptr,      /*  value returned  */
  const byte *s,                        /*  points to string to eval  */
  unsigned int r                        /*  radix */
)
{
  int err_ret = 0;
  byte c;
  unsigned int val, val_lim, dig_lim;

  val = 0;
  val_lim = (unsigned int) ((unsigned int)MAX_VAL_NUM_ITEM / r);
  dig_lim = (unsigned int) ((unsigned int)MAX_VAL_NUM_ITEM % r);

  while ( (c = *s++) != '\0')
  {
    if (c != ' ')
    {
      c = (byte) UPCASE (c);
      if (c >= '0' && c <= '9')
      {
        c -= '0';
      }
      else if (c >= 'A')
      {
        c -= 'A' - 10;
      }
      else
      {
        err_ret = -1;  /*  char code too small */
        break;
      }

      if (c >= r || val > val_lim
          || (val == val_lim && c > dig_lim))
      {
        err_ret = -2;  /*  char code too large */
        break;
      }
      else
      {
        err_ret = 1;            /*  arg found: OK so far*/
        val = (unsigned int) (val * r + c);
      }
    }
    *val_arg_ptr =  val;
  }
  
  return err_ret;

} /*  dsatutil_atoi */

void lwm2m_client_get_att_psk(byte * imei_str, char * psk_str){

	char imei[64];

	memset(imei, 0, 64);

	memcpy(imei, imei_str, 64);
	memcpy(&imei[64-7], "Quectel", 7);

	StrSHA256((const char *)imei, 64, psk_str);

	char i;
	char str[9] = {0};
	dsat_num_item_type ret_hex[8];
	for(i=0; i<8; i++)
	{
		memcpy(str, &psk_str[8*i], 8);
		dsatutil_atoi(&ret_hex[i], (const byte *)str, 16);
	}

	sprintf(psk_str, "%08x%08x%08x%08x", ret_hex[0]+ret_hex[4], ret_hex[1]+ret_hex[5], ret_hex[2]+ret_hex[6], ret_hex[3]+ret_hex[7]);

//    strcpy(psk_str,"ea1eed6ea7fd1ca2eefbfbd63ce72644");
    }

 LOCAL void lwm2m_init_server_info(cJSON *pserver_list,int id, char *url, boolean bootstrap)
 {
	 cJSON  *proot_obj = NULL;
	 cJSON  *pobj_list = NULL;
	 cJSON  *psubsub_obj = NULL;
	 cJSON  *psubsubsub_list = NULL;
	 cJSON  *psubsubsubsub_obj = NULL;
	 int    server_id = 0;
	 char  imei[64] = {0};
	 lwm2m_client_net_type net_type = dsat_lwm2m_get_lwm2m_net();

	 if(!(id == Bootstrap_Server_ID || id == DM_Server_ID || id == Diagnostics_Server_ID || id == Repository_Server_ID))
		 return;

	 if(id == Bootstrap_Server_ID)
		 server_id = 100;
	 if(id == DM_Server_ID)
		 server_id = 102;
	 if(id == Diagnostics_Server_ID)
		 server_id = 101;
	 if(id == Repository_Server_ID)
		 server_id = 1000;
	 proot_obj = cJSON_CreateObject();
	
	 cJSON_AddItemToArray(pserver_list, proot_obj);

	 cJSON_AddNumberToObject(proot_obj,"id",id);
	
	 pobj_list = cJSON_CreateArray();
		
	 cJSON_AddItemToObject(proot_obj,"e", pobj_list);	

	 psubsub_obj = cJSON_CreateObject();
	 cJSON_AddNumberToObject(psubsub_obj,"server_id",server_id);
	 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	
	 if(url != NULL)
	 {
		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddStringToObject(psubsub_obj,"url",url);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	 }
     LOG_ARG("lwm2m_init_server_info url =%s\r\n",url);
	 psubsub_obj = cJSON_CreateObject();

	 if(bootstrap == TRUE)
		 cJSON_AddTrueToObject(psubsub_obj,"bootstrap");
	 else 
		 cJSON_AddFalseToObject(psubsub_obj,"bootstrap");

	 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	
	
	 psubsub_obj = cJSON_CreateObject();
	 cJSON_AddItemToArray(pobj_list, psubsub_obj);
	 psubsubsub_list = cJSON_CreateArray();
	 cJSON_AddItemToObject(psubsub_obj, "security",psubsubsub_list);	

	 psubsubsubsub_obj = cJSON_CreateObject();
	 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);

     /* joe read imei*/
     //imei = GetDeviceImei();
     QL_LWM2M_GetImei(&imei);
     LOG_ARG("QL_LWM2M_GetImei imei =%s\r\n",imei);
     
	 switch(net_type)
	 {
		 case LWM2M_CLIENT_NET_VZW:
             /*
			 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",0);
			 psubsubsubsub_obj = cJSON_CreateObject();
			 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
			 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_id","urn:IMEI-MSISDN:862061049670002-6263401757");
			 psubsubsubsub_obj = cJSON_CreateObject();
			 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
			 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_key","d6160c2e7c90399ee7d207a22611e3d3a87241b0462976b935341d000a91e747");
			 */

             //add motive server bootstarp
			 {
				 char psk_id[128] = {0};				
				 char msisdn[64] = {0};
			
				 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",0);
				 psubsubsubsub_obj = cJSON_CreateObject();
				 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
				 GetDevicePhoneNumber(msisdn);
                 LOG_ARG("GetDevicePhoneNumber msisdn =%s\r\n",msisdn);
                 snprintf(psk_id, 128, "urn:imei-msisdn:%s-%s", imei, msisdn);
				 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_id",psk_id);
				
				 psubsubsubsub_obj = cJSON_CreateObject();
				 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
				 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_key","d6160c2e7c90399ee7d207a22611e3d3a87241b0462976b935341d000a91e747");
			 }

             /*
        	 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",0);

        	 psubsubsubsub_obj = cJSON_CreateObject();
        	 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
        	 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_id","urn:IMEI-MSISDN:862061049670002-6263401757");
        	 psubsubsubsub_obj = cJSON_CreateObject();
        	 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
        	 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_key","d6160c2e7c90399ee7d207a22611e3d3a87241b0462976b935341d000a91e747");
        	 */
			 break;
		 case LWM2M_CLIENT_NET_ATT:
		 {
             char name[64];
             char psk_id[64];
             char psk_str[128];
             memset(name, 0, 64);
             memset(psk_id, 0, 64);
             memset(psk_str, 0, 128);
             snprintf(name, 64, "urn:imei:%s", imei);
			 snprintf(psk_id, 64, "%s", imei);

			 lwm2m_client_get_att_psk(imei, psk_str);//joe modify //TBD
             LOG_ARG("lwm2m_client_get_att_psk psk_str =%s\r\n",psk_str);

			 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",0);
			 psubsubsubsub_obj = cJSON_CreateObject();
			 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
			 cJSON_AddStringToObject(psubsubsubsub_obj, "name", name);
			 psubsubsubsub_obj = cJSON_CreateObject();
			 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
			 cJSON_AddStringToObject(psubsubsubsub_obj, "psk_id", psk_id);
			 psubsubsubsub_obj = cJSON_CreateObject();
			 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
			 cJSON_AddStringToObject(psubsubsubsub_obj, "psk_key", psk_str);//TEMP
		 }
			 break;
		 default:
			 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",3);
			 break;
	 }
	
	 if(bootstrap == FALSE)
	 {
		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"life_time",20);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"period_min",1);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"period_max",60);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"disable_timeout",86400);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddTrueToObject(psubsub_obj,"storing");			
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddStringToObject(psubsub_obj,"binding","UQS");
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	 }
 }

LOCAL void lwm2m_init_hostdevice_info(cJSON *id_list,int id)
{
	cJSON  *proot_obj = NULL;
	char   temp_buf[128] = {0};

	proot_obj = cJSON_CreateObject();
		
	cJSON_AddItemToArray(id_list, proot_obj);

	//unique_id
	snprintf(temp_buf,128,"%s%d","HUID",id);
	cJSON_AddStringToObject(proot_obj,"unique_id", temp_buf);
	//manufacture
	memset(temp_buf,0,128);
	snprintf(temp_buf,128,"%s%d","HMAN",id);
	cJSON_AddStringToObject(proot_obj,"manufacture", temp_buf);	
	//model
	memset(temp_buf,0,128);
	snprintf(temp_buf,128, "%s%d","HMOD",id);
	cJSON_AddStringToObject(proot_obj,"model", temp_buf);	
	//sw version
	memset(temp_buf,0,128);
	snprintf(temp_buf,128,"%s%d","HSW",id);
	cJSON_AddStringToObject(proot_obj,"sw_version", temp_buf);	
	//fw version
	memset(temp_buf,0,128);
	snprintf(temp_buf,128,"%s%d","HFW",id);
	cJSON_AddStringToObject(proot_obj,"fw_version", temp_buf);	
	//hw version
	memset(temp_buf,0,128);
	snprintf(temp_buf,128,"%s%d","HHW",id);
	cJSON_AddStringToObject(proot_obj,"hw_version", temp_buf);	
	//upgrade time
	cJSON_AddNumberToObject(proot_obj,"upgrade_time", 1);	
}

 LOCAL cJSON  *lwm2m_save_server_info(int server_idx,lwm2m_server_info_t *server_info_ptr)
 {
	 cJSON  *proot_obj = NULL;
	 cJSON  *pobj_list = NULL;
	 cJSON  *psubsub_obj = NULL;
	 cJSON  *psubsubsub_list = NULL;
	 cJSON  *psubsubsubsub_obj = NULL;
	 char   *json_buf;

	 if(!(server_idx == Bootstrap_Server_ID || server_idx == DM_Server_ID || server_idx == Diagnostics_Server_ID || server_idx == Repository_Server_ID))
		 return NULL;
	 proot_obj = cJSON_CreateObject();

	 cJSON_AddNumberToObject(proot_obj,"id",server_idx);
	
	 pobj_list = cJSON_CreateArray();
		
	 cJSON_AddItemToObject(proot_obj,"e", pobj_list);	

	 psubsub_obj = cJSON_CreateObject();
	 cJSON_AddNumberToObject(psubsub_obj,"server_id",server_info_ptr->server_id);
	 cJSON_AddItemToArray(pobj_list, psubsub_obj);	

	 if(server_info_ptr->server_url[0] != '\0')
	 {
		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddStringToObject(psubsub_obj,"url",server_info_ptr->server_url);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	 }
	
	 psubsub_obj = cJSON_CreateObject();

	 if(server_info_ptr->bootstrap == TRUE)
		 cJSON_AddTrueToObject(psubsub_obj,"bootstrap");
	 else 
		 cJSON_AddFalseToObject(psubsub_obj,"bootstrap");

	 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	
	
	 psubsub_obj = cJSON_CreateObject();
	 cJSON_AddItemToArray(pobj_list, psubsub_obj);
	 psubsubsub_list = cJSON_CreateArray();
	 cJSON_AddItemToObject(psubsub_obj, "security",psubsubsub_list);	

	 psubsubsubsub_obj = cJSON_CreateObject();
	 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
	 cJSON_AddNumberToObject(psubsubsubsub_obj,"mode",server_info_ptr->security_mode);

	 if(server_info_ptr->security_mode == 0)//psk
	 {
		 psubsubsubsub_obj = cJSON_CreateObject();
		 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
		 cJSON_AddStringToObject(psubsubsubsub_obj,"name",server_info_ptr->name);

		 psubsubsubsub_obj = cJSON_CreateObject();
		 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
		 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_id",server_info_ptr->psk_id);

		 psubsubsubsub_obj = cJSON_CreateObject();
		 cJSON_AddItemToArray(psubsubsub_list, psubsubsubsub_obj);
		 cJSON_AddStringToObject(psubsubsubsub_obj,"psk_key",server_info_ptr->psk_key);
	 }
	 if(server_info_ptr->bootstrap == FALSE)
	 {
		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"life_time",server_info_ptr->life_time);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"period_min",server_info_ptr->period_min);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"period_max",server_info_ptr->period_max);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 cJSON_AddNumberToObject(psubsub_obj,"disable_timeout",server_info_ptr->disable_timeout);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 if(server_info_ptr->storing == TRUE)
			 cJSON_AddTrueToObject(psubsub_obj,"storing");			
		 else 
			 cJSON_AddFalseToObject(psubsub_obj,"storing");
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);

		 psubsub_obj = cJSON_CreateObject();
		 if(server_info_ptr->binding[0] == '\0')
			 cJSON_AddStringToObject(psubsub_obj,"binding","UQS");
		 else
			 cJSON_AddStringToObject(psubsub_obj,"binding",server_info_ptr->binding);
		 cJSON_AddItemToArray(pobj_list, psubsub_obj);	
	 }

	 return proot_obj;
 }


 void quec_lwm2m_load_verizon_bs_server_url(char* url){
     char *filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
     FILE *fp;
     char *verizon_bs_server_url=NULL;
     
    if (access(filename , F_OK) == 0)
    {
        LOG_ARG("Verizon bs AT CMD server URL=%s exist !\n",QUEC_LWM2M_BS_SERVER_VZW_INI);
        //read default test BS server set by AT CMD
        fp = fopen(filename,"r");
        if(fp == NULL)
        {
            LOG_ARG("open file %s error: \n",filename);
            perror("fopen: ");
            return ;
        }

        verizon_bs_server_url = lwm2m_malloc(sizeof(char)*256);
        memset(verizon_bs_server_url, 0, 256);
        fread(verizon_bs_server_url, sizeof(char)*256, 1, fp);
        LOG_ARG("load_get verizon_bs_server_url=%s",verizon_bs_server_url);
        if(verizon_bs_server_url != NULL){
           memcpy(url,verizon_bs_server_url,strlen(verizon_bs_server_url));
        }
        fclose(fp);
        lwm2m_free(verizon_bs_server_url);

    }else{
        LOG("set Verizon bs to commercial server: coaps://boot.lwm2m.vzwdm.com:5684!\n");
        strcpy(url,"coaps://boot.lwm2m.vzwdm.com:5684");
        //set default commercial BS server
    }
    security_url_store(url,100);

 }

//add by joe
 void quec_lwm2m_load_att_bs_server_url(char* url){
     char *filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
     FILE *fp;
     char *verizon_bs_server_url=NULL;
     
    if (access(filename , F_OK) == 0)
    {
        LOG_ARG("ATT bs AT CMD server URL=%s exist !\n",QUEC_LWM2M_BS_SERVER_VZW_INI);
        //read default test BS server set by AT CMD
        fp = fopen(filename,"r");
        if(fp == NULL)
        {
            LOG_ARG("open file %s error: \n",filename);
            perror("fopen: ");
            return ;
        }

        verizon_bs_server_url = lwm2m_malloc(sizeof(char)*256);
        memset(verizon_bs_server_url, 0, 256);
        fread(verizon_bs_server_url, sizeof(char)*256, 1, fp);
        LOG_ARG("load_get Att_bs_server_url=%s",verizon_bs_server_url);
        if(verizon_bs_server_url != NULL){
           memcpy(url,verizon_bs_server_url,strlen(verizon_bs_server_url));
        }
        fclose(fp);
        lwm2m_free(verizon_bs_server_url);

    }else{
        LOG("set att bs to commercial server: coaps://bootstrap.dm.iot.att.com:5694!\n");
        strcpy(url,"coaps://bootstrap.dm.iot.att.com:5694");
        //set default commercial BS server
    }

 }

void lwm2m_init_lwm2m_data_folder(){
     struct stat stat_buf1;
     struct stat stat_buf2;
     if (!(0 == lstat( "/data/lwm2m/", &stat_buf1) && (S_ISDIR (stat_buf1.st_mode))))
     {
         mkdir("/data/lwm2m/", ALLPERMS);
     }

     if (!(0 == lstat( "/persist/lwm2m/", &stat_buf2) && (S_ISDIR (stat_buf2.st_mode))))
     {
         mkdir("/persist/lwm2m/", ALLPERMS);
     }
}

LOCAL int lwm2m_initialize_configuration()
{
	struct stat stat_buf;
	int 	fd;
	char   *json_buf = NULL;
	cJSON  *server_obj = NULL;
	cJSON  *pobj_list;
	cJSON  *dev_list;
	bool needinit = false;
    /*
	if (!(0 == lstat( "/data/lwm2m/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
    {
		mkdir("/data/lwm2m/", ALLPERMS);
    }*/
    LOG("lwm2m_initialize_configuration start \r\n");
	 if(0 == lstat("/data/lwm2m/lwm2m_server.ini",&stat_buf))
	 {
		 fd = open("/data/lwm2m/lwm2m_server.ini", O_RDONLY);

		 if(fd >= 0)
		 {

			 int  json_buf_sz = 0;

			 json_buf = malloc(1024);
			 json_buf_sz = read(fd,json_buf, 1024);

			 close(fd);

			 pServer_Obj = cJSON_Parse(json_buf);
			 free(json_buf);
		 }
	 }
	if(pServer_Obj != NULL)
        server_obj = cJSON_GetObjectItem(pServer_Obj,"info");
	 if(server_obj == NULL){
		 LOG("server_obj == NULL\r\n");
		 needinit = true;
	 }else if(0 == cJSON_GetArraySize(server_obj)){
		LOG("ArraySize == 0 \r\n");
		needinit = true;
	 }

	 if(pServer_Obj == NULL || needinit){
         LOG(" pServer_Obj == NULL or needinit \r\n");
		 pServer_Obj = cJSON_CreateObject();

		 pobj_list = cJSON_CreateArray();

		 cJSON_AddItemToObject(pServer_Obj,"info", pobj_list);	
         
		lwm2m_client_net_type net_type = dsat_lwm2m_get_lwm2m_net();
        if(net_type == LWM2M_CLIENT_NET_VZW){
            LOG("lwm2m_init_server_info for vzw \r\n");
            char vzw_bs_server[256] = {0};
            quec_lwm2m_load_verizon_bs_server_url(vzw_bs_server);
            lwm2m_init_server_info(pobj_list,Bootstrap_Server_ID,vzw_bs_server, TRUE);
            //lwm2m_init_server_info(pobj_list, Bootstrap_Server_ID, "coaps://boot.lwm2m.vzwdm.com:5684", TRUE);
        }else if(net_type == LWM2M_CLIENT_NET_ATT){ 
            LOG("lwm2m_init_server_info for att \r\n");
            char att_bs_server[256] = {0};
            quec_lwm2m_load_att_bs_server_url(att_bs_server);
            lwm2m_init_server_info(pobj_list, DM_Server_ID, att_bs_server, TRUE);
        }

		 fd = open("/data/lwm2m/lwm2m_server.ini", O_CREAT|O_TRUNC|O_RDWR,0777);
		 if(fd < 0)
			 return 0;

		 json_buf = cJSON_Print(pServer_Obj);
		 write(fd, json_buf, strlen(json_buf));	
		 free(json_buf);
		 close(fd);	
		 needinit = false;
         LOG("lwm2m_init_server_info end\r\n");
	 }

     LOG("lwm2m_initialize_configuration hostdevice start \r\n");

	if(0 == lstat("/data/lwm2m/lwm2m_hostdevice.ini",&stat_buf))
	{
		fd = open("/data/lwm2m/lwm2m_hostdevice.ini", O_RDONLY);

		if(fd >= 0)
		{

			int  json_buf_sz = 0;

			json_buf = malloc(1024);
			json_buf_sz = read(fd,json_buf, 1024);

			close(fd);

			pHostDev_info_Obj = cJSON_Parse(json_buf);
			free(json_buf);
		}
	}
	if(pHostDev_info_Obj == NULL){	
		pHostDev_info_Obj = cJSON_CreateObject();
		dev_list = cJSON_CreateArray();

		cJSON_AddItemToObject(pHostDev_info_Obj,"id", dev_list);	

		lwm2m_init_hostdevice_info(dev_list, 0);
		lwm2m_init_hostdevice_info(dev_list, 1);

		json_buf = cJSON_Print(pHostDev_info_Obj);

		fd = open("/data/lwm2m/lwm2m_hostdevice.ini", O_CREAT|O_TRUNC|O_RDWR,0777);

		if(fd < 0)
			return 0;

		write(fd, json_buf, strlen(json_buf));
		free(json_buf);
		close(fd);
	}
	return 1;
}

LOCAL void lwm2m_load_server_info(cJSON *server_obj, lwm2m_server_info_t *server_info_ptr)
{
	cJSON *pitem_list = NULL;
	cJSON *pitem_obj = NULL;
	cJSON *psubsub_obj = NULL;
	int  item_id = 0;
	int  item_cnt = 0;	

	pitem_list = cJSON_GetObjectItem(server_obj,"e");
	if(pitem_list == NULL)
		return;
	item_cnt = cJSON_GetArraySize(pitem_list);
	
	server_info_ptr->valid = TRUE;
	
	while(item_id < item_cnt)
	{
		pitem_obj = cJSON_GetArrayItem(pitem_list, item_id);
		if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"server_id")) != NULL)
		{
			server_info_ptr->server_id = psubsub_obj->valueint;
		}
		if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"url")) != NULL)
		{
			memset(server_info_ptr->server_url, 0,256);
			strcpy(server_info_ptr->server_url, psubsub_obj->valuestring);
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"bootstrap")) != NULL)
		{
			if(psubsub_obj->type == cJSON_True)
				server_info_ptr->bootstrap = TRUE;
			else 
				server_info_ptr->bootstrap = FALSE;
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"security")) != NULL)
		{
			int id = 0;
			int cnt = cJSON_GetArraySize(psubsub_obj);
			cJSON *sec_item = NULL;
			cJSON *sec_obj = NULL;
			while(id < cnt && id < 4)
			{
				sec_item = cJSON_GetArrayItem(psubsub_obj, id);
                LOG_ARG("lwm2m_load_server_info sec_item = %d\n", sec_item);
				if((sec_obj = cJSON_GetObjectItem(sec_item,"mode")) != NULL)
				{
					server_info_ptr->security_mode = sec_obj->valueint;
                    LOG_ARG("lwm2m_load_server_info server_info_ptr->security_mode = %d\n", server_info_ptr->security_mode);
				}
				else if((sec_obj = cJSON_GetObjectItem(sec_item,"name")) != NULL)
				{
					memset(server_info_ptr->name, 0, 64);
					strcpy(server_info_ptr->name, sec_obj->valuestring);
                    LOG_ARG("lwm2m_load_server_info server_info_ptr->name = %s\n", server_info_ptr->name);
				}
				else if((sec_obj = cJSON_GetObjectItem(sec_item,"psk_id")) != NULL)
				{
					memset(server_info_ptr->psk_id, 0, 64);
					strcpy(server_info_ptr->psk_id, sec_obj->valuestring);
                    LOG_ARG("lwm2m_load_server_info server_info_ptr->psk_id = %s\n", server_info_ptr->psk_id);
				}
				else if((sec_obj = cJSON_GetObjectItem(sec_item,"psk_key")) != NULL)
				{
					memset(server_info_ptr->psk_key, 0, 128);
					strcpy(server_info_ptr->psk_key, sec_obj->valuestring);
                    LOG_ARG("lwm2m_load_server_info server_info_ptr->psk_key = %s\n", server_info_ptr->psk_key);
				}
				id++;
			}
			
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"life_time")) != NULL)
		{
			server_info_ptr->life_time = psubsub_obj->valueint;	
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"period_min")) != NULL)
		{
			server_info_ptr->period_min = psubsub_obj->valueint;		
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"period_max")) != NULL)
		{
			server_info_ptr->period_max = psubsub_obj->valueint;					
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"disable_timeout")) != NULL)
		{
			server_info_ptr->disable_timeout = psubsub_obj->valueint;					
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"storing")) != NULL)
		{
			if(psubsub_obj->type == cJSON_True)
				server_info_ptr->storing = TRUE;
			else 
				server_info_ptr->storing = FALSE;	
		}
		else if((psubsub_obj = cJSON_GetObjectItem(pitem_obj,"binding")) != NULL)
		{
			memset(server_info_ptr->binding, 0,4);
			strcpy(server_info_ptr->binding, psubsub_obj->valuestring);	
		}
		item_id++;
	}	
}

LOCAL int lwm2m_get_server_info_item(int server_idx)
{
	int item_id = 0;	
	cJSON  *server_list_obj = NULL;
	int item_cnt = 0;	
	cJSON *item_obj = NULL;
	cJSON *server_id_obj = NULL;	
	server_list_obj = cJSON_GetObjectItem(pServer_Obj,"info");

	item_cnt = cJSON_GetArraySize(server_list_obj);

	while(item_id < item_cnt)
	{
		item_obj =  cJSON_GetArrayItem(server_list_obj, item_id);
		server_id_obj = cJSON_GetObjectItem(item_obj, "id");
		if(server_id_obj->valueint == server_idx)
		{
			return item_id;
		}
		item_id++;
	}

	return -1;
}
void lwm2m_load_server_configuration()
{
	int item_id = 0;
	int item_cnt = 0;
	cJSON  *server_list_obj = NULL;
	cJSON *server_obj = NULL;
	cJSON *psub_obj = NULL;
	int  server_idx = -1;
	
	memset(&server_list[0], 0, sizeof(lwm2m_server_info_t));	
	memset(&server_list[1], 0, sizeof(lwm2m_server_info_t));
	memset(&server_list[2], 0, sizeof(lwm2m_server_info_t));
	memset(&server_list[3], 0, sizeof(lwm2m_server_info_t));

	if(lwm2m_initialize_configuration() == 0){
        LOG("lwm2m_initialize_configuration ==0 \r\n");
		return;
    }
	server_list_obj = cJSON_GetObjectItem(pServer_Obj,"info");
	if(server_list_obj == NULL){
        LOG("lwm2m_load_server_configuration server_list_obj == NULL\r\n");
		return;
    }
	item_cnt = cJSON_GetArraySize(server_list_obj);
    LOG_ARG("lwm2m_load_server_configuration item_cnt =%d\r\n",item_cnt);
	while(item_id < item_cnt)
	{
		server_obj = cJSON_GetArrayItem(server_list_obj, item_id);
		psub_obj = cJSON_GetObjectItem(server_obj, "id");
		
		server_idx = psub_obj->valueint;
	    if(!(server_idx == Bootstrap_Server_ID 
	   	    || server_idx == DM_Server_ID 
	   	    || server_idx == Diagnostics_Server_ID 
	   	    || server_idx == Repository_Server_ID))
	   	{
			item_id++;
			continue;
	   	}
        LOG_ARG("item_id = %d server_idx=%d\n", item_id,server_idx);
		lwm2m_load_server_info(server_obj, &server_list[server_idx]);
		item_id++;
	}
}

lwm2m_server_info_t *lwm2m_get_server_info(int server_idx)
{
   if(!(server_idx == Bootstrap_Server_ID || server_idx == DM_Server_ID || server_idx == Diagnostics_Server_ID || server_idx == Repository_Server_ID))
		return NULL;
   
	if(server_list[server_idx].valid == TRUE)
		return &server_list[server_idx];
	
	return NULL;
}


 bool lwm2m_update_server_info(int server_idx,lwm2m_server_info_t  *server_info_ptr)
 {
	cJSON *old_item = NULL;
	lwm2m_server_info_t *old_server_info = NULL;
	int  item_id = 0;
	cJSON *new_item = NULL;
	cJSON  *server_list_obj = NULL;
	int 	fd;
	char 	f_name[64] = {0};
	char   *json_buf = NULL;
	lwm2m_client_net_type net_type = dsat_lwm2m_get_lwm2m_net();

	if(!(server_idx == Bootstrap_Server_ID || server_idx == DM_Server_ID || server_idx == Diagnostics_Server_ID || server_idx == Repository_Server_ID))
		return NULL;
	old_server_info = lwm2m_get_server_info(server_idx);
	
	server_list_obj = cJSON_GetObjectItem(pServer_Obj,"info");

	if(old_server_info !=NULL)
	{
		new_item = lwm2m_save_server_info(server_idx,server_info_ptr);
		item_id = lwm2m_get_server_info_item(server_idx);
		cJSON_ReplaceItemInArray(server_list_obj, item_id,new_item);
	}
	else
	{
		new_item = lwm2m_save_server_info(server_idx,server_info_ptr);
		cJSON_AddItemToArray(server_list_obj, new_item);	
	}
	memset(&server_list[server_idx], 0, sizeof(lwm2m_server_info_t));
	memcpy(&server_list[server_idx],server_info_ptr, sizeof(lwm2m_server_info_t));
	server_list[server_idx].valid = TRUE;

	if(server_info_ptr->security_mode == 0)
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk key=%s", server_info_ptr->psk_key);
	
	json_buf = cJSON_Print(pServer_Obj);
	
	switch(net_type)
	{
		case LWM2M_CLIENT_NET_VZW:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_VZW_INI);
			break;
		case LWM2M_CLIENT_NET_ATT:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_ATT_INI);
			break;
		default:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_DEF_INI);
			break;
	}
	fd = open(f_name, O_CREAT|O_TRUNC|O_RDWR,0777);
	
	if(fd < 0)
	{
		free(json_buf);
		return FALSE;
	}
	write(fd, json_buf, strlen(json_buf));
	close(fd);	
	free(json_buf);

	return TRUE;
}

 void lwm2m_delete_server_info(int server_idx)
 {
	int  item_id = 0;
	cJSON  *server_list_obj = NULL;
	int 	fd;
	char 	f_name[64] = {0};
	char   *json_buf = NULL;
	lwm2m_client_net_type net_type = dsat_lwm2m_get_lwm2m_net();
	lwm2m_server_info_t *server_info = lwm2m_get_server_info(server_idx);

	if(server_info != NULL && server_info->valid == TRUE)
	{
		item_id = lwm2m_get_server_info_item(server_idx);

		server_info = lwm2m_get_server_info(server_idx);
		server_info->valid = FALSE;
		server_list_obj = cJSON_GetObjectItem(pServer_Obj,"info");

		cJSON_DeleteItemFromArray(server_list_obj, item_id);

		json_buf = cJSON_Print(pServer_Obj);
		
			
		switch(net_type)
		{
			case LWM2M_CLIENT_NET_VZW:
				strcpy(f_name, QUEC_EFS_LWM2M_SERVER_VZW_INI);
				break;
			case LWM2M_CLIENT_NET_ATT:
				strcpy(f_name, QUEC_EFS_LWM2M_SERVER_ATT_INI);
				break;
			default:
				strcpy(f_name, QUEC_EFS_LWM2M_SERVER_DEF_INI);
				break;
		}
		fd = open(f_name, O_CREAT|O_TRUNC|O_RDWR,0777);
		
		if(fd < 0)
		{
			free(json_buf);
			return ;
		}
		write(fd, json_buf, strlen(json_buf));
		close(fd);	
		free(json_buf);
	}
}


 void lwm2m_reload_server_config(lwm2m_client_net_type net_type)
 {
	char f_name[64] = {0};
	struct stat stat_buf;
	int 	fd;
	char   *json_buf = NULL;

	cJSON  *pobj_list;
	
	switch(net_type)
	{
		case LWM2M_CLIENT_NET_VZW:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_VZW_INI);
			break;
		case LWM2M_CLIENT_NET_ATT:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_ATT_INI);
			break;
		default:
			strcpy(f_name, QUEC_EFS_LWM2M_SERVER_DEF_INI);
			break;
	}
	
	memset(&server_list[0], 0, sizeof(lwm2m_server_info_t));	
	memset(&server_list[1], 0, sizeof(lwm2m_server_info_t));
	memset(&server_list[2], 0, sizeof(lwm2m_server_info_t));
	memset(&server_list[3], 0, sizeof(lwm2m_server_info_t));
	cJSON_Delete(pServer_Obj);
	pServer_Obj = NULL;
	
	if (!(0 == lstat( "/data/lwm2m/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
    {
		mkdir("/data/lwm2m/", ALLPERMS);
    }
	
	if(0 == lstat(f_name, &stat_buf))
	{
		fd = open(f_name, O_RDONLY);

		if(fd >= 0)
		{
			
			int  json_buf_sz = 0;

			json_buf = malloc(1024);
			json_buf_sz = read(fd,json_buf, 1024);

			close(fd);
			
			pServer_Obj = cJSON_Parse(json_buf);
			free(json_buf);
		}
	}
	
	if(pServer_Obj == NULL)
	{
		pServer_Obj = cJSON_CreateObject();

		pobj_list = cJSON_CreateArray();
		
		cJSON_AddItemToObject(pServer_Obj,"info", pobj_list);	

		switch(net_type)
		{
			case LWM2M_CLIENT_NET_VZW:
				lwm2m_init_server_info(pobj_list,Bootstrap_Server_ID,"coaps://xvzwcdpii.xdev.motive.com:5684", TRUE);
				break;
			case LWM2M_CLIENT_NET_ATT:
				lwm2m_init_server_info(pobj_list,Bootstrap_Server_ID,"coaps://bootstrap.dm.iot.att.com:5694", TRUE);
				break;
			default:
				lwm2m_init_server_info(pobj_list,Bootstrap_Server_ID,"coaps://xvzwcdpii.xdev.motive.com:5684", TRUE);
				break;
		}
		
		fd = open(f_name, O_CREAT|O_TRUNC|O_RDWR,0777);
		if(fd < 0)
			return;
		
		json_buf = cJSON_Print(pServer_Obj);
		write(fd, json_buf, strlen(json_buf));	
		free(json_buf);
		close(fd);	
	}

	
	int item_id = 0;
	int item_cnt = 0;
	cJSON  *server_list_obj = NULL;
	cJSON *server_obj = NULL;
	cJSON *psub_obj = NULL;
	int  server_idx = -1;
	
	server_list_obj = cJSON_GetObjectItem(pServer_Obj,"info");
	if(server_list_obj == NULL)
		return;
	item_cnt = cJSON_GetArraySize(server_list_obj);
	
	while(item_id < item_cnt)
	{
		server_obj = cJSON_GetArrayItem(server_list_obj, item_id);
		psub_obj = cJSON_GetObjectItem(server_obj, "id");
		
		server_idx = psub_obj->valueint;
	    if(!(server_idx == Bootstrap_Server_ID 
	   	    || server_idx == DM_Server_ID 
	   	    || server_idx == Diagnostics_Server_ID 
	   	    || server_idx == Repository_Server_ID))
	   	{
			item_id++;
			continue;
	   	}
		lwm2m_load_server_info(server_obj, &server_list[server_idx]);
		item_id++;
	}
}



void lwm2m_save_apn_conn_info(int id, lwm2m_apn_conn_info_t *apn_conn_info)
{
	cJSON *pApn_Conn_Obj = NULL;
    cJSON *apn_list_Obj = NULL;
	int 	fd;
	char   *json_buf = NULL;
	if(id > 1 || id < 0 || apn_conn_info == NULL){
		return ;
	}
	pApn_Conn_Obj = cJSON_CreateObject();
	
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  pApn_Conn_Obj=%p", pApn_Conn_Obj);
	
	if(pApn_Conn_Obj == NULL){
		return;	
	}

	if(apn_conn_info->mask & 0x0001){
		
		if(cJSON_GetObjectItem(pApn_Conn_Obj, "apn") != NULL){
			cJSON_DeleteItemFromObject(pApn_Conn_Obj,"apn");
		}

		cJSON_AddStringToObject(pApn_Conn_Obj,"apn",apn_conn_info->apn_name);			
	}

	if(apn_conn_info->mask & 0x0002){
			
		if(cJSON_GetObjectItem(pApn_Conn_Obj, "rab_estab_time") != NULL){
			cJSON_DeleteItemFromObject(pApn_Conn_Obj, "rab_estab_time");
		}
		cJSON_AddNumberToObject(pApn_Conn_Obj,"rab_estab_time",apn_conn_info->estab_time);
	}

	if(apn_conn_info->mask & 0x0004){
		
		if(cJSON_GetObjectItem(pApn_Conn_Obj, "rab_estab_result") != NULL){
			cJSON_DeleteItemFromObject(pApn_Conn_Obj, "rab_estab_result");
		}

		cJSON_AddNumberToObject(pApn_Conn_Obj,"rab_estab_result",apn_conn_info->estab_result);
	}

	if(apn_conn_info->mask & 0x0008){

		if(cJSON_GetObjectItem(pApn_Conn_Obj, "rab_estab_rej_cause") != NULL){
			cJSON_DeleteItemFromObject(pApn_Conn_Obj, "rab_estab_rej_cause");
		}

		cJSON_AddNumberToObject(pApn_Conn_Obj,"rab_estab_rej_cause",apn_conn_info->estab_rej_cause);
	}

	if(apn_conn_info->mask & 0x0010){
		if(cJSON_GetObjectItem(pApn_Conn_Obj, "rab_rel_time") != NULL){
			cJSON_DeleteItemFromObject(pApn_Conn_Obj, "rab_rel_time");
		}

		cJSON_AddNumberToObject(pApn_Conn_Obj,"rab_rel_time",apn_conn_info->release_time);
	}

	json_buf = cJSON_Print(pApn_Conn_Obj);
	
	if(json_buf != NULL){
		fd = open("/data/lwm2m/lwm2m_apn_conn_info.ini", O_CREAT|O_TRUNC|O_RDWR,0777);
		
		if(fd > 0)
		{
			write(fd, json_buf, strlen(json_buf));
			
			close(fd);
		}
		free(json_buf);
	}
}

bool lwm2m_load_apn_conn_info(int id, lwm2m_apn_conn_info_t *apn_conn_info)
{
	int 	fd;
	char   *json_buf = NULL;
	int     json_buf_sz = 0;
	cJSON  *pApn_Conn_Obj = NULL;
	cJSON  *apn_name = NULL;
	cJSON  *estab_time = NULL;
	cJSON  *estab_result = NULL;
	cJSON  *estab_rej_cause = NULL;
	cJSON  *release_time = NULL;

	
	fd = open("/data/lwm2m/lwm2m_apn_conn_info.ini", O_RDONLY);

	if(fd < 0)
		return FALSE;

	json_buf = malloc(512);
	json_buf_sz = read(fd,json_buf, 512);
		
	close(fd);
					
	pApn_Conn_Obj = cJSON_Parse(json_buf);
	free(json_buf);

	if(pApn_Conn_Obj == NULL)
		return FALSE;

	memset(apn_conn_info, 0, sizeof(lwm2m_apn_conn_info_t));
	
	if((apn_name = cJSON_GetObjectItem(pApn_Conn_Obj,"apn")) != NULL){
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  type=%d", apn_name->type);
		memset(apn_conn_info->apn_name, 0, 256);
		strlcpy(apn_conn_info->apn_name, apn_name->valuestring,strlen(apn_name->valuestring));
		apn_conn_info->mask |= 0x0001;
	}

	if((estab_time = cJSON_GetObjectItem(pApn_Conn_Obj,"rab_estab_time")) != NULL){
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  type=%d", estab_time->type);
		apn_conn_info->estab_time = estab_time->valueint;
		apn_conn_info->mask |= 0x0002;
	}

	if((estab_result = cJSON_GetObjectItem(pApn_Conn_Obj,"rab_estab_result")) != NULL){
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  type=%d", estab_result->type);
		apn_conn_info->estab_result = estab_result->valueint;
		apn_conn_info->mask |= 0x0004;
	}

	if((estab_rej_cause = cJSON_GetObjectItem(pApn_Conn_Obj,"rab_estab_rej_cause")) != NULL){
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  type=%d", estab_rej_cause->type);
		apn_conn_info->estab_rej_cause = estab_rej_cause->valueint;
		apn_conn_info->mask |= 0x0008;
	}

	if((release_time = cJSON_GetObjectItem(pApn_Conn_Obj,"rab_rel_time")) != NULL){
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  type=%d", release_time->type);
		apn_conn_info->release_time = release_time->valueint;
		apn_conn_info->mask |= 0x0010;
	}

	return TRUE;

}

void lwm2m_save_hostdevice_info(int id, lwm2m_hostdevice_info_t *dev_info)
{
	cJSON  *proot_obj = NULL;
	cJSON  *info_list_obj = NULL;
	char   *json_buf = NULL;
	int fd;
	if(id > 1 || id < 0 || dev_info == NULL){
		return ;
	}
	if(pHostDev_info_Obj == NULL){
		pHostDev_info_Obj = cJSON_CreateObject();
		info_list_obj = cJSON_CreateArray();
		cJSON_AddItemToObject(pHostDev_info_Obj,"id", info_list_obj);	
	}else{
		info_list_obj = cJSON_GetObjectItem(pHostDev_info_Obj,"id");
	}
	
	proot_obj = cJSON_CreateObject();
		
	//unique_id
	cJSON_AddStringToObject(proot_obj,"unique_id", dev_info->unique_id);
	//manufacture
	cJSON_AddStringToObject(proot_obj,"manufacture", dev_info->manufacture);	
	//model
	cJSON_AddStringToObject(proot_obj,"model", dev_info->model);	
	//sw version
	cJSON_AddStringToObject(proot_obj,"sw_version", dev_info->sw_version);	
	//fw version
	cJSON_AddStringToObject(proot_obj,"fw_version", dev_info->fw_version);	
	//hw version
	cJSON_AddStringToObject(proot_obj,"hw_version", dev_info->hw_version);	
	//upgrade time
	cJSON_AddNumberToObject(proot_obj,"upgrade_time", dev_info->upgrade_time);	

	cJSON_ReplaceItemInArray(info_list_obj,id, proot_obj);

	fd = open("/data/lwm2m/lwm2m_hostdevice.ini", O_CREAT|O_TRUNC|O_RDWR,0777);

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  fd=%d", fd);
	if(fd > 0)
	{
		json_buf = cJSON_Print(pHostDev_info_Obj);
		write(fd, json_buf, strlen(json_buf));
		close(fd);
		free(json_buf);
	}
}

bool lwm2m_load_hostdevice_info(int id, lwm2m_hostdevice_info_t *dev_info)
{
	cJSON  *info_list_obj = NULL;
	cJSON  *info_obj = NULL;
	cJSON  *id_item = NULL;
	cJSON  *manu_item = NULL;
	cJSON  *model_item = NULL;
	cJSON  *sw_item = NULL;
	cJSON  *fw_item = NULL;
	cJSON  *hw_item = NULL;
	cJSON  *upgrade_item = NULL;
	

	if(id > 1 || id < 0 || dev_info == NULL ||pHostDev_info_Obj == NULL){
		return FALSE;
	}

	info_list_obj = cJSON_GetObjectItem(pHostDev_info_Obj,"id");

	info_obj = cJSON_GetArrayItem(info_list_obj, id);

	if(info_obj == NULL)
		return FALSE;
	
	id_item = cJSON_GetObjectItem(info_obj,"unique_id");
	manu_item = cJSON_GetObjectItem(info_obj,"manufacture");
	model_item = cJSON_GetObjectItem(info_obj,"model");
	sw_item = cJSON_GetObjectItem(info_obj,"sw_version");
	fw_item = cJSON_GetObjectItem(info_obj,"fw_version");
	hw_item = cJSON_GetObjectItem(info_obj,"hw_version");
	upgrade_item = cJSON_GetObjectItem(info_obj,"upgrade_time");

	if(id_item != NULL){
		memset(dev_info->unique_id, 0, 128);
		strcpy(dev_info->unique_id, id_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->unique_id=%s", id_item->valuestring);
	}

	if(manu_item != NULL){
		memset(dev_info->manufacture, 0, 128);
		strcpy(dev_info->manufacture, manu_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->manufacture=%s", manu_item->valuestring);
	}

	if(model_item != NULL){
		memset(dev_info->model, 0, 128);
		strcpy(dev_info->model, model_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->model=%s", model_item->valuestring);
	}

	if(sw_item != NULL){
		memset(dev_info->sw_version, 0, 128);
		strcpy(dev_info->sw_version, sw_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->sw_version=%s", sw_item->valuestring);
	}

	if(fw_item != NULL){
		memset(dev_info->fw_version, 0, 128);
		strcpy(dev_info->fw_version, fw_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->fw_version=%s", fw_item->valuestring);
	}

	if(hw_item != NULL){
		memset(dev_info->hw_version, 0, 128);
		strcpy(dev_info->hw_version, hw_item->valuestring);
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->hw_version=%s", hw_item->valuestring);
	}

	if(upgrade_item != NULL){
		dev_info->upgrade_time = upgrade_item->valueint;
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  dev_info->upgrade_time=%d", upgrade_item->valueint);
	}
	return TRUE;
}

// void  lwm2m_update_fota_pkg_dload_info(int dload_pkg_size, uint8 delete)
// {
	// int fd = -1;

	// if(delete == 1){
		// efs_unlink("/data/lwm2m/lwm2m_fota_dload.ctx");
	// }else{
		// fd = efs_open("/data/lwm2m/lwm2m_fota_dload.ctx", O_CREAT|O_TRUNC|O_RDWR);
		// if(fd > 0){
			// char  write_buf[128] = {0};
			// int write_len = 0;
		
			// write_len += snprintf(write_buf, 128, "dload_size:%d",dload_pkg_size);

			// efs_write(fd, write_buf, write_len);

			// efs_close(fd);
		// } 
	// }
// }

// boolean lwm2m_get_fota_pkg_dload_info(int *dload_size)
// {
	// struct fs_stat stat_buf;
	// int fd = -1;
	
	// *dload_size = 0;
	
	// if(0 == efs_lstat("/data/lwm2m/lwm2m_fota_dload.ctx",&stat_buf))
	// {
		// fd = efs_open("/data/lwm2m/lwm2m_fota_dload.ctx", O_RDONLY);
		// if(fd > 0){
			// char  read_buf[128] = {0};
			// char  *read_buf_ptr = read_buf;
			// int read_len = 0;
			
			// efs_read(fd, read_buf, stat_buf.st_size);
			// MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  read_buf_ptr=%s", read_buf_ptr);
			// read_buf_ptr += strlen("dload_size:");
			// MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  read_buf_ptr=%s", read_buf_ptr);
			// *dload_size = atoi(read_buf_ptr);
			
			// efs_close(fd);

			// return TRUE;
		// } 
	// }

	// return FALSE;
// }
att_apn_id * lwm2m_get_apnid(){
    return &s_apnid;
}

char * get_ipaddr(){
    return &ip_addr;
}

char * get_ipaddr_class2(){
    return &ip_addr_class2;
}
void lwm2m_init_apn_info(){

    s_apnid.global_id = s_global_id;
    s_apnid.attz_id = s_attz_id;
    memset(s_apnid.global_id, 0, MAX_ID_LENGTH);
    memset(s_apnid.attz_id, 0, MAX_ID_LENGTH);
}

//add by joe start
void init_vzw_apn_info(){

    memset(&s_vzw_apnid, 0, sizeof(vzw_apn_id));
}

vzw_apn_id * get_vzw_apnid(vzw_apn_id *apninfo){
    return &s_vzw_apnid;
}

int get_vzw_apn3_callid(){
    LOG_ARG("s_vzw_apnid.vzwinternet_apn3.callid =%d\n",s_vzw_apnid.vzwinternet_apn3.callid);
   return s_vzw_apnid.vzwinternet_apn3.callid;
}

void reset_vzw_apn3_callid(){
    s_vzw_apnid.vzwinternet_apn3.callid = -2;
    LOG_ARG("reset s_vzw_apnid.vzwinternet_apn3.callid =%d\n",s_vzw_apnid.vzwinternet_apn3.callid);
}

void get_vzw_apn2_ifname(char * ifname){
    strcpy(ifname,s_vzw_apnid.vzwadmin_apn2.iface_name);
    LOG_ARG("get_vzw_apn2_ifname =%s, ifname=%s\n",s_vzw_apnid.vzwadmin_apn2.iface_name, ifname);
}

void get_vzw_apn3_ifname(char * ifname){
    strcpy(ifname,s_vzw_apnid.vzwinternet_apn3.iface_name);
    LOG_ARG("get_vzw_apn3_ifname =%s, ifname=%s\n",s_vzw_apnid.vzwinternet_apn3.iface_name, ifname);
}

void get_vzw_apn2_name(char * apn_name){
    strcpy(apn_name,s_vzw_apnid.vzwadmin_apn2.apn_name);
    LOG_ARG("get_vzw_apn2_apn_name =%s\n",s_vzw_apnid.vzwadmin_apn2.apn_name);
}

void get_vzw_apn3_name(char * apn_name){
    strcpy(apn_name,s_vzw_apnid.vzwinternet_apn3.apn_name);
    LOG_ARG("get_vzw_apn3_apn_name =%s\n",s_vzw_apnid.vzwinternet_apn3.apn_name);
}

const char* strstri(const char* str, const char* subStr)
{
    int len = strlen(subStr);
    if(len == 0)
    {
        return NULL;
    }

    while(*str)
    {
        if(strncasecmp(str, subStr, len) == 0)
        {
            return str;
        }
        ++str;
    }
    return NULL;
}

int format_apn_info(char* source, char* buf)
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

bool getvzwApnName(char* buff, char* apn_name){

    char *p;
    p=strtok(buff,",");
    if(p){
        LOG_ARG("AT rsp get %s", p);
    }
    
    p=strtok(NULL,",");
    if(p){
        LOG_ARG("AT rsp get %s", p);
    }
    
    p=strtok(NULL,",");
    if(p){
        LOG_ARG("AT rsp get apn name: %s", p);
        format_apn_info(p,apn_name);
        LOG_ARG("AT rsp get apn name: %s", apn_name);
        //strcpy(apn_name,p);
    }
    p = NULL;
}

#ifdef LWM2M_SUPPORT_QLRIL_API
int getVzwApnInfo (vzw_apn_id *apnid)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;
    char profileid[5] = {-1};
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    char class2apninfo[256]= {0};
    char class3apninfo[256]= {0};
    char class6apninfo[256]= {0};

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, "AT+CGDCONT?\r\n");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

    temp = strtok(atc_cmd_resp, "\r\n");
    while(temp)
    {
        LOG_ARG ("temp is =%s\n", temp);
        if(strstr(temp, "IPV4V6") == NULL){
            temp = strtok(NULL,"\n");
            continue;
        }

        sscanf(temp, "%*s%[^,]", profileid);
        LOG_ARG ("profileid is =%s\n", profileid);
        if(atoi(profileid) == 2) {
            strcpy(class2apninfo,temp);
            LOG_ARG ("%s\n", class2apninfo);
        }
        if(atoi(profileid) == 3) {
            strcpy(class3apninfo,temp);
            LOG_ARG ("%s\n", class3apninfo);
        }
        if(atoi(profileid) == 6) {
            strcpy(class6apninfo,temp);
            LOG_ARG ("%s\n", class6apninfo);
        }
        temp = strtok(NULL,"\n");
        LOG ("get profile loop run\n");
    }

    //get vzw class2 apn
    getvzwApnName(class2apninfo,apnid->vzwadmin_apn2.apn_name);
    apnid->vzwadmin_apn2.profileid = 2; // vzw class 2 apn profile == 2
    //get vzw class3 apn
    getvzwApnName(class3apninfo,apnid->vzwinternet_apn3.apn_name);
    if(strcmp(apnid->vzwinternet_apn3.apn_name,"VZWTEST") == 0){
        strcpy(apnid->vzwinternet_apn3.apn_name,"VZWINTERNET");
        sendATcmdRenameAPN(3,"VZWINTERNET");
    }
    apnid->vzwinternet_apn3.profileid = 3; // vzw class 3 apn profile == 3
    //get vzw class6 apn
    getvzwApnName(class6apninfo,apnid->vzwclass6_apn6.apn_name);
    apnid->vzwclass6_apn6.profileid = 6; // vzw class 6 apn profile == 6

    return 0;
error1:
    return -3;
}

#else
int getVzwApnInfo (vzw_apn_id *apnid)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;
    char profileid[5] = {-1};
    atc_client_handle_type h_atc = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    char class2apninfo[256]= {0};
    char class3apninfo[256]= {0};
    char class6apninfo[256]= {0};

    //int apn_proid = 0;
    if(h_atc == 0){
        ret = QL_ATC_Client_Init(&h_atc);
    }
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret != E_QL_OK){
        return -1;
    }

    //sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, "AT+CGDCONT?\r\n");
    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

    temp = strtok(atc_cmd_resp, "\r\n");
    while(temp)
    {
        LOG_ARG ("temp is =%s\n", temp);
        if(strstr(temp, "IPV4V6") == NULL){
            temp = strtok(NULL,"\n");
            continue;
        }

        sscanf(temp, "%*s%[^,]", profileid);
        LOG_ARG ("profileid is =%s\n", profileid);
        if(atoi(profileid) == 2) {
            strcpy(class2apninfo,temp);
            LOG_ARG ("%s\n", class2apninfo);
        }
        if(atoi(profileid) == 3) {
            strcpy(class3apninfo,temp);
            LOG_ARG ("%s\n", class3apninfo);
        }
        if(atoi(profileid) == 6) {
            strcpy(class6apninfo,temp);
            LOG_ARG ("%s\n", class6apninfo);
        }
        temp = strtok(NULL,"\n");
        LOG ("get profile loop run\n");
    }

    //get vzw class2 apn
    getvzwApnName(class2apninfo,apnid->vzwadmin_apn2.apn_name);
    apnid->vzwadmin_apn2.profileid = 2; // vzw class 2 apn profile == 2
    //get vzw class3 apn
    getvzwApnName(class3apninfo,apnid->vzwinternet_apn3.apn_name);
    if(strcmp(apnid->vzwinternet_apn3.apn_name,"VZWTEST") == 0){
        strcpy(apnid->vzwinternet_apn3.apn_name,"VZWINTERNET");
        sendATcmdRenameAPN(3,"VZWINTERNET");
    }
    apnid->vzwinternet_apn3.profileid = 3; // vzw class 3 apn profile == 3
    //get vzw class6 apn
    getvzwApnName(class6apninfo,apnid->vzwclass6_apn6.apn_name);
    apnid->vzwclass6_apn6.profileid = 6; // vzw class 6 apn profile == 6

    ret = QL_ATC_Client_Deinit(h_atc);
    h_atc = 0;
    LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    return 0;
error1:
    ret = QL_ATC_Client_Deinit(h_atc);
    h_atc = 0;
    LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    if (ret < 0) {
        return -2;
    }

    return -3;
}
#endif 

//add by joe end
#ifdef LWM2M_SUPPORT_QLRIL_API
int lwm2m_getApnInfo (att_apn_id *apnid)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;

    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, "AT+CGDCONT?\r\n");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

    temp = strtok(atc_cmd_resp, "\r\n");
    while(temp)
    {
        //LOG_ARG("%s\n",temp);
        if (strstr(temp, ATT_M2M_GLOBAL) != NULL) {
            LOG_ARG ("%s\n", temp);
            sscanf(temp, "%*s%[^,]", apnid->global_id);
        }
        if (strstr(temp, M2M_COM_ATTZ) != NULL) {
            LOG_ARG ("%s\n", temp);
            sscanf(temp, "%*s%[^,]", apnid->attz_id);
        }
        temp = strtok(NULL,"\n");
    }

    return 0;
error1:
    return ret;
}

#else
int lwm2m_getApnInfo (att_apn_id *apnid)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;

    static atc_client_handle_type h_atc = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

//    fp = fopen(HCISMD_SET_FILE, "w");
//    if (fp == NULL) {
//        return -1;
//    }

//    if (fputs("1", fp) < 0) {
//        perror("fputs 1");
//        goto error2;
//    }

//    fflush(fp);

    ret = QL_ATC_Client_Init(&h_atc);
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret != E_QL_OK){
//        goto error2;
        return ret;
    }

    sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, "AT+CGDCONT?\r\n");
    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

    temp = strtok(atc_cmd_resp, "\r\n");
    while(temp)
    {
        //LOG_ARG("%s\n",temp);
        if (strstr(temp, ATT_M2M_GLOBAL) != NULL) {
            LOG_ARG ("%s\n", temp);
            sscanf(temp, "%*s%[^,]", apnid->global_id);
        }
        if (strstr(temp, M2M_COM_ATTZ) != NULL) {
            LOG_ARG ("%s\n", temp);
            sscanf(temp, "%*s%[^,]", apnid->attz_id);
        }
        temp = strtok(NULL,"\n");
    }

error1:
    ret = QL_ATC_Client_Deinit(h_atc);
    LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    if (ret < 0) {
        return -2;
    }

//error2:
//    fclose(fp);

    return ret;
}
#endif
#ifdef LWM2M_SUPPORT_QLRIL_API
int lwm2m_setApnEnable (int enable, char *id)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;
    char str[100];

    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    if(id == NULL){
        perror("error: id = null!");
        return -1;
    }

    sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
    sprintf(atc_cmd_req,"AT+CGACT=%d,%s\r\n", enable, id);
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

error1:
    return ret;
}

#else
int lwm2m_setApnEnable (int enable, char *id)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;
    char str[100];

    static atc_client_handle_type h_atc = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    if(id == NULL){
        perror("error: id = null!");
        return -1;
    }
//    fp = fopen(HCISMD_SET_FILE, "w");
//    if (fp == NULL) {
//        return -1;
//    }

//    if (fputs("1", fp) < 0) {
//        perror("fputs 1");
//        goto error2;
//    }

//    fflush(fp);

    ret = QL_ATC_Client_Init(&h_atc);
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret != E_QL_OK){
//        goto error2;
        return ret;
    }

    sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    sprintf(atc_cmd_req,"AT+CGACT=%d,%s\r\n", enable, id);
    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

error1:
    ret = QL_ATC_Client_Deinit(h_atc);
    LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    if (ret < 0) {
        return -2;
    }

//error2:
//    fclose(fp);

    return ret;
}
#endif

#ifdef LWM2M_SUPPORT_QLRIL_API
int lwm2m_enableApn(bool enable, char *id, char *apn){
    int ret = 0;
    int radioTech = 0;
    char protocol[10] = {0};
    char dns1[20] = {0};
    char dns2[20] = {0};
    char cmdStr[20] = {0};
    RIL_Data_Call_Response_v11 *dataCall = NULL;
    bool is_vzwinternet_apn3 = false;

    //radioTech = 14;//default LTE
	radioTech = QL_LWM2M_GetDataRegistrationState();
	LOG_ARG("radioTech = %d\n",radioTech);
	if(radioTech == -1){
	    radioTech = 14;//default LTE
	}    
    int profileid = atoi(id);

    ret = QLRIL_SetupDataCall(&g_qlril_handle, radioTech, profileid, (strlen(apn) == 0?NULL:apn),
                        "", "", 0, "IPV4V6", (void **)&dataCall);
    if (ret > 0){
        memset(dns1, 0, sizeof(dns1));
        memset(dns2, 0, sizeof(dns2));
        if (strlen(dataCall->dnses) >= 15) {
            sscanf(dataCall->dnses, "%s%*[ ]%s", dns1, dns2);
            //printf ("buf: %s, msg: %s\n", buf, msg);
        } else if (strlen(dataCall->dnses) >= 7) {
            sscanf(dataCall->dnses, "%s", dns1);
            //printf ("buf: %s\n", buf);
        } else {
            snprintf(dns1, sizeof(dns1), "%s", "8.8.8.8");
            snprintf(dns2, sizeof(dns1), "%s", "4.2.2.2");
        }

        memcpy(s_apnid.global_ifname,dataCall->ifname,strlen(dataCall->ifname));
        s_apnid.callid = 1;
        if (strncmp(dataCall->type, "IPV6", 4) != 0) {
            if (strlen(dataCall->ifname) > 0) {
                ret = QLRIL_SetRouteAndDNS(dataCall->ifname, dns1, dns2);
                if (ret != 0)
                    LOG_ARG("QLRIL_SetRouteAndDNS failed with return:%d\n", ret);
                else
                    LOG("QLRIL_SetupDataCall & QLRIL_SetRouteAndDNS success\n");
            } else
                    LOG("QLRIL_SetupDataCall called failed without interface name\n");
        } else {
            LOG("QLRIL_SetRouteAndDNSForIPV6() Please look forward to!");
        }
        if (dataCall->type)
            free(dataCall->type);
        if (dataCall->ifname)
            free(dataCall->ifname);
        if (dataCall->addresses)
            free(dataCall->addresses);
        if (dataCall->dnses)
            free(dataCall->dnses);
        if (dataCall->gateways)
            free(dataCall->gateways);
        if (dataCall->pcscf)
            free(dataCall->pcscf);
        
        if (dataCall) {
            free(dataCall);
            dataCall = NULL;
        }

    }else{
        LOG_ARG("QLRIL_SetupDataCall failed ret = %d", ret);
    }

}

#else
int lwm2m_enableApn (bool enable, char *id, char *apn){
    E_QL_ERROR_CODE_T ret;
    QL_MCM_DATA_CALL_CONFIG_INFO_T  t_cfg = {0};
    QL_MCM_DATA_CALL_RESULT_INFO_T  t_call_result = {0};

    ret = QL_MCM_DATA_Client_Init(&h_data);
	if(ret != E_QL_OK){
		return ret;
	}
    LOG_ARG("-->QL_MCM_DATA_Client_Init ret = %d, h_data=0x%X\n", ret, h_data);

   sleep(1); 

    ret = QL_MCM_DATA_SetProfile(&t_cfg, atoi(id), enable);
	if(ret != E_QL_OK){
		return ret;
	}
    LOG_ARG("-->QL_MCM_DATA_SetProfile ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    ret = QL_MCM_DATA_SetAPN(&t_cfg, apn, enable);
	if(ret != E_QL_OK){
		return ret;
	}
    LOG_ARG("-->QL_MCM_DATA_SetAPN ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    if(enable){
        ret = QL_MCM_DATA_StartDataCall(h_data, &t_cfg, &t_call_result);
        s_apnid.callid = t_call_result.call_id;
        LOG_ARG("-->QL_MCM_DATA_StartDataCall ret = %d, t_cfg=0x%X\n", ret, t_cfg);
        LOG_ARG("-->call_id = %d\n",t_call_result.call_id);
    }else{
        ret = QL_MCM_DATA_StopDataCall(h_data, s_apnid.callid);
        LOG_ARG("-->QL_MCM_DATA_StopDataCall ret = %d\n", ret);
    }
    return ret;
}
#endif

#ifdef LWM2M_SUPPORT_QLRIL_API
/*enable default apn (attz)*/
int lwm2m_enableDefaultApn (){
    int ret = 0;
    int radioTech = 0;
    char protocol[10] = {0};
    char dns1[20] = {0};
    char dns2[20] = {0};
    char cmdStr[20] = {0};
    char str[200] = {0};
    char dnses[200] = {0};	
    RIL_Data_Call_Response_v11 *dataCall = NULL;
    bool is_vzwinternet_apn3 = false;

    radioTech = 14;//default LTE
    int profileid = 1;
    char *apn = "m2m.com.attz";
    ret = QLRIL_SetupDataCall(&g_qlril_handle, radioTech, profileid, (strlen(apn) == 0?NULL:apn),
                        "", "", 0, "IPV4V6", (void **)&dataCall);
    if (ret > 0){
        if (dataCall->active == 0) {
            LOG("lwm2m_enableDefaultApn,dataCall->not active will return\n");
            return;
        }
		if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {//FIXME maybe
			//str = dataCall->dnses;
            strncpy(str,dataCall->dnses,strlen(dataCall->dnses));
		} else {
			ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "4.2.2.2");
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200::1");
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200:baba::1");
			//str = dnses;
            strncpy(str,dnses,strlen(dnses));
		}
        LOG_ARG("lwm2m_enableDefaultApn:dns str: %s\n", str);
		//ret = QLRIL_SetDNSForIPV4V6(str);
		ret = QLRIL_AppendDNSForIPV4V6(str);
		if (ret <= 0){
			LOG_ARG("QLRIL_AppendDNSForIPV4V6 failure return: %d\n", ret);
		}else{
			LOG("QLRIL_AppendDNSForIPV4V6 success\n");
		}
		
		if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
            remove_default_route();
		    configure_default_route(dataCall->ifname);
			ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
			if (ret < 0)
				LOG_ARG("QLRIL_SetRouteForIPV4V6 failed with return:%d\n", ret);
			else
				LOG("QLRIL_SetRouteForIPV4V6 success\n");
		} else{
			LOG("QLRIL_SetupDataCall called failed without interface name\n");
		}
		
		if (dataCall->type)
			free(dataCall->type);
		if (dataCall->ifname)
			free(dataCall->ifname);
		if (dataCall->addresses)
			free(dataCall->addresses);
		if (dataCall->dnses)
			free(dataCall->dnses);
		if (dataCall->gateways)
			free(dataCall->gateways);
		if (dataCall->pcscf)
			free(dataCall->pcscf);

		if (dataCall) {
			free(dataCall);
			dataCall = NULL;
		}

    }else{
        LOG_ARG("QLRIL_SetupDataCall failed ret = %d", ret);
    }
    return 0;
}

#else
int lwm2m_enableDefaultApn (){
    int     h_mobap = 0;
    E_QL_ERROR_CODE_T ret;
    QL_MOBAP_IPV4_WWAN_CONFIG_INFO_T    t_wwan_cfg = {0};
    struct in_addr addr;
    char command[128];

    ret = QL_MCM_MobileAP_Init(&h_mobap);
    LOG_ARG("QL_MCM_MobileAP_Init ret = %d, h_mobap=0x%X\n", ret, h_mobap);
	if(ret != E_QL_OK){
		return ret;
	}
    sleep(1);

	if(mobaq == 0){
		mobaq = h_mobap;
	}

//	ret = QL_MCM_MobileAP_AddRxIndMsgHandler(ql_mcm_mobileap_nfy, (void*)0x12345678);
//    LOG_ARG("QL_MCM_MobileAP_AddRxIndMsgHandler ret = %d\n", ret);
	if(h_mobap == 0){
		h_mobap = mobaq;
	}
    ret = QL_MCM_MobileAP_Enable(h_mobap);
    LOG_ARG("QL_MCM_MobileAP_Enable ret = %d\n", ret);
	if(ret != E_QL_OK){
		return ret;
	}

    ret = QL_MCM_MobileAP_ConnectBackhaul(h_mobap, E_QL_MOBILEAP_IP_TYPE_V4);
    LOG_ARG("QL_MCM_MobileAP_ConnectBackhaul ret = %d\n", ret);
	if(ret != E_QL_OK){
		return ret;
	}

    ret = QL_MCM_MobileAP_GetIPv4WWANCfg(h_mobap, &t_wwan_cfg);
    LOG_ARG("QL_MCM_MobileAP_GetIPv4WWANCfg ret = %d, IPV4 Config Info:...\n", ret);
	if(ret != E_QL_OK){
		return ret;
	}
    if(ret == E_QL_OK){
        addr.s_addr = ntohl(t_wwan_cfg.v4_addr);
        LOG_ARG("v4_addr_valid:%d v4_addr:%s \n",t_wwan_cfg.v4_addr_valid,inet_ntoa(addr));
        sprintf(ip_addr, "%s",inet_ntoa(addr));
        addr.s_addr = ntohl(t_wwan_cfg.v4_prim_dns_addr);
        LOG_ARG("v4_prim_dns_addr_valid:%d v4_prim_dns_addr:%s \n",t_wwan_cfg.v4_prim_dns_addr_valid,inet_ntoa(addr));
        snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",inet_ntoa(addr));
        system(command);
        addr.s_addr = ntohl(t_wwan_cfg.v4_sec_dns_addr);
        LOG_ARG("v4_sec_dns_addr_valid:%d v4_sec_dns_addr:%s \n",t_wwan_cfg.v4_sec_dns_addr_valid,inet_ntoa(addr));
        snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",inet_ntoa(addr));
        system(command);
    }
    return ret;
}
#endif
//add by joe start
char *addr2str(QL_MCM_DATA_CALL_ADDR_T *addr, char *buf, int len, int *pIsIpv6)
{
    char *ptr;
    int i;
    int isIpv6 = 0;

    if(addr->valid_addr==0) {
        return NULL;
    }
    
    if(addr->addr.ss_family == AF_INET6) {
        isIpv6 = 1;
    }

    if(pIsIpv6) {
        pIsIpv6[0] = isIpv6;
    }
    if(isIpv6==0) {
        snprintf(buf, len, "%d.%d.%d.%d", addr->addr.__ss_padding[0], 
                addr->addr.__ss_padding[1],addr->addr.__ss_padding[2],addr->addr.__ss_padding[3]);
    }
    else {
        inet_ntop(AF_INET6, &(addr->addr), buf, sizeof(addr->addr));
    }

    return buf;
}


static int quec_system_call
(
    char    *cmd,
    char    *args[]
)
{
    int childExitStatus = 0;
    pid_t pid = fork();
    if ( pid == 0 )
    {
        execvp(cmd, args);
    }
    waitpid(pid, &childExitStatus, 0);
    return childExitStatus;
}

void setIprouteforApnVZWADMIN(char *ifname, char *ipaddr, char *gw){
    char *args[10] = {0};
    int32 ret = 0;
    if(ipaddr == NULL)
       return;

    if(gw == NULL || strlen(gw) == 0){
        args[0] = "ip";
        args[1] = "ro";
        args[2] = "add";
        args[3] = "to";
        args[4] = ipaddr;
        args[5] = "dev";
        args[6] = ifname;
    }else{
        args[0] = "ip";
        args[1] = "ro";
        args[2] = "add";
        args[3] = "to";
        args[4] = ipaddr;
        args[5] = "via";
        args[6] = gw;
        args[7] = "dev";
        args[8] = ifname;
    }

    if( (ret = quec_system_call("ip", args)) != 0)
    {
        LOG("Failed to add host route\n");
    }else
    LOG("add host route success\n");

    return ret;
    
}


static void quec_data_call_nfy_cb
(
    data_client_handle_type         h_data,
    E_QL_DATA_NET_EVENT_MSG_ID_T    e_msg_id,
    QL_MCM_DATA_CALL_INDICATION_INFO_T *data,
    void                            *contextPtr    
)
{
    int is_ipv6 = 0;
    char *event[] = {   "unknown", 
                        "NET_UP", 
                        "NET_DOWN", 
                        "NEW_ADDR", 
                        "DEL_ADDR", 
                        "SERVICE_STATUS", 
                        "BEARER_TECH_STATUS",
                        "DORMANCY_STATUS"};
    char *call_status[] = {   "INVALID", 
                        "INVALID", 
                        "CONNECTING",
                        "CONNECTED",
                        "DISCONNECTING",
                        "DISCONNECTED"};
    LOG_ARG("### %s got %s event!\n", __func__, event[e_msg_id - 0x5000]);

    switch(e_msg_id)
    {
    case E_QL_DATA_NET_UP_EVENT:
        break;
    case E_QL_DATA_NET_DOWN_EVENT:
        break;
    case E_QL_DATA_NET_NEW_ADDR_EVENT:
        break;
    case E_QL_DATA_NET_DEL_ADDR_EVENT:
        break;
    case E_QL_DATA_NET_SERVICE_STATUS_EVENT:
        break;
    case E_QL_DATA_NET_BEARER_TECH_STATUS_EVENT:
        break;
    case E_QL_DATA_NET_DORMANCY_STATUS_EVENT:
        break;
    }
    
    if(data->call_id_valid) 
    {
        LOG_ARG("quec_data_call_nfy_cb call_id : %d\n", data->call_id);
    }
    if(data->call_status_valid) 
    {
        LOG_ARG("quec_data_call_nfy_cb call_status : %s\n", call_status[data->call_status]);
    }
    if(data->call_tech_valid) 
    {
        LOG_ARG("quec_data_call_nfy_cb call_tech : %d\n", data->call_tech);
    }
    if(data->reg_status_valid) 
    {
        LOG_ARG("quec_data_call_nfy_cb reg_status : %d\n", data->reg_status);
    }
    if(data->dorm_status_valid)
    {
        LOG_ARG("quec_data_call_nfy_cb dorm_status : %d\n", data->dorm_status);
    }
    if(data->addr_count_valid) 
    {
        LOG_ARG("quec_data_call_nfy_cb addr_count  : %d\n", data->addr_count);
    }
    if(data->addr_info_valid) 
    {
        int i;
        QL_MCM_DATA_CALL_ADDR_INFO_T *addr;
        char *ptr;
        char tmpBuf[246];
        struct in_addr ia;
        LOG_ARG("addr_info_len : %d\n", data->addr_info_len);
        for(i=0; i<data->addr_info_len; i++) 
        {
            addr = &data->addr_info[i];
            ptr = addr2str(&addr->iface_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) 
            {
                LOG_ARG("ipaddr : %s/%d \n", ptr, addr->iface_mask);
            }

            ptr = addr2str(&addr->gtwy_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) 
            {
                LOG_ARG("gateway : %s/%d \n", ptr, addr->gtwy_mask);
            }

            ptr = addr2str(&addr->dnsp_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) 
            {
                LOG_ARG("Primary DNS : %s\n", ptr);
            }

            ptr = addr2str(&addr->dnss_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) 
            {
                LOG_ARG("Secondary DNS : %s\n", ptr);
            }
        }
    }
    if(data->vce_reason_valid) 
    {
        LOG_ARG("data call end reason : %d\n", data->vce_reason);
    }
}

#ifdef LWM2M_SUPPORT_QLRIL_API
int enableApnNetworkByprofile(bool enable, int profileid, char *apn){
    int ret = 0;
    int radioTech = 0;
    char protocol[10] = {0};
    char dns1[20] = {0};
    char dns2[20] = {0};
    char cmdStr[20] = {0};
    char str[200] = {0};
	char dnses[200] = {0};
    RIL_Data_Call_Response_v11 *dataCall = NULL;
    bool is_vzwinternet_apn3 = false;

    //radioTech = 14;//default LTE
	radioTech = QL_LWM2M_GetDataRegistrationState();
	LOG_ARG("radioTech = %d\n",radioTech);
	if(radioTech == -1){
	    radioTech = 14;//default LTE
	}
	
    if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
        remove_default_route();
    }    
    ret = QLRIL_SetupDataCall(&g_qlril_handle, radioTech, profileid, (strlen(apn) == 0?NULL:apn),
                        "", "", 0, "IPV4V6", (void **)&dataCall);
    if(dataCall == NULL){
        LOG_ARG("active profileid: %d failed,dataCall = NULL will return\n",profileid);
        return;
    }
	LOG_ARG("QLRIL_SetupDataCall return: profileid =%d dataCall->ifname=%s\n",profileid, dataCall->ifname);
    if (ret > 0){
		if (dataCall->active == 0) {
            if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                LOG( "Network vzwadmin_apn2 active FAILED!\n");
                char* class2_network_ready = "VZWADMIN Network active failed !";
                quec_lwm2m_client_info_notify(class2_network_ready);
            }else if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
                LOG( "Network vzwinternet_apn3 active FAILED!\n");
                char* class3_network_ready = "VZWINTERNET Network active failed !";
                quec_lwm2m_client_info_notify(class3_network_ready);            
            }
            return;
		}else{
                //Save vzw apn data call info
                if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                    s_vzw_apnid.vzwadmin_apn2.callid = 1;//data call set to 1// from SC20 Linux
                    if(strlen(dataCall->addresses) > 15){
                        memcpy(ip_addr_class2,dataCall->addresses,15);
                    }else{
                        memcpy(ip_addr_class2,dataCall->addresses,strlen(dataCall->addresses));
                    }
                    //strcpy(ip_addr_class2, dataCall->addresses);//save vzwadmin apn2 ip addr info                  
                    memcpy(s_vzw_apnid.vzwadmin_apn2.iface_name,dataCall->ifname,strlen(dataCall->ifname));
                    memcpy(s_vzw_apnid.vzwadmin_apn2.gw_addr,dataCall->gateways,strlen(dataCall->gateways));
                    LOG( "Network vzwadmin_apn2 active successful !\n");
                    char* class2_network_ready = "VZWADMIN Network actived successful!";
                    quec_lwm2m_client_info_notify(class2_network_ready);

                    //setup DNS iproute
                    if(dataCall->dnses != NULL && strlen(dataCall->dnses) > 0){
                        setup_dns_ip_route(dataCall->ifname,dataCall->dnses);
                    }

                }else if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
                    LOG_ARG( "enableApnNetworkByprofile ip%s !\n",dataCall->addresses);
                    s_vzw_apnid.vzwinternet_apn3.callid = 1;
                    
                    is_vzwinternet_apn3 = true;
                    if(strlen(dataCall->addresses) > 15){
                        memcpy(ip_addr,dataCall->addresses,15);
                    }else{
                        memcpy(ip_addr,dataCall->addresses,strlen(dataCall->addresses));
                    }
                    //strcpy(ip_addr, dataCall->addresses);//save vzwinternet apn3 ip addr info
                    memcpy(s_vzw_apnid.vzwinternet_apn3.iface_name,dataCall->ifname,strlen(dataCall->ifname));
                    configure_default_route(dataCall->ifname);
                    LOG( "Network vzwinternet_apn3 active successful !\n");
                    char* class3_network_ready = "VZWINTERNET Network actived successful!";
                    quec_lwm2m_client_info_notify(class3_network_ready);            
                }else if(profileid == s_vzw_apnid.vzwclass6_apn6.profileid){
                    s_vzw_apnid.vzwclass6_apn6.callid = 1;
                    memcpy(s_vzw_apnid.vzwclass6_apn6.iface_name,dataCall->ifname,strlen(dataCall->ifname));
                }
                
        		if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {//FIXME maybe
        			//str = dataCall->dnses;
                    strncpy(str,dataCall->dnses,strlen(dataCall->dnses));
        		} else {
        			ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
        			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "4.2.2.2");
        			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200::1");
        			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200:baba::1");
        			//str = dnses;
                    strncpy(str,dnses,strlen(dnses));
        		}
        		//ret = QLRIL_SetDNSForIPV4V6(str);
                ret = QLRIL_AppendDNSForIPV4V6(str);
        		if (ret <= 0){
        		    LOG_ARG("QLRIL_AppendDNSForIPV4V6 failure return: %d\n", ret);
        		}else{
        		    LOG("QLRIL_AppendDNSForIPV4V6 success\n");
        		}

        		if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
        			ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
        			if (ret < 0)
        				LOG_ARG("QLRIL_SetRouteForIPV4V6 failed with return:%d\n", ret);
        			else
        				LOG("QLRIL_SetRouteForIPV4V6 success\n");
        		} else{
        			LOG("QLRIL_SetupDataCall called failed without interface name\n");
        		}
        }
		if (dataCall->type)
			free(dataCall->type);
		if (dataCall->ifname)
			free(dataCall->ifname);
		if (dataCall->addresses)
			free(dataCall->addresses);
		if (dataCall->dnses)
			free(dataCall->dnses);
		if (dataCall->gateways)
			free(dataCall->gateways);
		if (dataCall->pcscf)
			free(dataCall->pcscf);

		if (dataCall) {
			free(dataCall);
			dataCall = NULL;
		}        
    }else{
        LOG_ARG("QLRIL_SetupDataCall failed ret = %d", ret);
    }

}

#else
int enableApnNetworkByprofile(bool enable, int profileid, char *apn){
    E_QL_ERROR_CODE_T ret = 0;
    QL_MCM_DATA_CALL_CONFIG_INFO_T  t_cfg = {0};
    QL_MCM_DATA_CALL_RESULT_INFO_T  t_call_result = {0};
    QL_MCM_DATA_CALL_ADDR_INFO_LIST_T   t_addr_list = {0};
    int i;    
    char tmpBuf[256];
    char *ptr;
    int is_ipv6 = 0;
    QL_MCM_DATA_CALL_ADDR_INFO_T *addr;
    char dev_name[13] = {0};
    int call_id = -1;
    char command[128];
    bool is_vzwinternet_apn3 = false;
    
    if(h_data == 0){//add by joe only init once
        ret = QL_MCM_DATA_Client_Init(&h_data);
    }
    if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
        remove_default_route();
    }

    //ret = QL_MCM_DATA_AddRxIndMsgHandler(quec_data_call_nfy_cb, h_data);
    //LOG_ARG(" QL_MCM_DATA_AddRxIndMsgHandler ret = %d\n", ret);
            
    LOG_ARG("-->QL_MCM_DATA_Client_Init ret = %d, h_data=0x%X\n", ret, h_data);

    ret = QL_MCM_DATA_SetProfile(&t_cfg, profileid, enable);
    LOG_ARG("-->QL_MCM_DATA_SetProfile ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    ret = QL_MCM_DATA_SetAPN(&t_cfg, apn, enable);
    LOG_ARG("-->QL_MCM_DATA_SetAPN ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    if(enable){
        ret = QL_MCM_DATA_StartDataCall(h_data, &t_cfg, &t_call_result);
        LOG_ARG("-->QL_MCM_DATA_StartDataCall ret = %d, t_cfg=0x%X\n", ret, t_cfg);
        LOG_ARG("-->call_id = %d\n",t_call_result.call_id);
        if(t_call_result.call_id == -1){
            return -1;
        }
        if(ret == 9 || ret == -9){
            QL_MCM_DATA_Client_Deinit(&h_data);
            h_data = 0;
            return -1;
        }
        call_id = t_call_result.call_id;

        //get dev name
        ret = QL_MCM_DATA_GetDeviceName(h_data, call_id, dev_name, 13);
        LOG_ARG("QL_MCM_DATA_GetDeviceName ret = %d with dev_name=%s\n", ret, dev_name);

        if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
            s_vzw_apnid.vzwadmin_apn2.callid = call_id;
            memcpy(s_vzw_apnid.vzwadmin_apn2.iface_name,dev_name,strlen(dev_name));
            if(call_id == 0){//set default value
                memcpy(s_vzw_apnid.vzwadmin_apn2.iface_name,VZW_CLASS2_APN_IFNAME,strlen(VZW_CLASS2_APN_IFNAME));
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf","198.224.157.135");
                system(command);
                setIprouteforApnVZWADMIN(VZW_CLASS2_APN_IFNAME,"198.224.157.135",NULL);
                if(s_vzw_apnid.vzwinternet_apn3.callid == -2){
                   // setIprouteforApnVZWADMIN(VZW_CLASS2_APN_IFNAME,"0.0.0.0",NULL);
                }
            }
        }else if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
            s_vzw_apnid.vzwinternet_apn3.callid = call_id;
            is_vzwinternet_apn3 = true;
            LOG_ARG("joe set s_vzw_apnid.vzwinternet_apn3.callid callid = %d \n", s_vzw_apnid.vzwinternet_apn3.callid);
            memcpy(s_vzw_apnid.vzwinternet_apn3.iface_name,dev_name,strlen(dev_name));
            if(call_id == 0){//set default value
                memcpy(s_vzw_apnid.vzwinternet_apn3.iface_name,VZW_CLASS3_APN_IFNAME,strlen(VZW_CLASS3_APN_IFNAME));
                configure_default_route(VZW_CLASS3_APN_IFNAME);
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf","198.224.154.135");
                system(command);
            }
        }else if(profileid == s_vzw_apnid.vzwclass6_apn6.profileid){
            s_vzw_apnid.vzwclass6_apn6.callid = call_id;
            memcpy(s_vzw_apnid.vzwclass6_apn6.iface_name,dev_name,strlen(dev_name));
        }
        
        if(ret != 0){
            return -1;
        }
        //fill network ip info

        ret = QL_MCM_DATA_GetDeviceAddress(h_data, t_call_result.call_id, &t_addr_list);
        LOG_ARG("-->QL_MCM_DATA_GetDeviceAddress = %d\n",ret);
        if(ret != 0){
            return -1;
        }
        
        LOG_ARG("QL_MCM_DATA_GetDeviceAddress ret = %d, valid addr count=%d, details.....\n", ret, t_addr_list.addr_info_len);
        for(i=0; i<1; i++) {//get first ip from list
        
            QL_MCM_DATA_ROUTE_INFO_T            t_info = {0};
        
            addr = &(t_addr_list.addr_info[i]);

            ptr = addr2str(&addr->iface_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                LOG_ARG("ipaddr : %s/%d \n", ptr, addr->iface_mask);
                memcpy(t_info.ip_addr,ptr,strlen(ptr));
                if(is_vzwinternet_apn3){
                    strcpy(ip_addr, ptr);//save vzwinternet apn3 ip addr info
                }
                if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                    strcpy(ip_addr_class2, ptr);//save vzwadmin apn2 ip addr info
                }
                
            }
            else {
                LOG("Failed to parse addr\n");
            }

            ptr = addr2str(&addr->gtwy_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                LOG_ARG("gateway : %s/%d \n", ptr, addr->gtwy_mask);
                memcpy(t_info.gateway,ptr,strlen(ptr));
              //save apn2 vzwadmin gw ip
                if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                    memcpy(s_vzw_apnid.vzwadmin_apn2.gw_addr,ptr,strlen(ptr));
                }           
            }

           //add route
            memcpy(t_info.iface_name,dev_name,strlen(dev_name));
            ret = QL_MCM_DATA_AddRoute(h_data, &t_info);
            LOG_ARG("QL_MCM_DATA_AddRoute ret = %d\n", ret);

           //set dns
            ptr = addr2str(&addr->dnsp_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",ptr);
                system(command);
                LOG_ARG("Primary DNS : %s command=%s\n", ptr,command);
                if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                    setIprouteforApnVZWADMIN(t_info.iface_name,ptr,NULL);
                    if(s_vzw_apnid.vzwinternet_apn3.callid == -2){
                       // setIprouteforApnVZWADMIN(t_info.iface_name,"0.0.0.0",NULL);
                    }

                    snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf","198.224.157.135");
                    system(command);
                    setIprouteforApnVZWADMIN(t_info.iface_name,"198.224.157.135",NULL);
                }
            }
            
            ptr = addr2str(&addr->dnss_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",ptr);
                if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                    setIprouteforApnVZWADMIN(t_info.iface_name,ptr,NULL);
                     if(s_vzw_apnid.vzwinternet_apn3.callid == -2){
                       // setIprouteforApnVZWADMIN(t_info.iface_name,"0.0.0.0",NULL);
                    }
                }
                LOG_ARG("Secondary DNS : %s command=%s\n", ptr,command);
            }

            /*
            if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
                 ret = QL_MCM_DATA_Add_Route_in_default_table(h_data, &t_info);
                 LOG_ARG("QL_MCM_DATA_Add_Route ret = %d\n", ret);
            }*/
             //set default route
            if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
                 //set vzwinternet apn3 network as default network
                 //ret = QL_MCM_DATA_Add_Default_Route(h_data, &t_info);
                 configure_default_route(t_info.iface_name);
                 LOG("configure_default_route end\n");
            }

        }
        
        if(ret != 0){
            return -1;
        }
        return ret;
    }else{
        if(profileid == s_vzw_apnid.vzwadmin_apn2.profileid){
            call_id = s_vzw_apnid.vzwadmin_apn2.callid ;
        }else if(profileid == s_vzw_apnid.vzwinternet_apn3.profileid){
            call_id = s_vzw_apnid.vzwinternet_apn3.callid ;
        }else if(profileid == s_vzw_apnid.vzwclass6_apn6.profileid){
            call_id = s_vzw_apnid.vzwclass6_apn6.callid ;
        }
        ret = QL_MCM_DATA_StopDataCall(h_data, call_id);
        LOG_ARG("-->QL_MCM_DATA_StopDataCall ret = %d\n", ret);
    }
    //QL_MCM_DATA_Client_Deinit(&h_data);
    return ret;
}
#endif
#ifdef LWM2M_SUPPORT_QLRIL_API
int send_AT_Command (char * command, char * resp)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;

    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    LOG("LwM2M enter send_AT_Command !\n");

    sleep(1);
    LOG("LwM2M  send_AT_Command  memset!\n");
    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, command);
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }
    if(resp != NULL){
        strcpy(resp,atc_cmd_resp);
    }

error1:
    return ret;
}

#else
int send_AT_Command (char * command, char * resp)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    FILE *fp = NULL;
    char *temp = NULL;

    static atc_client_handle_type h_atc = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    // fp = fopen(HCISMD_SET_FILE, "w");
    // if (fp == NULL) {
        // return -1;
    // }

    // if (fputs("1", fp) < 0) {
        // perror("fputs 1");
        // goto error2;
    // }

    // fflush(fp);

    ret = QL_ATC_Client_Init(&h_atc);
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret != E_QL_OK){
//        goto error2;
    }

    sleep(1);

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, command);
    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }
    if(resp != NULL){
        strcpy(resp,atc_cmd_resp);
    }

error1:
    ret = QL_ATC_Client_Deinit(h_atc);
    h_atc= 0;
    LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    if (ret < 0) {
        return -2;
    }

//error2:
//    fclose(fp);

    QL_ATC_Client_Deinit(h_atc);
    return ret;
}
#endif

#ifdef LWM2M_SUPPORT_QLRIL_API
int enableATTGlobalNetworkByprofile(bool enable, int profileid, char *apn){
    int ret = 0;
    int radioTech = 0;
    char protocol[10] = {0};
    char dns1[20] = {0};
    char dns2[20] = {0};
    char cmdStr[20] = {0};
    char str[200] = {0};
    char dnses[200] = {0};
    RIL_Data_Call_Response_v11 *dataCall = NULL;
    bool is_vzwinternet_apn3 = false;

    //radioTech = 14;//default LTE
   	radioTech = QL_LWM2M_GetDataRegistrationState();
	LOG_ARG("radioTech = %d\n",radioTech);
	if(radioTech == -1){
	    radioTech = 14;//default LTE
	}
    
    ret = QLRIL_SetupDataCall(&g_qlril_handle, radioTech, profileid, (strlen(apn) == 0?NULL:apn),
                        "", "", 0, "IPV4V6", (void **)&dataCall);

    if(dataCall == NULL){
        LOG_ARG("active profileid: %d failed,dataCall = NULL will return\n",profileid);
        return;
    }
    
    if (ret > 0){
        if (dataCall->active == 0) {
            LOG_ARG("active profileid: %d failed,dataCall->not active will return\n",profileid);
            return;
        }
		if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {//FIXME maybe
			//str = dataCall->dnses;
            strncpy(dnses,dataCall->dnses,strlen(dataCall->dnses));
            strncpy(str,dataCall->dnses,strlen(dataCall->dnses));
		} else {
			ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "4.2.2.2");
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200::1");
			ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200:baba::1");
			//str = dnses;
            strncpy(str,dnses,strlen(dnses));
		}
        LOG_ARG("enableATTGlobalNetworkByprofile:dns str: %s\n", str);
		//ret = QLRIL_SetDNSForIPV4V6(str);
		ret = QLRIL_AppendDNSForIPV4V6(str);
		if (ret <= 0){
			LOG_ARG("QLRIL_AppendDNSForIPV4V6 failure return: %d\n", ret);
		}else{
			LOG("QLRIL_AppendDNSForIPV4V6 success\n");
            //setup DNS iproute
            if(dataCall->dnses != NULL && strlen(dataCall->dnses) > 0){
                setup_dns_ip_route(dataCall->ifname,dnses);
            }
		}
		
		if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
		    memcpy(s_apnid.global_ifname,dataCall->ifname,strlen(dataCall->ifname));
			ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
			if (ret < 0)
				LOG_ARG("QLRIL_SetRouteForIPV4V6 failed with return:%d\n", ret);
			else
				LOG("QLRIL_SetRouteForIPV4V6 success\n");
		} else{
			LOG("QLRIL_SetupDataCall called failed without interface name\n");
		}
		
		if (dataCall->type)
			free(dataCall->type);
		if (dataCall->ifname)
			free(dataCall->ifname);
		if (dataCall->addresses)
			free(dataCall->addresses);
		if (dataCall->dnses)
			free(dataCall->dnses);
		if (dataCall->gateways)
			free(dataCall->gateways);
		if (dataCall->pcscf)
			free(dataCall->pcscf);

		if (dataCall) {
			free(dataCall);
			dataCall = NULL;
		}

    }else{
        LOG_ARG("QLRIL_SetupDataCall failed ret = %d", ret);
    }

}

#else
int enableATTGlobalNetworkByprofile(bool enable, int profileid, char *apn){
    E_QL_ERROR_CODE_T ret = 0;
    QL_MCM_DATA_CALL_CONFIG_INFO_T  t_cfg = {0};
    QL_MCM_DATA_CALL_RESULT_INFO_T  t_call_result = {0};
    QL_MCM_DATA_CALL_ADDR_INFO_LIST_T   t_addr_list = {0};
    int i;
    char tmpBuf[256];
    char *ptr;
    int is_ipv6 = 0;
    QL_MCM_DATA_CALL_ADDR_INFO_T *addr;
    char dev_name[13] = {0};
    int call_id = -1;
    char command[128];

    //if(h_data == 0){//add by joe only init once
        ret = QL_MCM_DATA_Client_Init(&h_data);
    //}
    LOG_ARG("-->QL_MCM_DATA_Client_Init ret = %d, h_data=0x%X\n", ret, h_data);

    ret = QL_MCM_DATA_SetProfile(&t_cfg, profileid, enable);
    LOG_ARG("-->QL_MCM_DATA_SetProfile ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    ret = QL_MCM_DATA_SetAPN(&t_cfg, apn, enable);
    LOG_ARG("-->QL_MCM_DATA_SetAPN ret = %d, t_cfg=0x%X\n", ret, t_cfg);

    if(enable){
        ret = QL_MCM_DATA_StartDataCall(h_data, &t_cfg, &t_call_result);
        LOG_ARG("-->QL_MCM_DATA_StartDataCall ret = %d, t_cfg=0x%X\n", ret, t_cfg);
        LOG_ARG("-->call_id = %d\n",t_call_result.call_id);
        if(t_call_result.call_id == -1){
            return -1;
        }

        call_id = t_call_result.call_id;

        //get dev name
        ret = QL_MCM_DATA_GetDeviceName(h_data, call_id, dev_name, 13);
        LOG_ARG("QL_MCM_DATA_GetDeviceName ret = %d with dev_name=%s\n", ret, dev_name);
        if(call_id == 0){
            memcpy(s_apnid.global_ifname,VZW_CLASS2_APN_IFNAME,strlen(VZW_CLASS2_APN_IFNAME));
        }else{
            memcpy(s_apnid.global_ifname,dev_name,strlen(dev_name));
        }

        if(ret != 0){
            return -1;
        }
        //fill network ip info

        ret = QL_MCM_DATA_GetDeviceAddress(h_data, t_call_result.call_id, &t_addr_list);
        LOG_ARG("-->QL_MCM_DATA_GetDeviceAddress = %d\n",ret);
        if(ret != 0){
            return -1;
        }
        
        LOG_ARG("QL_MCM_DATA_GetDeviceAddress ret = %d, valid addr count=%d, details.....\n", ret, t_addr_list.addr_info_len);
        for(i=0; i<1; i++) {//get first ip from list
            QL_MCM_DATA_ROUTE_INFO_T            t_info = {0};

            addr = &(t_addr_list.addr_info[i]);

            ptr = addr2str(&addr->iface_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                LOG_ARG("ipaddr : %s/%d \n", ptr, addr->iface_mask);
                memcpy(t_info.ip_addr,ptr,strlen(ptr));
            }
            else {
                LOG("Failed to parse addr\n");
            }

            ptr = addr2str(&addr->gtwy_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                LOG_ARG("gateway : %s/%d \n", ptr, addr->gtwy_mask);
                memcpy(t_info.gateway,ptr,strlen(ptr));
            }

           //add route
            memcpy(t_info.iface_name,dev_name,strlen(dev_name));
            ret = QL_MCM_DATA_AddRoute(h_data, &t_info);
            LOG_ARG("QL_MCM_DATA_AddRoute ret = %d\n", ret);

           //set dns
            ptr = addr2str(&addr->dnsp_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",ptr);
                system(command);
                LOG_ARG("Primary DNS : %s command=%s\n", ptr,command);
                setIprouteforApnVZWADMIN(t_info.iface_name,ptr,NULL);
            }

            ptr = addr2str(&addr->dnss_addr_s, tmpBuf, sizeof(tmpBuf), &is_ipv6);
            if(ptr) {
                snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",ptr);
                LOG_ARG("Secondary DNS : %s command=%s\n", ptr,command);
                setIprouteforApnVZWADMIN(t_info.iface_name,ptr,NULL);
            }

        }

        if(ret != 0){
            return -1;
        }
        return ret;
    }
    //QL_MCM_DATA_Client_Deinit(&h_data);
    return ret;
}
#endif

