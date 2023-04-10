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
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/
#define LOG_TAG "Mango-core/liblwm2m.c"
#include "lwm2m_android_log.h"
#include "call_stack.h"

#include "internals.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


lwm2m_context_t * lwm2m_init(void * userData)
{
    lwm2m_context_t * contextP;

    LOG("lwm2m_init X");
    contextP = (lwm2m_context_t *)lwm2m_malloc(sizeof(lwm2m_context_t));
    if (NULL != contextP)
    {
        memset(contextP, 0, sizeof(lwm2m_context_t));
        contextP->userData = userData;
        srand((int)lwm2m_gettime());
        contextP->nextMID = rand();
        contextP->fota_fd = -1;//add by joe
    }
	LOG("lwm2m_init E");

    return contextP;
}

#ifdef LWM2M_CLIENT_MODE
void lwm2m_deregister(lwm2m_context_t * context)
{
    lwm2m_server_t * server = context->serverList;

    LOG("Entering");
    while (NULL != server)
    {
        registration_deregister(context, server);
        server = server->next;
    }
}

static void prv_deleteServer(lwm2m_server_t * serverP, void *userData)
{
    // TODO parse transaction and observation to remove the ones related to this server
    if (serverP->sessionH != NULL)
    {
         lwm2m_close_connection(serverP->sessionH, userData);
    }
    if (NULL != serverP->location)
    {
        lwm2m_free(serverP->location);
    }
    free_block1_buffer(serverP->block1Data);
    lwm2m_free(serverP);
}

static void prv_deleteServerList(lwm2m_context_t * context)
{
    while (NULL != context->serverList)
    {
        lwm2m_server_t * server;
        server = context->serverList;
        context->serverList = server->next;
        prv_deleteServer(server, context->userData);
    }
}

static void prv_deleteBootstrapServer(lwm2m_server_t * serverP, void *userData)
{
    // TODO should we free location as in prv_deleteServer ?
    // TODO should we parse transaction and observation to remove the ones related to this server ?
    if (serverP->sessionH != NULL)
    {
         lwm2m_close_connection(serverP->sessionH, userData);
    }
    free_block1_buffer(serverP->block1Data);
    lwm2m_free(serverP);
}

static void prv_deleteBootstrapServerList(lwm2m_context_t * context)
{
    while (NULL != context->bootstrapServerList)
    {
        lwm2m_server_t * server;
        server = context->bootstrapServerList;
        context->bootstrapServerList = server->next;
        prv_deleteBootstrapServer(server, context->userData);
    }
}

static void prv_deleteObservedList(lwm2m_context_t * contextP)
{
    while (NULL != contextP->observedList)
    {
        lwm2m_observed_t * targetP;
        lwm2m_watcher_t * watcherP;

        targetP = contextP->observedList;
        contextP->observedList = contextP->observedList->next;

        for (watcherP = targetP->watcherList ; watcherP != NULL ; watcherP = watcherP->next)
        {
            if (watcherP->parameters != NULL) lwm2m_free(watcherP->parameters);
        }
        LWM2M_LIST_FREE(targetP->watcherList);

        lwm2m_free(targetP);
    }
}
#endif

#ifdef AUTHORIZATION_SUPPORT
lwm2m_client_state_t lwm2m_get_client_state(lwm2m_context_t *ctx_p)
{
  if(ctx_p != NULL)
    return ctx_p->state;
  else
    return STATE_INITIAL;
}
#endif

void prv_deleteTransactionList(lwm2m_context_t * context)
{
    while (NULL != context->transactionList)
    {
        lwm2m_transaction_t * transaction;

        transaction = context->transactionList;
        context->transactionList = context->transactionList->next;
        transaction_free(transaction);
    }
}

void lwm2m_close(lwm2m_context_t * contextP)
{
#ifdef LWM2M_CLIENT_MODE

    LOG("Entering");
    lwm2m_deregister(contextP);
    prv_deleteServerList(contextP);
    prv_deleteBootstrapServerList(contextP);
    prv_deleteObservedList(contextP);
    lwm2m_free(contextP->endpointName);
    if (contextP->msisdn != NULL)
    {
        lwm2m_free(contextP->msisdn);
    }
    if (contextP->altPath != NULL)
    {
        lwm2m_free(contextP->altPath);
    }

#endif

#ifdef LWM2M_SERVER_MODE
    while (NULL != contextP->clientList)
    {
        lwm2m_client_t * clientP;

        clientP = contextP->clientList;
        contextP->clientList = contextP->clientList->next;

        registration_freeClient(clientP);
    }
#endif

    prv_deleteTransactionList(contextP);
    lwm2m_free(contextP);
}

