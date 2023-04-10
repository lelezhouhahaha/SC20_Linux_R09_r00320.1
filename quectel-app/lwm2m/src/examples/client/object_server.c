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
 *    Julien Vermillard, Sierra Wireless
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Scott Bertin, AMETEK, Inc. - Please refer to git log
 *    
 *******************************************************************************/

/*
 *  Resources:
 *
 *          Name                       | ID | Operations | Instances | Mandatory |  Type    |  Range  | Units |
 *  Short ID                           |  0 |     R      |  Single   |    Yes    | Integer  | 1-65535 |       |
 *  Lifetime                           |  1 |    R/W     |  Single   |    Yes    | Integer  |         |   s   |
 *  Default Min Period                 |  2 |    R/W     |  Single   |    No     | Integer  |         |   s   |
 *  Default Max Period                 |  3 |    R/W     |  Single   |    No     | Integer  |         |   s   |
 *  Disable                            |  4 |     E      |  Single   |    No     |          |         |       |
 *  Disable Timeout                    |  5 |    R/W     |  Single   |    No     | Integer  |         |   s   |
 *  Notification Storing               |  6 |    R/W     |  Single   |    Yes    | Boolean  |         |       |
 *  Binding                            |  7 |    R/W     |  Single   |    Yes    | String   |         |       |
 *  Registration Update                |  8 |     E      |  Single   |    Yes    |          |         |       |
#ifndef LWM2M_VERSION_1_0
 *  Registration Priority Order        | 13 |    R/W     |  Single   |    No     | Unsigned |         |       |
 *  Initial Registration Delay Timer   | 14 |    R/W     |  Single   |    No     | Unsigned |         |   s   |
 *  Registration Failure Block         | 15 |    R/W     |  Single   |    No     | Boolean  |         |       |
 *  Bootstrap on Registration Failure  | 16 |    R/W     |  Single   |    No     | Boolean  |         |       |
 *  Communication Retry Count          | 17 |    R/W     |  Single   |    No     | Unsigned |         |       |
 *  Communication Retry Timer          | 18 |    R/W     |  Single   |    No     | Unsigned |         |   s   |
 *  Communication Sequence Delay Timer | 19 |    R/W     |  Single   |    No     | Unsigned |         |   s   |
 *  Communication Sequence Retry Count | 20 |    R/W     |  Single   |    No     | Unsigned |         |       |
#endif
 *
 */

#define LOG_TAG "Mango-client/object_server.c"
#include "lwm2m_android_log.h"
#include "call_stack.h"

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include "object_server.h"
#include "internals.h"

/* joe move to .h file
typedef struct _server_instance_
{
    struct _server_instance_ * next;   // matches lwm2m_list_t::next
    uint16_t    instanceId;            // matches lwm2m_list_t::id
    uint16_t    shortServerId;
    uint32_t    lifetime;
    uint32_t    defaultMinPeriod;
    uint32_t    defaultMaxPeriod;
    uint32_t    disableTimeout;
    bool        storing;
    char        binding[4];
#ifndef LWM2M_VERSION_1_0
    int         registrationPriorityOrder; // <0 when it doesn't exist
    int         initialRegistrationDelayTimer; // <0 when it doesn't exist
    int8_t      registrationFailureBlock; // <0 when it doesn't exist, 0 for false, > 0 for true
    int8_t      bootstrapOnRegistrationFailure; // <0 when it doesn't exist, 0 for false, > 0 for true
    int         communicationRetryCount; // <0 when it doesn't exist
    int         communicationRetryTimer; // <0 when it doesn't exist
    int         communicationSequenceDelayTimer; // <0 when it doesn't exist
    int         communicationSequenceRetryCount; // <0 when it doesn't exist
#endif
    resource_instance_int_list_t*    custom30000List;
} server_instance_t;
*/

//add by joe start
bool allow_write = false;
#define MAX_BINDING_LEN 4
#define LWM2M_SERVER_RESOURCE_INST_IS_REGISTERED                  0
#define LWM2M_SERVER_RESOURCE_INST_IS_CLIENT_HOLD_OFF_TIMER       1

#define MAX_CUST_30000_RES_INSTANCES 10

typedef struct _server_persistent_info_
{
  char        binding[MAX_BINDING_LEN];
  uint32_t    lifetime;
  uint32_t    defaultMinPeriod;
  uint32_t    defaultMaxPeriod;
  uint32_t    disableTimeout;
  uint16_t    instanceId;            // matches lwm2m_list_t::id
  uint16_t    shortServerId;
  uint16_t    bindingLen;
  bool        storing;
  bool        isValid;
  pers_resource_instance_int_list_t custom30000List[MAX_CUST_30000_RES_INSTANCES];
} server_persistent_info_t;

