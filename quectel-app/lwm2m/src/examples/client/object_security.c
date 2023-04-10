/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
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
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Ville Skyttä - Please refer to git log
 *    
 *******************************************************************************/

/*
 *  Resources:
 *
 *          Name            | ID | Operations | Instances | Mandatory |  Type   |  Range  | Units |
 *  Server URI              |  0 |            |  Single   |    Yes    | String  |         |       |
 *  Bootstrap Server        |  1 |            |  Single   |    Yes    | Boolean |         |       |
 *  Security Mode           |  2 |            |  Single   |    Yes    | Integer |   0-3   |       |
 *  Public Key or ID        |  3 |            |  Single   |    Yes    | Opaque  |         |       |
 *  Server Public Key or ID |  4 |            |  Single   |    Yes    | Opaque  |         |       |
 *  Secret Key              |  5 |            |  Single   |    Yes    | Opaque  |         |       |
 *  SMS Security Mode       |  6 |            |  Single   |    No     | Integer |  0-255  |       |
 *  SMS Binding Key Param.  |  7 |            |  Single   |    No     | Opaque  |   6 B   |       |
 *  SMS Binding Secret Keys |  8 |            |  Single   |    No     | Opaque  | 32-48 B |       |
 *  Server SMS Number       |  9 |            |  Single   |    No     | String  |         |       |
 *  Short Server ID         | 10 |            |  Single   |    No     | Integer | 1-65535 |       |
 *  Client Hold Off Time    | 11 |            |  Single   |    No     | Integer |         |   s   |
 *  BS Account Timeout      | 12 |            |  Single   |    No     | Integer |         |   s   |
 *
 */

/*
 * Here we implement a very basic LWM2M Security Object which only knows NoSec security mode.
 */

#define LOG_TAG "Mango-client/object_security.c"
#include "lwm2m_android_log.h"
#include "call_stack.h"
#include "object_security.h"
#include "liblwm2m.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include "internals.h"
#include "lwm2m_configure.h"
#include "j_aes.h"
#include "quectel_getinfo.h"

//add by joe start
#define MAX_KEY_LEN 128
#define MAX_URI_LEN 128
#define PERSISTENCE_MAGIC_WORD_SIZE 16

#define MAX_CERT_NAME_LEN    64 
#define HOLD_OFF_TIMER_DEFAULT 10
#define MAX_SMS_NUMBER 32