#ifdef LWM2M_CLIENT_MODE
static int prv_refreshServerList(lwm2m_context_t * contextP)
{
    lwm2m_server_t * targetP;
    lwm2m_server_t * nextP;

    // Remove all servers marked as dirty
    targetP = contextP->bootstrapServerList;
    contextP->bootstrapServerList = NULL;
    LOG_ARG("bootstrapServerList targetP = %p", targetP);
    while (targetP != NULL)
    {
        nextP = targetP->next;
        targetP->next = NULL;
        if (!targetP->dirty)
        {
            LOG_ARG("joe LWM2M_LIST_ADD targetP = %p", targetP);
            targetP->status = STATE_DEREGISTERED;
            contextP->bootstrapServerList = (lwm2m_server_t *)LWM2M_LIST_ADD(contextP->bootstrapServerList, targetP);
        }
        else
        {
            LOG_ARG("joe prv_deleteServer targetP = %p", targetP);
            prv_deleteServer(targetP, contextP->userData);
        }
        targetP = nextP;
    }
    targetP = contextP->serverList;
	LOG_ARG("joe serverList targetP = %p", targetP);
    contextP->serverList = NULL;
	LOG_ARG("serverList targetP = %p", targetP);
    while (targetP != NULL)
    {
        nextP = targetP->next;
        targetP->next = NULL;
        if (!targetP->dirty)
        {
            // TODO: Should we revert the status to STATE_DEREGISTERED ?
            LOG_ARG("contextP serverList LWM2M_LIST_ADD targetP= %p", targetP);
            contextP->serverList = (lwm2m_server_t *)LWM2M_LIST_ADD(contextP->serverList, targetP);
        }
        else
        {
            prv_deleteServer(targetP, contextP->userData);
        }
        targetP = nextP;
    }
    LOG_ARG(" object_getServers contextP= %p", contextP);
    return object_getServers(contextP, false);
}