int load_server_persistent_info(lwm2m_object_t *serObjP)
{
  int fd = 0, ret = 0, index = 0;
  server_instance_t *targetP = NULL;
  server_persistent_info_t info;
  resource_instance_int_list_t *resList = NULL;

  if (NULL == serObjP)
  {
    return EFAILURE;
  }

  fd = open(SERVER_PERSISTENCE_FILE, 00);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }
  
  clean_server_object(serObjP);//call by joe to delete auto create server object first

  while (1)
  {
    memset(&info, 0x00, sizeof(info));
    targetP = NULL;

    ret = read(fd, &info, sizeof(info));
    if (ret <= 0)
    {
      break;
    }
    if (LWM2M_FALSE == info.isValid)
    {
      continue;
    }

    targetP = lwm2m_malloc(sizeof(server_instance_t));
    memset(targetP, 0, sizeof(server_instance_t));//add by joe
    if (NULL == targetP)
    {
      ret = -1;
      break;
    }
    //LOG_ARG("load_server_persistent_info [LWM2M]info.bindingLen: %d\n", info.bindingLen);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH, "[LWM2M]info.bindingLen: %d",info.bindingLen);
    if (0 != info.bindingLen)
    {
#ifdef FACTORY_BOOTSTRAPPING    
      targetP->binding = lwm2m_malloc(info.bindingLen + 1);
      if (NULL == targetP->binding)
      {
        ret = -1;
        break;
      }
#endif	  
      strlcpy(targetP->binding, info.binding, info.bindingLen+1);
      //LOG_ARG("load_server_persistent_info targetP->binding: %s\n", targetP->binding);
	 // MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH, "[LWM2M]targetP->binding: %s",targetP->binding);
#ifdef FACTORY_BOOTSTRAPPING  
      targetP->bindingLen = info.bindingLen;
#endif
    }
    LOG("load_server_persistent_info \n");
    targetP->lifetime = info.lifetime;
    //LOG_ARG("load_server_persistent_info info.lifetime=%d info.defaultMinPeriod=%d info.defaultMaxPeriod=%d info.instanceId=%d\n", info.lifetime, info.defaultMinPeriod, info.defaultMaxPeriod, info.instanceId);
	targetP->defaultMinPeriod = info.defaultMinPeriod;
    targetP->defaultMaxPeriod = info.defaultMaxPeriod;
    targetP->disableTimeout = info.disableTimeout;
    targetP->instanceId = info.instanceId;
    targetP->shortServerId = info.shortServerId;
    targetP->storing = info.storing;
    //LOG_ARG("load_server_persistent_info targetP->lifetime: %d\n", targetP->lifetime);
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
       // LOG_ARG("load_server_persistent_info 9 resList->resInstId=%d resList->InstValue=%d\n",resList->InstValue,resList->resInstId);
       // LOG_ARG("load_server_persistent_info 91 targetP->custom30000List=%p\n",targetP->custom30000List);

       //by joe
       targetP->custom30000List = (resource_instance_int_list_t *)LWM2M_LIST_ADD(targetP->custom30000List, resList);
        // targetP->custom30000List = (resource_instance_int_list_t *)lwm2m_reosurce_list_add(&(targetP->custom30000List), resList);
      //  LOG_ARG("load_server_persistent_info 92 targetP->custom30000List=%p\n",targetP->custom30000List);
    }
    //LOG("load_server_persistent_info 10\n");

    serObjP->instanceList = LWM2M_LIST_ADD(serObjP->instanceList, targetP);
  }
  LOG_ARG("load_server_persistent_info ret: %d\n", ret);
  if (ret < 0)
  {
    close(fd);
    clean_server_object(serObjP);
    return EFAILURE;
  }
  close(fd);
  return ESUCCESS;
}


int store_server_persistent_info(server_instance_t *targetP,  bool store)
{
  int fd = 0, index=0;
  server_persistent_info_t info;
  resource_instance_int_list_t *resList;

  if (NULL == targetP)
  {
    return EFAILURE;
  }

  fd = open(SERVER_PERSISTENCE_FILE, O_CREAT | O_WRONLY, 00644);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }

  memset(&info, 0x00, sizeof(info));
#ifdef FACTORY_BOOTSTRAPPING
  if (NULL != targetP->binding)
  {
    strlcpy(info.binding, targetP->binding, MAX_BINDING_LEN);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH, "[LWM2M]targetP->bindingLen: %d",targetP->bindingLen);
    //LOG_ARG("store_server_persistent_info [LWM2M]targetP->bindingLen: %d\n", targetP->bindingLen);
    info.bindingLen = targetP->bindingLen;
  }
#else
   strcpy(info.binding, targetP->binding);
   info.bindingLen = strlen(targetP->binding);
#endif
  info.lifetime = targetP->lifetime;
  info.defaultMinPeriod = targetP->defaultMinPeriod;
  info.defaultMaxPeriod = targetP->defaultMaxPeriod;
  info.disableTimeout = targetP->disableTimeout;
  info.instanceId = targetP->instanceId;
  info.shortServerId = targetP->shortServerId;
  info.storing = targetP->storing;
  //LOG_ARG("store_server_persistent_info info.lifetime=%d info.defaultMinPeriod=%d info.defaultMaxPeriod=%d info.instanceId=%d\n", info.lifetime, info.defaultMinPeriod, info.defaultMaxPeriod, info.instanceId);
  LOG("store_server_persistent_info");
  if(store)
  {
    info.isValid = LWM2M_TRUE; 
  }
  else
  {
    info.isValid = LWM2M_FALSE; 
  }

   resList = targetP->custom30000List;
  while (resList && index < MAX_CUST_30000_RES_INSTANCES)
  {
      info.custom30000List[index].isValid = LWM2M_TRUE;
      info.custom30000List[index].resInstId = resList->resInstId;
      info.custom30000List[index].InstValue = resList->InstValue;
      LOG_ARG("store_server_persistent_info info.custom30000List[index].resInstId=%d info.custom30000List[index].InstValue=%d",info.custom30000List[index].resInstId,info.custom30000List[index].InstValue);
      resList = resList->next;
	  index++;
  }
  while (index < MAX_CUST_30000_RES_INSTANCES)
  {
      info.custom30000List[index++].isValid = LWM2M_FALSE;
  }
  lseek(fd, info.instanceId * sizeof(info), SEEK_SET);
  if (EFAILURE == write(fd, &info, sizeof(info)))
  {
    close(fd);
    LOG("store_server_persistent_info return EFAILURE");
    return EFAILURE;
  }
  close(fd);
  LOG("store_server_persistent_info return ESUCCESS");
  return ESUCCESS;
}

int dump_server_persistent_info(lwm2m_object_t *serObjP)
{
  server_instance_t *targetP = NULL;

  if (NULL == serObjP)
  {
    return EFAILURE;
  }
  targetP = (server_instance_t *)serObjP->instanceList;
  while (targetP)
  {
    if (EFAILURE == store_server_persistent_info(targetP, LWM2M_TRUE))
    {
      return EFAILURE;
    }
    targetP = targetP->next;
  }
  LOG("dump_server_persistent_info return ESUCCESS");
  return ESUCCESS;
}