#define MAX_CUST_30000_RES_INSTANCES 10

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) \
        (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif
//add by joe end

/* joe move to .h file
typedef struct _security_instance_
{
    struct _security_instance_ * next;        // matches lwm2m_list_t::next
    uint16_t                     instanceId;  // matches lwm2m_list_t::id
    char *                       uri;
    bool                         isBootstrap;    
    uint8_t                      securityMode;
    char *                       publicIdentity;
    uint16_t                     publicIdLen;
    char *                       serverPublicKey;
    uint16_t                     serverPublicKeyLen;
    char *                       secretKey;
    uint16_t                     secretKeyLen;
    uint8_t                      smsSecurityMode;
    char *                       smsParams; // SMS binding key parameters
    uint16_t                     smsParamsLen;
    char *                       smsSecret; // SMS binding secret key
    uint16_t                     smsSecretLen;
    uint16_t                     shortID;
    uint32_t                     clientHoldOffTime;
    uint32_t                     bootstrapServerAccountTimeout;
    char *                       sms_number;
#if defined (LWM2M_BOOTSTRAP)
    resource_instance_int_list_t*    custom30000List;
#endif  
    cert_source_enum             cert_source;
} security_instance_t;
*/

//add by joe start
typedef struct _security_persistent_info_
{

  char            magicWord[PERSISTENCE_MAGIC_WORD_SIZE];
  char            clientCertFName[MAX_CERT_NAME_LEN];
  char            serverCertFName[MAX_CERT_NAME_LEN];
  char            uri[MAX_URI_LEN];
  uint16_t        instanceId; 
  uint16_t        shortID;
  uint16_t        secretKeyLen;
  uint16_t        serverPublicKeyLen;
  uint16_t        publicIdLen;
  uint16_t        uriLen;
  bool            isBootstrap;
  bool            isValid;
  uint8_t         securityMode;
  uint16_t        clientHoldOffTime;
  char            sms_number[MAX_SMS_NUMBER];
  char            sms_number_len;
  pers_resource_instance_int_list_t custom30000List[MAX_CUST_30000_RES_INSTANCES];
  cert_source_enum	cert_source;

} security_persistent_info_t;

char g_lwm2m_persstence_magic_word[PERSISTENCE_MAGIC_WORD_SIZE] =
{
    0x4d, 0x32, 0x4d, 0x5f, 0x50, 0x72, 0x5f, 0x53, 0x4f, 0x62, 0x6a, 0x5f, 0x56, 0x31, 0x32, 0x00
};

extern char VZW_100_Server_url[256];
extern char VZW_101_Server_url[256];
extern char VZW_102_Server_url[256];
extern char VZW_1000_Server_url[256];

void purge_persistent_values()
{
  unlink(SECURITY_PERSISTENCE_FILE);
  unlink(SERVER_PERSISTENCE_FILE);
  unlink(ACL_PERSISTENCE_FILE);
  unlink(DEVICE_PERSISTENCE_FILE);
  unlink(FIRMWARE_PERSISTENCE_FILE);
  unlink(REGISTRATION_STATUS_PERSISTENCE_FILE);
  unlink(LWM2M_PERSITENT_OBSERVATION_STORAGE);

  unlink(LWM2M_LOCATION_OBJ_PERSISTENCE_PATH);
  unlink(LWM2M_DEVICE_OBJ_PERSISTENCE_PATH);
  unlink(LWM2M_CONN_MON_OBJ_PERSISTENCE_PATH);
  unlink(LWM2M_CONN_STAT_OBJ_PERSISTENCE_PATH);
  unlink(LWM2M_FIRMWARE_OBJ_PERSISTENCE_PATH);
  
  unlink(LWM2M_REGISTERED_EXTOBJ_PERSISTENCE_FILE);
}

int security_url_store(char* serverUri, uint16_t shortID)
{
  int ret = 0;
  int fd = 0; 
  struct stat stat_buf;

  //int offset = 0;
  //int buf_size = 0;
  //int i = 0, id_len = 0;

  char cert_fname_bin[MAX_CERT_NAME_LEN];

  if(serverUri == NULL )
    return -1;

    memset(cert_fname_bin, 0, MAX_CERT_NAME_LEN);
    snprintf((char *)cert_fname_bin, MAX_CERT_NAME_LEN, "/data/lwm2m/%u_url_server.url", shortID);
    memset(&stat_buf, 0, sizeof( struct stat));
    if(0 == lstat(cert_fname_bin,&stat_buf))
    {
        unlink(cert_fname_bin);
    }

  fd = open(cert_fname_bin, O_CREAT | O_WRONLY|O_TRUNC,0777);
  
  if (EFAILURE != fd)
  {
    write(fd, serverUri, strlen(serverUri)*sizeof(unsigned char)); //modify by joe
    close(fd);
  }

  switch(shortID){
    case 100:
        memcpy(VZW_100_Server_url,serverUri,strlen(serverUri));
        LOG_ARG("Object security set VZW_100_Server_url = %s\n",VZW_100_Server_url);
        break;
    
    case 101:
        memcpy(VZW_101_Server_url,serverUri,strlen(serverUri));
        LOG_ARG("Object security set VZW_101_Server_url = %s\n",VZW_101_Server_url);
        break;    
    
    case 102:
        memcpy(VZW_102_Server_url,serverUri,strlen(serverUri));
        LOG_ARG("Object security set VZW_102_Server_url = %s\n",VZW_102_Server_url);
        break; 

    case 1000:
        memcpy(VZW_1000_Server_url,serverUri,strlen(serverUri));
        LOG_ARG("Object security set VZW_1000_Server_url = %s\n",VZW_1000_Server_url);
        break; 

    default:
        break;
        
  }
  LOG_ARG("Object security save serverUrl = %s\n",serverUri);
  return ret;
}

int security_psk_key_store(security_instance_t *instance, uint16_t shortID)
{
  int ret = 0;
  int fd = 0; 
  struct stat stat_buf;
  ssl_psk_struct_t cert_info;
	
  void *ssl_cert_buf  = NULL;
  void *ssl_key_buf    = NULL;

  size_t ssl_cert_buf_size = 0;
  size_t ssl_key_buf_size   = 0;

  int offset = 0;
  int buf_size = 0;
  int i = 0, id_len = 0;

  char cert_fname_bin[MAX_CERT_NAME_LEN];

  if(instance == NULL )
    return -1;

  memset(&cert_info, 0x00, sizeof(ssl_psk_struct_t));

  if (instance->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY &&
  	   ((instance->publicIdLen > 0) && (instance->secretKeyLen > 0) && 
       (instance->publicIdentity != NULL) && (instance->secretKey  != NULL)))
  { 
  	if(true == lwm2m_check_is_att_netwrok())
  	{
		return 0;
  	}
    memset(cert_fname_bin, 0, MAX_CERT_NAME_LEN);
    snprintf((char *)cert_fname_bin, MAX_CERT_NAME_LEN, "/data/lwm2m/%u_server.psk", shortID);
	memset(&stat_buf, 0, sizeof( struct stat));
	if(0 == lstat(cert_fname_bin,&stat_buf))
    {
		unlink(cert_fname_bin);
	}

    /* Handling PSK Mode*/
    buf_size = instance->publicIdLen + (instance->secretKeyLen *2) + 4;

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] buf_size=%d", buf_size);
	
    cert_info.psk_Buf = (uint8_t *)lwm2m_malloc(buf_size);
    if( NULL == cert_info.psk_Buf) 
	  	return -1;

	memset(cert_info.psk_Buf,0,buf_size);
	/*
	store format:
	|psk_id|:psk_key
	*/
    id_len = snprintf((char *)cert_info.psk_Buf, (buf_size),"%s%s%s%s", "|", instance->publicIdentity, "|",":") ;
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] id_len=%d", id_len);
    /* Copying 2byte into psk_data_buf buffer */
    for(i = 0; i < instance->secretKeyLen; i++)
    {
        offset += snprintf((char *)(cert_info.psk_Buf + id_len + offset), ((buf_size) - id_len - offset),"%02x", instance->secretKey[i]) ;
    }
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] offset=%d", offset);
    cert_info.psk_Buf[id_len+offset] = '\n';
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] cert_info.psk_Buf=%s", (char *)(cert_info.psk_Buf));
    cert_info.psk_Size = id_len+offset;
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] cert_info.psk_Size=%d", cert_info.psk_Size);
  }
  else 
  {
    return -1;
  }
    
  fd = open(cert_fname_bin, O_CREAT | O_WRONLY|O_TRUNC,0777);
  
  if (EFAILURE != fd)
  {
    write(fd, cert_info.psk_Buf, cert_info.psk_Size*sizeof(unsigned char)); //modify by joe
  	close(fd);
  }
  
  if(cert_info.psk_Buf != NULL)
  {
      lwm2m_free(cert_info.psk_Buf);
      cert_info.psk_Buf = NULL;
      cert_info.psk_Size = 0;
  }
  
  return ret;
}