//klein: add endpointName, objectList to lwm2m_context_t
int lwm2m_configure(lwm2m_context_t * contextP,
                    const char * endpointName,
                    const char * msisdn,
                    const char * altPath,
                    uint16_t numObject,
                    lwm2m_object_t * objectList[])
{
    int i;
    uint8_t found;

    LOG_ARG("endpointName: \"%s\", msisdn: \"%s\", altPath: \"%s\", numObject: %d", endpointName, msisdn, altPath, numObject);
    LOGD("lwm2m_configure endpointName: \"%s\", msisdn: \"%s\", altPath: \"%s\", numObject: %d", endpointName, msisdn, altPath, numObject);

    // This API can be called only once for now
    if (contextP->endpointName != NULL || contextP->objectList != NULL) return COAP_400_BAD_REQUEST;

    if (endpointName == NULL) return COAP_400_BAD_REQUEST;
    if (numObject < 3) return COAP_400_BAD_REQUEST;
    // Check that mandatory objects are present
    found = 0;
    for (i = 0 ; i < numObject ; i++)
    {
        if (objectList[i]->objID == LWM2M_SECURITY_OBJECT_ID) found |= 0x01;
        if (objectList[i]->objID == LWM2M_SERVER_OBJECT_ID) found |= 0x02;
        if (objectList[i]->objID == LWM2M_DEVICE_OBJECT_ID) found |= 0x04;
    }
    //klein: security, server and device object are mandatory
    if (found != 0x07) return COAP_400_BAD_REQUEST;
    if (altPath != NULL)
    {
        if (0 == utils_isAltPathValid(altPath))
        {
            return COAP_400_BAD_REQUEST;
        }
        if (altPath[1] == 0)
        {
            altPath = NULL;
        }
    }
    contextP->endpointName = lwm2m_strdup(endpointName);
    if (contextP->endpointName == NULL)
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (msisdn != NULL)
    {
        contextP->msisdn = lwm2m_strdup(msisdn);
        if (contextP->msisdn == NULL)
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    if (altPath != NULL)
    {
        contextP->altPath = lwm2m_strdup(altPath);
        if (contextP->altPath == NULL)
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    for (i = 0; i < numObject; i++)
    {
        objectList[i]->next = NULL;
        contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_ADD(contextP->objectList, objectList[i]);
    }

    return COAP_NO_ERROR;
}

int lwm2m_add_object(lwm2m_context_t * contextP,
                     lwm2m_object_t * objectP)
{
    lwm2m_object_t * targetP;

    LOG_ARG("ID: %d", objectP->objID);
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(contextP->objectList, objectP->objID);
    if (targetP != NULL) return COAP_406_NOT_ACCEPTABLE;
    objectP->next = NULL;

    contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_ADD(contextP->objectList, objectP);

    if (contextP->state == STATE_READY)
    {
        return lwm2m_update_registration(contextP, 0, true);
    }

    return COAP_NO_ERROR;
}

int lwm2m_remove_object(lwm2m_context_t * contextP,
                        uint16_t id)
{
    lwm2m_object_t * targetP;

    LOG_ARG("ID: %d", id);
    contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_RM(contextP->objectList, id, &targetP);

    if (targetP == NULL) return COAP_404_NOT_FOUND;

    if (contextP->state == STATE_READY)
    {
        return lwm2m_update_registration(contextP, 0, true);
    }

    return 0;
}

#endif

//add by joe start
resource_instance_string_list_t * find_string_resource_inst(resource_instance_string_list_t * head,
    uint16_t id)
{
  while (NULL != head && head->resInstId < id)
  {
    head = head->next;
  }

  if (NULL != head && head->resInstId == id) return head;

  return NULL;
}

resource_instance_int_list_t * find_resource_inst(resource_instance_int_list_t * head,
    uint16_t id)
{
  while (NULL != head && head->resInstId < id)
  {
    head = head->next;
  }

  if (NULL != head && head->resInstId == id) return head;

  return NULL;
}

    
resource_instance_int_list_t * lwm2m_reosurce_list_add(resource_instance_int_list_t * head,
                              resource_instance_int_list_t * node)
{
    resource_instance_int_list_t * target;
    LOG("lwm2m_reosurce_list_add 1\n");

    if (NULL == head) return node;
    LOG_ARG("lwm2m_reosurce_list_add head->id=%d \n",head->resInstId);
    LOG_ARG("lwm2m_reosurce_list_add node->id=%d \n",node->resInstId);
    LOG("lwm2m_reosurce_list_add 2\n");

    if (head->resInstId > node->resInstId)
    {
    LOG("lwm2m_reosurce_list_add 3\n");
        node->next = head;
        return node;
    }
    LOG("lwm2m_reosurce_list_add 4\n");

    target = head;
    while (NULL != target->next && target->next->resInstId < node->resInstId)
    {
    LOG("lwm2m_reosurce_list_add 5\n");
        target = target->next;
    }
    LOG("lwm2m_reosurce_list_add 6\n");

    node->next = target->next;
    target->next = node;

    return head;
}

/**
 * @fn static bool prv_write_resource_inst_val()
 * @brief Function to create a resource instance or overwrite existing resource instance for int
 * @return true on success, 
 *         false on failure
 *
 */
bool prv_write_resource_inst_val(resource_instance_int_list_t** rsList,
    uint16_t resId, uint16_t resValue)
{
  bool ret = false;
  resource_instance_int_list_t *resInst = NULL, *temp = *rsList;
  
  LOG("prv_write_resource_inst_val START");
  while(temp)
  {
    if(temp->resInstId == resId)
    {
      temp->InstValue = resValue;
      return true;
    }
    temp = temp->next;
  }
  LOG("prv_write_resource_inst_val");

  resInst = (resource_instance_int_list_t *)lwm2m_malloc(sizeof(resource_instance_int_list_t));
  if (resInst==NULL)
  {
      LOG("prv_write_resource_inst_val resInst==NULL");
    return ret;
  }
  else
  {
    memset(resInst, 0, sizeof(resource_instance_int_list_t));
    resInst->resInstId   = resId;
    resInst->InstValue   = resValue;

    //*rsList = (resource_instance_int_list_t*)lwm2m_reosurce_list_add(*rsList, resInst);
    //by joe
    *rsList = (resource_instance_int_list_t*)LWM2M_LIST_ADD(*rsList, resInst);
    LOG_ARG("prv_write_resource_inst_val resInst->resInstId=%d resInst->InstValue=%d",resInst->resInstId,resInst->InstValue);
    ret = true;
  }

  return ret;
}

/**
 * @fn static bool prv_write_string_resource_inst_val()
 * @brief Function to create a resource instance or overwrite existing resource instance for string
 * @return true on success, 
 *         false on failure
 *
 */
bool prv_write_string_resource_inst_val(resource_instance_string_list_t** rsList,
    uint16_t resId, char* resValue)
{
  bool ret = false;
  resource_instance_string_list_t *resInst = NULL, *temp = *rsList;
  while(temp)
  {
    if(temp->resInstId == resId)
    {
      strlcpy(temp->InstValue,resValue, MAX_STRING_LEN);
      return true;
    }
    temp = temp->next;
  }

  resInst = (resource_instance_string_list_t *)lwm2m_malloc(sizeof(resource_instance_string_list_t));
  if (resInst==NULL)
  {
    return ret;
  }
  else
  {
    memset(resInst, 0, sizeof(resource_instance_string_list_t));
    resInst->resInstId   = resId;
    strlcpy(resInst->InstValue,resValue, MAX_STRING_LEN);
    *rsList = (resource_instance_string_list_t*)LWM2M_LIST_ADD(*rsList, resInst);
    ret = true;
  }

  return ret;
}
//add by joe end
int lwm2m_step(lwm2m_context_t * contextP,
               time_t * timeoutP)
{

    //LOG_ARG("lwm2m_step X, contextP->state = %d", contextP->state); //Joe del due to too many log output
    //dump_stack("mango");
    time_t tv_sec;

    //LOG_ARG("timeoutP: %" PRId64, *timeoutP);//Joe del due to too many log output
    tv_sec = lwm2m_gettime();
    if (tv_sec < 0) return COAP_500_INTERNAL_SERVER_ERROR;

#ifdef LWM2M_CLIENT_MODE
   // LOG_ARG("State: %s", STR_STATE(contextP->state));//Joe del due to too many log output
    // state can also be modified in bootstrap_handleCommand()

next_step:
    switch (contextP->state)
    {
    case STATE_INITIAL:
		LOG("STATE_INITIAL");
        if (0 != prv_refreshServerList(contextP)) return COAP_503_SERVICE_UNAVAILABLE;
		LOG_ARG("contextP->serverList: %p", contextP->serverList);		
		LOG_ARG("contextP->bootstrapServerList: %p", contextP->bootstrapServerList);
        if (contextP->serverList != NULL)
        {
            contextP->state = STATE_REGISTER_REQUIRED;
            char* re_ongonig = "Registering!";
            quec_lwm2m_client_info_notify(re_ongonig);
        }
        else
        {
            // Bootstrapping
            contextP->state = STATE_BOOTSTRAP_REQUIRED;
            char* bs_ongonig = "Bootstraping !";
            quec_lwm2m_client_info_notify(bs_ongonig);
        }
        goto next_step;
        break;

    case STATE_BOOTSTRAP_REQUIRED:
		LOG("STATE_BOOTSTRAP_REQUIRED");
#ifdef LWM2M_BOOTSTRAP
        if (contextP->bootstrapServerList != NULL)
        {
            bootstrap_start(contextP);
            contextP->state = STATE_BOOTSTRAPPING;
            bootstrap_step(contextP, tv_sec, timeoutP);
        }
        else
#endif
        {
            return COAP_503_SERVICE_UNAVAILABLE;
        }
        break;

#ifdef LWM2M_BOOTSTRAP
    case STATE_BOOTSTRAPPING:
		//LOG("STATE_BOOTSTRAPPING");
        switch (bootstrap_getStatus(contextP))
        {
        case STATE_BS_FINISHED:
            contextP->state = STATE_INITIAL;
            char* bs_finish = "Bootstrap successfully ! ID :100";
            quec_lwm2m_client_info_notify(bs_finish);
            goto next_step;
            break;

        case STATE_BS_FAILED:
            return COAP_503_SERVICE_UNAVAILABLE;

        default:
            // keep on waiting
            bootstrap_step(contextP, tv_sec, timeoutP);
            break;
        }
        break;
#endif
    case STATE_REGISTER_REQUIRED:
    {
        LOG("STATE_REGISTER_REQUIRED");	
        int result = registration_start(contextP, true);
        if (COAP_NO_ERROR != result) return result;
        contextP->state = STATE_REGISTERING;
    }
    break;

    case STATE_REGISTERING:
    {
        //LOG("STATE_REGISTERING");
        switch (registration_getStatus(contextP))
        {
        case STATE_REGISTERED:
            contextP->state = STATE_READY;
            break;

        case STATE_REG_FAILED:
            // TODO avoid infinite loop by checking the bootstrap info is different
            //contextP->state = STATE_BOOTSTRAP_REQUIRED;
            //goto next_step;
            //break;
            //del by joe
        case STATE_REG_PENDING:
        default:
            // keep on waiting
            break;
        }
    }
    break;

    case STATE_READY:
	//	LOG("STATE_READY");//Joe del due to too many log output
	/*//del by joe
        if (registration_getStatus(contextP) == STATE_REG_FAILED)
        {
            // TODO avoid infinite loop by checking the bootstrap info is different
            contextP->state = STATE_BOOTSTRAP_REQUIRED;
            goto next_step;
            break;
        }
        */
        break;

    default:
        // do nothing
        break;
    }

    observe_step(contextP, tv_sec, timeoutP);
#endif

    registration_step(contextP, tv_sec, timeoutP);
    transaction_step(contextP, tv_sec, timeoutP);

    //LOG_ARG("Final timeoutP: %" PRId64, *timeoutP);//Joe del due to too many log output
#ifdef LWM2M_CLIENT_MODE
   // LOG_ARG("Final state: %s", STR_STATE(contextP->state));//Joe del due to too many log output
#endif
    //LOG("lwm2m_step E");//Joe del due to too many log output
    return 0;
}

int32_t lwm2m_set_object_info(lwm2m_context_t * contextP,lwm2m_object_data_t * lwm2m_data , uint16_t object_ID)
{
  lwm2m_object_t * targetP = NULL;
  int32_t result = 0 ;  
  lwm2m_uri_t uriP;

  if (lwm2m_data == NULL)
  {
    return -1;
  }

  memset(&uriP, 0, sizeof(lwm2m_uri_t));
  targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(contextP->objectList, object_ID);
  if((NULL == targetP) || (!targetP->setValueFunc))
  {
    return -1;
  }
  result = targetP->setValueFunc(contextP,lwm2m_data, targetP, &uriP);
  if( 1 != result)
  {
    return -1;
  }

  lwm2m_resource_value_changed(contextP, &uriP);
  return 0;
}

//add by joe start
int lwm2m_read_from_efs_file(    const char * name, void ** buffer_ptr, size_t * buffer_size)
{
  size_t bytes_read;
  int efs_fd = -1;
  struct stat stat_s = {0};
  uint8_t *file_buf = NULL;
  //LOG_ARG("load lwm2m_read_from_efs_file name =%s buffer_ptr=%p buffer_size=%d\n",name ,buffer_ptr,buffer_size);

  if((!name) || (!buffer_ptr) || (!buffer_size)) {
    return -1;
  }

  memset( &stat_s, 0, sizeof(struct stat )); 
  
  //LOG("load lwm2m_read_from_efs_file call fstat\n");
  if(stat(name, &stat_s) < 0) {
    return -1;
  }
  //LOG("load lwm2m_read_from_efs_file call open\n");
  if((efs_fd = open(name, O_RDONLY)) < 0) {
    return -1;
  } 

  file_buf = lwm2m_malloc(stat_s.st_size + 4); /*Added 4 bytes extra for padding*/
  //LOG_ARG("load lwm2m_read_from_efs_file file_buf=%p\n",file_buf);

  if(file_buf == NULL) {
  	close(efs_fd);
    return -1;
  }

  memset(file_buf, 0, stat_s.st_size + 4);
  //LOG("load lwm2m_read_from_efs_file call read\n");
  bytes_read = read(efs_fd, file_buf, stat_s.st_size);
  if((bytes_read != stat_s.st_size) && (bytes_read == 0)) {
    lwm2m_free(file_buf);
    close(efs_fd);
    return -1;
  }
  //LOG("load lwm2m_read_from_efs_file call read end\n");

  *buffer_ptr = file_buf;
  *buffer_size = stat_s.st_size;

  close(efs_fd);

  return 0;
}

static bool lwm2m_char_to_hex(char  c, unsigned char *hex_val)
{
	*hex_val = 0;
	if(c >= 'A' && c <= 'F'){
		*hex_val = (c-'A')+10;
		 return true;
	}else if(c >= 'a' && c <= 'f'){
		*hex_val = (c-'a')+10;
		return true;
	}else if(c >= '0' && c <= '9' ){
		*hex_val = (c- '0');
		return true;
	}else{
		*hex_val = 0;
		return false;
	}
}
bool lwm2m_string2hex(char *str, unsigned char **hex_array, int *hex_array_len)
{
	int str_len = 0;
	int i = 0;
	unsigned char h = 0, l = 0;
	unsigned char *local_buff = NULL;
	unsigned char *b = NULL;
	if(str == NULL || hex_array_len == NULL)
		return false;

	str_len = strlen(str);

	if(str_len%2 != 0)
		return false;
	
	if(*hex_array_len <(str_len)/2)
		return false;

	*hex_array_len = (str_len)/2;
	local_buff = (unsigned char *)lwm2m_malloc(*hex_array_len);
	if (NULL == local_buff){
		return false;
	}

	b = local_buff;
	
	for(i = 0 ; i < str_len; i+=2,b++)
	{
		if(lwm2m_char_to_hex(str[i],&h) == false
		   || lwm2m_char_to_hex(str[i+1],&l) == false){
		   lwm2m_free(local_buff);
		   local_buff = NULL;
			return false;
		 }
		 *b = ((h <<4)|l);
	}
	*hex_array = (void *)local_buff;
	
	return true;
}


 /**
* @fn bool write_Tlv_to_file()
* @brief This function is used to write a TagLengthValue to file
*
* @param[in] fd 	file to write
* @param[in] tag	TAG to write
* @param[in] length length of data to write
* @param[in] value	data to write
*
* @return	 True	Write to file success
			 False	Write to file failure 
*		 
*/

bool write_Tlv_to_file(int fd, int tag, uint8_t length, uint8_t* value)
{
	if (NULL == fd || NULL == value)
	{
	  return LWM2M_FALSE;
	}
			   
	if (EFAILURE == write(fd, &tag, sizeof(tag)))
	{
	   return LWM2M_FALSE;
	} 
	if (EFAILURE == write(fd, &length, sizeof(length)))
	{
	   return LWM2M_FALSE;
	}
	if (EFAILURE == write(fd, value, length))
	{
	   return LWM2M_FALSE;
	} 
	return LWM2M_TRUE;
}
			   
/** 
* @fn	  read_Tlv_from_file()
* @brief  This function is used to read resource value from persistence file
* @param[in]  fd		  - file descripter
  @param[out] tag		  - tag
  @param[out] length	  - length of value
  @param[out] value 	  - value
* @return on 1	  -  read success
		  on 0	  -  reach end of file
		  on EFAILURE - read failure
*/ 
int read_Tlv_from_file(int fd, int* tag, uint8_t* length, uint8_t** value)
{
	 int ret	= 0;
			   
	 if (NULL == fd || NULL == tag || NULL == length)
	 {
	   return 0;
	 }
			   
	 ret = read(fd, tag, sizeof(int));
	 if(0 == ret)
	 { 
	 	//reach end of file
	   return 0;
	 }
	 else if(ret != sizeof(int))
	 { 
	 	//error happen
		return EFAILURE;
	 }
				   
	 if (sizeof(uint8_t) != read(fd, length, sizeof(uint8_t)))
	 {
	     return EFAILURE;
	 } 

	 *value = lwm2m_malloc((*length) + 1);
	 if(NULL == *value)
	 {
         return EFAILURE;    
	 }
				 
	 memset(*value, 0, (*length)+1);
	 if (*length != read(fd, *value, *length))
	 {
	   lwm2m_free(*value);
	   *value = NULL;
	   return EFAILURE;
	 } 
	 return 1;
}

//add by joe end