/* 
 * @fn     bool update_isRegistered_for_server()
 * @brief  This function is used to update isRegistered  resource to server object instance 
 * @param  serv_objectP - pointer to security object 
 serverObjInstID - Id of security object instance 
 * @return void     
 */ 
void update_isRegistered_for_server (lwm2m_object_t * serv_object, uint16_t ssid, uint8_t value)
{ 
  server_instance_t * targetP = NULL;
  lwm2m_list_t *temp_list;
  uint8_t inst = 0, max_inst = 0;
  if (serv_object== NULL ) 
  {
    return ;
  }
  temp_list = serv_object->instanceList;
  for(max_inst = 0;temp_list !=NULL;max_inst++)
  {
    max_inst = temp_list->id;
    temp_list = temp_list->next;
  }
  do
  {
    targetP = (server_instance_t *)LWM2M_LIST_FIND(serv_object->instanceList, inst);
    inst++;
    if (targetP == NULL)
    {
      continue;
    }
    if(targetP->shortServerId == ssid)
    {
      prv_write_resource_inst_val(&(targetP->custom30000List),LWM2M_SERVER_RESOURCE_INST_IS_REGISTERED, value);
      store_server_persistent_info(targetP, LWM2M_TRUE);
      break;
    }
  }while(inst < max_inst);
  return ;
}

void create_diagnostic_server_instance_for_vzw(server_instance_t * serverInstance){
//joe add create default diagnostic server value
    serverInstance->instanceId = LWM2M_VZW_DIAGNOSTIC_SERVER_INSTANCEID;
    serverInstance->shortServerId = LWM2M_VZW_DIAGNOSTIC_SHORT_SERVERID;
    serverInstance->lifetime = 2592000;
    serverInstance->storing = true;
    serverInstance->disableTimeout = 86400;
    serverInstance->defaultMinPeriod = 1;
    serverInstance->defaultMaxPeriod = 6000;
    memcpy (serverInstance->binding, "UQS", strlen("UQS"));
    
    resource_instance_int_list_t *resList = NULL;
    int index = 0;
    for (index = 0; index < 2; index++)//two elements
    {
        resList = NULL;

        resList = lwm2m_malloc(sizeof(resource_instance_int_list_t));
        if (NULL == resList)
        {
          return;
        }
		memset(resList,0,sizeof(resource_instance_int_list_t));
        if(index == 0){
            resList->resInstId = 0;
            resList->InstValue= 0;

        }else if(index == 1){
            resList->resInstId = 1;
            resList->InstValue= 0;
        }
       //by joe
       serverInstance->custom30000List = (resource_instance_int_list_t *)LWM2M_LIST_ADD(serverInstance->custom30000List, resList);
        // targetP->custom30000List = (resource_instance_int_list_t *)lwm2m_reosurce_list_add(&(targetP->custom30000List), resList);
      //  LOG_ARG("load_server_persistent_info 92 targetP->custom30000List=%p\n",targetP->custom30000List);
    }

#ifndef LWM2M_VERSION_1_0
    serverInstance->registrationPriorityOrder = -1;
    serverInstance->initialRegistrationDelayTimer = -1;
    serverInstance->registrationFailureBlock = -1;
    serverInstance->bootstrapOnRegistrationFailure = -1;
    serverInstance->communicationRetryCount = -1;
    serverInstance->communicationRetryTimer = -1;
    serverInstance->communicationSequenceDelayTimer = -1;
    serverInstance->communicationSequenceRetryCount = -1;
#endif

}

int create_diagnostic_server_obecjt_for_vzw(lwm2m_object_t * serverObj){
    int ret = -1;
    
    if(lwm2m_check_is_vzw_netwrok()){ //create Diagnostic server for vzw
    
        if (NULL != LWM2M_LIST_FIND(serverObj->instanceList, LWM2M_VZW_DIAGNOSTIC_SERVER_INSTANCEID)){
            ret = -1;
            LOG("vzw diagnostic server object already exist return");
            return ret;
        }
        
        server_instance_t * server_instance_t_Diagnostic = NULL;
        server_instance_t_Diagnostic = (server_instance_t *)lwm2m_malloc(sizeof(server_instance_t));
        memset(server_instance_t_Diagnostic, 0, sizeof(server_instance_t));
        if(server_instance_t_Diagnostic != NULL){
            create_diagnostic_server_instance_for_vzw(server_instance_t_Diagnostic);
            serverObj->instanceList = LWM2M_LIST_ADD(serverObj->instanceList, server_instance_t_Diagnostic);
            ret = 1;
            LOG("vzw diagnostic server add to server object list");
            
            store_server_persistent_info(server_instance_t_Diagnostic, LWM2M_TRUE);
        }
    }
    return ret;
}
//add by joe end