int security_psk_key_load(security_instance_t *instance, uint16_t shortID)
{
	char cert_fname_bin[MAX_CERT_NAME_LEN];
	bool ret = 0;
 	void  *psk_buf = NULL;
	size_t psk_buf_size   = 0;
  	char *psk_id_end_ptr = NULL;
	int psk_key_len = 0;
	char *psk_key_str = NULL;

	memset(cert_fname_bin, 0, MAX_CERT_NAME_LEN);
    snprintf((char *)cert_fname_bin, MAX_CERT_NAME_LEN,"/data/lwm2m/%u_server.psk", shortID);
    LOG_ARG("load security_psk_key_load shortID =%d\n", shortID);
	if(lwm2m_read_from_efs_file(cert_fname_bin, &psk_buf,&psk_buf_size) != 0)
	{
		if(psk_buf != NULL)
			lwm2m_free(psk_buf);
        LOG("load lwm2m_read_from_efs_file return -1\n");
		return -1;
	}
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk_buf_size =%d", psk_buf_size );
    LOG_ARG("load security_psk_key_load psk_buf_size =%d\n", psk_buf_size);
	if(psk_buf != NULL)
	   //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk_buf =%s", psk_buf );
        LOG_ARG("load security_psk_key_load psk_buf =%s\n", psk_buf);
	/*
	store format:
	|psk_id|:psk_key
	*/
	psk_id_end_ptr = (char*)(strchr((char *)psk_buf+1, '|'));
	
	if(psk_id_end_ptr == NULL)
	{
		if(psk_buf != NULL)
			lwm2m_free(psk_buf);
		return -1;
	}
	
	instance->publicIdLen = psk_id_end_ptr - ((char *)psk_buf+1);

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] instance->publicIdLen =%d", instance->publicIdLen );
    LOG_ARG("load security_psk_key_load instance->publicIdLen =%d\n", instance->publicIdLen);
	instance->publicIdentity = (char *)lwm2m_malloc(instance->publicIdLen +1);
    if (instance->publicIdentity  == NULL)
    {
       if(psk_buf != NULL)
		  lwm2m_free(psk_buf);
       return -1;
    }
    memset(instance->publicIdentity, 0, instance->publicIdLen + 1);
    memscpy(instance->publicIdentity, instance->publicIdLen +1, (char*)psk_buf+1, instance->publicIdLen);

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] instance->publicIdentity=%s", instance->publicIdentity);
    LOG_ARG("load security_psk_key_load instance->publicIdentity=%s\n", instance->publicIdentity);
	psk_key_len = psk_buf_size-instance->publicIdLen-3;

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk_key_len=%d", psk_key_len);
    LOG_ARG("load security_psk_key_load psk_key_len=%d\n", psk_key_len);
	psk_key_str = lwm2m_malloc(psk_key_len+1);

	memset(psk_key_str, 0, psk_key_len+1);
	memscpy(psk_key_str, psk_key_len +1, (char*)psk_id_end_ptr+2, psk_key_len);

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk_key_str=%s", psk_key_str);
    fprintf(stderr, "load security_psk_key_load psk_key_str=%s\n", psk_key_str);
	instance->secretKeyLen = 64;
	
	ret = lwm2m_string2hex(psk_key_str, &instance->secretKey,(int *)&instance->secretKeyLen);
	lwm2m_free(psk_key_str);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] ret=%d", ret);
    LOG_ARG("load security_psk_key_load ret=%d\n", ret);
	if(ret == false){
		lwm2m_free(instance->publicIdentity);
		if(psk_buf != NULL)
			lwm2m_free(psk_buf);
		return -1;
	}else{
		//test code
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] instance->secretKeyLen=%d", instance->secretKeyLen);
        LOG_ARG("load security_psk_key_load instance->secretKeyLen=%d\n", instance->secretKeyLen);
		if(instance->secretKeyLen %8 == 0){
		  	int i = 0;
			for(i = 0; i < instance->secretKeyLen; i+=8){
				//MSG_SPRINTF_8(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] psk key:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x", 
				//	instance->secretKey[i],instance->secretKey[i+1],instance->secretKey[i+2],instance->secretKey[i+3],
				//	instance->secretKey[i+4],instance->secretKey[i+5],instance->secretKey[i+6],instance->secretKey[i+7]);
				LOG_ARG("load security_psk_key_load [LWM2M]psk key:%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x\n", 
				instance->secretKey[i],instance->secretKey[i+1],instance->secretKey[i+2],instance->secretKey[i+3],
				instance->secretKey[i+4],instance->secretKey[i+5],instance->secretKey[i+6],instance->secretKey[i+7]);
			}
		}
	}
	if(psk_buf != NULL)
		lwm2m_free(psk_buf);
	return 0;
}

int load_security_persistent_info(lwm2m_object_t *secObjInstP)
{
  int fd = 0 , ret = 0; 

  security_instance_t *targetP = NULL;
  security_persistent_info_t info;
#if defined (LWM2M_BOOTSTRAP)
  resource_instance_int_list_t *resList = NULL;
  int index = 0;
#endif

  if (NULL == secObjInstP)
  {
    return EFAILURE;
  }

  fd = open(SECURITY_PERSISTENCE_FILE, O_RDONLY);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }
  clean_security_object(secObjInstP);//call by joe to delete auto create security object first

  while (1)
  {
    memset(&info, 0x00, sizeof(info));
    targetP = NULL;

    ret = read(fd, &info, sizeof(info));
    if (ret <= 0)
    {
      break;
    }
	
	/*Check if the structure entry in file is valid*/
	if (LWM2M_FALSE == info.isValid)
    {
      continue;
    }

    /*Added new magic word with file version. If the version mismatch, then it would do the factory reset. 
      This option would take care of previous files compatibitly issue */
    if (strncmp(info.magicWord, g_lwm2m_persstence_magic_word,  PERSISTENCE_MAGIC_WORD_SIZE) != 0) {
      clean_security_object(secObjInstP);
      close(fd);
      purge_persistent_values();
      return EFAILURE;
    }

    targetP = lwm2m_malloc(sizeof(security_instance_t));
    if (NULL == targetP)
    {
      ret = -1;
      break;
    }
	memset(targetP, 0, sizeof(security_instance_t));
    //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] info.uriLen=%d", info.uriLen);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] info.instanceId=%d", info.instanceId);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] info.sms_number_len=%d", info.sms_number_len);
	
    LOG_ARG("load_security_persistent_info info.uriLen=%d\n", info.uriLen);
    LOG_ARG("load_security_persistent_info info.instanceId=%d\n", info.instanceId);
    LOG_ARG("load_security_persistent_info info.sms_number_len=%d\n", info.sms_number_len);
    targetP->instanceId = info.instanceId;
    targetP->shortID = info.shortID;
    targetP->isBootstrap = info.isBootstrap;
    targetP->securityMode = info.securityMode;
	targetP->cert_source = info.cert_source;
	
    targetP->clientHoldOffTime = info.clientHoldOffTime;
    LOG_ARG("load_security_persistent_info  targetP->shortID=%d\n", targetP->shortID);
    LOG_ARG("load_security_persistent_info  targetP->securityMode=%d\n", targetP->securityMode);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->shortID=%d", targetP->shortID);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->securityMode=%d", targetP->securityMode);
	
    if (0 != info.uriLen)
    {
      targetP->uri = lwm2m_malloc(info.uriLen + 1);
      if (NULL == targetP->uri)
      {
        ret = -1;
        break;
      }
	  memset(targetP->uri, 0, info.uriLen+1);
      strlcpy(targetP->uri, info.uri, info.uriLen+1);
	  //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->uri=%s", targetP->uri);
      LOG_ARG("load_security_persistent_info targetP->uri=%s\n", targetP->uri);
    }

    if (0 != info.sms_number_len)
    {
      targetP->sms_number = lwm2m_malloc(info.sms_number_len + 1);
      if (NULL == targetP->sms_number)
      {
        ret = -1;
        break;
      }
      memset(targetP->sms_number, 0, info.sms_number_len+1);
      strlcpy(targetP->sms_number, info.sms_number, info.sms_number_len+1);
    }

 	if(targetP->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY)
	{
		if(security_psk_key_load(targetP, targetP->shortID) != 0){
			ret = -1;
			break;
		}
		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->publicIdentity=%s", targetP->publicIdentity);
        LOG_ARG("load_security_persistent_info targetP->publicIdentity=%s\n", targetP->publicIdentity);
  		//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->secretKey=%s", targetP->secretKey);
	}

#if defined (LWM2M_BOOTSTRAP)
    for (index = 0; index < MAX_CUST_30000_RES_INSTANCES; index++)
    {
        resList = NULL;
        if (LWM2M_FALSE == info.custom30000List[index].isValid)
          continue;
        resList = lwm2m_malloc(sizeof(resource_instance_int_list_t));
        if (NULL == resList)
        {
          ret = -1;
          break;
        }
		
		memset(resList,0,sizeof(resource_instance_int_list_t));
		
        resList->resInstId = info.custom30000List[index].resInstId;
        resList->InstValue= info.custom30000List[index].InstValue;
        targetP->custom30000List = (resource_instance_int_list_t *)LWM2M_LIST_ADD(targetP->custom30000List, resList);
        LOG_ARG("load_security_persistent_info resList->resInstId=%d resList->InstValue=%d\n", resList->resInstId,resList->InstValue);

    }
#endif


    secObjInstP->instanceList = LWM2M_LIST_ADD(secObjInstP->instanceList, targetP);
   }
  //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] ret=%d", ret);
  LOG_ARG("load_security_persistent_info [LWM2M] ret=%d\n", ret);
  if (ret < 0)
  {
    clean_security_object(secObjInstP);
    close(fd);
    return EFAILURE;
  }
  close(fd);
  
   
  return ESUCCESS;
}

int store_security_persistent_info(security_instance_t *secInstanceP, bool store)
{
  int fd = 0; 
  security_persistent_info_t info;
#if defined (LWM2M_BOOTSTRAP)
  int index = 0;
  resource_instance_int_list_t *resList;
#endif 

  if(!(secInstanceP->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY 
	   ||secInstanceP->securityMode == LWM2M_SECURITY_MODE_NONE ))
  {
  	 return EFAILURE;
  }
  if (NULL == secInstanceP)
  {
    return EFAILURE;
  }

  fd = open(SECURITY_PERSISTENCE_FILE, O_CREAT | O_WRONLY, 00644);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }

  memset(&info, 0x00, sizeof(info));

  /*Coping the secure word with version# for handling pervious/future version of the same file */
  strlcpy(info.magicWord, g_lwm2m_persstence_magic_word,  PERSISTENCE_MAGIC_WORD_SIZE);

  if (NULL != secInstanceP->uri)
  {
    strlcpy(info.uri, secInstanceP->uri, MAX_URI_LEN);
    info.uriLen = strlen(secInstanceP->uri);
  }

  if (NULL != secInstanceP->sms_number)
  {
    strlcpy(info.sms_number, secInstanceP->sms_number, MAX_SMS_NUMBER);
    info.sms_number_len = strlen(secInstanceP->sms_number);
  }
   /* Storing the PSK or certificates into secure location and remove the data
     from the structue. */
  if(secInstanceP->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY)
  {
  	   if((NULL != secInstanceP->secretKey) && (secInstanceP->secretKeyLen > 0) && 
           (NULL != secInstanceP->publicIdentity)  && (secInstanceP->publicIdLen > 0))
  	   	{
  	   		if(security_psk_key_store(secInstanceP, secInstanceP->shortID) == 0)
	        {
	        	if(secInstanceP->cert_source != CERT_SOURCE_NETWORK)
				{	
					/*
		           if((secInstanceP->publicIdentity != NULL) && (secInstanceP->publicIdLen > 0) && 
				   	  (secInstanceP->secretKey != NULL)      && (secInstanceP->secretKeyLen > 0))
		           {
					 secInstanceP->cert_source = CERT_SOURCE_NETWORK;
		           }
				   else
				   {
					 secInstanceP->cert_source = CERT_SOURCE_PRELOAD;	 
				   }*/
				   secInstanceP->cert_source = CERT_SOURCE_NETWORK;
				 }
				 info.secretKeyLen = secInstanceP->secretKeyLen;
				 info.publicIdLen= secInstanceP->publicIdLen;
	        }
      } 
  }

  info.instanceId = secInstanceP->instanceId;
  //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] info.instanceId=%d", info.instanceId);
  fprintf(stderr, "store_security_persistent_info [LWM2M] info.instanceId=%d\n", info.instanceId);
  info.shortID = secInstanceP->shortID;
  info.isBootstrap = secInstanceP->isBootstrap;
  info.securityMode = secInstanceP->securityMode;
  info.cert_source  = secInstanceP->cert_source;
#if defined (LWM2M_BOOTSTRAP)
  resList = secInstanceP->custom30000List;
  while (resList && index < MAX_CUST_30000_RES_INSTANCES)
  {
      info.custom30000List[index].isValid = LWM2M_TRUE;
      info.custom30000List[index].resInstId = resList->resInstId;
      info.custom30000List[index].InstValue = resList->InstValue;
      resList = resList->next;
	  index++;
  }
  while (index < MAX_CUST_30000_RES_INSTANCES)
  {
     info.custom30000List[index++].isValid = LWM2M_FALSE;
  }
#endif
  info.clientHoldOffTime = secInstanceP->clientHoldOffTime; 

  if(store)
  {
    info.isValid = LWM2M_TRUE; 
  }
  else
  {
    info.isValid = LWM2M_FALSE; 
  }

  lseek(fd, info.instanceId * sizeof(info), SEEK_SET);
  if (EFAILURE == write(fd, &info, sizeof(info)))
  {
    close(fd);
    return EFAILURE;
  }
  close(fd);
  return ESUCCESS; 
}

#define PSKID_MAX_SIZE 64
#define PSK_MAX_SIZE 128