static uint8_t prv_get_value(lwm2m_data_t * dataP,
                             server_instance_t * targetP)
{
    switch (dataP->id)
    {
    case LWM2M_SERVER_SHORT_ID_ID:
        lwm2m_data_encode_int(targetP->shortServerId, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_LIFETIME_ID:
        lwm2m_data_encode_int(targetP->lifetime, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_MIN_PERIOD_ID:
        lwm2m_data_encode_int(targetP->defaultMinPeriod, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_MAX_PERIOD_ID:
        lwm2m_data_encode_int(targetP->defaultMaxPeriod, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_DISABLE_ID:
        return COAP_405_METHOD_NOT_ALLOWED;

    case LWM2M_SERVER_TIMEOUT_ID:
        if(targetP->disableTimeout == 0)//add by joe
          targetP->disableTimeout = LWM2M_SERVER_DEFAULT_DISABLE_TIMEOUT;
        
        lwm2m_data_encode_int(targetP->disableTimeout, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_STORING_ID:
        lwm2m_data_encode_bool(targetP->storing, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_BINDING_ID:
        lwm2m_data_encode_string(targetP->binding, dataP);
        return COAP_205_CONTENT;

    case LWM2M_SERVER_UPDATE_ID:
        return COAP_405_METHOD_NOT_ALLOWED;
    //add by joe start
    case LWM2M_SERVER_RESOURCE_30000 :
      {
            #if 0
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
            LOG_ARG("get value LWM2M_SERVER_RESOURCE_30000 ri=%d\n",ri);
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
//add by joe end
#ifndef LWM2M_VERSION_1_0
    case LWM2M_SERVER_REG_ORDER_ID:
        if (targetP->registrationPriorityOrder >= 0)
        {
            lwm2m_data_encode_uint(targetP->registrationPriorityOrder, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_INITIAL_REG_DELAY_ID:
        if (targetP->initialRegistrationDelayTimer >= 0)
        {
            lwm2m_data_encode_uint(targetP->initialRegistrationDelayTimer, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_REG_FAIL_BLOCK_ID:
        if (targetP->registrationFailureBlock >= 0)
        {
            lwm2m_data_encode_bool(targetP->registrationFailureBlock > 0, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID:
        if (targetP->bootstrapOnRegistrationFailure >= 0)
        {
            lwm2m_data_encode_bool(targetP->bootstrapOnRegistrationFailure > 0, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_COMM_RETRY_COUNT_ID:
        if (targetP->communicationRetryCount >= 0)
        {
            lwm2m_data_encode_uint(targetP->communicationRetryCount, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_COMM_RETRY_TIMER_ID:
        if (targetP->communicationRetryTimer >= 0)
        {
            lwm2m_data_encode_uint(targetP->communicationRetryTimer, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_SEQ_DELAY_TIMER_ID:
        if (targetP->communicationSequenceDelayTimer >= 0)
        {
            lwm2m_data_encode_uint(targetP->communicationSequenceDelayTimer, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

    case LWM2M_SERVER_SEQ_RETRY_COUNT_ID:
        if (targetP->communicationSequenceRetryCount >= 0)
        {
            lwm2m_data_encode_uint(targetP->communicationSequenceRetryCount, dataP);
            return COAP_205_CONTENT;
        }
        else
        {
            return COAP_404_NOT_FOUND;
        }

#endif

    default:
        return COAP_404_NOT_FOUND;
    }
}

static uint8_t prv_server_read(uint16_t instanceId,
                               int * numDataP,
                               lwm2m_data_t ** dataArrayP,
                               lwm2m_object_t * objectP)
{
    server_instance_t * targetP;
    uint8_t result;
    int i;
    int ri = 0;//add by joe
    resource_instance_int_list_t *custom30000RI;//add by joe end
    targetP = (server_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    // is the server asking for the full instance ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
            LWM2M_SERVER_SHORT_ID_ID,
            LWM2M_SERVER_LIFETIME_ID,
            LWM2M_SERVER_MIN_PERIOD_ID,
            LWM2M_SERVER_MAX_PERIOD_ID,
            LWM2M_SERVER_TIMEOUT_ID,
            LWM2M_SERVER_STORING_ID,
            LWM2M_SERVER_BINDING_ID,
#ifndef LWM2M_VERSION_1_0
            LWM2M_SERVER_REG_ORDER_ID,
            LWM2M_SERVER_INITIAL_REG_DELAY_ID,
            LWM2M_SERVER_REG_FAIL_BLOCK_ID,
            LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID,
            LWM2M_SERVER_COMM_RETRY_COUNT_ID,
            LWM2M_SERVER_COMM_RETRY_TIMER_ID,
            LWM2M_SERVER_SEQ_DELAY_TIMER_ID,
            LWM2M_SERVER_SEQ_RETRY_COUNT_ID,
#endif
        };
        int nbRes = sizeof(resList)/sizeof(uint16_t);
        int temp = nbRes;
        //add by joe start
        for (custom30000RI = targetP->custom30000List, ri=0; custom30000RI!=NULL; 
             custom30000RI = custom30000RI->next, ri++);
         if(ri!=0)
             nbRes++;

             LOG_ARG("server read LWM2M_SERVER_RESOURCE_30000 ri=%d nbRes=%d\n",ri,nbRes);
		//add by joe end
#ifndef LWM2M_VERSION_1_0
        /* Remove optional resources that don't exist */
        if(targetP->registrationPriorityOrder < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_ORDER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->initialRegistrationDelayTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_INITIAL_REG_DELAY_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->registrationFailureBlock < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_FAIL_BLOCK_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->bootstrapOnRegistrationFailure < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationRetryCount < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_COMM_RETRY_COUNT_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationRetryTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_COMM_RETRY_TIMER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationSequenceDelayTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_SEQ_DELAY_TIMER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationSequenceRetryCount < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_SEQ_RETRY_COUNT_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
#endif

        *dataArrayP = lwm2m_data_new(nbRes);

        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;

        *numDataP = nbRes;
        for (i = 0 ; i < temp ; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
        //add by joe
         if (ri!=0 )
        {
          (*dataArrayP)[temp].id = LWM2M_SERVER_RESOURCE_30000 ;
        }
         //add by joe
    }

    i = 0;
    do
    {
        result = prv_get_value((*dataArrayP) + i, targetP);
        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

static uint8_t prv_server_discover(uint16_t instanceId,
                                   int * numDataP,
                                   lwm2m_data_t ** dataArrayP,
                                   lwm2m_object_t * objectP)
{
    server_instance_t * targetP;
    uint8_t result;
    int i;
    resource_instance_int_list_t*    custResource;//add by joe
    int ri = 0;//add by joe
    result = COAP_205_CONTENT;

    targetP = (server_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

//add by joe start
    if (targetP != NULL)
    {
        for (custResource = targetP->custom30000List, ri=0;
          custResource!=NULL;
          custResource = custResource->next, ri++);
    }
    LOG_ARG("server discover LWM2M_SERVER_RESOURCE_30000 ri=%d\n",ri);
//add by joe end
    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
            LWM2M_SERVER_SHORT_ID_ID,
            LWM2M_SERVER_LIFETIME_ID,
            LWM2M_SERVER_MIN_PERIOD_ID,
            LWM2M_SERVER_MAX_PERIOD_ID,
            LWM2M_SERVER_DISABLE_ID,
            LWM2M_SERVER_TIMEOUT_ID,
            LWM2M_SERVER_STORING_ID,
            LWM2M_SERVER_BINDING_ID,
            LWM2M_SERVER_UPDATE_ID,
#ifndef LWM2M_VERSION_1_0
            LWM2M_SERVER_REG_ORDER_ID,
            LWM2M_SERVER_INITIAL_REG_DELAY_ID,
            LWM2M_SERVER_REG_FAIL_BLOCK_ID,
            LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID,
            LWM2M_SERVER_COMM_RETRY_COUNT_ID,
            LWM2M_SERVER_COMM_RETRY_TIMER_ID,
            LWM2M_SERVER_SEQ_DELAY_TIMER_ID,
            LWM2M_SERVER_SEQ_RETRY_COUNT_ID,
#endif
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

#ifndef LWM2M_VERSION_1_0
        /* Remove optional resources that don't exist */
        if(targetP->registrationPriorityOrder < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_ORDER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->initialRegistrationDelayTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_INITIAL_REG_DELAY_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->registrationFailureBlock < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_FAIL_BLOCK_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->bootstrapOnRegistrationFailure < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationRetryCount < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_COMM_RETRY_COUNT_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationRetryTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_COMM_RETRY_TIMER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationSequenceDelayTimer < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_SEQ_DELAY_TIMER_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
        if(targetP->communicationSequenceRetryCount < 0)
        {
            for (i=0; i < nbRes; i++)
            {
                if (resList[i] == LWM2M_SERVER_SEQ_RETRY_COUNT_ID)
                {
                    nbRes -= 1;
                    memmove(&resList[i], &resList[i+1], (nbRes-i)*sizeof(resList[i]));
                    break;
                }
            }
        }
#endif

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        {
            (*dataArrayP)[i].id = resList[i];
        }
        //add by joe start
    if (ri != 0)  // no values!
    {
      (*dataArrayP)[nbRes].id = LWM2M_SERVER_RESOURCE_30000;
      (*dataArrayP)[nbRes].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
      (*dataArrayP)[i].value.asChildren.count = ri;
    }
	//add by joe end
    }
	
    else
    {
        for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
        {
            switch ((*dataArrayP)[i].id)
            {
            //add by joe start
             case LWM2M_SERVER_RESOURCE_30000:
            {
               (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
               (*dataArrayP)[i].value.asChildren.count = ri;
                break;
            }
			//add by joe end
            case LWM2M_SERVER_SHORT_ID_ID:
            case LWM2M_SERVER_LIFETIME_ID:
            case LWM2M_SERVER_MIN_PERIOD_ID:
            case LWM2M_SERVER_MAX_PERIOD_ID:
            case LWM2M_SERVER_DISABLE_ID:
            case LWM2M_SERVER_TIMEOUT_ID:
            case LWM2M_SERVER_STORING_ID:
            case LWM2M_SERVER_BINDING_ID:
            case LWM2M_SERVER_UPDATE_ID:
                break;
#ifndef LWM2M_VERSION_1_0
            case LWM2M_SERVER_REG_ORDER_ID:
                if(targetP->registrationPriorityOrder < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_INITIAL_REG_DELAY_ID:
                if(targetP->initialRegistrationDelayTimer < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_REG_FAIL_BLOCK_ID:
                if(targetP->registrationFailureBlock < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID:
                if(targetP->bootstrapOnRegistrationFailure < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_COMM_RETRY_COUNT_ID:
                if(targetP->communicationRetryCount < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_COMM_RETRY_TIMER_ID:
                if(targetP->communicationRetryTimer < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_SEQ_DELAY_TIMER_ID:
                if(targetP->communicationSequenceDelayTimer < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;

            case LWM2M_SERVER_SEQ_RETRY_COUNT_ID:
                if(targetP->communicationSequenceRetryCount < 0)
                {
                    result = COAP_404_NOT_FOUND;
                }
                break;
#endif

            default:
                result = COAP_404_NOT_FOUND;
                break;
            }
        }
    }

    return result;
}

static uint8_t prv_set_int_value(lwm2m_data_t * dataArray, uint32_t * data) {
    uint8_t result;
    int64_t value;

    if (1 == lwm2m_data_decode_int(dataArray, &value))
    {
        if (value >= 0 && value <= 0xFFFFFFFF)
        {
            *data = value;
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
    return result;
}

static uint8_t prv_server_write(uint16_t instanceId,
                                int numData,
                                lwm2m_data_t * dataArray,
                                lwm2m_object_t * objectP)
{
    server_instance_t * targetP;
    int i;
    uint8_t result;
    LOG_ARG("prv_server_write instanceId=%d \n",instanceId);
    targetP = (server_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    lwm2m_binding_t   bindingMode;
    lwm2m_server_t *target_serlistP = NULL;
    lwm2m_context_t *contextP = (lwm2m_context_t *)objectP->ctxP;
    if (NULL == targetP)
    {
        return COAP_404_NOT_FOUND;
    }
    //add by joe start
    if(contextP != NULL) 
    {
      target_serlistP  = contextP->serverList ;
      while (target_serlistP != NULL)
      {
        if(target_serlistP->shortID == targetP->shortServerId)
        {
          break;
        }
        target_serlistP = target_serlistP->next;
       }
     }
    //add by joe end

    i = 0;
    do
    {
        switch (dataArray[i].id)
        {
        case LWM2M_SERVER_SHORT_ID_ID:
            {
                uint32_t value = targetP->shortServerId;
                result = prv_set_int_value(dataArray + i, &value);
                if (COAP_204_CHANGED == result)
                {
                    if (0 < value && 0xFFFF >= value)
                    {
                        targetP->shortServerId = value;
                    }
                    else
                    {
                        result = COAP_406_NOT_ACCEPTABLE;
                    }
                }
            }
            break;

        case LWM2M_SERVER_LIFETIME_ID:
            result = prv_set_int_value(dataArray + i, (uint32_t *)&(targetP->lifetime));
            LOG_ARG("targetP->lifetime = %d \n", targetP->lifetime);


            if(contextP != NULL)
            {
               /* if (target_serlistP == NULL){
                    LOG_ARG("target_serlistP is null");
                } else {
                    LOGD("target_serlistP is not null");
                    LOGD("target_serlistP->lifetime = %d", target_serlistP->lifetime);
                    target_serlistP->lifetime = targetP->lifetime;
                    LOGD("target_serlistP->lifetime = %d", target_serlistP->lifetime);
                }*/

                /*if (target_serlistP != NULL){
                    while (target_serlistP != NULL)
                    {
                        LOGD("target_serlistP->shortID = %d, targetP->shortServerId = %d", target_serlistP->shortID, targetP->shortServerId);
                        if(target_serlistP->shortID == targetP->shortServerId)
                        {
                            break;
                        }
                        target_serlistP = target_serlistP->next;
                    }

                    LOGD("target_serlistP->lifetime = %d", target_serlistP->lifetime);
                    target_serlistP->lifetime = targetP->lifetime;
                    LOGD("target_serlistP->lifetime = %d", target_serlistP->lifetime);
                 }*/
                 if (contextP->connect_info != NULL){
                     LOG_ARG("contextP->connect_info->lifetime = %d , targetP->lifetime = %d \n",contextP->connect_info->lifetime,targetP->lifetime);
                     contextP->connect_info->lifetime = targetP->lifetime;
                     contextP->connect_info->diff = 1;
                 }
            }
            //add by joe start
            if(target_serlistP && target_serlistP->lifetime != targetP->lifetime)
            {
              if (target_serlistP)
              {
                target_serlistP->lifetime = targetP->lifetime;
              }
            
              /* Indicate the application that the server life time has been updated */ 
             
            }
            //add by joe end

            break;

        case LWM2M_SERVER_MIN_PERIOD_ID:
            result = prv_set_int_value(dataArray + i, &(targetP->defaultMinPeriod));
            //add by joe start
               if(target_serlistP)
              {
                target_serlistP->defaultMinPeriod = targetP->defaultMinPeriod; 
              }
               //add by joe end
            break;

        case LWM2M_SERVER_MAX_PERIOD_ID:
            result = prv_set_int_value(dataArray + i, &(targetP->defaultMaxPeriod));
            //add by joe start
           if(target_serlistP)
          {
            target_serlistP->defaultMaxPeriod = targetP->defaultMaxPeriod; 
          }
           //add by joe end
            break;

        case LWM2M_SERVER_DISABLE_ID:
            result = COAP_405_METHOD_NOT_ALLOWED;
            break;

        case LWM2M_SERVER_TIMEOUT_ID:
            result = prv_set_int_value(dataArray + i, &(targetP->disableTimeout));
            //add by joe start
            if(result !=COAP_204_CHANGED)
            {
              break;
            }
            if(target_serlistP)
            {
              target_serlistP->disable_timeOut = targetP->disableTimeout; 
            }
            //add by joe
        break;            
            break;

        case LWM2M_SERVER_STORING_ID:
        {
            bool value;

            if (1 == lwm2m_data_decode_bool(dataArray + i, &value))
            {
                targetP->storing = value;
                result = COAP_204_CHANGED;
             if(target_serlistP)
            {
                target_serlistP->storing = value; 
            }               
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;

        case LWM2M_SERVER_BINDING_ID:
            #if 0
            if ((dataArray[i].type == LWM2M_TYPE_STRING || dataArray[i].type == LWM2M_TYPE_OPAQUE)
             && dataArray[i].value.asBuffer.length > 0 && dataArray[i].value.asBuffer.length <= 3
             && (strncmp((char*)dataArray[i].value.asBuffer.buffer, "U", dataArray[i].value.asBuffer.length) == 0
              || strncmp((char*)dataArray[i].value.asBuffer.buffer, "UQ", dataArray[i].value.asBuffer.length) == 0
              || strncmp((char*)dataArray[i].value.asBuffer.buffer, "S", dataArray[i].value.asBuffer.length) == 0
              || strncmp((char*)dataArray[i].value.asBuffer.buffer, "SQ", dataArray[i].value.asBuffer.length) == 0
              || strncmp((char*)dataArray[i].value.asBuffer.buffer, "US", dataArray[i].value.asBuffer.length) == 0
              || strncmp((char*)dataArray[i].value.asBuffer.buffer, "UQS", dataArray[i].value.asBuffer.length) == 0))
            {
                strncpy(targetP->binding, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length);
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
            #else
            
            if(contextP != NULL)
            {
                if ((dataArray[i].type == LWM2M_TYPE_STRING || dataArray[i].type == LWM2M_TYPE_OPAQUE)
                    && dataArray[i].value.asBuffer.length > 0 && dataArray[i].value.asBuffer.length <= 3)
                {
                    if(strncmp((char*)dataArray[i].value.asBuffer.buffer, "U", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_U;
                    else if(strncmp((char*)dataArray[i].value.asBuffer.buffer, "UQ", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_UQ;
                    else if(strncmp((char*)dataArray[i].value.asBuffer.buffer, "S", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_S;
                    else if (strncmp((char*)dataArray[i].value.asBuffer.buffer, "SQ", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_SQ;
                    else if(strncmp((char*)dataArray[i].value.asBuffer.buffer, "US", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_US;
                    else if(strncmp((char*)dataArray[i].value.asBuffer.buffer, "UQS", dataArray[i].value.asBuffer.length) == 0)
                      bindingMode = BINDING_UQS;
                    else {
                      return COAP_400_BAD_REQUEST;
                    }

                  if(target_serlistP && (target_serlistP->binding == bindingMode))
                  {
                    result = COAP_204_CHANGED;
                    break;
                  }

#ifdef FACTORY_BOOTSTRAPPING
                  if(targetP->binding)
                  {
                    lwm2m_free(targetP->binding);
                  }
                  targetP->binding = (char *)lwm2m_malloc(dataArray[i].value.asBuffer.length + 1);
                  if (NULL == targetP->binding) 
                  {
                    result = COAP_500_INTERNAL_SERVER_ERROR;
                    break;
                  }
                  targetP->bindingLen = dataArray[i].value.asBuffer.length;
        		  
        		//  MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] targetP->bindingLen =%d", targetP->bindingLen );
#endif

                  strlcpy(targetP->binding, (char*)dataArray[i].value.asBuffer.buffer, dataArray[i].value.asBuffer.length + 1); 
                  result = COAP_204_CHANGED;
                  if(target_serlistP)
                  {
                    target_serlistP->binding = bindingMode; 
                  }
                }
         }
        else
        {
          result = COAP_400_BAD_REQUEST;
        }
        break;
            #endif

        case LWM2M_SERVER_UPDATE_ID:
            result = COAP_405_METHOD_NOT_ALLOWED;
            break;
        //add by joe start
        case  LWM2M_SERVER_RESOURCE_30000:
           {
               lwm2m_context_t *contextP = (lwm2m_context_t *)objectP->ctxP;
                
                LOG_ARG("dataArray[i].id == LWM2M_SERVER_RESOURCE_30000 lwm2m_get_client_state(contextP)=%d",lwm2m_get_client_state(contextP));
               if( (lwm2m_get_client_state(contextP) == STATE_BOOTSTRAP_REQUIRED) || ( lwm2m_get_client_state(contextP) == STATE_BOOTSTRAPPING ) )
               {
               
                #if 0
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
               }
               else
               {
                 result = COAP_405_METHOD_NOT_ALLOWED;
               }
               break;
           }
//add by joe end
#ifndef LWM2M_VERSION_1_0
        case LWM2M_SERVER_REG_ORDER_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->registrationPriorityOrder = value;
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

        case LWM2M_SERVER_INITIAL_REG_DELAY_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->initialRegistrationDelayTimer = value;
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

        case LWM2M_SERVER_REG_FAIL_BLOCK_ID:
        {
            bool value;
            if (1 == lwm2m_data_decode_bool(dataArray + i, &value))
            {
                targetP->registrationFailureBlock = value;
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
        }

        case LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID:
        {
            bool value;
            if (1 == lwm2m_data_decode_bool(dataArray + i, &value))
            {
                targetP->bootstrapOnRegistrationFailure = value;
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
            break;
        }

        case LWM2M_SERVER_COMM_RETRY_COUNT_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->communicationRetryCount = value;
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

        case LWM2M_SERVER_COMM_RETRY_TIMER_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->communicationRetryTimer = value;
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

        case LWM2M_SERVER_SEQ_DELAY_TIMER_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->communicationSequenceDelayTimer = value;
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

        case LWM2M_SERVER_SEQ_RETRY_COUNT_ID:
        {
            uint64_t value;
            if (1 == lwm2m_data_decode_uint(dataArray + i, &value))
            {
                if (value <= INT_MAX)
                {
                    targetP->communicationSequenceRetryCount = value;
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
#endif

        default:
            LOG("prv_server_write goto default not found will  return\n");
            return COAP_404_NOT_FOUND;
        }
        i++;
    } while (i < numData && result == COAP_204_CHANGED);

//add by joe start
    if (COAP_204_CHANGED == result && lwm2m_check_is_vzw_netwrok())
        store_server_persistent_info(targetP, LWM2M_TRUE);
//add by joe end
    return result;
}

static uint8_t prv_server_execute(uint16_t instanceId,
                                  uint16_t resourceId,
                                  uint8_t * buffer,
                                  int length,
                                  lwm2m_object_t * objectP)

{
    server_instance_t * targetP;

    targetP = (server_instance_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == targetP) return COAP_404_NOT_FOUND;

	lwm2m_server_t *target_serlistP = NULL;
	lwm2m_context_t *contextP = (lwm2m_context_t *)objectP->ctxP;


	if(contextP != NULL)
	{
	  target_serlistP  = contextP->serverList ;
	  while (target_serlistP != NULL)
	  {
		if(target_serlistP->shortID == targetP->shortServerId)
		{
		  break;
		}
		target_serlistP = target_serlistP->next;
	   }
	}

    switch (resourceId)
    {
    case LWM2M_SERVER_DISABLE_ID:
        // executed in core, if COAP_204_CHANGED is returned
        if (0 < targetP->disableTimeout){
            LOG_ARG("target_serlistP->shortID%d target_serlistP->disable set as true\n",target_serlistP->shortID);
           target_serlistP->disable = true;
           return COAP_204_CHANGED;
        } else {
           return COAP_405_METHOD_NOT_ALLOWED;
        }
    case LWM2M_SERVER_UPDATE_ID:
        // executed in core, if COAP_204_CHANGED is returned
        return COAP_204_CHANGED;
    default:
        return COAP_405_METHOD_NOT_ALLOWED;
    }
}

static uint8_t prv_server_delete(uint16_t id,
                                 lwm2m_object_t * objectP)
{
    server_instance_t * serverInstance;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&serverInstance);
    if (NULL == serverInstance) return COAP_404_NOT_FOUND;

//add by joe
if(lwm2m_check_is_vzw_netwrok())
    store_server_persistent_info(serverInstance, LWM2M_FALSE);

    lwm2m_free(serverInstance);

    return COAP_202_DELETED;
}

static uint8_t prv_server_create(uint16_t instanceId,
                                 int numData,
                                 lwm2m_data_t * dataArray,
                                 lwm2m_object_t * objectP)
{
    server_instance_t * serverInstance;
    uint8_t result;

    serverInstance = (server_instance_t *)lwm2m_malloc(sizeof(server_instance_t));
    if (NULL == serverInstance) return COAP_500_INTERNAL_SERVER_ERROR;
    memset(serverInstance, 0, sizeof(server_instance_t));

    serverInstance->instanceId = instanceId;
#ifndef LWM2M_VERSION_1_0
    serverInstance->registrationPriorityOrder = -1;
    serverInstance->initialRegistrationDelayTimer = -1;
    serverInstance->registrationFailureBlock = -1;
    serverInstance->bootstrapOnRegistrationFailure = -1;
    serverInstance->communicationRetryCount = -1;
    serverInstance->communicationRetryTimer = -1;
    serverInstance->communicationSequenceDelayTimer = -1;
    serverInstance->communicationSequenceRetryCount = -1;
#endif
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, serverInstance);

    result = prv_server_write(instanceId, numData, dataArray, objectP);

    if (result != COAP_204_CHANGED)
    {
        (void)prv_server_delete(instanceId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }

    //add by joe for vzw start
    create_diagnostic_server_obecjt_for_vzw(objectP);
    //add by joe for vzw end
    
    return result;
}

void copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc)
{
    memcpy(objectDest, objectSrc, sizeof(lwm2m_object_t));
    objectDest->instanceList = NULL;
    objectDest->userData = NULL;
    server_instance_t * instanceSrc = (server_instance_t *)objectSrc->instanceList;
    server_instance_t * previousInstanceDest = NULL;
    while (instanceSrc != NULL)
    {
        server_instance_t * instanceDest = (server_instance_t *)lwm2m_malloc(sizeof(server_instance_t));
        if (NULL == instanceDest)
        {
            return;
        }
        memcpy(instanceDest, instanceSrc, sizeof(server_instance_t));
        // not sure it's necessary:
        strcpy(instanceDest->binding, instanceSrc->binding);
        instanceSrc = (server_instance_t *)instanceSrc->next;
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

void display_server_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    LOG_ARG("  /%u: Server object, instances:\r\n", object->objID);
    server_instance_t * serverInstance = (server_instance_t *)object->instanceList;
    while (serverInstance != NULL)
    {
        LOG_ARG("   /%u/%u: instanceId: %u, shortServerId: %u, lifetime: %u, storing: %s, binding: %s",
                object->objID, serverInstance->instanceId,
                serverInstance->instanceId, serverInstance->shortServerId, serverInstance->lifetime,
                serverInstance->storing ? "true" : "false", serverInstance->binding);
#ifndef LWM2M_VERSION_1_0
        if(serverInstance->registrationPriorityOrder >= 0)
            fprintf(stdout, ", registrationPriorityOrder: %d", serverInstance->registrationPriorityOrder);
        if(serverInstance->initialRegistrationDelayTimer >= 0)
            fprintf(stdout, ", initialRegistrationDelayTimer: %d", serverInstance->initialRegistrationDelayTimer);
        if(serverInstance->registrationFailureBlock >= 0)
            fprintf(stdout, ", registrationFailureBlock: %s",
                    serverInstance->registrationFailureBlock > 0 ? "true" : "false");
        if(serverInstance->bootstrapOnRegistrationFailure >= 0)
            fprintf(stdout, ", bootstrapOnRegistrationFaulure: %s",
                    serverInstance->bootstrapOnRegistrationFailure > 0 ? "true" : "false");
        if(serverInstance->communicationRetryCount >= 0)
            fprintf(stdout, ", communicationRetryCount: %d", serverInstance->communicationRetryCount);
        if(serverInstance->communicationRetryTimer >= 0)
            fprintf(stdout, ", communicationRetryTimer: %d", serverInstance->communicationRetryTimer);
        if(serverInstance->communicationSequenceDelayTimer >= 0)
            fprintf(stdout, ", communicationSequenceDelayTimer: %d", serverInstance->communicationSequenceDelayTimer);
        if(serverInstance->communicationSequenceRetryCount >= 0)
            fprintf(stdout, ", communicationSequenceRetryCount: %d", serverInstance->communicationSequenceRetryCount);
#endif
        fprintf(stdout, "\r\n");
        serverInstance = (server_instance_t *)serverInstance->next;
    }
#endif
}

lwm2m_object_t * get_server_object(int serverId,
                                   const char* binding,
                                   int lifetime,
                                   bool storing, uint16_t instanceId)
{

    LOG_ARG("serverId = %d, binding = %s, lifetime = %d, storing = %d", serverId, binding, lifetime, storing);
    lwm2m_object_t * serverObj;

    serverObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != serverObj)
    {
        server_instance_t * serverInstance;

        memset(serverObj, 0, sizeof(lwm2m_object_t));

        serverObj->objID = 1;

        // Manually create an hardcoded server
        serverInstance = (server_instance_t *)lwm2m_malloc(sizeof(server_instance_t));
        if (NULL == serverInstance)
        {
            lwm2m_free(serverObj);
            return NULL;
        }

        memset(serverInstance, 0, sizeof(server_instance_t));
        serverInstance->instanceId = instanceId;
        serverInstance->shortServerId = serverId;
        serverInstance->lifetime = lifetime;
        serverInstance->storing = storing;
        memcpy (serverInstance->binding, binding, strlen(binding)+1);
#ifndef LWM2M_VERSION_1_0
        serverInstance->registrationPriorityOrder = -1;
        serverInstance->initialRegistrationDelayTimer = -1;
        serverInstance->registrationFailureBlock = -1;
        serverInstance->bootstrapOnRegistrationFailure = -1;
        serverInstance->communicationRetryCount = -1;
        serverInstance->communicationRetryTimer = -1;
        serverInstance->communicationSequenceDelayTimer = -1;
        serverInstance->communicationSequenceRetryCount = -1;
#endif
        serverObj->instanceList = LWM2M_LIST_ADD(serverObj->instanceList, serverInstance);

        serverObj->readFunc = prv_server_read;
        serverObj->discoverFunc = prv_server_discover;
        serverObj->writeFunc = prv_server_write;
        serverObj->createFunc = prv_server_create;
        serverObj->deleteFunc = prv_server_delete;
        serverObj->executeFunc = prv_server_execute;
    }

    return serverObj;
}

void clean_server_object(lwm2m_object_t * object)
{
    while (object->instanceList != NULL)
    {
        server_instance_t * serverInstance = (server_instance_t *)object->instanceList;
        object->instanceList = object->instanceList->next;
        lwm2m_free(serverInstance);
    }
}