void create_diagnostic_security_instance_for_vzw(security_instance_t * targetP){

    char* serverUri = "coaps://diag.lwm2m.vzwdm.com:5684";
    
    memset(targetP, 0, sizeof(security_instance_t));
    targetP->instanceId = LWM2M_VZW_DIAGNOSTIC_SERVER_INSTANCEID;//vzw Diagnostic server instance ID set to 2
    targetP->uri = (char*)lwm2m_malloc(strlen(serverUri)+1); 
    strcpy(targetP->uri, serverUri);
    
    targetP->securityMode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY;

    //get psk id
    char  imei[PSKID_MAX_SIZE] = {0};
    char msisdn[64] = {0};
    QL_LWM2M_GetImei(&imei);
    LOG_ARG("diagnostic_security GetDeviceImei(imei)=%s strlen(imei)=%d\n", imei,strlen(imei));
    targetP->publicIdentity = (char*)lwm2m_malloc(strlen(imei));
    if (targetP->publicIdentity == NULL)
    {
        return -1;
    }
    memset(targetP->publicIdentity,0,strlen(imei));
    //memcpy(targetP->publicIdentity,imei,strlen(imei));
    strcpy(targetP->publicIdentity,imei);
    LOG_ARG("diagnostic_security targetP->publicIdentity=%s\n", targetP->publicIdentity);
    targetP->publicIdLen = strlen(targetP->publicIdentity);
    LOG_ARG("diagnostic_security targetP->publicIdLen=%d\n", targetP->publicIdLen);

    //get psk
    char psk_str[PSKID_MAX_SIZE] = {0};
    uint16_t pskLen = 64;//-1;
    char * pskBuffer = NULL;
    snprintf(psk_str, PSKID_MAX_SIZE, "%s%s", imei,"101");
    LOG_ARG("diagnostic_security psk_str=%s\n", psk_str);
    //char *str_diagmostic_psk = "a72beffb36692ed9e29fb9c4743c33f424893f525095ec2f94352c00abf9527d";
    char *str_diagmostic_psk[64] = {0};

   // StrSHA256("862061049670002101", strlen("862061049670002101"), str_diagmostic_psk);
    StrSHA256(psk_str, strlen(psk_str), str_diagmostic_psk);
    LOG_ARG("diagnostic_security StrSHA256 str_diagmostic_psk=%s\n", str_diagmostic_psk);
    //LOG_ARG("diagnostic_security by StrSHA256 targetP->secretKey=%s\n", targetP->secretKey);
    if (strlen(str_diagmostic_psk) > 0){
           pskLen = strlen(str_diagmostic_psk) / 2;
           pskBuffer = malloc(pskLen);
    
           if (NULL == pskBuffer)
           {
               fprintf(stderr, "Failed to create PSK binary buffer\r\n");
               return -1;
           }
           // Hex string to binary
           char *h = str_diagmostic_psk;
           char *b = pskBuffer;
           char xlate[] = "0123456789ABCDEF";
    
           for ( ; *h; h += 2, ++b)
           {
               char *l = strchr(xlate, toupper(*h));
               char *r = strchr(xlate, toupper(*(h+1)));
    
               if (!r || !l)
               {
                   fprintf(stderr, "Failed to parse Pre-Shared-Key HEXSTRING\r\n");
                   return -1;
               }
    
               *b = ((l - xlate) << 4) + (r - xlate);
           }
    }
    
    targetP->secretKey = (char*)lwm2m_malloc(pskLen);
    if (targetP->secretKey == NULL)
    {
        return -1;
    }

    memset(targetP->secretKey,0,pskLen);
    memcpy(targetP->secretKey,pskBuffer,pskLen);
    free(pskBuffer);
    LOG_ARG("diagnostic_security default set targetP->secretKey=%s\n", targetP->secretKey);
    targetP->secretKeyLen = strlen(str_diagmostic_psk)/2;//strlen(targetP->secretKey);
    LOG_ARG("diagnostic_security targetP->secretKeyLen=%d\n", targetP->secretKeyLen);
    targetP->isBootstrap = false;
    targetP->shortID = LWM2M_VZW_DIAGNOSTIC_SHORT_SERVERID;
    targetP->clientHoldOffTime = 10;

    //sms info
    #if 0
    char *Sms_number_info = "900060005026";
    LOG_ARG("diagnostic_security strlen(Sms_number_info)=%d\n", strlen(Sms_number_info));
    targetP->sms_number = (char*)lwm2m_malloc(strlen(Sms_number_info));
    if (targetP->sms_number == NULL)
    {
        return -1;
    }
    memset(targetP->sms_number,0,strlen(Sms_number_info));
    memcpy(targetP->sms_number,"900060005026",strlen(Sms_number_info));
    LOG_ARG("diagnostic_security targetP->sms_number=%s\n", targetP->sms_number);
    #endif
}

int create_diagnostic_security_object_for_vzw(lwm2m_object_t * securityObj){
    int ret = -1;
    if(lwm2m_check_is_vzw_netwrok()){ //create Diagnostic server for vzw

        if (NULL != LWM2M_LIST_FIND(securityObj->instanceList, LWM2M_VZW_DIAGNOSTIC_SERVER_INSTANCEID)){
            ret = -1;
            LOG("vzw diagnostic security object already exist return");
            return ret;
        }
        security_instance_t * targetP_Diagnostic;
        targetP_Diagnostic = (security_instance_t *)lwm2m_malloc(sizeof(security_instance_t));
        memset(targetP_Diagnostic, 0, sizeof(targetP_Diagnostic));
        if(targetP_Diagnostic != NULL){
            create_diagnostic_security_instance_for_vzw(targetP_Diagnostic);
            securityObj->instanceList = LWM2M_LIST_ADD(securityObj->instanceList, targetP_Diagnostic);
            ret = 1;
            LOG("vzw diagnostic security object add to server object list");
            store_security_persistent_info(targetP_Diagnostic, LWM2M_TRUE);
            security_url_store(targetP_Diagnostic->uri,targetP_Diagnostic->shortID);
        }
    
    }
    return ret;
}
//add by joe end


static uint8_t prv_get_value(lwm2m_data_t * dataP,
                             security_instance_t * targetP)
{
    LOG_ARG("prv_get_value, dataP->id = %d", dataP->id);
    switch (dataP->id)
    {
    case LWM2M_SECURITY_URI_ID:
        lwm2m_data_encode_string(targetP->uri, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_BOOTSTRAP_ID:
        lwm2m_data_encode_bool(targetP->isBootstrap, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SECURITY_ID:
        lwm2m_data_encode_int(targetP->securityMode, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_PUBLIC_KEY_ID:
        lwm2m_data_encode_opaque((uint8_t*)targetP->publicIdentity, targetP->publicIdLen, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID:
        lwm2m_data_encode_opaque((uint8_t*)targetP->serverPublicKey, targetP->serverPublicKeyLen, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SECRET_KEY_ID:
		LOG_ARG("prv_get_value,(uint8_t*)targetP->secretKey = %s, targetP->secretKeyLen = %d",(uint8_t*)targetP->secretKey, targetP->secretKeyLen);
        lwm2m_data_encode_opaque((uint8_t*)targetP->secretKey, targetP->secretKeyLen, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SMS_SECURITY_ID:
        lwm2m_data_encode_int(targetP->smsSecurityMode, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SMS_KEY_PARAM_ID:
        lwm2m_data_encode_opaque((uint8_t*)targetP->smsParams, targetP->smsParamsLen, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SMS_SECRET_KEY_ID:
        lwm2m_data_encode_opaque((uint8_t*)targetP->smsSecret, targetP->smsSecretLen, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SMS_SERVER_NUMBER_ID:
        lwm2m_data_encode_int(0, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_SHORT_SERVER_ID:
        lwm2m_data_encode_int(targetP->shortID, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_HOLD_OFF_ID:
        lwm2m_data_encode_int(targetP->clientHoldOffTime, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID:
        lwm2m_data_encode_int(targetP->bootstrapServerAccountTimeout, dataP);
        return COAP_205_CONTENT;
//add by joe start
#if defined (LWM2M_BOOTSTRAP)  //add by joe
    case LWM2M_SECURITY_RESOURCE_30000 :
      {
      #if 0
        uint16_t resInstId = INVALID_RES_INST_ID; //joe copy form modem lwm2m, this is always INVALID_RES_INST_ID
         if (resInstId != INVALID_RES_INST_ID)
         {
            resource_instance_int_list_t *resInst = find_resource_inst(targetP->custom30000List,resInstId);
            if(resInst != NULL)
            {
              lwm2m_data_encode_int(resInst->InstValue, dataP);
              return COAP_205_CONTENT;
            }
            else
              return COAP_404_NOT_FOUND;
         }
         else 
        #endif
         {
            int ri = 0;
            resource_instance_int_list_t* custom30000RI = NULL;
            for (custom30000RI = targetP->custom30000List, ri=0;
                custom30000RI!=NULL;
                custom30000RI = custom30000RI->next, ri++);

            if (ri==0)  // no values!
            {
              return COAP_205_CONTENT;
            }
            else
            {
              lwm2m_data_t* subTlvP = lwm2m_data_new(ri);
              if (NULL == subTlvP) 
                return COAP_500_INTERNAL_SERVER_ERROR;
              for (custom30000RI = targetP->custom30000List, ri = 0;
                  custom30000RI!= NULL;
                  custom30000RI = custom30000RI->next, ri++)
              {
                subTlvP[ri].id = (uint16_t)custom30000RI->resInstId;

                lwm2m_data_encode_int(custom30000RI->InstValue, &subTlvP[ri]);
              }
              lwm2m_data_encode_instances(subTlvP, ri, dataP);
              return COAP_205_CONTENT;
            }
        }
     }
#endif
//add by joe end
    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_security_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP)
{
    LOG_ARG("prv_security_read, instanceId = %d", instanceId);

    //klein add
    security_instance_t *  tmpTargetP;
    int count = 0;
#if defined (LWM2M_BOOTSTRAP)//add by joe start
  int  ri = 0;
  resource_instance_int_list_t *custom30000RI;
#endif//add by joe end
    for (tmpTargetP = objectP->instanceList; tmpTargetP != NULL; tmpTargetP = tmpTargetP->next)
    {
        LOG_ARG("tmpTargetP[%d] = count, tmpTargetP->instanceId = %d, tmpTargetP->uri = %s, tmpTargetP->isBootstrap = %d", count++, tmpTargetP->instanceId, tmpTargetP->uri, tmpTargetP->isBootstrap);
    }
    ///for debug

    security_instance_t * targetP;
    uint8_t result;
    int i;

    targetP = (security_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    // is the server asking for the full instance ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {LWM2M_SECURITY_URI_ID,
                              LWM2M_SECURITY_BOOTSTRAP_ID,
                              LWM2M_SECURITY_SECURITY_ID,
                              LWM2M_SECURITY_PUBLIC_KEY_ID,
                              LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID,
                              LWM2M_SECURITY_SECRET_KEY_ID,
                              LWM2M_SECURITY_SMS_SECURITY_ID,
                              LWM2M_SECURITY_SMS_KEY_PARAM_ID,
                              LWM2M_SECURITY_SMS_SECRET_KEY_ID,
                              LWM2M_SECURITY_SMS_SERVER_NUMBER_ID,
                              LWM2M_SECURITY_SHORT_SERVER_ID,
                              LWM2M_SECURITY_HOLD_OFF_ID,
                              LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID};
        int nbRes = sizeof(resList)/sizeof(uint16_t);
#if defined (LWM2M_BOOTSTRAP)//add by joe start
            for (custom30000RI = targetP->custom30000List, ri=0;
                  custom30000RI!=NULL;
                  custom30000RI = custom30000RI->next, ri++);
        
            if (ri!=0)  // no values!
            {
               nbRes++;
            }
#endif//add by joe end

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        result = prv_get_value((*dataArrayP) + i, targetP);
        i++;
		LOG_ARG("((*dataArrayP) + i)->value.asBuffer.length = %d", ((*dataArrayP) + i)->value.asBuffer.length);

    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

#ifdef LWM2M_BOOTSTRAP

static uint8_t prv_security_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP)
{
    LOG_ARG("prv_security_write X: instanceId = %d", instanceId);

    security_instance_t * targetP;
    int i;
    uint8_t result = COAP_204_CHANGED;

    targetP = (security_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP)
    {
        return COAP_404_NOT_FOUND;
    }

    i = 0;
    do {
        switch (dataArray[i].id)
        {
        case LWM2M_SECURITY_URI_ID:
            if (targetP->uri != NULL) lwm2m_free(targetP->uri);
            targetP->uri = (char *)lwm2m_malloc(dataArray[i].value.asBuffer.length + 1);
            memset(targetP->uri, 0, dataArray[i].value.asBuffer.length + 1);
            if (targetP->uri != NULL)
            {
                strncpy(targetP->uri, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case LWM2M_SECURITY_BOOTSTRAP_ID:
            if (1 == lwm2m_data_decode_bool(dataArray + i, &(targetP->isBootstrap)))
            {
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
#if defined (LWM2M_BOOTSTRAP)//add by joe start
            if(targetP->isBootstrap == true && lwm2m_check_is_vzw_netwrok()) {
              prv_write_resource_inst_val(&(targetP->custom30000List),LWM2M_SECURITY_RESOURCE_INST_IS_HOLD_OFF_TIMER, HOLD_OFF_TIMER_DEFAULT); }
#endif//add by joe end

        case LWM2M_SECURITY_SECURITY_ID:
        {
            int64_t value;

            if (1 == lwm2m_data_decode_int(dataArray + i, &value))
            {
                if (value >= 0 && value <= 3)
                {
                    targetP->securityMode = value;
                    result = COAP_204_CHANGED;
                }
                else
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;
        case LWM2M_SECURITY_PUBLIC_KEY_ID:
            if (targetP->publicIdentity != NULL) lwm2m_free(targetP->publicIdentity);
            targetP->publicIdentity = (char *)lwm2m_malloc(dataArray[i].value.asBuffer.length +1);
            memset(targetP->publicIdentity, 0, dataArray[i].value.asBuffer.length + 1);
            if (targetP->publicIdentity != NULL)
            {
                memcpy(targetP->publicIdentity, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                targetP->publicIdLen = dataArray[i].value.asBuffer.length;
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID:
            if (targetP->serverPublicKey != NULL) lwm2m_free(targetP->serverPublicKey);
            targetP->serverPublicKey = (char *)lwm2m_malloc(dataArray[i].value.asBuffer.length +1);
            memset(targetP->serverPublicKey, 0, dataArray[i].value.asBuffer.length + 1);
            if (targetP->serverPublicKey != NULL)
            {
                memcpy(targetP->serverPublicKey, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                targetP->serverPublicKeyLen = dataArray[i].value.asBuffer.length;
				LOG_ARG("targetP->serverPublicKey = %s, targetP->serverPublicKeyLen = %d", targetP->serverPublicKey, targetP->serverPublicKeyLen);
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case LWM2M_SECURITY_SECRET_KEY_ID:
            if (targetP->secretKey != NULL) lwm2m_free(targetP->secretKey);
            targetP->secretKey = (char *)lwm2m_malloc(dataArray[i].value.asBuffer.length +1);
            memset(targetP->secretKey, 0, dataArray[i].value.asBuffer.length + 1);
            if (targetP->secretKey != NULL)
            {
                memcpy(targetP->secretKey, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                targetP->secretKeyLen = dataArray[i].value.asBuffer.length;
                LOG_ARG("targetP->secretKey = %s, targetP->secretKeyLen = %d", targetP->secretKey, targetP->secretKeyLen);
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_500_INTERNAL_SERVER_ERROR;
            }
            break;

        case LWM2M_SECURITY_SMS_SECURITY_ID:
            // Let just ignore this
            result = COAP_204_CHANGED;
            break;

        case LWM2M_SECURITY_SMS_KEY_PARAM_ID:
            // Let just ignore this
            result = COAP_204_CHANGED;
            break;

        case LWM2M_SECURITY_SMS_SECRET_KEY_ID:
            // Let just ignore this
            result = COAP_204_CHANGED;
            break;

        case LWM2M_SECURITY_SMS_SERVER_NUMBER_ID:
            // Let just ignore this
            result = COAP_204_CHANGED;
            break;

        case LWM2M_SECURITY_SHORT_SERVER_ID:
        {
            int64_t value;

            if (1 == lwm2m_data_decode_int(dataArray + i, &value))
            {
                if (value >= 0 && value <= 0xFFFF)
                {
                    targetP->shortID = value;
                    result = COAP_204_CHANGED;
                }
                else
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;

        case LWM2M_SECURITY_HOLD_OFF_ID:
        {
            int64_t value;

            if (1 == lwm2m_data_decode_int(dataArray + i, &value))
            {
                if (value >= 0 && value <= UINT32_MAX)
                {
                    targetP->clientHoldOffTime = value;
                    result = COAP_204_CHANGED;
                }
                else
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
        }

        case LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID:
        {
            int64_t value;

            if (1 == lwm2m_data_decode_int(dataArray + i, &value))
            {
                if (value >= 0 && value <= UINT32_MAX)
                {
                    targetP->bootstrapServerAccountTimeout = value;
                    result = COAP_204_CHANGED;
                }
                else
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
        }
//add by joe start
#if defined (LWM2M_BOOTSTRAP)
              case  LWM2M_SECURITY_RESOURCE_30000:
                {
                    #if 0 // by joe
                    if(resInstId != INVALID_RES_INST_ID)
                    {
                      int64_t value = 0 ; 
                      lwm2m_data_decode_int(dataArray + i, &value);
                      if (!prv_write_resource_inst_val(&(targetP->custom30000List),resInstId, value))
                      {
                        result = COAP_500_INTERNAL_SERVER_ERROR;
                      }
                      else
                      {
                        result = COAP_204_CHANGED;
                      }
                    }
                    else
                    #endif
                    {
                      if (dataArray[i].type!= LWM2M_TYPE_MULTIPLE_RESOURCE)
                      {
                        result = COAP_400_BAD_REQUEST;
                      }
                      else
                      {
                        int ri = 0;
                        lwm2m_data_t* subTlvArray = NULL;
                        int64_t value = 0;
        
                        resource_instance_int_list_t* custom30000ListSave = targetP->custom30000List;
                        targetP->custom30000List = NULL;
        
                        subTlvArray = dataArray[i].value.asChildren.array;
        
                        if (dataArray[i].value.asChildren.count == 0)
                        {
                          result = COAP_204_CHANGED;
                        }
                        else if (subTlvArray==NULL)
                        {
                          result = COAP_400_BAD_REQUEST;
                        }
                        else
                        {
                          for (ri=0; ri < dataArray[i].value.asChildren.count; ri++)
                          {
                            if (1 != lwm2m_data_decode_int(&subTlvArray[ri], &value))
                            {
                              result = COAP_400_BAD_REQUEST;
                              break;
                            }
                            else if (value < 0 || value > 0xFFFF)
                            {
                              result = COAP_400_BAD_REQUEST;
                              break;
                            }
                            else if (!prv_write_resource_inst_val(&(targetP->custom30000List),subTlvArray[ri].id, (uint16_t)value))
                            {
                              result = COAP_500_INTERNAL_SERVER_ERROR;
                              break;
                            }
                            else
                            {
                              result = COAP_204_CHANGED;
                            }
                          }
                        }
        
                        if (result != COAP_204_CHANGED)
                        {
                          // free pot. partial created new ones
                          LWM2M_LIST_FREE(targetP->custom30000List);
                          // restore old values:
                          targetP->custom30000List = custom30000ListSave;
                        }
                        else
                        {
                          // final free saved value list
                          LWM2M_LIST_FREE(custom30000ListSave);
                        }
                      }
                    }
                    break;
                }
        
#endif 
//add by joe end
        default:
            return COAP_400_BAD_REQUEST;
        }
        i++;
    } while (i < numData && result == COAP_204_CHANGED);
//add by joe start
    if (COAP_204_CHANGED == result && lwm2m_check_is_vzw_netwrok()){
        store_security_persistent_info(targetP, LWM2M_TRUE);
        security_url_store(targetP->uri,targetP->shortID);
    }
//add by joe end
    return result;
}

static uint8_t prv_security_delete(uint16_t id,
                                   lwm2m_object_t * objectP)
{

    LOG_ARG("prv_security_delete X: id = %d", id);

    security_instance_t * targetP;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&targetP);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

 //add by joe start
 if(lwm2m_check_is_vzw_netwrok())
    store_security_persistent_info(targetP,LWM2M_FALSE);
#if defined (LWM2M_BOOTSTRAP) 
 
   LWM2M_LIST_FREE(targetP->custom30000List);
 
#endif
//add by joe end
    if (NULL != targetP->uri)
    {
        lwm2m_free(targetP->uri);
    }

    lwm2m_free(targetP);

    return COAP_202_DELETED;
}

static uint8_t prv_security_create(uint16_t instanceId,
                                   int numData,
                                   lwm2m_data_t * dataArray,
                                   lwm2m_object_t * objectP)
{

    LOG_ARG("prv_security_create X: instanceId = %d", instanceId);
    //dump_stack("mango");
    security_instance_t * targetP;
    uint8_t result;

    targetP = (security_instance_t *)lwm2m_malloc(sizeof(security_instance_t));
    if (NULL == targetP) return COAP_500_INTERNAL_SERVER_ERROR;
    memset(targetP, 0, sizeof(security_instance_t));

    targetP->instanceId = instanceId;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, targetP);

    result = prv_security_write(instanceId, numData, dataArray, objectP);

    if (result != COAP_204_CHANGED)
    {
        (void)prv_security_delete(instanceId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }

    //add by joe for vzw start
    create_diagnostic_security_object_for_vzw(objectP);
    //add by joe for vzw end

    return result;
}
#endif

void copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc)
{

    LOG("copy_security_object X");  
    memcpy(objectDest, objectSrc, sizeof(lwm2m_object_t));
    objectDest->instanceList = NULL;
    objectDest->userData = NULL;
    security_instance_t * instanceSrc = (security_instance_t *)objectSrc->instanceList;
    security_instance_t * previousInstanceDest = NULL;
    while (instanceSrc != NULL)
    {
        security_instance_t * instanceDest = (security_instance_t *)lwm2m_malloc(sizeof(security_instance_t));
        if (NULL == instanceDest)
        {
            return;
        }
        memcpy(instanceDest, instanceSrc, sizeof(security_instance_t));
        instanceDest->uri = (char*)lwm2m_malloc(strlen(instanceSrc->uri) + 1);
        strcpy(instanceDest->uri, instanceSrc->uri);
        if (instanceSrc->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY)
        {
            instanceDest->publicIdentity = lwm2m_strdup(instanceSrc->publicIdentity);
            instanceDest->secretKey = lwm2m_strdup(instanceSrc->secretKey);
        }
        instanceSrc = (security_instance_t *)instanceSrc->next;
        if (previousInstanceDest == NULL)
        {
            objectDest->instanceList = (lwm2m_list_t *)instanceDest;
        }
        else
        {
            previousInstanceDest->next = instanceDest;
        }
        previousInstanceDest = instanceDest;
    }
}

void display_security_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    fprintf(stdout, "  /%u: Security object, instances:\r\n", object->objID);
    security_instance_t * instance = (security_instance_t *)object->instanceList;
    while (instance != NULL)
    {
        fprintf(stdout, "    /%u/%u: instanceId: %u, uri: %s, isBootstrap: %s, shortId: %u, clientHoldOffTime: %u\r\n",
                object->objID, instance->instanceId,
                instance->instanceId, instance->uri, instance->isBootstrap ? "true" : "false",
                instance->shortID, instance->clientHoldOffTime);
        instance = (security_instance_t *)instance->next;
    }
#endif
}

void clean_security_object(lwm2m_object_t * objectP)
{
    LOG("clean_security_object X");  

    while (objectP->instanceList != NULL)
    {
        security_instance_t * securityInstance = (security_instance_t *)objectP->instanceList;
        objectP->instanceList = objectP->instanceList->next;
        if (NULL != securityInstance->uri)
        {
            lwm2m_free(securityInstance->uri);
        }
        if (securityInstance->securityMode == LWM2M_SECURITY_MODE_PRE_SHARED_KEY)
        {
            lwm2m_free(securityInstance->publicIdentity);
            lwm2m_free(securityInstance->secretKey);
        }
#if defined (LWM2M_BOOTSTRAP) //add by joe start
      LWM2M_LIST_FREE(securityInstance->custom30000List);
#endif      //add by joe end
        lwm2m_free(securityInstance);
    }
}

lwm2m_object_t * get_security_object(int serverId,
                                     const char* serverUri,
                                     char * bsPskId,
                                     char * psk,
                                     uint16_t pskLen,
                                     bool isBootstrap)
{
    LOG_ARG("get_security_object X: isBootstrap = %d", isBootstrap);

    lwm2m_object_t * securityObj;

    securityObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != securityObj)
    {
        security_instance_t * targetP;

        memset(securityObj, 0, sizeof(lwm2m_object_t));

        securityObj->objID = 0;

        // Manually create an hardcoded instance
        targetP = (security_instance_t *)lwm2m_malloc(sizeof(security_instance_t));
        if (NULL == targetP)
        {
            lwm2m_free(securityObj);
            return NULL;
        }

        memset(targetP, 0, sizeof(security_instance_t));
        targetP->instanceId = 0;
        targetP->uri = (char*)lwm2m_malloc(strlen(serverUri)+1); 
        strcpy(targetP->uri, serverUri);

        targetP->securityMode = LWM2M_SECURITY_MODE_NONE;
        targetP->publicIdentity = NULL;
        targetP->publicIdLen = 0;
        targetP->secretKey = NULL;
        targetP->secretKeyLen = 0;
        if (bsPskId != NULL || psk != NULL)
        {
            targetP->securityMode = LWM2M_SECURITY_MODE_PRE_SHARED_KEY;
            if (bsPskId)
            {
                targetP->publicIdentity = strdup(bsPskId);
                targetP->publicIdLen = strlen(bsPskId);
            }
            if (pskLen > 0)
            {
                targetP->secretKey = (char*)lwm2m_malloc(pskLen);
                if (targetP->secretKey == NULL)
                {
                    clean_security_object(securityObj);
                    return NULL;
                }
                memcpy(targetP->secretKey, psk, pskLen);
                targetP->secretKeyLen = pskLen;
            }
        }
        targetP->isBootstrap = isBootstrap;
        targetP->shortID = serverId;
        targetP->clientHoldOffTime = 10;

        securityObj->instanceList = LWM2M_LIST_ADD(securityObj->instanceList, targetP);

        securityObj->readFunc = prv_security_read;
#ifdef LWM2M_BOOTSTRAP
        securityObj->writeFunc = prv_security_write;
        securityObj->createFunc = prv_security_create;
        securityObj->deleteFunc = prv_security_delete;
#endif
    }
    LOG("get_security_object E");

    return securityObj;
}

char * get_server_uri(lwm2m_object_t * objectP,
                      uint16_t secObjInstID)
{
	LOG("get_server_uri X");  

    security_instance_t * targetP = (security_instance_t *)LWM2M_LIST_FIND(objectP->instanceList, secObjInstID);

    if (NULL != targetP)
    {
        return lwm2m_strdup(targetP->uri);
    }

    return NULL;
}
