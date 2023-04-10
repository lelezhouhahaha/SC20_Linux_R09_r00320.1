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
 *    Benjamin Cabé - Please refer to git log
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Axel Lorente - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Christian Renz - Please refer to git log
 *    Ricky Liu - Please refer to git log
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
 Bosch Software Innovations GmbH - Please refer to git log

*/

#define LOG_TAG    "Mango-lwm2mclient"

#include "lwm2mclient.h"
#include "liblwm2m.h"
#include "commandline.h"
#ifdef WITH_TLS
#include "dtlsconnection.h"
#else
#include "connection.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include "lwm2m_android_log.h"
#include "lwm2m_configure.h"
#include <sys/msg.h>
#include "platform.h"
#include <fcntl.h>
#include <pthread.h>
#include <cutils/properties.h>
#include "object_security.h"
#include "object_server.h"
#include "object_access_control.h"
#include "quectel_getinfo.h"

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/inotify.h>
#define MAX_SIZE 125

#define FIFO "/tmp/ififo"
#include "internals.h"
#include "quectel_sms.h"
#include <sys/time.h>
#include <signal.h>
#include <time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef LWM2M_SUPPORT_QLRIL_API
#include <RilPublic.h>
#endif

#define MAX_PACKET_SIZE 1024
#define DEFAULT_SERVER_IPV6 "[::1]"
#define DEFAULT_SERVER_IPV4 "127.0.0.1"

#define ADD_ATTZ_APN "at+cgdcont=1,\"IPV4V6\",\"m2m.com.attz\",\"0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0\",0,0,0,0"
#define ADD_GLOBAL_APN "at+cgdcont=4,\"IPV4V6\",\"attm2mglobal\",\"0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0\",0,0,0,0"

int g_reboot = 0;
static int g_quit = 0;

#ifdef ENABLE_SOFTWARE_MGNT_OBJ
#define OBJ_COUNT 13
#else
#define OBJ_COUNT 12
#endif
//jambo modify
lwm2m_object_t * objArray[OBJ_COUNT];

// only backup security and server objects
#define BACKUP_OBJECT_COUNT 2
lwm2m_object_t * backupObjectArray[BACKUP_OBJECT_COUNT];

//joe add start
lwm2m_context_t *g_lwm2mH_ptr;
int g_is_vzw_network = -1;
int g_is_att_network = -1;
char device_imei[64] = {0};
char device_phoneNumber[64] = {0};
char software_version[64] = {0};

int session_retry_timer[6] ={0,120,240,360,480,86400};
static int current_retry_count = 0;
#define MAX_RETRY_CONUT  4
bool isFullRegistNeed = false;
#define BACKLOG 10
int server_sockfd = -1;
int client_sockfd = -1;
#ifdef LWM2M_SUPPORT_QLRIL_API
//add by joe for qlrild handler
int g_qlril_handle = 0;
static int qlril_handle_flag = 1;
#endif
char VZW_100_Server_url[256] = {0};
char VZW_101_Server_url[256] = {0};
char VZW_102_Server_url[256] = {0};
char VZW_1000_Server_url[256] = {0};
//joe add end

typedef struct
{
    lwm2m_object_t * securityObjP;
    lwm2m_object_t * serverObject;
    int sock;
#ifdef WITH_TLS
    dtls_connection_t * connList;
    lwm2m_context_t * lwm2mH;
#else
    connection_t * connList;
#endif
    int addressFamily;
    int listenfd;
    int connectfd;  /* socket descriptors */
    struct sockaddr_in QTclient;

} client_data_t;

typedef struct
{	
	//q_link_type             	 link;
	int                      	 pdp_cid;
	//apptcpip_ip_addr_struct  	 ip_addr;
	char                         host_str[256];
	int                          ip_port;
	// lwm2m_server_info_t          *bootstrap_server;
	// lwm2m_server_info_t          *dm_server;
	// lwm2m_server_info_t          *diagnostics_server;
	// lwm2m_server_info_t          *repository_server;
	int                          instance_id;
	int                          short_id;
	//WOLFSSL_CTX              	*ssl_ctx_ptr;
	//WOLFSSL                  	*ssl_sess_ptr;
	//Q_INT32                  	 retransmit_timer;
	//dtls_session_t*              ssl_session_ptr;

	uint16_t                   	local_port;
	char                     	version[64];

#if defined(CHINA_MOBILE_LWM2M_CLIENT)
	char    				 	app_key[256];
	char     				 	app_pwd[256];
#endif

	int                      	ep_name_type;

	lwm2m_context_t          	*lwm2m_ctx;
	lwm2m_object_t           	*lwm2m_objs[OBJ_COUNT];

	int                      	socket_fd;
	//connection_t             	*connList;

	//lwm2m_client_stat        	stat;
	int                      	obj_list[OBJ_COUNT];
	//lwm2m_client_listener    	notify_fcn;
	//apptcpip_timer_id           register_timer;
	int                         retry_cnt;
	int                         registered_server_cnt;
	int                         fota_fd;
}lwm2m_client;

typedef struct
{
	int      client_hdl;
	int      short_server_id;
	bool     with_obj; 
}lwm2m_client_update_t;
typedef struct
{
	int   client_hdl;
	int   opt; //1: add 2:delete
	int   obj_id;
}lwm2m_client_object_update_t;


void lwm2m_free_lwm2m_data(lwm2m_object_data_t *lwm2m_dataP)
{
  if (lwm2m_dataP != NULL)
  {
  	  lwm2m_free_instance_info(lwm2m_dataP->instance_info);
      lwm2m_free(lwm2m_dataP);
  }
}

void lwm2m_free_instance_info (lwm2m_instance_info_t  *lwm2m_instanceP)
{
  if (lwm2m_instanceP != NULL)
  {
    lwm2m_free_resource_info(lwm2m_instanceP->resource_info);
    lwm2m_free (lwm2m_instanceP);
  }
}

void lwm2m_free_resource_info (lwm2m_resource_info_t *lwm2m_resourceP)
{
  lwm2m_resource_info_t *lwm2m_resource_linkP = NULL;
  
  while (lwm2m_resourceP != NULL)
  {
    lwm2m_resource_linkP = lwm2m_resourceP->next;

    if ((lwm2m_resourceP->type == LWM2M_TYPE_STRING || lwm2m_resourceP->type == LWM2M_TYPE_OPAQUE) && lwm2m_resourceP->value.asBuffer.buffer != NULL)
       lwm2m_free(lwm2m_resourceP->value.asBuffer.buffer);

	if(lwm2m_resourceP->type == LWM2M_TYPE_MULTIPLE_RESOURCE)
		lwm2m_data_free(lwm2m_resourceP->value.asChildren.count, lwm2m_resourceP->value.asChildren.array);
   
    lwm2m_free(lwm2m_resourceP);
    lwm2m_resourceP = lwm2m_resource_linkP;
  }
}

int  lwm2m_client_update(int short_server_id, bool with_obj)
{
	//lwm2m_client  *client_ptr = (lwm2m_client *)client_hdl;
	lwm2m_client_update_t  *update_info = NULL;

	update_info = lwm2m_malloc(sizeof(lwm2m_client_update_t));

	if(update_info == NULL)
	{
		return LWM2M_CLIENT_ERR_OUT_MEM;
	}
	//if(client_ptr == NULL)
	//	return LWM2M_CLIENT_ERR_INVALID_PARAM;

	//update_info->client_hdl = client_hdl;
	update_info->short_server_id = short_server_id;
	update_info->with_obj = with_obj;

	//apptcpip_send_cmd(APPTCPIP_LWM2M_CLIENT_UPDATE, (void *)update_info);
	return LWM2M_CLIENT_OK;
}


static void prv_quit(char * buffer,
                     void * user_data)
{
    g_quit = 1;
}

void handle_sigint(int signum)
{
    g_quit = 2;
}

void handle_value_changed(lwm2m_context_t * lwm2mH,
                          lwm2m_uri_t * uri,
                          const char * value,
                          size_t valueLength)
{
    lwm2m_object_t * object = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, uri->objectId);

    if (NULL != object)
    {
        if (object->writeFunc != NULL)
        {
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                fprintf(stderr, "Internal allocation failure !\n");
                return;
            }
            dataP->id = uri->resourceId;
            lwm2m_data_encode_nstring(value, valueLength, dataP);

            result = object->writeFunc(uri->instanceId, 1, dataP, object);
            LOG_ARG("object->writeFunc result=%d\n",result);
            if (COAP_405_METHOD_NOT_ALLOWED == result || (uri->objectId == LWM2M_FIRMWARE_UPDATE_OBJECT_ID && COAP_404_NOT_FOUND == result))
            {
                switch (uri->objectId)
                {
                case LWM2M_DEVICE_OBJECT_ID:
                    result = device_change(dataP, object);
                    break;
                //add by joe start
                case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
                    LOG("call firmware_change\n");
                    result = firmware_change(dataP, object);
                    break;
                //add by joe end
                default:
                    break;
                }
            }

            if (COAP_204_CHANGED != result)
            {
                fprintf(stderr, "Failed to change value!\n");
            }
            else
            {
                fprintf(stderr, "value changed!\n");
                lwm2m_resource_value_changed(lwm2mH, uri);
            }
            lwm2m_data_free(1, dataP);
            return;
        }
        else
        {
            fprintf(stderr, "write not supported for specified resource!\n");
        }
        return;
    }
    else
    {
        fprintf(stderr, "Object not found !\n");
    }
}

#ifdef WITH_TINYDTLS
void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
  LOGD("lwm2m_connect_server X");
  client_data_t * dataP;
  lwm2m_list_t * instance;
  dtls_connection_t * newConnP = NULL;
  dataP = (client_data_t *)userData;
  lwm2m_object_t  * securityObj = dataP->securityObjP;

  instance = LWM2M_LIST_FIND(dataP->securityObjP->instanceList, secObjInstID);
  if (instance == NULL) return NULL;


  newConnP = connection_create(dataP->connList, dataP->sock, securityObj, instance->id, dataP->lwm2mH, dataP->addressFamily);
  if (newConnP == NULL)
  {
      fprintf(stderr, "Connection creation failed.\n");
      return NULL;
  }

  dataP->connList = newConnP;
  LOGD("lwm2m_connect_server E");
  return (void *)newConnP;
}
#else
void * lwm2m_connect_server(uint16_t secObjInstID,
                            void * userData)
{
    LOGD("lwm2m_connect_server X");
    client_data_t * dataP;
    lwm2m_list_t * instance;
    dtls_connection_t * newConnP = NULL;
    dataP = (client_data_t *)userData;
    lwm2m_object_t	* securityObj = dataP->securityObjP;

    instance = LWM2M_LIST_FIND(dataP->securityObjP->instanceList, secObjInstID);
    if (instance == NULL) return NULL;


    newConnP = connection_create(dataP->connList, dataP->sock, securityObj, instance->id, dataP->lwm2mH, dataP->addressFamily);
    if (newConnP == NULL)
    {
        fprintf(stderr, "Connection creation failed.\n");
        return NULL;
    }

    dataP->connList = newConnP;
    LOGD("lwm2m_connect_server E");
    return (void *)newConnP;
}
#endif

void lwm2m_close_connection(void * sessionH,
                            void * userData)
{
    client_data_t * app_data;
#ifdef WITH_TLS
    dtls_connection_t * targetP;
#else
    connection_t * targetP;
#endif

    app_data = (client_data_t *)userData;
#ifdef WITH_TLS
    targetP = (dtls_connection_t *)sessionH;
#else
    targetP = (connection_t *)sessionH;
#endif

    if (targetP == app_data->connList)
    {
        app_data->connList = targetP->next;
        lwm2m_free(targetP);
    }
    else
    {
#ifdef WITH_TLS
        dtls_connection_t * parentP;
#else
        connection_t * parentP;
#endif

        parentP = app_data->connList;
        while (parentP != NULL && parentP->next != targetP)
        {
            parentP = parentP->next;
        }
        if (parentP != NULL)
        {
            parentP->next = targetP->next;
            lwm2m_free(targetP);
        }
    }
}

static void prv_output_servers(char * buffer,
                               void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *) user_data;
    lwm2m_server_t * targetP;

    targetP = lwm2mH->bootstrapServerList;

    if (lwm2mH->bootstrapServerList == NULL)
    {
        fprintf(stdout, "No Bootstrap Server.\r\n");
    }
    else
    {
        fprintf(stdout, "Bootstrap Servers:\r\n");
        for (targetP = lwm2mH->bootstrapServerList ; targetP != NULL ; targetP = targetP->next)
        {
            fprintf(stdout, " - Security Object ID %d", targetP->secObjInstID);
            fprintf(stdout, "\tHold Off Time: %lu s", (unsigned long)targetP->lifetime);
            fprintf(stdout, "\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                fprintf(stdout, "DEREGISTERED\r\n");
                break;
            case STATE_BS_HOLD_OFF:
                fprintf(stdout, "CLIENT HOLD OFF\r\n");
                break;
            case STATE_BS_INITIATED:
                fprintf(stdout, "BOOTSTRAP INITIATED\r\n");
                break;
            case STATE_BS_PENDING:
                fprintf(stdout, "BOOTSTRAP PENDING\r\n");
                break;
            case STATE_BS_FINISHED:
                fprintf(stdout, "BOOTSTRAP FINISHED\r\n");
                break;
            case STATE_BS_FAILED:
                fprintf(stdout, "BOOTSTRAP FAILED\r\n");
                break;
            default:
                fprintf(stdout, "INVALID (%d)\r\n", (int)targetP->status);
            }
        }
    }

    if (lwm2mH->serverList == NULL)
    {
        fprintf(stdout, "No LWM2M Server.\r\n");
    }
    else
    {
        fprintf(stdout, "LWM2M Servers:\r\n");
        for (targetP = lwm2mH->serverList ; targetP != NULL ; targetP = targetP->next)
        {
            fprintf(stdout, " - Server ID %d", targetP->shortID);
            fprintf(stdout, "\tstatus: ");
            switch(targetP->status)
            {
            case STATE_DEREGISTERED:
                fprintf(stdout, "DEREGISTERED\r\n");
                break;
            case STATE_REG_PENDING:
                fprintf(stdout, "REGISTRATION PENDING\r\n");
                break;
            case STATE_REGISTERED:
                fprintf(stdout, "REGISTERED\tlocation: \"%s\"\tLifetime: %lus\r\n", targetP->location, (unsigned long)targetP->lifetime);
                break;
            case STATE_REG_UPDATE_PENDING:
                fprintf(stdout, "REGISTRATION UPDATE PENDING\r\n");
                break;
            case STATE_DEREG_PENDING:
                fprintf(stdout, "DEREGISTRATION PENDING\r\n");
                break;
            case STATE_REG_FAILED:
                fprintf(stdout, "REGISTRATION FAILED\r\n");
                break;
            default:
                fprintf(stdout, "INVALID (%d)\r\n", (int)targetP->status);
            }
        }
    }
}

static void prv_change(char * buffer,
                       void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *) user_data;
    lwm2m_uri_t uri;
    char * end = NULL;
    int result;

    end = get_end_of_arg(buffer);
    if (end[0] == 0) goto syntax_error;

    result = lwm2m_stringToUri(buffer, end - buffer, &uri);
    if (result == 0) goto syntax_error;

    buffer = get_next_arg(end, &end);

    if (buffer[0] == 0)
    {
        fprintf(stderr, "report change!\n");
        lwm2m_resource_value_changed(lwm2mH, &uri);
    }
    else
    {
        handle_value_changed(lwm2mH, &uri, buffer, end - buffer);
    }
    return;

syntax_error:
    fprintf(stdout, "Syntax error !\n");
}

static void prv_object_list(char * buffer,
                            void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    lwm2m_object_t * objectP;

    for (objectP = lwm2mH->objectList; objectP != NULL; objectP = objectP->next)
    {
        if (objectP->instanceList == NULL)
        {
            fprintf(stdout, "/%d ", objectP->objID);
        }
        else
        {
            lwm2m_list_t * instanceP;

            for (instanceP = objectP->instanceList; instanceP != NULL ; instanceP = instanceP->next)
            {
                fprintf(stdout, "/%d/%d  ", objectP->objID, instanceP->id);
            }
        }
        fprintf(stdout, "\r\n");
    }
}

static void prv_instance_dump(lwm2m_object_t * objectP,
                              uint16_t id)
{
    int numData;
    lwm2m_data_t * dataArray;
    uint16_t res;

    numData = 0;
    res = objectP->readFunc(id, &numData, &dataArray, objectP);
    if (res != COAP_205_CONTENT)
    {
        LOG("Error ");
        print_status(stdout, res);
        LOG("\r\n");
        return;
    }

    dump_tlv(stdout, numData, dataArray, 0);
}


static void prv_object_dump(char * buffer,
                            void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *) user_data;
    lwm2m_uri_t uri;
    char * end = NULL;
    int result;
    lwm2m_object_t * objectP;

    end = get_end_of_arg(buffer);
    if (end[0] == 0) goto syntax_error;

    result = lwm2m_stringToUri(buffer, end - buffer, &uri);
    if (result == 0) goto syntax_error;
    if (LWM2M_URI_IS_SET_RESOURCE(&uri)) goto syntax_error;

    objectP = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, uri.objectId);
    if (objectP == NULL)
    {
        fprintf(stdout, "Object not found.\n");
        return;
    }

    if (LWM2M_URI_IS_SET_INSTANCE(&uri))
    {
        prv_instance_dump(objectP, uri.instanceId);
    }
    else
    {
        lwm2m_list_t * instanceP;

        for (instanceP = objectP->instanceList; instanceP != NULL ; instanceP = instanceP->next)
        {
            fprintf(stdout, "Instance %d:\r\n", instanceP->id);
            prv_instance_dump(objectP, instanceP->id);
            fprintf(stdout, "\r\n");
        }
    }

    return;

syntax_error:
    fprintf(stdout, "Syntax error !\n");
}

static void prv_update(char * buffer,
                       void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    if (buffer[0] == 0) goto syntax_error;

    uint16_t serverId = (uint16_t) atoi(buffer);
    int res = lwm2m_update_registration(lwm2mH, serverId, false);
    if (res != 0)
    {
        fprintf(stdout, "Registration update error: ");
        print_status(stdout, res);
        fprintf(stdout, "\r\n");
    }
    return;

syntax_error:
    fprintf(stdout, "Syntax error !\n");
}

static void update_battery_level(lwm2m_context_t * context)
{
    static time_t next_change_time = 0;
    time_t tv_sec;

    tv_sec = lwm2m_gettime();
    if (tv_sec < 0) return;

    if (next_change_time < tv_sec)
    {
        char value[15];
        int valueLength;
        lwm2m_uri_t uri;
        int level = rand() % 100;

        if (0 > level) level = -level;
        if (lwm2m_stringToUri("/3/0/9", 6, &uri))
        {
            valueLength = sprintf(value, "%d", level);
            fprintf(stderr, "New Battery Level: %d\n", level);
            handle_value_changed(context, &uri, value, valueLength);
        }
        level = rand() % 20;
        if (0 > level) level = -level;
        next_change_time = tv_sec + level + 10;
    }
}

static void prv_add(char * buffer,
                    void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    lwm2m_object_t * objectP;
    int res;

    objectP = get_test_object();
    if (objectP == NULL)
    {
        fprintf(stdout, "Creating object 31024 failed.\r\n");
        return;
    }
    res = lwm2m_add_object(lwm2mH, objectP);
    if (res != 0)
    {
        fprintf(stdout, "Adding object 31024 failed: ");
        print_status(stdout, res);
        fprintf(stdout, "\r\n");
    }
    else
    {
        fprintf(stdout, "Object 31024 added.\r\n");
    }
    return;
}

static void prv_remove(char * buffer,
                       void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    int res;

    res = lwm2m_remove_object(lwm2mH, 31024);
    if (res != 0)
    {
        fprintf(stdout, "Removing object 31024 failed: ");
        print_status(stdout, res);
        fprintf(stdout, "\r\n");
    }
    else
    {
        fprintf(stdout, "Object 31024 removed.\r\n");
    }
    return;
}

#ifdef LWM2M_BOOTSTRAP

static void prv_initiate_bootstrap(char * buffer,
                                   void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    lwm2m_server_t * targetP;

    // HACK !!!
    lwm2mH->state = STATE_BOOTSTRAP_REQUIRED;
    targetP = lwm2mH->bootstrapServerList;
    while (targetP != NULL)
    {
        targetP->lifetime = 0;
        targetP = targetP->next;
    }
}

static void prv_display_objects(char * buffer,
                                void * user_data)
{
    lwm2m_context_t * lwm2mH = (lwm2m_context_t *)user_data;
    lwm2m_object_t * object;

    for (object = lwm2mH->objectList; object != NULL; object = object->next){
        if (NULL != object) {
            switch (object->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                display_security_object(object);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                display_server_object(object);
                break;
            case LWM2M_ACL_OBJECT_ID:
                break;
            case LWM2M_DEVICE_OBJECT_ID:
                display_device_object(object);
                break;
            case LWM2M_CONN_MONITOR_OBJECT_ID:
                break;
            case LWM2M_FIRMWARE_UPDATE_OBJECT_ID:
                display_firmware_object(object);
                break;
            case LWM2M_LOCATION_OBJECT_ID:
                display_location_object(object);
                break;
            case LWM2M_CONN_STATS_OBJECT_ID:
                break;
            case TEST_OBJECT_ID:
                display_test_object(object);
                break;
            }
        }
    }
}

static void prv_display_backup(char * buffer,
        void * user_data)
{
   int i;
   for (i = 0 ; i < BACKUP_OBJECT_COUNT ; i++) {
       lwm2m_object_t * object = backupObjectArray[i];
       if (NULL != object) {
           switch (object->objID)
           {
           case LWM2M_SECURITY_OBJECT_ID:
               display_security_object(object);
               break;
           case LWM2M_SERVER_OBJECT_ID:
               display_server_object(object);
               break;
           default:
               break;
           }
       }
   }
}

static void prv_backup_objects(lwm2m_context_t * context)
{
    uint16_t i;

    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
        backupObjectArray[i] = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
        memset(backupObjectArray[i], 0, sizeof(lwm2m_object_t));
    }

    /*
     * Backup content of objects 0 (security) and 1 (server)
     */
    copy_security_object(backupObjectArray[0], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID));
    copy_server_object(backupObjectArray[1], (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID));
}

static void prv_restore_objects(lwm2m_context_t * context)
{
    lwm2m_object_t * targetP;

    /*
     * Restore content  of objects 0 (security) and 1 (server)
     */
    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SECURITY_OBJECT_ID);
    // first delete internal content
    clean_security_object(targetP);
    // then restore previous object
    copy_security_object(targetP, backupObjectArray[0]);

    targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(context->objectList, LWM2M_SERVER_OBJECT_ID);
    // first delete internal content
    clean_server_object(targetP);
    // then restore previous object
    copy_server_object(targetP, backupObjectArray[1]);

    // restart the old servers
    fprintf(stdout, "[BOOTSTRAP] ObjectList restored\r\n");
    LOGD("prv_restore_objects [BOOTSTRAP] ObjectList restored");
}

static void update_bootstrap_info(lwm2m_client_state_t * previousBootstrapState,
        lwm2m_context_t * context)
{
    if (*previousBootstrapState != context->state)
    {
        *previousBootstrapState = context->state;
        switch(context->state)
        {
            case STATE_BOOTSTRAPPING:
#ifdef WITH_LOGS
                fprintf(stdout, "[BOOTSTRAP] backup security and server objects\r\n");
#endif
                LOGD("update_bootstrap_info - [BOOTSTRAP] backup security and server objects, break");
                prv_backup_objects(context);
                break;
            default:
                break;
        }
    }
}

static void close_backup_object()
{
    int i;
    for (i = 0; i < BACKUP_OBJECT_COUNT; i++) {
        if (NULL != backupObjectArray[i]) {
            switch (backupObjectArray[i]->objID)
            {
            case LWM2M_SECURITY_OBJECT_ID:
                clean_security_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            case LWM2M_SERVER_OBJECT_ID:
                clean_server_object(backupObjectArray[i]);
                lwm2m_free(backupObjectArray[i]);
                break;
            default:
                break;
            }
        }
    }
}
#endif

void print_usage(void)
{
    fprintf(stdout, "Usage: lwm2mclient [OPTION]\r\n");
    fprintf(stdout, "Launch a LWM2M client.\r\n");
    fprintf(stdout, "Options:\r\n");
    fprintf(stdout, "  -n NAME\tSet the endpoint name of the Client. Default: testlwm2mclient\r\n");
    fprintf(stdout, "  -l PORT\tSet the local UDP port of the Client. Default: 56830\r\n");
    fprintf(stdout, "  -h HOST\tSet the hostname of the LWM2M Server to connect to. Default: localhost\r\n");
    fprintf(stdout, "  -p PORT\tSet the port of the LWM2M Server to connect to. Default: "LWM2M_STANDARD_PORT_STR"\r\n");
    fprintf(stdout, "  -4\t\tUse IPv4 connection. Default: IPv6 connection\r\n");
    fprintf(stdout, "  -t TIME\tSet the lifetime of the Client. Default: 300\r\n");
    fprintf(stdout, "  -b\t\tBootstrap requested.\r\n");
    fprintf(stdout, "  -c\t\tChange battery level over time.\r\n");
#ifdef WITH_TLS
    fprintf(stdout, "  -i STRING\tSet the device management or bootstrap server PSK identity. If not set use none secure mode\r\n");
    fprintf(stdout, "  -s HEXSTRING\tSet the device management or bootstrap server Pre-Shared-Key. If not set use none secure mode\r\n");
#endif
    fprintf(stdout, "\r\n");
}

// jambo add 20200226
#define BUFLEN 20480
static void lwm2m_notify_network_change(){
	int fd, retval;
	char buf[BUFLEN] = {0};
	int len = BUFLEN;
	struct sockaddr_nl addr;
	struct nlmsghdr *nh;
	struct ifinfomsg *ifinfo;
	struct rtattr *attr;
	att_apn_id *apn_id = lwm2m_get_apnid();

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
	memset(&addr, 0, sizeof(addr));
	addr.nl_family = AF_NETLINK;
	addr.nl_groups = RTNLGRP_LINK;
	bind(fd, (struct sockaddr*)&addr, sizeof(addr));
	while ((retval = read(fd, buf, BUFLEN)) > 0)
	{
		for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))
		{
			if (nh->nlmsg_type == NLMSG_DONE)
				break;
			else if (nh->nlmsg_type == NLMSG_ERROR)
				return -1;
			else if (nh->nlmsg_type != RTM_NEWLINK)
				continue;
			ifinfo = NLMSG_DATA(nh);
			LOG_ARG("%u: %s", ifinfo->ifi_index,
					(ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );

			attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));
			len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));
			for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))
			{
				if (attr->rta_type == IFLA_IFNAME)
				{
					LOG_ARG(" %s", (char*)RTA_DATA(attr));
                    if(!(ifinfo->ifi_flags & IFF_LOWER_UP)){
                    LOG("init network.");
                    if(lwm2m_check_is_att_netwrok()){
                        sleep(3);
                        if(strcmp(apn_id->global_ifname,(char*)RTA_DATA(attr)) == 0 ){
                            enableATTGlobalNetworkByprofile(true,4,ATT_M2M_GLOBAL);
                        }else{
                            lwm2m_enableDefaultApn();
                        }
                    }
                }
			    break;
				}
			}
			printf("\n");
		}
	}
	return 0;
}


static void lwm2m_client_receive_at_command(lwm2m_context_t * lwm2mH)
{
	int fd;
	char buf[512];
    char*p;
    lwm2m_hostdevice_info_t  hostdev_info;

	if(mkfifo(FIFO, 0777) != 0 && errno != EEXIST)
	{
		 perror("Mkfifo error");
	}
	printf("open for reading... \n");
//	fd=open(FIFO,O_RDONLY);
	fd=open(FIFO,O_RDONLY|O_NONBLOCK);
	printf("opened ... \n");
	if(fd < 0)
	{
		perror("Failed to open fifo:");
		return;
	}

	while(0 == g_quit)
	{
		int count;
		count = read(fd,buf,511);
		if(count > 0)
		{
			buf[count] = 0;
			printf("fifoReader receive a string:%s\n",buf);

            p=strtok(buf,",");
            if(p){
                if(strstr(p,"qlwreset") != 0){//joe add for vzw reset lwm2m
                    system("rm -rf /data/lwm2m");
                    system("rm -rf /persist/lwm2m");
                    exit(1);
                }else
                snprintf((char *)hostdev_info.unique_id, 128,"%s", p);
            }
            p=strtok(NULL,",");
            if(p){
                snprintf((char *)hostdev_info.manufacture, 128,"%s",p);
            }
            p=strtok(NULL,",");
            if(p){
                snprintf((char *)hostdev_info.model, 128,"%s", p);
            }
            p=strtok(NULL,",");
            if(p){
                snprintf((char *)hostdev_info.sw_version, 128,"%s", p);
            }
            lwm2m_client_write_object_resource(lwm2mH, 16, 0, 0, &hostdev_info);
            lwm2m_client_trigger_update_regist(g_lwm2mH_ptr);
		}
        sleep(1);//add to reduce CPU usage
	}
    LOG("closed ... \n");
	close(fd);
    unlink(FIFO);
    return;
}
#ifdef LWM2M_SUPPORT_QLRIL_API
//add by joe qlrild interface
static void lwm2m_notify_conn_info_change(lwm2m_context_t * lwm2mH){

    while(0 == g_quit)
    {

        lwm2m_object_t * object = (lwm2m_object_t *)LWM2M_LIST_FIND(lwm2mH->objectList, LWM2M_CONN_MONITOR_OBJECT_ID);

        //Update bearer_type Info
        int bearer_type = QL_LWM2M_GetNetworkType();
        if (NULL != object){
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                fprintf(stderr, "Internal allocation failure !\n");
                return;
            }
            dataP->id = 0;//RES_M_NETWORK_BEARER
            lwm2m_data_encode_int(bearer_type, dataP);
            connectivity_moni_change(dataP,object);
            lwm2m_data_free(1, dataP);
        }

        //Update signalStrength Info
        int signalStrength = QL_LWM2M_GetSignalStrength();
        if (NULL != object){
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                fprintf(stderr, "Internal allocation failure !\n");
                return;
            }
            dataP->id = 2;//RES_M_RADIO_SIGNAL_STRENGTH
            lwm2m_data_encode_int(signalStrength, dataP);
            connectivity_moni_change(dataP,object);
            lwm2m_data_free(1, dataP);
        }

        //Update linkQuality Info
        QL_LWM2M_CONN_INFO_T conn_info;
        QL_LWM2M_GetConnInfo(&conn_info);
        if (NULL != object){
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                fprintf(stderr, "Internal allocation failure !\n");
                return;
            }
            dataP->id = 3;//RES_O_LINK_QUALITY
            lwm2m_data_encode_int(conn_info.linkQuality, dataP);
            connectivity_moni_change(dataP,object);
            lwm2m_data_free(1, dataP);
        }

        //Update cellid Info
        long cellId = QL_LWM2M_GetCellInfo();
        if (NULL != object){
            lwm2m_data_t * dataP;
            int result;

            dataP = lwm2m_data_new(1);
            if (dataP == NULL)
            {
                fprintf(stderr, "Internal allocation failure !\n");
                return;
            }
            dataP->id = 8;//RES_O_CELL_ID
            lwm2m_data_encode_int(cellId, dataP);
            connectivity_moni_change(dataP,object);
            lwm2m_data_free(1, dataP);
        }

        sleep(60);//update network connect info every 1 minute
    }
    LOG("closed ... \n");
    return;

}

void lwm2m_processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
	int i = 0;
	int num = 0;
	int ret = 0;
	void *privData = NULL;
	static int times = 0;

	char radioState[100] = {0};
	char NITZTimeData[100] = {0};

	RIL_SignalStrength_v10 ss;

	if (times < 3) {
		times++;

		ret = QLRIL_GetPrivateData(handle, &privData);
		if (ret == 0) {
			//printf("QLRIL_GetPrivateData fool = %d\n", *(int *)privData);
		}
	}

	//c_printf ("[b]%s%d%s%d\n", "The unsolicited event[", event, "] come from slot ID:", slotId);

	switch (event) {
		case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED://1000
			ret = QLRIL_ParseEvent_RadioState(radioState, sizeof(radioState), data, size);
			if (ret < 0) {
				//c_printf("[r]%s%d\n", "QLRIL_ParseEvent_RadioStateChanged error with ret = ", ret);
			} else {
				/* Radio State:
				 * 0 - RADIO_OFF
				 * 1 - RADIO_UNAVAILABLE
				 * 2 - RADIO_SIM_NOT_READY
				 * 3 - RADIO_SIM_LOCKED_OR_ABSENT
				 * 4 - RADIO_SIM_READY
				 * 5 - RADIO_RUIM_NOT_READY
				 * 6 - RADIO_RUIM_READY
				 * 7 - RADIO_RUIM_LOCKED_OR_ABSENT
				 * 8 - RADIO_NV_NOT_READY
				 * 9 - RADIO_NV_READY
				 * 10 - RADIO_ON
				 */
				//c_printf("[y]%s%s%s%d\n", "RadioState: [", radioState, "] ret = ", ret);
			}
			break;
        #if 0    
		case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED://1002
		{
			char **voiceRegistrationState = NULL;
			ret = QLRIL_GetVoiceRegistrationState(handle, slotId, &voiceRegistrationState);
			if (ret < 0) {
				//c_printf("[r]%s%d\n", "QLRIL_GetVoiceRegistrationState error with ret = ", ret);
			} else {
				/*c_printf("[b]%s\n", "response[0]: e.g. 1 - Registered, home network\n"
				  "response[1]: LAC\n"
				  "response[2]: Cell ID\n"
				  "response[3]: refer to RIL_RadioTechnology in ril.h e.g.:\n"
				  "3  - UMTS; 14 - LTE; 16 - GSM;");*/
				//c_printf("[y]%s%d\n", "QLRIL_GetVoiceRegistrationState ret = ", ret);
				for (i = 0; i < ret && i < 14; i++) {
					if (voiceRegistrationState[i] == NULL)
						continue;

					/* For more information to RIL_REQUEST_VOICE_REGISTRATION_STATE in
					 * include/telephony/ril.h
					 * response[0]:
					 *		1 - Registered, home network
					 *		2 - Not registered, but MT is currently searching a new operator to register
					 *		...
					 * response[1]: LAC
					 * response[2]: Cell ID
					 * response[3]: refer to RIL_RadioTechnology in ril.h e.g.:
					 *	3  - UMTS
					 *	14 - LTE
					 *	16 - GSM
					 */
					if (i == 0)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](1: Registered, home network; 2:Not registered...):",
								voiceRegistrationState[i]);
					else if (i == 2)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](Cell ID):",
								voiceRegistrationState[i]);
					else if (i == 3)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](RadioTechnology: 3: UMTS; 14: LTE; 16:GSM):",
								voiceRegistrationState[i]);
					else
						c_printf("[g]%s%d%s%s\n", "response[", i, "]:", voiceRegistrationState[i]);
					free(voiceRegistrationState[i]);
				}

				if (voiceRegistrationState) {
					free(voiceRegistrationState);
					voiceRegistrationState = NULL;
				}
			}
			break;
		}
        #endif
		case RIL_UNSOL_RESPONSE_NEW_SMS://1003
		{
			char *sms[3];
			ret = QLRIL_ParseEvent_NewSMS(&sms, sizeof(sms), data, size);
			if (ret < 0) {
				LOG_ARG("QLRIL_ParseEvent_NewSMS error with ret =%d ", ret);
			} else {
				if (sms[0]) {
					LOG_ARG("Sms details:%s", sms[0]);
					free(sms[0]);
				}

				if (sms[1]) {
					LOG_ARG("Sms message: %s", sms[1]);
                    lwm2m_client_handle_wap_sms(g_lwm2mH_ptr,sms[1],strlen(sms[1]));
					free(sms[1]);
				}
                /*
				if (sms[2]) {
					LOG_ARG("Sms raw data of pdu: %s", sms[2]);
					free(sms[2]);
				}
				*/
			}
            #if 0
			ret = QLRIL_AcknowledgeLastIncomingGsmSms(handle, 1, 0xD3);
			if (ret == 0)
				c_printf("[g]%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
			else
				c_printf("[r]%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms ret = ", ret);
			break;
            #endif
            break;
		}
		case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS://1020
		{
			RIL_CDMA_SMS_Message *sms = NULL;
			RIL_CDMA_SMS_ClientBd *cbd = NULL;
			ret = QLRIL_ParseEvent_CdmaSMS((void **)&sms, data, size);
			if (ret != 0)
				LOG_ARG("QLRIL_ParseEvent_CdmaSMS error:%d\n", ret);
			else {
				LOG_ARG("address:%s\n", sms->sAddress.digits);
				ret = QLRIL_DecodeBearerData((void **)&cbd, sms->aBearerData, sms->uBearerDataLen);
				if (ret != 0)
					LOG_ARG("QLRIL_DecodeBearerData error:%d\n", ret);
				else {
					LOG_ARG("type:%d\n", cbd->message_id.type);
					LOG_ARG("id_number:%d\n", cbd->message_id.id_number);
					LOG_ARG("udh_present:%d\n", cbd->message_id.udh_present);
					LOG_ARG("encoding:%d\n", cbd->user_data.encoding);
					LOG_ARG("number_of_digits:%d\n", cbd->user_data.number_of_digits);
					LOG_ARG("content:%s\n", cbd->user_data.data);
					LOG_ARG("data_len:%d\n", cbd->user_data.data_len);
					LOG_ARG("padding_bits:%d\n", cbd->user_data.padding_bits);
				if (cbd->user_data.data) {
					LOG_ARG("cmda message: %s\n", cbd->user_data.data);
                    lwm2m_client_handle_wap_sms(g_lwm2mH_ptr,cbd->user_data.data,cbd->user_data.data_len);
				}                    
				}
			}

			if (sms) {
				free(sms);
				sms = NULL;
			}

			if (cbd) {
				free(cbd);
				cbd = NULL;
			}
			break;
		}
        #if 0
		case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT://1004
		{
			char sms_status[50] = {0};
			ret = QLRIL_ParseEvent_String(sms_status, sizeof(sms_status), data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_String error with ret = ", ret);
			} else {
				c_printf("[y]%s%d\n", "QLRIL_ParseEvent_String success length = ", ret);
				c_printf("[b]%s[g]%s\n", "the PDU of an SMS-STATUS-REPORT: ", sms_status);
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM://1005
		{
			int *index = NULL;
			ret = QLRIL_ParseEvent_Ints((int **)&index, data, size);
			if (ret < 0)
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_Ints failed with ret = ", ret);
			else {
				c_printf("[y]%s\n", "The slot index on the SIM that contains the new message:");
				c_printf("[g]%s%d\n", "QLRIL_ParseEvent_Ints ret = ", ret);
				if (index == NULL)
					break;

				for(i = 0; i < ret; i++) {
					c_printf("[g]%s%d%s%d\n", "index[", i, "] = ", index[i]);
				}

				if (index) {
					free(index);
					index = NULL;
				}
			}

			break;
		}
        #endif
        #if 0
		case RIL_UNSOL_SIGNAL_STRENGTH://1009
			ret = QLRIL_ParseEvent_SignalStrength(&ss, sizeof(ss), data, size);
			if (ret != 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_SignalStrength error with ret = ", ret);
			} else {
				c_printf("[y]%s%d%s\n", "SIM card[", slotId, "] signal strength Info:", slotId);
				/**
				 * UINT_MAX = 2147483647
				 */
				printf("[signalStrength=%d, bitErrorRate=%d, "
						"CDMA_SS.dbm=%d, CDMA_SSecio=%d, "
						"EVDO_SS.dbm=%d, EVDO_SS.ecio=%d, "
						"EVDO_SS.signalNoiseRatio=%d, \n"
						"LTE_SS.signalStrength=%d, LTE_SS.rsrp=%d, LTE_SS.rsrq=%d, "
						"LTE_SS.rssnr=%d, LTE_SS.cqi=%d, TDSCDMA_SS.rscp=%d]\n",
						ss.GW_SignalStrength.signalStrength,
						ss.GW_SignalStrength.bitErrorRate,
						ss.CDMA_SignalStrength.dbm,
						ss.CDMA_SignalStrength.ecio,
						ss.EVDO_SignalStrength.dbm,
						ss.EVDO_SignalStrength.ecio,
						ss.EVDO_SignalStrength.signalNoiseRatio,
						ss.LTE_SignalStrength.signalStrength,
						ss.LTE_SignalStrength.rsrp,
						ss.LTE_SignalStrength.rsrq,
						ss.LTE_SignalStrength.rssnr,
						ss.LTE_SignalStrength.cqi,
						ss.TD_SCDMA_SignalStrength.rscp);
			}
			break;
        #endif
        #if 0
		case RIL_UNSOL_DATA_CALL_LIST_CHANGED://1010
		{
			char buf[200] = {0};

			RIL_Data_Call_Response_v11 *dc = NULL;
			ret = QLRIL_ParseEvent_DataCallList((void **)&dc, data, size);
			if (ret < 0) {
				c_printf("[b]%s%d\n", "QLRIL_ParseEvent_DataCallList failed with return: ", ret);
			} else {
				c_printf ("[b]%s%d\n", "QLRIL_ParseEvent_DataCallList items = ", ret);

				if (ret == 0)
					break;

				if (dc == NULL) {
					c_printf("[r]%s\n", "Error RIL_Data_Call_Response_v11 memory pointer is NULL");
					break;
				}

				if (ret >= 20) {
					c_printf("[r]%s\n", "Warning!!! Data call list items is too many");
				}

				for (i = 0; i < ret; i++) {
					c_printf ("[b]%s\n", "DataCallList response:");
					snprintf (buf, sizeof(buf), "status:%d, retry:%d, callID:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s, pcscf:%s, mtu:%d\n",
							dc[i].status, dc[i].suggestedRetryTime,
							dc[i].cid, (dc[i].active == 0)?"down":"up",
							dc[i].type, dc[i].ifname, dc[i].addresses,
							dc[i].dnses, dc[i].gateways, dc[i].pcscf, dc[i].mtu);
					buf[sizeof(buf) - 1] = 0;//prevent stack overflows
					c_printf("[g]%s\n", buf);

					if (dc[i].type)
						free(dc[i].type);
					if (dc[i].ifname)
						free(dc[i].ifname);
					if (dc[i].addresses)
						free(dc[i].addresses);
					if (dc[i].dnses)
						free(dc[i].dnses);
					if (dc[i].gateways)
						free(dc[i].gateways);
					if (dc[i].pcscf)
						free(dc[i].pcscf);
				}
			}

			if (dc) {
				free(dc);
				dc = NULL;
			}

			break;
		}
        #endif
        #if 0
		case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED://1035
			ret = QLRIL_ParseEvent_VoiceRadioTechChanged(radioState, sizeof(radioState), data, size);
			if (ret < 0) {
				//c_printf("[r]%s%d\n", "QLRIL_ParseEvent_VoiceRadioTechChanged error with ret = ", ret);
			} else {
				//c_printf("[y]%s%s%s%d\n", "RadioState: [", radioState, "] ret = ", ret);
			}
			break;
        #endif
		default:
			break;
	}
}

void lwm2m_init_QLRIL(){
	int ret = 0;
    //while(qlril_handle_flag) {
        //ret = QLMISC_IsRunning("qlrild");
        //LOG_ARG("get qlrild isRunning %d \n",ret);
        //if (ret > 0) {
            if (g_qlril_handle == 0) {
                ret = QLRIL_Init(&g_qlril_handle);
                if (ret == 0){
                    LOG_ARG("QLRIL_Init success g_qlril_handle=%d\n",g_qlril_handle);

                    int fool = 12345;//for test private data
                    ret = QLRIL_AddUnsolEventsListener(&g_qlril_handle, lwm2m_processUnsolInd_cb, (void *)&fool);

                    ret = QLRIL_RegisterUnsolEvents(&g_qlril_handle, 1000, 1048);
                }else{
                    LOG_ARG("QLRIL_Init failure ret = %d", ret);
                }
            }
           // qlril_handle_flag = 0;
        //}
    //}
    LOG("lwm2m_init_QLRIL end \n");

}
#endif
//add by joe start
bool lwm2m_check_is_vzw_netwrok(){
	char imsi_vzw[][8]  = {"310590", "310890", "311480", "311270", "312770"};
	char imsi[64] = {0};
	int i = 0;
	memset(imsi, 0, 64);
    if(g_is_vzw_network == 1){
        return true;
    }else if(g_is_vzw_network == 0){
        return false;
    }
    LOG("LwM2M start QL_LWM2M_GetImsi!\n");
	int ret =QL_LWM2M_GetImsi(&imsi);
    if(ret != 0){
        LOG("lwm2m_check_is_vzw_netwrok QL_LWM2M_GetImsi failed\n");
        g_is_vzw_network = 0;//set flag
        g_is_att_network = 0;
        return false;
    }
	for (i = 0; i < sizeof(imsi_vzw) / sizeof(imsi_vzw[0]); i++) {
         if (strncasecmp(imsi, imsi_vzw[i], strlen(imsi_vzw[i])) == 0 ) {
               g_is_vzw_network = 1;//set flag
               g_is_att_network = 0;
               LOG("g_is_vzw_network set to true\n");
               return true;
            }
    }

	return false;
}

bool lwm2m_check_is_att_lab(){
	char imsi_att_lab[8]  = {"310170"};
	char value_get[64] = {0};
	if(0 == QL_LWM2M_GetImsi(&value_get))
	{
		if(strncasecmp(value_get, imsi_att_lab, strlen(imsi_att_lab)) == 0){
			return true;
		}
	}
	return false;
}

bool lwm2m_check_is_att_netwrok(){
	char imsi_att[][8]  = {"310030", "310170", "310410",
                           "313110", "312670", "313100", "313120", "313130", "313140"};
    char iccid_att[][8] = {"8901410", "8901030", "8901150", "8901170", "8901560",
                           "8901680", "890138"};
	char value_get[64] = {0};
	int i = 0;
    if(g_is_att_network == 1){
        return true;
    }else if(g_is_att_network == 0){
        return false;
    }
    LOG("LwM2M start QL_LWM2M_GetImsi check att!\n");

	if(0 == QL_LWM2M_GetImsi(&value_get))
	{
		for (i = 0; i < sizeof(imsi_att) / sizeof(imsi_att[0]); i++)
		{
			if (strncasecmp(value_get, imsi_att[i], strlen(imsi_att[i])) == 0 )
			{
                g_is_att_network = 1;//set flag
                g_is_vzw_network = 0;
                LOG("g_is_att_network set to true\n");
				return true;
			}
		}
	}else if(0 == QL_LWM2M_GetIccid(&value_get))
	{
		for (i = 0; i < sizeof(iccid_att) / sizeof(iccid_att[0]); i++)
		{
			if (strncasecmp(value_get, iccid_att[i], strlen(iccid_att[i])) == 0 )
			{
			    g_is_att_network = 1;//set flag
			    g_is_vzw_network = 0;
                LOG("g_is_att_network set to true\n");
				return true;
			}
		}
	}else{
        LOG("lwm2m_check_is_att_netwrok QL_LWM2M_GetImsi&QL_LWM2M_GetIccid failed\n");
        g_is_vzw_network = 0;//set flag
        g_is_att_network = 0;
        return false;
    }

	return false;
}

void lwm2m_verizon_save_BS_server_url_from_atcmd(char* server_url){
    FILE *fp;
    int i;
    char* full_regist_state = "true";
    char *filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
    LOG_ARG("Got AT cmd verizon BS server_url=%s\n",server_url);
  
    if (access(filename, F_OK) == 0)
    {
        LOG("if QUEC_LWM2M_BS_SERVER_VZW_INI exist, remove it");
		remove(QUEC_LWM2M_BS_SERVER_VZW_INI);
    }

    fp = fopen(QUEC_LWM2M_BS_SERVER_VZW_INI,"w+");
    if(fp == NULL)
    {
        LOG("file QUEC_LWM2M_BS_SERVER_VZW_INI error: \n");
        perror("fopen: ");
        return -1;
    }
    fwrite(server_url, strlen(server_url), 1, fp);
    fclose(fp);
    LOG("Save verizon BS server_url exit\n");

}

bool lwm2m_verion_bs_lab_test_server_exist(){

    char *bs_server_filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
    LOG("enter check lwm2m_verion_bs_lab_test_server_exist\n");
    if (access(bs_server_filename, F_OK) == 0)
    {
        LOG_ARG("Verizon bs lab test server URL=%s exist !\n",QUEC_LWM2M_BS_SERVER_VZW_INI);
        return true;
    }

    return false;

}

void lwm2m_verizon_load_persist_bs_server_info(){
    int fd;
    char buf[512];
    char*p;
    int at_cmd_quit = 0;
    
    char *bs_server_filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
    LOG("enter  lwm2m_verizon_load_persist_bs_server_info\n");
    if (access(bs_server_filename, F_OK) == 0)
    {
        LOG_ARG("Verizon bs server URL=%s exist !\n",QUEC_LWM2M_BS_SERVER_VZW_INI);
        return;
    }

    if(mkfifo(FIFO, 0777) != 0 && errno != EEXIST)
    {
         perror("Mkfifo error");
    }
    LOG("open for reading... \n");
//  fd=open(FIFO,O_RDONLY);
    fd=open(FIFO,O_RDONLY|O_NONBLOCK);
    LOG("opened ... \n");
    if(fd < 0)
    {
        perror("Failed to open fifo:");
        return;
    }

    while(at_cmd_quit == 0)
    {
        int count;
        count = read(fd,buf,511);
        if(count > 0)
        {
            buf[count] = 0;
            LOG_ARG("fifoReader receive a string:%s\n",buf);

            p=strtok(buf,",");
            if(p && strstr(p,"qlwcfgbs") != 0){
                LOG_ARG("receive token0 string:%s\n",p);
            }
            p=strtok(NULL,",");
            if(p){
                LOG_ARG("receive token1 string:%s\n",p);
                lwm2m_verizon_save_BS_server_url_from_atcmd(p);
            }
            at_cmd_quit = 1;
        }
    }
    LOG("closed ... \n");
    close(fd);
    unlink(FIFO);
    LOG("exit lwm2m_verizon_load_persist_bs_server_info\n");
    return;
}

bool lwm2m_client_handle_wap_sms(lwm2m_context_t *lwm2mH, uint8_t *data, uint16_t data_len)
{
	/*
	01 Transaction ID / Push ID WSP layer (start WSP headers) 
	06 PDU type (push) 
	03 Headerslength (content type+headers) 
	C4 Content type code MIME-Type 
	AF X-WAP-Application-ID 
	87 Id for urn: x-wap-application:syncml.dm WSP layer (end WSP headers)
	*/
	const char wsp_header1[6] = {0x01, 0x06, 0x03, 0xC4, 0xAF, 0x87 };
	const char wsp_header2[6] = {0x01, 0x06, 0x03, 0xC4, 0xAF, 0x9a };
	const char wsp_header3[8] = {0x01, 0x06, 0x05, 0x1F, 0x01, 0xC4, 0xAF, 0x9a };
	lwm2m_context_t * mLwm2mcontxt_ptr = g_lwm2mH_ptr;
	coap_packet_t  message;
	//lwm2m_uri_t *uriP_sms = NULL;
    lwm2m_uri_t uriP_sms ;
	coap_status_t  ret_code;
	lwm2m_object_t * objectP = NULL;
	uint8_t server_count = 0;
	uint16_t shortId[10] = {0};
    
#ifdef AUTHORIZATION_SUPPORT
	acc_ctrl_oi_t *acc_ctrl_oi = NULL;
  	acc_ctrl_ri_t *acc_ctrl_ri = NULL;
#endif    
	bool isAuthorized = LWM2M_TRUE; //Joe modify for upate regist ,default is LWM2M_FALSE
	lwm2m_server_t * server = NULL;
	lwm2m_server_t * targetP = NULL ;
    lwm2m_object_t *serv_obj;	
	server_instance_t* serv_obj_inst;
	int i = 0,j =0;
	uint8_t match = 0;
    int wsp_header_length = 0;
    
	if(mLwm2mcontxt_ptr == NULL){
        LOG(" [LWM2M]mLwm2mcontxt_ptr== NULL \n");
		return false;
       }   

    for (int i = 0; i < data_len; i++)
    {
        LOG_ARG("[LWM2M]data:  %d\n",data[i]);//joe del because too many logs
    }
    
	for(j = 0; j < 15; j++){
	    if (memcmp(&data[j], wsp_header1, sizeof(wsp_header1)) == 0
			||memcmp(&data[j], wsp_header2, sizeof(wsp_header2)) == 0)
   		{
    		match = 1;
            wsp_header_length = 6;
            LOG_ARG(" [LWM2M] match = 1 wsp_header_length=%d\n",wsp_header_length);
			break;
		}else if(memcmp(&data[j], wsp_header3, sizeof(wsp_header3)) == 0){
    		match = 1;
            wsp_header_length = 8;
            LOG_ARG(" [LWM2M] match = 1 wsp_header_length=%d\n",wsp_header_length);
			break;
        }
	}

    LOG_ARG(" [LWM2M]sms match: %d\n",match);
	if(match == 0){
		return false;
	}
   
	data_len -= j;
	data += j; //skip invalid data

	data_len -= wsp_header_length;//exclude wsp header
	data += wsp_header_length;
    
   // uriP_sms = malloc(sizeof(lwm2m_uri_t));
    
	memset(&message, 0, sizeof(coap_packet_t));
    LOG_ARG("[LWM2M]data_len:  %d\n",data_len);
    if(data_len == 0){
        LOG("[LWM2M]Verizon wap push received, but decode message wrong data_len==0!!!\n");
        return;
    }

    #if 0
    for (int i = 0; i < data_len; i++)
    {
        LOG_ARG("[LWM2M]data:  %d\n",data[i]);//joe del because too many logs
    }
    #endif

	ret_code = coap_parse_message(&message, data, data_len);
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]ret_code: %d", ret_code);
    LOG_ARG("[LWM2M]ret_code:  %d\n",ret_code);
	if(ret_code != NO_ERROR)
	{
		return false;
	}
   // LOG_ARG("[LWM2M]version: %d,type:%d, code:%d\n", message.version, message.type, message.code);
    LOG_ARG("[LWM2M]version: %d\n", message.version);
	if (message.version != 1) 
    {
        LOG("[LWM2M]Verizon wap push received, but decode message wrong!!!\n");
        return false;
    }
    
    if (message.type != COAP_TYPE_NON && false == lwm2m_check_is_att_netwrok()) 
    {
        return false;
    }
    
    if (message.code != COAP_POST ) 
    {
       return false;
    }
	
	LOG_ARG("[LWM2M] message.code: %d message.type=%d\n", message.code ,message.type );	
	lwm2m_request_type_t retType = uri_decode(NULL, message.uri_path, &uriP_sms);
	LOG_ARG("[LWM2M] retType = %d uriP_sms: %p\n", retType, uriP_sms);	
    /*
	if(uriP_sms == NULL)
	{
		return false;
	}*/
	LOG_ARG("[LWM2M]uriP_sms: %d/%d/%d\n", uriP_sms.objectId, uriP_sms.instanceId, uriP_sms.resourceId);


    for (objectP = mLwm2mcontxt_ptr->objectList; objectP != NULL; objectP = objectP->next)
    {
        if (objectP->objID == LWM2M_SECURITY_OBJECT_ID)  
        {
          security_instance_t *instanceP = NULL;
          instanceP = (security_instance_t *)objectP->instanceList;
          while (NULL != instanceP)
          {
            if (instanceP->isBootstrap != LWM2M_TRUE) 
            {
              shortId[i] = instanceP->shortID;
              i++;
            }
            instanceP = instanceP->next;
          }
          break;
        }
    }
    server_count = i;
	LOG_ARG("[LWM2M]server_count: %d", server_count);
	if(server_count == 0)
    {
        return false;
    }
	
#ifdef AUTHORIZATION_SUPPORT
    acc_ctrl_oi = get_acc_cl_objectinst(mLwm2mcontxt_ptr, &uriP_sms);
	LOG_ARG("[LWM2M]acc_ctrl_oi: %p", acc_ctrl_oi);	

    if(NULL == acc_ctrl_oi)
    {
        isAuthorized = LWM2M_TRUE;
    } 
	else{
		LOG_ARG("[LWM2M]owner:%d,obj:%d", acc_ctrl_oi->accCtrlOwner,acc_ctrl_oi->objectId);	
	}
#endif
	if(isAuthorized == LWM2M_FALSE){
	    i = 0;
	    while(i < server_count){
	        server = mLwm2mcontxt_ptr->serverList;
	        while (server)
	        {
	          /* Short server ID match - server context found */
	          if (server->shortID == shortId[i])
	            break; 

	          server = server->next; 
	        }
#ifdef AUTHORIZATION_SUPPORT
					
	        mLwm2mcontxt_ptr->activeServerP = server;

	        acc_ctrl_ri = get_acc_cl_resourceinst(mLwm2mcontxt_ptr, acc_ctrl_oi);
	        mLwm2mcontxt_ptr->activeServerP = NULL;
	        if(NULL == acc_ctrl_ri && shortId[i] == acc_ctrl_oi->accCtrlOwner)
	        {
	          isAuthorized = LWM2M_TRUE;
	          break;
	        }
	        if (NULL != acc_ctrl_ri)
	        {
	          if(ACC_CTRL_PERM_EXECUTE == (ACC_CTRL_CAN_EXEC(acc_ctrl_ri->accCtrlValue))) 
	          {
	            isAuthorized = LWM2M_TRUE;
	            break;
	          }
	        }
#endif		
	        i++;
	   }

	   if(isAuthorized == LWM2M_FALSE)
   	   {
          return false;
       }    	
	}

   if((uriP_sms.objectId== LWM2M_DEVICE_OBJECT_ID) && (uriP_sms.instanceId == 0))
   {
        if(uriP_sms.resourceId == RES_O_FACTORY_RESET)
        {
            //reset
            printf("[LWM2M] %s", "factory reset");
            g_reboot = 2; //joe modify
			return true;
        }
		if(uriP_sms.resourceId == RES_M_REBOOT)
		{
			printf("[LWM2M] %s", "reboot");
            g_reboot = 1;
			return true;
		}

   }

   if(uriP_sms.objectId == 65535 && uriP_sms.instanceId == 65535 &&  uriP_sms.resourceId == 65535 && lwm2m_check_is_vzw_netwrok()){
       LOG("for vzw set default SMS URI 1/1/8\n");//for vzw network in some case sms parse error
       uriP_sms.objectId = 1;
       uriP_sms.instanceId = 1;
       uriP_sms.resourceId =8;
   }

   if(uriP_sms.objectId == LWM2M_SERVER_OBJECT_ID)
   {
       LOG("[LWM2M]uriP_sms.objectId:LWM2M_SERVER_OBJECT_ID");
        if(uriP_sms.resourceId == LWM2M_SERVER_UPDATE_ID)
        {
            LOG("[LWM2M]uriP_sms.objectId:LWM2M_SERVER_UPDATE_ID");
          targetP = mLwm2mcontxt_ptr->serverList;
  
          serv_obj      = (lwm2m_object_t *)LWM2M_LIST_FIND(mLwm2mcontxt_ptr->objectList , LWM2M_SERVER_OBJECT_ID);
          if (serv_obj == NULL)
          {
            return true;
          }
          serv_obj_inst = (server_instance_t *)LWM2M_LIST_FIND(serv_obj->instanceList, uriP_sms.instanceId);
          if (serv_obj_inst == NULL)
          {
            return true;
          }

          while(targetP != NULL)
          {
             LOG_ARG("[LWM2M] targetP->shortID=%d serv_obj_inst->shortServerId=%d",targetP->shortID,serv_obj_inst->shortServerId);
             if(targetP->shortID == serv_obj_inst->shortServerId)
               break;
             targetP = targetP->next;
          }

          if(targetP != NULL)
          {
           // targetP->pending_reg_update = TRUE;
            //targetP->isRegupdate_for_sms = TRUE;
            if(false == lwm2m_check_is_att_netwrok())
            {
                //check if resumption is needed for vzw server
                //lwm2m_client_dtls_session_resumption_for_single_server(mLwm2mcontxt_ptr,targetP);//try not resumption every minute, resumption behavior TBD
                LOG("[LWM2M] not ATT call lwm2m_update_registration\n");
                //LOG_ARG("%d Registration failed", targetP->shortID);
                char wap_sms_update[64]={0};
                snprintf(wap_sms_update, 64, "SMS received, update required ! server ID: %d", targetP->shortID);
                quec_lwm2m_client_info_notify(wap_sms_update);
                targetP->pending_reg_update = true;
                if(targetP->shortID == 102 && get_fw_update_download_state() == 1){
                    targetP->pending_reg_update = false; //do not update register when downloading
                }
				//lwm2m_update_registration(mLwm2mcontxt_ptr, targetP->shortID, 0);
            }
			else
			{
				if(targetP->disable)
				{
                    LOG("[LWM2M] targetP->disable");
					targetP->disable = false;
				}
				else
				{
                    LOG("[LWM2M] call lwm2m_update_registration");
					lwm2m_update_registration(mLwm2mcontxt_ptr, targetP->shortID, 0);
				}
			}
          }
        }

        //lwm2m_free(uriP_sms);
        //uriP_sms = NULL;
   }

   return true;
}

int lwm2m_init_att_connection(){
    int ret;
    lwm2m_init_apn_info();
    att_apn_id *apn_id = lwm2m_get_apnid();
	ret = lwm2m_getApnInfo(apn_id);
    if(ret != 0){
        LOG("lwm2m_init_linux_connection getApnInfo Fail.\n");
		return ret;
	}

	if(lwm2m_check_is_att_lab()){
//		send_AT_Command(ADD_GLOBAL_APN,NULL);
//		send_AT_Command(ADD_ATTZ_APN,NULL);
	}
//    if(apn_id->global_id != NULL){
//        lwm2m_setApnEnable(1,apn_id->global_id);
//    }
//    if(apn_id->attz_id != NULL){
//        lwm2m_setApnEnable(1,apn_id->attz_id);
//    }

    enableATTGlobalNetworkByprofile(true,4,ATT_M2M_GLOBAL);
    ret = lwm2m_enableDefaultApn();//jambo add 5-0-4-5-24 MCM_API_TEST

    return ret;
}

//add by joe start
/*currently not use*/
void lwm2m_client_trigger_update_regist(lwm2m_context_t *lwm2mH){
	int tv_sec = lwm2m_gettime();
    if(!lwm2m_check_is_vzw_netwrok()){
        return; //currently update triger only works for VZW network
    }

    dtls_connection_t * connP = NULL;
    //add by joe start
    lwm2m_server_t * temptargetP = NULL;

    temptargetP = lwm2mH->serverList;
    int serverCount = lwm2mH->serverListCount;
    for (int i = 0; i < serverCount; i++)
    //while (temptargetP != NULL )
    {
        connP = (dtls_connection_t*)temptargetP->sessionH;
        if (connP == NULL){
            temptargetP = temptargetP->next; //add by joe
            continue;
        }
        if(temptargetP->shortID == 1000 && checkifApn3unAvailable()){ //add by joe for repo server
            LOG("do not update repo server");
            temptargetP->pending_reg_update = false;
        }else{
            temptargetP->pending_reg_update = true; //set need update flag
            if(temptargetP->shortID == 102 && get_fw_update_download_state() == 1){
                temptargetP->pending_reg_update = false; //do not update register when downloading
            }
          //  lwm2m_update_registration(lwm2mH, temptargetP->shortID, 0);
           // lwm2m_update_regist_push_sms(lwm2mH, temptargetP, 0);
        }
        LOG_ARG("[LWM2M] call lwm2m_update_registration by AT cmd temptargetP->shortID=%d",temptargetP->shortID);
        temptargetP = temptargetP->next; //add by joe 
    }

}

void lwm2m_client_checkif_update_all_server_regist(lwm2m_context_t *lwm2mH){
	int tv_sec = lwm2m_gettime();

    dtls_connection_t * connP = NULL;
    //add by joe start
    lwm2m_server_t * temptargetP = NULL;

    temptargetP = lwm2mH->serverList;
    int serverCount = lwm2mH->serverListCount;
    for (int i = 0; i < serverCount; i++)   
    //while (temptargetP != NULL )
    {
        connP = (dtls_connection_t*)temptargetP->sessionH;
        if (connP == NULL){
            temptargetP = temptargetP->next; //add by joe 
            continue;
        }
        if(temptargetP->pending_reg_update == true){
            LOG_ARG("[LWM2M] temptargetP->shortID=%d, pending_reg_update==true\n",temptargetP->shortID);
            temptargetP->pending_reg_update = false;
            sleep(1);
            lwm2m_client_dtls_session_resumption_for_single_server(lwm2mH,temptargetP);//try not resumption every minute, resumption behavior TBD
            sleep(1);
            if(temptargetP->status != STATE_REG_FAILED)
            temptargetP->status = STATE_REG_UPDATE_NEEDED;
            LOG_ARG("[LWM2M] call lwm2m_update_registration temptargetP->shortID=%d\n",temptargetP->shortID);
        }/*
        else if(temptargetP->isRegisterUpdateing == false && get_fw_update_download_state() == 2 && tv_sec - temptargetP->registration >= 30 && temptargetP->shortID == 102){
            lwm2m_client_dtls_session_resumption_for_single_server(lwm2mH,temptargetP);//try not resumption every minute, resumption behavior TBD
            temptargetP->status = STATE_REG_UPDATE_NEEDED; //update register for vzw dm server when fota package is downloaded
        }*/
        
        temptargetP = temptargetP->next; //add by joe 
    }

}

void lwm2m_client_dtls_session_resumption_for_single_server(lwm2m_context_t *lwm2mH, lwm2m_server_t * targetP)
{
	int tv_sec = lwm2m_gettime();

    dtls_connection_t * connP = NULL;
    lwm2m_server_t * temptargetP = NULL;
    if(!lwm2m_check_is_vzw_netwrok()){
        return; //currently resumption only works for VZW network
    }
#ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
    //my need to check network status
    if(get_vzw_apn3_callid() == -2){
        quec_check_class2_apn_network_available();
    }else{
        quec_check_apn_network_available();
    }
#endif

    temptargetP = targetP;
    if(temptargetP != NULL)
    {
        connP = (dtls_connection_t*)temptargetP->sessionH;
        if (connP == NULL){
            return;
        }
        for (int i =0; i < MAX_RETRY_CONUT; i++)
        {
            if( connP->dlts_resumption == 0 && ((tv_sec - connP->last_conn_time) >= DEFAULT_SESSION_RESUMPTION_TIME)){
                LOG_ARG("[LWM2M] tv_sec=%d connP->last_conn_time=%d",tv_sec,connP->last_conn_time); 
                connP->dlts_resumption = 1;
                bool ret = lwm2m_dtls_session_resumption(connP, lwm2mH);
                if(ret){
                    LOG_ARG("Server ShortID=%d session resumption successfully finished!!\n",temptargetP->shortID);
                    break;
                }else{
                    LOG_ARG("Server ShortID=%d session resumption failed, will retry!!\n",temptargetP->shortID);
                }
            }
        }

    }
}

/*currently not use*/
void lwm2m_client_dtls_session_resumption(lwm2m_context_t *lwm2mH)
{
	int tv_sec = lwm2m_gettime();

    dtls_connection_t * connP = NULL;
    //add by joe start
    lwm2m_server_t * temptargetP = NULL;
    //bool need_update_server = false;
    if(!lwm2m_check_is_vzw_netwrok()){
        return; //currently resumption only works for VZW network
        //LOG("client no alarm Set");
    }

    temptargetP = lwm2mH->serverList;
    
    int serverCount = lwm2mH->serverListCount;
    for (int i = 0; i < serverCount; i++)        
    //while (temptargetP != NULL )
    {
        connP = (dtls_connection_t*)temptargetP->sessionH;
        if (connP == NULL){
            temptargetP = temptargetP->next; //add by joe. jump to next server
            continue;
        }
        if( connP->dlts_resumption == 0 && ((tv_sec - connP->last_conn_time) >= DEFAULT_SESSION_RESUMPTION_TIME && (tv_sec - connP->last_conn_time) <= 219200)){
            LOG_ARG("[LWM2M] tv_sec=%d connP->last_conn_time=%d",tv_sec,connP->last_conn_time); 

            if(temptargetP->shortID == 1000){ //add by joe for repo server ID == 1000
                //do not update
                if(checkifApn3unAvailable()){
                    connP->last_conn_time = tv_sec;
                    temptargetP->network_down = true;
                    if(get_vzw_apn3_callid() == -2){
                        quec_check_class2_apn_network_available();
                    }
                    LOG("do not resumption repo server");
                }else if(!checkifApn3unAvailable() && temptargetP->network_down == true){
                    //recovery from network off, try update
                    LOG("do not resumption but regist repo server");
                    connP->last_conn_time = tv_sec;
                    temptargetP->network_down = false;
                    lwm2m_dtls_session_resumption(connP, lwm2mH);
                    temptargetP->status = STATE_REG_UPDATE_NEEDED;
                }else if(!checkifApn3unAvailable() && temptargetP->network_down == false){
                    //regularly resumption
                    connP->dlts_resumption = 1;
                    lwm2m_dtls_session_resumption(connP, lwm2mH);                
                    LOG_ARG("Server ShortID=%d session resumption finished!!",temptargetP->shortID);
                }
                
            }else{

                connP->dlts_resumption = 1;
                lwm2m_dtls_session_resumption(connP, lwm2mH);
                LOG_ARG("Server ShortID=%d session resumption finished!!",temptargetP->shortID);
                if(checkifApn3unAvailable()){
                    temptargetP->network_down = true;
                }else if(!checkifApn3unAvailable() && temptargetP->network_down == true){
                    //recovery from network off, try update
                    temptargetP->network_down = false;
                    temptargetP->status = STATE_REG_UPDATE_NEEDED;
                }
            }

        }
        temptargetP = temptargetP->next; //add by joe
    }
}

bool quec_lwm2m_get_local_hostip(char* ipAddr){
    int i=0;
    int sockfd;
    struct ifconf ifconf;
    char buf[512];
    struct ifreq *ifreq;
    char* ip;
    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    LOG("quec_lwm2m_get_local_hostip enter\n");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0))<0)
    {
        LOG("quec_lwm2m_get_local_hostip sockfd failed\n");
        return false;
    }
    ioctl(sockfd, SIOCGIFCONF, &ifconf);
    close(sockfd);
    ifreq = (struct ifreq*)buf;
    for(i=(ifconf.ifc_len/sizeof(struct ifreq)); i>0; i--)
    {
        ip = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
        LOG_ARG("get local ip : %s\n",ip);

        if(strcmp(ip,"127.0.0.1")==0 || strcmp(ip,"192.168.225.1")==0)
        {
            ifreq++;
            continue;
        }
        strcpy(ipAddr,ip);
        break;
    }
    LOG_ARG("return local ip : %s\n",ipAddr);
    return true;
}

void * pthread_init_device_info_and_network(void *arg)       
{
     LOG("pthread_init_device_info_and_network start!\n");
     
     //init apn network for vzw
     if(lwm2m_check_is_vzw_netwrok()){
          //add by joe
         init_vzw_apn_info();
         vzw_apn_id *vzw_apn_id = get_vzw_apnid();
         getVzwApnInfo(vzw_apn_id);
         
        #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
         //vzw_apn_id->vzwadmin_apn2.profileid =2 ;
         if(vzw_apn_id->vzwadmin_apn2.profileid != -1){
             LOG_ARG("------->enable vzw apn profileid=%d, apn name=%s\n",vzw_apn_id->vzwadmin_apn2.profileid,vzw_apn_id->vzwadmin_apn2.apn_name);
             enableApnNetworkByprofile(true,vzw_apn_id->vzwadmin_apn2.profileid,vzw_apn_id->vzwadmin_apn2.apn_name);
         }
        sleep(1);
         //vzw_apn_id->vzwinternet_apn3.profileid =3;
         if(vzw_apn_id->vzwinternet_apn3.profileid != -1){
             LOG_ARG("------->enable vzw apn profileid=%d, apn name=%s\n",vzw_apn_id->vzwinternet_apn3.profileid,vzw_apn_id->vzwinternet_apn3.apn_name);
             enableApnNetworkByprofile(true,vzw_apn_id->vzwinternet_apn3.profileid,vzw_apn_id->vzwinternet_apn3.apn_name);
         }
        #endif
     }

     return NULL;
}

char* GetDeviceImei(){
    return device_imei;
}

void GetDevicePhoneNumber(char * num){
    strcpy(num,device_phoneNumber);
   // return device_phoneNumber;
}

char* GetSoftwareVersion(){
    return software_version;    
}

bool Get_full_regist_flag(){
    LOG_ARG("isFullRegistNeed =%d \n",isFullRegistNeed);
    return isFullRegistNeed;
}

void Set_full_regist_flag(bool state){
    isFullRegistNeed = state;
    LOG_ARG("isFullRegistNeed =%d \n",isFullRegistNeed);
}

lwm2m_context_t* GetLwm2mH_ptr(){
    return g_lwm2mH_ptr;    
}

int quec_init_local_socket_server(){
    //int server_sockfd,client_sockfd;
    int server_len,client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int i,btye;
    char* char_send;
    int opt = 1;
    int ret;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");//htonl(INADDR_ANY);//
    server_address.sin_port = htons(56800);//5680;
    server_len = sizeof(server_address);
    
    server_sockfd = socket(AF_INET,SOCK_STREAM,0);

    if(0 != setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))){
        LOG("socket setsockopt fail \n");
        return 0;
    }

    struct timeval timeout = {15,0};//wait for 15s
    if (0 != setsockopt(server_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) )
    {
        printf("set accept timeout failed \n");
        return 0;
    }

    if(-1 == bind(server_sockfd,(struct sockaddr *)&server_address,server_len)){
        LOG("bind socket fail \n");
        return 0;
    }
    if(-1 == listen(server_sockfd,5)){
        LOG("listen socket fail \n");
        return 0;
    }
    LOG("server waiting 15 Seconds for connect\n");
    
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,(socklen_t *)&client_len);
    if(client_sockfd == -1){
        LOG("accept socket failed\n");
    }else{
        LOG("accept socket success\n");
    }

    char info[] = "Hello Qt client!";
    LOG_ARG("send info :%s size=%d\n",info,sizeof(info));
    if(btye = send(client_sockfd,info,sizeof(info),0) == -1)
    {
        LOG("send socket failed\n");
    }
    //shutdown(client_sockfd,2);
    //shutdown(server_sockfd,2); 
    return 0;

}


void lwm2m_listen_to_qt_client_connection(){
    int server_len,client_len;
    struct sockaddr_in client_address;
    int i,btye;
    client_len = sizeof(client_address);
    int sockfd = -1;
    while( 0 == g_quit){
        //if(client_sockfd == -1)
        {
            sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,(socklen_t *)&client_len);
            if(sockfd != -1){
                client_sockfd = sockfd;
            }
        }
        usleep(100*1000);//add to reduce CPU usage
    }
}

void quec_lwm2m_client_info_notify(char *info){
    int btye;
    char char_send[256];
    if(client_sockfd == -1) return;
    if(info == NULL || strlen(info) == 0) return;

    strcpy(char_send,info);
    if(btye = send(client_sockfd,char_send,strlen(char_send),0) == -1)
    {
        LOG("send socket failed\n");
    }else{
        LOG_ARG("send notify info :%s size=%d\n",char_send,strlen(char_send));
    }

}

bool quec_check_firmware_version_changed(char* FirmwareVerison){
    char *old_firmwareVersion=NULL;
    char *filename = QUEC_LWM2M_FIRMWARE_VERSION_SAVE;
    char *filename_temp = QUEC_LWM2M_FIRMWARE_VERSION_SAVE_TEMP;
    FILE *fp, *fp_new;
    bool firmwareVersionChanged = false;
    int i;

    old_firmwareVersion = lwm2m_malloc(sizeof(char)*64);
    memset(old_firmwareVersion, 0, 64);
    LOG("enter system_check_firmwareVersion_Changed");
    if (access(QUEC_LWM2M_FIRMWARE_VERSION_SAVE, F_OK) == 0){
        fp = fopen(filename,"rb");//open a exist file
    }else{
        fp = fopen(filename,"wb+");//create a new file
    }
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",filename);
        perror("fopen: ");
        return firmwareVersionChanged;
    }

    fread(old_firmwareVersion, sizeof(char)*64, 1, fp);
    LOG_ARG("get old_firmwareVersion=%s",old_firmwareVersion);
    if(old_firmwareVersion == NULL || strlen(old_firmwareVersion) == 0){
        firmwareVersionChanged = false;
        fwrite(FirmwareVerison, strlen(FirmwareVerison), 1, fp);
        fclose(fp);
        LOG("firmwareVersion length ==0, firmwareversion first save treat as not changed \n");
    }else if(strcmp(old_firmwareVersion,FirmwareVerison) == 0){
        firmwareVersionChanged = false;
        fclose(fp);
        LOG("firmwareVersion not changed \n");
    }else{
        firmwareVersionChanged = true;
        fp_new = fopen(filename_temp,"wb+");
        if(fp_new == NULL)
        {
            LOG("open file newfirmwareVersionChanged.dat error: \n");
            fclose(fp);
            return firmwareVersionChanged;
        }
        fwrite(FirmwareVerison, strlen(FirmwareVerison), 1, fp_new);
        fclose(fp);
        remove(fp);
        fclose(fp_new);
        rename(filename_temp, filename);
        LOG("FirmwareVerison changed and saved\n");
    }
    lwm2m_free(old_firmwareVersion);
    LOG("write_firmwareVersionChanged data exit\n");
    return firmwareVersionChanged;

}

static void prv_set_firmware_object_update_state_changed(lwm2m_context_t *lwm2mH)
{
  lwm2m_uri_t uri;

  if(lwm2m_stringToUri("/5/0/5", strlen("/5/0/5"), &uri))//firmware update state
  {
    handle_value_changed(lwm2mH, &uri, (const char *)"1", 1);
  }
}

void prv_set_firmware_object_download_state_changed(lwm2m_context_t *lwm2mH, char* state)
{
  lwm2m_uri_t uri;

  if(lwm2m_stringToUri("/5/0/3", strlen("/5/0/3"), &uri))//firmware package download state
  {
    handle_value_changed(lwm2mH, &uri, (const char *)state, 1);
  }
}

bool quec_check_phoneNumber_changed(char* number)
{
    char *old_number=NULL;
    char *filename = QUEC_LWM2M_PHONE_NUMBER_SAVE;
    char *filename_temp = QUEC_LWM2M_PHONE_NUMBER_SAVE_TEMP;
    FILE *fp, *fp_new;
    bool numberChanged = false;
    int i;

    old_number = lwm2m_malloc(sizeof(char)*64);
    memset(old_number, 0, 64);
    LOG("enter system_check_phoneNumber_changed");
     if (access(QUEC_LWM2M_PHONE_NUMBER_SAVE, F_OK) == 0){
        fp = fopen(filename,"rb");//open a exist file
    }else{
        fp = fopen(filename,"wb+");//create a new file
    }   
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",filename);
        perror("fopen: ");
        return numberChanged;
    }

    fread(old_number, sizeof(char)*64, 1, fp);
    LOG_ARG("get old_number=%s\n",old_number);
    if(old_number == NULL || strlen(old_number)==0){
        fwrite(number, strlen(number), 1, fp);
        numberChanged = true;
        fclose(fp);
        quec_save_full_regist_flag(true);
        LOG("phone number first save \n");

    }else if(strcmp(old_number,number) == 0){
        numberChanged = false;
        fclose(fp);
        LOG("phone number not changed \n");
    }else{
        numberChanged = true;
        fp_new = fopen(filename_temp,"wb+");
        if(fp_new == NULL)
        {
            LOG("open file newphonenumberfile.dat error: \n");
            fclose(fp);
            return numberChanged;
        }
        fwrite(number, strlen(number), 1, fp_new);
        fclose(fp);
        remove(fp);
        fclose(fp_new);
        rename(filename_temp, filename);
        LOG("phone number changed and saved\n");
        quec_save_full_regist_flag(true);
    }
    lwm2m_free(old_number);
    LOG("write_phonenumber data exit\n");
    return numberChanged;
}   

void quec_load_full_regist_flag(){
    char *filename = QUEC_LWM2M_FULL_REGIST_FALG_SAVE;
    FILE *fp;
    fp = fopen(filename,"rb");
    char *full_reg=NULL;
    
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",filename);
        perror("fopen: ");
        return ;
    }
    
    full_reg = lwm2m_malloc(sizeof(char)*64);
    memset(full_reg, 0, 64);
    fread(full_reg, sizeof(char)*64, 1, fp);
    LOG_ARG("load_get full_reg=%s",full_reg);
    if(full_reg == NULL){
        isFullRegistNeed = false;
        return;
    }
    if(strstr(full_reg,"true") != 0){
        isFullRegistNeed = true;

    }else {
        isFullRegistNeed = false;
    }
    
    LOG_ARG("load_get isFullRegistNeed=%d",isFullRegistNeed);
    fclose(fp);
    lwm2m_free(full_reg);
}

void quec_save_full_regist_flag(bool state){
    FILE *fp;
    int i;
    char* full_regist_state = "true";
    char *filename = QUEC_LWM2M_FULL_REGIST_FALG_SAVE;
    LOG_ARG("quec_save_full_regist_flag enter state=%d",state);

    if (access(filename, F_OK) == 0)
    {
        LOG("if QUEC_LWM2M_FULL_REGIST_FALG_SAVE exist, remove it");
		remove(QUEC_LWM2M_FULL_REGIST_FALG_SAVE);
    }

    fp = fopen(QUEC_LWM2M_FULL_REGIST_FALG_SAVE,"wb+");
    if(fp == NULL)
    {
        printf("file QUEC_LWM2M_FULL_REGIST_FALG_SAVE error: \n");
        perror("fopen: ");
        return -1;
    }
    if(state){
        full_regist_state = "true";
    }else{
        full_regist_state = "false";
    }
    fwrite(full_regist_state, strlen(full_regist_state), 1, fp);
    fclose(fp);
    LOG("quec_save_full_regist_flag exit");

}

void quec_queue_update_register_for_vzw_dm_server(){
	lwm2m_server_t * targetP = NULL ;
    targetP = g_lwm2mH_ptr->serverList;

    while(targetP != NULL)
    {
       if(targetP->shortID == 102)
         break;
       targetP = targetP->next;
    }
    targetP->pending_reg_update = true;
}

bool lwm2m_save_fota_pkg_date(unsigned char *data, int length, bool eof)
{
	lwm2m_event_info  event_info;
	
	memset(&event_info, 0, sizeof(lwm2m_event_info));
	
    LOG_ARG("g_lwm2mH_ptr->fota_fd=%d",g_lwm2mH_ptr->fota_fd);

	if(g_lwm2mH_ptr->fota_fd == -1){
        remove(QUEC_LWM2M_FOTA_UPDATE_FILE);
		g_lwm2mH_ptr->fota_fd = open(QUEC_LWM2M_FOTA_UPDATE_FILE,O_CREAT|O_TRUNC|O_RDWR,0777);
		if(g_lwm2mH_ptr->fota_fd < 0)
			return false;
		event_info.fw_data_info.start = 1;
	}
	event_info.fw_data_info.length = length;
    LOG_ARG("fw_data_info.length=%d",length);
	if(write(g_lwm2mH_ptr->fota_fd,data, length) != length){
		close(g_lwm2mH_ptr->fota_fd);
		g_lwm2mH_ptr->fota_fd = -1;
        LOG("write != length return");
		return false;
	}
	
	if(eof == true){
		event_info.fw_data_info.end = 1;
		close(g_lwm2mH_ptr->fota_fd);
		g_lwm2mH_ptr->fota_fd = -1;
        char* inband_finish = "Inband package download finished!";
        quec_lwm2m_client_info_notify(inband_finish);
        LOG("eof == true return");
	}

	//if(client_ptr->notify_fcn != NULL)
	//	client_ptr->notify_fcn((int)client_ptr, LWM2M_CLIENT_FW_DATA_IND, &event_info);
	return true;
}

bool lwm2m_check_if_observe_and_update_app(uint8_t id_set, 
                                                                uint16_t object_ID, 
                                                                uint16_t instance_ID, 
                                                                uint16_t resource_ID, 
                                                                lwm2m_object_data_t *data
                                                                )
{
    bool result = false;

	if(object_ID == LWM2M_FIRMWARE_UPDATE_OBJECT_ID)
	{
		if(instance_ID != 0)
			return false;
		if(resource_ID == 0 /*RES_M_PACKAGE*/){
            LOG_ARG("block1_more=%d",data->instance_info->resource_info->value.asBuffer.block1_more);
			if(data->instance_info->resource_info->value.asBuffer.block1_more){
				result = lwm2m_save_fota_pkg_date(data->instance_info->resource_info->value.asBuffer.buffer, 
								  data->instance_info->resource_info->value.asBuffer.length, false);
            }
			else{
				result = lwm2m_save_fota_pkg_date(data->instance_info->resource_info->value.asBuffer.buffer, 
				      data->instance_info->resource_info->value.asBuffer.length, true);
                //prv_set_firmware_object_download_state_changed(g_lwm2mH_ptr,"2");
             }
			return result;
		}
		if(resource_ID == 1/*RES_M_PACKAGE_URI*/)
		{
			//report urc
			lwm2m_event_info  event_info;
			memset(&event_info, 0, sizeof(lwm2m_event_info));

			if(data->instance_info->resource_info->value.asBuffer.length > 256)
				return false;
			memcpy(event_info.pkg_url, data->instance_info->resource_info->value.asBuffer.buffer, 
				   data->instance_info->resource_info->value.asBuffer.length);
            
            if(fota_outband_download_with_url(event_info.pkg_url)){
                result = true;
                //prv_set_firmware_object_download_state_changed(g_lwm2mH_ptr,"2");
            }
			return true;
		}
		if(resource_ID == 2/*RES_M_UPDATE*/)
		{
			//report urc
	        //call update funtion
            int result = quec_fota_update_work_thread();
			//client_ptr->notify_fcn((int)client_ptr, LWM2M_CLIENT_FW_UPDATE_IND, NULL);
            if(result != 0){
                return false;
            }else{
                //prv_set_firmware_object_download_state_changed(g_lwm2mH_ptr,"3");
			    return true;
            }
		}
	}
    #if 0
    else if(object_ID == LWM2M_DEVICE_OBJECT_ID){
		if(resource_ID == 4/*RES_M_REBOOT*/){
			if(client_ptr->notify_fcn != NULL)
				client_ptr->notify_fcn((int)client_ptr, LWM2M_CLIENT_REBOOT_IND, NULL);
			return true;
		}else if(resource_ID == 5 /*RES_O_FACTORY_RESET*/){
			lwm2m_client_reset_object_info();
			if(client_ptr->notify_fcn != NULL)
				client_ptr->notify_fcn((int)client_ptr, LWM2M_CLIENT_REBOOT_IND, NULL);
			return true;
		}
	}
    #endif
	return false;
}

//joe add for Verizon server URLs 20210320

void lwm2m_preload_server_url(){
    int max_file_list = 4; //4 server url for VZW
    char* urlFileList[] = {
                QUEC_LWM2M_VZW_100_SERVER_URL_FILE,
                QUEC_LWM2M_VZW_101_SERVER_URL_FILE,
                QUEC_LWM2M_VZW_102_SERVER_URL_FILE,
                QUEC_LWM2M_VZW_1000_SERVER_URL_FILE
            };

    char* serverUrlList[] = {
                VZW_100_Server_url,
                VZW_101_Server_url,
                VZW_102_Server_url,
                VZW_1000_Server_url
            };

    int index = 0;
    char *filename;
    char *ServerURL;

    size_t bytes_read;
    int fs_fd = -1;
    struct stat stat_s = {0};
    uint8_t *file_buf = NULL;

    LOG("lwm2m_preload_server_url enter\n");
    for (index  = 0; index  < max_file_list; index++ ){
        memset( &stat_s, 0, sizeof(struct stat )); 
        filename = urlFileList[index];
        if(stat(filename, &stat_s) < 0) {
            LOG_ARG("lwm2m_preload_server_url file state return, filename=%s\n",filename);
          continue;
        }

        if((fs_fd = open(filename, O_RDONLY)) < 0) {
            LOG_ARG("lwm2m_preload_server_url file open return, filename=%s\n",filename);
          continue;
        } 

        file_buf = lwm2m_malloc(stat_s.st_size + 4); /*Added 4 bytes extra for padding*/
        //LOG_ARG("load lwm2m_read_from_efs_file file_buf=%p\n",file_buf);

        if(file_buf == NULL) {
          close(fs_fd);
          LOG_ARG("lwm2m_preload_server_url file_buf null return, filename=%s\n",filename);
          continue;
        }

        memset(file_buf, 0, stat_s.st_size + 4);
        bytes_read = read(fs_fd, file_buf, stat_s.st_size);
        if((bytes_read != stat_s.st_size) && (bytes_read == 0)) {
          lwm2m_free(file_buf);
          close(fs_fd);
        LOG_ARG("lwm2m_preload_server_url file read return,  filename=%s\n",filename);
          continue;
        }

        LOG_ARG("load_get ServerURL file_buf=%s\n",file_buf);
        memcpy(serverUrlList[index],file_buf,strlen(file_buf));
        
        LOG_ARG("set serverUrlList[%d]=%s\n",index,serverUrlList[index]);
        // *buffer_ptr = file_buf;
        // *buffer_size = stat_s.st_size;

        close(fs_fd);

   }

}

#if defined(VZW_FATAL_ALERT_SUPPORT)                         
void lwm2m_process_fatal_alert(lwm2m_server_t * targetP)
{
    if(targetP == NULL)
    {
        LOG("targetP err\n");
        return;
    }
    LOG("handle fatal alert message,set server to STATE_REG_FAILED!!!\n");
    targetP->status = STATE_REG_FAILED;
    targetP->register_retry_count = 0;
    targetP->register_next_try_timer = 0;
    Set_full_regist_flag(true);
}
#endif
//add by joe end
#define LWM2M_CFG_NAME  "/persist/lwm2m_conf/lwm2m.conf"
int isLwenable = -1;

int quec_connect_check()
{
	int net_fd;
	char statue[20];

	net_fd=open("/sys/devices/virtual/net/rmnet_data0/operstate",O_RDONLY);
	if(net_fd<0)
	{
		LOG("open err\n");
		return 0;
	}
	printf("open success\n");
	memset(statue,0,sizeof(statue));
	int ret=read(net_fd,statue,10);
	LOG_ARG("statue is %s",statue);
	if(NULL!=strstr(statue,"down"))
	{
		LOG("off line\n");
		close(net_fd);
		return 1;
	}
	else
	{
		LOG("unknown or up\n");
		close(net_fd);
		return 2;
	}
}

#ifdef WITH_MBEDTLS

char *conf_get_status_by_key(const char *fname, const char *key)
{
    int len = 0;
    int key_len = 0;
    char buf[256];
    FILE *fp = NULL;
    char *p = NULL;

    if (fname == NULL || key == NULL) {
        return NULL;
    }
    key_len = strlen(key);
    if (key_len < 1) {
        return NULL;
    }
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#') {
            continue;
        }
        len = strlen(buf);
        if (len < 2) {
            continue;
        }
        /* Remove the EOL */
        /* EOL == CR */
        if (buf[len - 1] == '\r') {
            buf[len - 1] = 0;
            len -= 1;
        } else if (buf[len - 1] == '\n') {
            /* EOL = CR+LF */
            if (buf[len - 2] == '\r') {
                buf[len - 2] = 0;
                len -= 2;
            } else {
                /* EOL = LF */
                buf[len - 1] = 0;
                len -= 1;
            }
        }
        /* Skip the empty line */
        if (len == 0) {
            continue;
        }
        if (strncasecmp(buf, key, key_len) == 0 && buf[key_len] == '=') {
            p = malloc(len - key_len);
            if (p == NULL) {
                fclose(fp);
                return NULL;
            }
            strcpy(p, buf + key_len + 1);
        }
    }
    fclose(fp);
    return p;
}

bool lw_enable_form_conf(void)
{
    char *status;
    if(isLwenable == -1){
        status = conf_get_status_by_key(LWM2M_CFG_NAME, "DEFAULT_STATUS");
        if(status == NULL){
        #if defined(ENABLE_QUECTEL_LWM2M_DEFAULT)
            LOG("LwM2M lwm2m.conf not exist in persist,set default to start!\n");
            isLwenable = 1;
        #else
            LOG("LwM2M lwm2m.conf not exist in persist,set default Not start!\n");
            isLwenable = 0;
        #endif
        }
        else if(!strncmp(status, "ON", 2)){
            isLwenable = 1;
            LOG("LwM2M SET isLwenable to true!\n");
        }
        else{
            isLwenable = 0;
            LOG("LwM2M SET isLwenable to false!\n");
        }
    }
        return (isLwenable == 1);
}



int main(int argc, char *argv[])
{

    if(!lw_enable_form_conf()){
        exit(0);
    }
#if 1
    char server[MAX_SIZE]  = "InteropLwM2M.dm.iot.att.com";
    char serverPort[MAX_SIZE]  = "5694";
    char name[MAX_SIZE] = "urn:imei:865067035347007";
    char pskId[MAX_SIZE] = "urn:imei:865067035347007";
    char psk[MAX_SIZE] = "ea1eed6ea7fd1ca2eefbfbd63ce72644";
#endif

    client_data_t data;
    int result;
    lwm2m_context_t * lwm2mH = NULL;
    int i;
    const char * localPort = "56830";

    int lifetime = 60;
    int batterylevelchanging = 0;
    time_t reboot_time = 0;
    int opt;
    bool bootstrapRequested = true;
    bool serverPortChanged = false;
    bool is_receive_started = false;
    bool isColdStart = true;

//add by joe start
	lwm2m_object_t *secObjP = NULL;
	lwm2m_object_t *serObjP = NULL;
	lwm2m_object_t *aclObjP = NULL;
	int fd;
//add by joe end
#ifdef LWM2M_BOOTSTRAP
    lwm2m_client_state_t previousState = STATE_INITIAL;
#endif
    g_atc_apn_changed_time = 0;

    uint16_t pskLen = 64;//-1;
    char * pskBuffer = NULL;

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = AF_INET;

    //init lwm2m data folder
    lwm2m_init_lwm2m_data_folder();
    LOG("Waiting for LwM2M to start!\n");
    //wait 15s for qlrild
    sleep(15);
    LOG("LwM2M ready to start!\n");
#ifdef LWM2M_SUPPORT_QLRIL_API
    //joe init qlril
    lwm2m_init_QLRIL();
#endif
	//init network operator info
	LOG("LwM2M check if vzw network !\n");
	lwm2m_check_is_vzw_netwrok();
	LOG("LwM2M check if att network !\n");
	lwm2m_check_is_att_netwrok();

    LOG_ARG("print  g_is_vzw_network= %d  Or g_is_att_network= %d \n",g_is_vzw_network,g_is_att_network);
    if(g_is_vzw_network != 1 && g_is_att_network != 1){
        LOG("It's not VZW Or ATT network, LwM2Mclient will start sleep here!\n");
        while(1){
            LOG("It's not VZW Or ATT network, LwM2Mclient will stay asleep here!\n");
            sleep(86400);
        }
    }
#ifdef LWM2M_SUPPORT_QLRIL_API
    QL_LWM2M_GetFirmwareVersion(software_version);
#else
    //joe call sms client init
    quectel_sms_init(lwm2mH);

	//init about data
    ql_module_about_info_s about;
    QL_LWM2M_GetAbout(&about);
    strcpy(software_version,about.firmware_version);
#endif
	//init phone number
    FUNC_SIM_GetMsisdnGsmNum(device_phoneNumber);
    if(quec_check_phoneNumber_changed(device_phoneNumber)){//need do bootstrap when SIM changed
        LOG("Phonenumber changed! Bootstrap required!\n");
        isFullRegistNeed = true;
        bootstrapRequested = true;
        system("rm -rf /data/lwm2m");
        //init lwm2m data folder
        lwm2m_init_lwm2m_data_folder();
        quec_check_phoneNumber_changed(device_phoneNumber);//save new phonenumber here
    }
	if(lwm2m_check_is_att_netwrok()){
        result = lwm2m_init_att_connection();
	    if(result != 0){
		    LOG("lwm2m_init_att_connection getApnInfo Fail.\n");
            return -1;
        }
	}else if(lwm2m_check_is_vzw_netwrok()){
        //init connect with QT app
        /*Del by joe Qt application connect only for lab test use*/
        if(lwm2m_verion_bs_lab_test_server_exist()){
            LOG("test LwM2M with QT UI for indicate status Info \n");
            quec_init_local_socket_server();

            pthread_t qt_thread_id;
            pthread_create(&qt_thread_id, NULL, lwm2m_listen_to_qt_client_connection, NULL);
        }
	    //add thread to init network
        pthread_t tidp;
        if (pthread_create(&tidp, NULL, pthread_init_device_info_and_network, NULL) == -1)
        {
            printf("create error!\n");
            pthread_exit((void*)0);
            return -1;
        }
        pthread_join(tidp,NULL);//wait for network start
	}
#ifndef LWM2M_SUPPORT_QLRIL_API
    char* class2_network_ready = "VZWADMIN Network actived !";
    quec_lwm2m_client_info_notify(class2_network_ready);
    char* class3_network_ready = "VZWINTERNET Network actived !";
    quec_lwm2m_client_info_notify(class3_network_ready);
#endif
    /*
     *This call an internal function that create an IPV6 socket on the port 5683.
     */
    lwm2m_load_server_configuration();

    lwm2m_server_info_t  *server_info_ptr;//get server info from file
    if(lwm2m_check_is_att_netwrok()){
       server_info_ptr = lwm2m_get_server_info(DM_Server_ID);
    }else if(lwm2m_check_is_vzw_netwrok()){
       server_info_ptr = lwm2m_get_server_info(Bootstrap_Server_ID);
    }else{
       server_info_ptr = lwm2m_get_server_info(DM_Server_ID);
    }
    
    if(server_info_ptr != NULL && isColdStart || lwm2m_check_is_vzw_netwrok()){
        //lwm2m_parse_server_info_from_file(server_info_ptr,server,serverPort,psk,pskId,name);
        memset(name, 0, MAX_SIZE);
        memset(pskId, 0, MAX_SIZE);
        memset(psk, 0, MAX_SIZE);
        strcpy(pskId, server_info_ptr->psk_id);
        if(lwm2m_check_is_vzw_netwrok()){
            strcpy(name, server_info_ptr->psk_id);
        }else{
            strcpy(name, server_info_ptr->name);
        }
        strcpy(psk, server_info_ptr->psk_key);
        LOG_ARG("isColdStart==true read from file pskId = %s name = %s psk = %s\r\n",pskId,name,psk);
    }
    
    if(lwm2m_check_is_vzw_netwrok()){
        bootstrapRequested = !parse_is_bootstrapd_for_vzw();//decide to bootstrap or not
        quec_load_full_regist_flag();

        LOG_ARG("device_phoneNumber = %s isFullRegistNeed=%d\r\n",device_phoneNumber,isFullRegistNeed);
        lwm2m_preload_server_url();

    }else{
        parse_connect_info(&isColdStart, server, serverPort, name, pskId, psk, &pskLen);
        
        if (!isColdStart){
            bootstrapRequested = false;
    	}
    
    }
    LOG_ARG("isColdStart = %d, pskLen = %d \n", isColdStart, pskLen);

    /*
     *This call an internal function that create an IPV6 socket on the port 5683.
     */
    fprintf(stderr, "Trying to bind LWM2M Client to port %s\r\n", localPort);

    data.sock = create_socket(localPort, data.addressFamily);
    if (data.sock < 0)
    {
        fprintf(stderr, "Failed to open socket: %d %s\r\n", errno, strerror(errno));
       // return -1;
    }

    LOG("create_socket done \r\n");

    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
#ifdef WITH_TLS
    if (strlen(psk) > 0 && isColdStart)
    {
        pskLen = strlen(psk) / 2;
        pskBuffer = malloc(pskLen);

        if (NULL == pskBuffer)
        {
            fprintf(stderr, "Failed to create PSK binary buffer\r\n");
            return -1;
        }
        // Hex string to binary
        char *h = psk;
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
    }else{ 
		printf("psk = %s, pskLen = %d \r\n", psk, pskLen);
        if(lwm2m_check_is_vzw_netwrok()){
            pskLen = strlen(psk) / 2;
        }
		pskBuffer = malloc(pskLen);
		memcpy(pskBuffer, psk, pskLen);
	}
#endif

    char serverUri[256];
    int serverId = 123;
	//add by joe start
    if(lwm2m_check_is_vzw_netwrok()){
        serverId = 100;
    }
	//add by joe end
    sprintf (serverUri, "coaps://%s:%s", server, serverPort);
    
    if(server_info_ptr != NULL && isColdStart || lwm2m_check_is_vzw_netwrok()){
        memset(serverUri,0,256);
        memcpy(serverUri,server_info_ptr->server_url,sizeof(server_info_ptr->server_url));//url read for file
        LOG_ARG("isColdStart==true read from file serverUri = %s\n", serverUri);
    }

    LOG_ARG("serverUri = %s, serverPort = %s, psk = %s, pskLen = %d \r\n", serverUri, serverPort, psk, pskLen);

    LOG_ARG("pskBuffer = %s \n", pskBuffer);
#ifdef LWM2M_BOOTSTRAP
    LOG("LWM2M_BOOTSTRAP defined \r\n");
    objArray[0] = secObjP = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, bootstrapRequested);
#else
    LOG("LWM2M_BOOTSTRAP not defined \r\n");
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
#endif

    LOG("get_security_object done \r\n");


    if (NULL == objArray[0])
    {
        fprintf(stderr, "Failed to create security object\r\n");
        return -1;
    }
    data.securityObjP = objArray[0];

    //klein
    //todo: it's a workaround fix
    uint16_t serverInstanceId = 0;
    if (!isColdStart){
        serverInstanceId = 2;
    }
	//add by joe start
    if(lwm2m_check_is_vzw_netwrok()){
        serverInstanceId = 0;
    }
    //add by joe end

    objArray[1] = serObjP = get_server_object(serverId, "U", lifetime, false, serverInstanceId);
    if (NULL == objArray[1])
    {
        fprintf(stderr, "Failed to create server object\r\n");
        return -1;
    }

    int instId = 0;
    objArray[2] = aclObjP = acc_ctrl_create_object();
    
    if (NULL == objArray[2])
    {
        fprintf(stderr, "Failed to create Access Control object\r\n");
        return -1;
    }
    else if (acc_ctrl_obj_add_inst(objArray[2], instId, 3, 0, serverId)==false)
    {
        fprintf(stderr, "Failed to create Access Control object instance\r\n");
        return -1;
    }
    else if (acc_ctrl_oi_add_ac_val(objArray[2], instId, 0, 0b000000000001111)==false)
    {
        fprintf(stderr, "Failed to create Access Control ACL default resource\r\n");
        return -1;
    }
    else if (acc_ctrl_oi_add_ac_val(objArray[2], instId, 999, 0b000000000000001)==false)
    {
        fprintf(stderr, "Failed to create Access Control ACL resource for serverId: 999\r\n");
        return -1;
    }
    
    objArray[3] = get_object_device();
    if (NULL == objArray[3])
    {
        fprintf(stderr, "Failed to create Device object\r\n");
        return -1;
    }
    
    LOG("start to create  connectivity monitoring object\n");

    objArray[4] = get_object_conn_m();
    if (NULL == objArray[4])
    {
        fprintf(stderr, "Failed to create connectivity monitoring object\r\n");
        return -1;
    }
    LOG("start to create  Firmware object\n");
    objArray[5] = get_object_firmware();
    if (NULL == objArray[5])
    {
        fprintf(stderr, "Failed to create Firmware object\r\n");
        return -1;
    }
    LOG("start to create  location object\n");
    objArray[6] = get_object_location();
    if (NULL == objArray[6])
    {
        fprintf(stderr, "Failed to create location object\r\n");
        return -1;
    }
/*
    objArray[5] = get_test_object();
    if (NULL == objArray[5])
    {
        fprintf(stderr, "Failed to create test object\r\n");
        return -1;
    }
*/
//jambo delete and modify array

    objArray[7] = get_object_conn_s();
    if (NULL == objArray[7])
    {
        fprintf(stderr, "Failed to create connectivity statistics object\r\n");
        return -1;
    }
    
    if(/*lwm2m_check_is_vzw_netwrok()*/0){// currently do not support VZW host device object
        
        objArray[8] = get_vzw_object_host_dev();
        if (NULL == objArray[8])
        {
            fprintf(stderr, "Failed to create vzw host object\r\n");
            return -1;
        }    

    }else{
        objArray[8] = get_object_host_dev();
        if (NULL == objArray[8])
        {
            fprintf(stderr, "Failed to create host dev object\r\n");
            return -1;
        }
    }

    objArray[9] = get_object_apn_conn();
    if (NULL == objArray[9])
    {
        fprintf(stderr, "Failed to create apn conn object\r\n");
        return -1;
    }
    objArray[10] = get_object_conn_ex();
    if (NULL == objArray[10])
    {
        fprintf(stderr, "Failed to create conn ex object\r\n");
        return -1;
    }
    //add by joe for vzw start
    objArray[11] = get_object_deviceCap();
    if (NULL == objArray[11])
    {
        fprintf(stderr, "Failed to create devicecap object\r\n");
        return -1;
    }
	//add by joe for vzw end
	#ifdef ENABLE_SOFTWARE_MGNT_OBJ
    objArray[12] = get_object_software_mgnt();
    if (NULL == objArray[12])
    {
        fprintf(stderr, "Failed to create sw mgnt object\r\n");
        return -1;
    }
	#endif
    LOGD("init all objArray done");

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    //klein: add client_data_t to lwm2m_context_t
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        fprintf(stderr, "lwm2m_init() failed\r\n");
        return -1;
    }
	lwm2mH->isColdStart = isColdStart;

    data.lwm2mH = lwm2mH;

    for (int i = 0; i < OBJ_COUNT; i++){
        objArray[i]->ctxP = lwm2mH;
    }
    /*
     * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
     * the number of objects we will be passing through and the objects array
     */
    //klein: add endpointName, objectList to lwm2m_context_t
    //add by joe start
    if (true == lwm2m_check_is_vzw_netwrok() && EFAILURE != (fd = open(SECURITY_PERSISTENCE_FILE, O_RDONLY)))
    {
         fprintf(stderr, "load security info from fs!!!\n");
         close(fd);
         load_security_persistent_info(secObjP);
         
         fd = open(SERVER_PERSISTENCE_FILE, O_RDONLY);
         if (EFAILURE != fd)
         {
            close(fd);
            load_server_persistent_info(serObjP);
         }
         fd = open(ACL_PERSISTENCE_FILE, O_RDONLY);
         if (EFAILURE != fd)
         {
            close(fd);
            load_ac_persistent_info(aclObjP);
         }
         
    }
    //add by joe end
    char msisdn[64] = {0};
    GetDevicePhoneNumber(msisdn);
    result = lwm2m_configure(lwm2mH, name, msisdn, NULL, OBJ_COUNT, objArray);
    if (result != 0)
    {
        fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }

    signal(SIGINT, handle_sigint);

    /**
     * Initialize value changed callback.
     */
    init_value_change(lwm2mH);


    fprintf(stdout, "LWM2M Client \"%s\" started on port %s\r\n", name, localPort);

    if(lwm2m_check_is_att_netwrok()){
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, lwm2m_client_receive_at_command, lwm2mH);
        //lwm2m_client_receive_at_command();
    }
    //quectel_set_lwm2mcontxt_ptr(lwm2mH);

	pthread_t net_thread_id;
	pthread_create(&net_thread_id, NULL, lwm2m_notify_network_change,NULL);

	pthread_t obj_conn_moni_thread_id;
	pthread_create(&obj_conn_moni_thread_id, NULL, lwm2m_notify_conn_info_change,lwm2mH);

    g_lwm2mH_ptr = lwm2mH;

    /*
     * We now enter in a while loop that will handle the communications from the server
     */
     time_t startTime = lwm2m_gettime();
     int retryTimes = 1;
     write_enable = true;
    while (0 == g_quit)
    {
        //LOG("while (0 == g_quit) X");//joe del because too many logs
        struct timeval tv;
        fd_set readfds;

        if (g_reboot == 1)
        {
            time_t tv_sec;

            tv_sec = lwm2m_gettime();

            if (0 == reboot_time)
            {
                reboot_time = tv_sec + 5;
            }
            if (reboot_time < tv_sec)
            {
                /*
                 * Message should normally be lost with reboot ...
                 */
                fprintf(stderr, "reboot time expired, rebooting ...");
                system_reboot();
            }
            else
            {
                tv.tv_sec = reboot_time - tv_sec;
            }
        }else if(g_reboot == 2){//factoryreset
            time_t tv_sec;

            tv_sec = lwm2m_gettime();

            if (0 == reboot_time)
            {
                reboot_time = tv_sec + 5;
            }
            if (reboot_time < tv_sec)
            {
                /*
                 * Message should normally be lost with reboot ...
                 */
                fprintf(stderr, "reboot time expired, rebooting ...");
                system_factoryreset();
            }
            else
            {
                tv.tv_sec = reboot_time - tv_sec;
            }
        }
        else if (batterylevelchanging)
        {
            update_battery_level(lwm2mH);
            tv.tv_sec = 5;
        }
        else
        {
            tv.tv_sec = 60;
        }
        tv.tv_usec = 0;



        /*
         * This function does two things:
         *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
         *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
         *    (eg. retransmission) and the time between the next operation
         */
        result = lwm2m_step(lwm2mH, &(tv.tv_sec));
        //fprintf(stdout, " -> State: ");//joe del because too many logs
        //LOG_ARG("lwm2mH->state = %d \r\n", lwm2mH->state);//joe del because too many logs
        switch (lwm2mH->state)
        {
        case STATE_INITIAL:
            LOG("STATE_INITIAL\r\n");
            break;
        case STATE_BOOTSTRAP_REQUIRED:
            LOG("STATE_BOOTSTRAP_REQUIRED\r\n");
            break;
        case STATE_BOOTSTRAPPING:
            //LOG("STATE_BOOTSTRAPPING\r\n");
            break;
        case STATE_REGISTER_REQUIRED:
            //LOG("STATE_REGISTER_REQUIRED\r\n");
            break;
        case STATE_REGISTERING:
            //LOG( "STATE_REGISTERING\r\n");
            break;
        case STATE_READY:
            //LOG("STATE_READY\r\n");//joe del because too many logs
            break;
        default:
            fprintf(stdout, "Unknown...\r\n");
            break;
        }
        if (result != 0)
        {
            LOG_ARG("current time=%d lwm2m_step() failed: 0x%X\r\n",lwm2m_gettime(), result);
            LOG_ARG("lwm2m_step() failed: 0x%X", result);
            //add by joe start
            if(result == COAP_503_SERVICE_UNAVAILABLE /*&& lwm2m_check_is_vzw_netwrok()*/){
                //do retry wait
                if(current_retry_count <= MAX_RETRY_CONUT){
                    current_retry_count++;
                }
                #if 0 //do not kill lwm2m client
                else{
                    LOG("----connect fail,exit \r\n");
                    return -1;//restart lwm2m client
                }
                #endif
                #ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
                if(lwm2m_check_is_vzw_netwrok()){
                    quec_check_apn_network_available();
                }
                #endif
                LOG_ARG("will retry in %d seconds\n",session_retry_timer[current_retry_count]);
                sleep(session_retry_timer[current_retry_count]);
                LOG_ARG("start retry, last state=%d current time=%d\n",lwm2mH->state,lwm2m_gettime());
                continue;
            }else if(result == COAP_400_BAD_REQUEST && lwm2m_check_is_vzw_netwrok()){
                sleep(LWM2M_SERVER_BS_RETRY_TIMEOUT);//waite for 24 hours to retry
                LOG_ARG("retry, last state=%d current time=%d\n",lwm2mH->state,lwm2m_gettime());
                continue;
            }
            //add by joe end
            if(previousState == STATE_BOOTSTRAPPING)
            {
#ifdef WITH_LOGS
                fprintf(stdout, "[BOOTSTRAP] restore security and server objects\r\n");
#endif
                prv_restore_objects(lwm2mH);
                lwm2mH->state = STATE_INITIAL;
            }
            else{//add by jambo
					if(2 != quec_connect_check()){
						LOG("connect fial,wait 30s and retry...\r\n");
						sleep(30);
					}
                continue;
            }
        }
#ifdef LWM2M_BOOTSTRAP
        update_bootstrap_info(&previousState, lwm2mH);
#endif

        //printf("----tv.tv_sec = %d \r\n", tv.tv_sec);

		dtls_connection_t * connP = NULL;
		//add by joe start
        lwm2m_server_t * subtargetP;
        int serverCount = 0;
        if(lwm2mH->state == STATE_BOOTSTRAP_REQUIRED
            || lwm2mH->state == STATE_BOOTSTRAPPING){
            subtargetP = data.lwm2mH->bootstrapServerList;
            serverCount = data.lwm2mH->bootstrapServerListCount;

        }else if(lwm2mH->state == STATE_REGISTER_REQUIRED
            || lwm2mH->state == STATE_REGISTERING
            || lwm2mH->state == STATE_READY){
            subtargetP = data.lwm2mH->serverList;
            serverCount = data.lwm2mH->serverListCount;
        }

        if(!lwm2m_check_is_vzw_netwrok()){
            serverCount = 1;
        }
        //while (subtargetP != NULL )
        for (int i = 0; i < serverCount; i++)
        {
            if(subtargetP == NULL){
                break;
            }
        if(lwm2m_check_is_vzw_netwrok()){
            connP = (dtls_connection_t*)subtargetP->sessionH;
        }else if(lwm2m_check_is_att_netwrok()){
            connP = data.connList;
        }

        if (connP == NULL){
            if(lwm2m_check_is_vzw_netwrok()){
                subtargetP = subtargetP->next; //add by joe. jump to next server
                continue;//jump to next server
            }
            if(lwm2m_check_is_att_netwrok()){
               // LOG("--connP is null-- \r\n");
                if(lwm2mH->state == STATE_READY){
                    LOG("--reconnect-- \r\n");
                    //parse_connect_info(&isColdStart, server, serverPort, name, pskId, psk, &pskLen);
                    lwm2mH->isColdStart = !within_life_time();
                    lwm2mH->state = STATE_REGISTER_REQUIRED;
                    sleep(5);
                }
               // if(2 != quec_connect_check()){
                //    sleep(10);
               // }
                break;
            }
        }

        FD_ZERO(&readfds);
        FD_SET(connP->net_ctx->fd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
		//printf("----fd set done \r\n");//joe del because too many logs

        /*
         * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
         * with the precedent function)
         */
        int result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);
		//LOG_ARG("data comming or select timeout,  result = %d\n", result);//joe del because too many logs

        if (result < 0)
        {
                LOG_ARG("result < 0  result = %d\n", result);
            if (errno != EINTR)
            {
                fprintf(stderr, "Error in select(): %d %s\r\n", errno, strerror(errno));
            }
        }
        else if (result > 0)
        {
            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(connP->net_ctx->fd, &readfds))
            {

				int ret;
				uint8_t buffer[MAX_PACKET_SIZE];
				int numBytes = sizeof(buffer) - 1;;
				memset(buffer, 0, sizeof(buffer));

				if (connP != NULL)
				{
    				    int read_retry = 0;
					while(1)
					{
                            LOG( "read waiting...");
                            if( connP->dlts_resumption == 1){
                                LOG( "dlts_resumption == 1 read waiting..break..");
    							break;
                            }
						    ret = mbedtls_ssl_read(connP->ssl_ctx, buffer, numBytes);
                            #if defined(VZW_FATAL_ALERT_SUPPORT)
                            if(MBEDTLS_ERR_SSL_FATAL_ALERT_MESSAGE == ret){//joe add for vzw fatal alert handle
                                LOG( "fatal error alert Happed..");
                                lwm2m_process_fatal_alert(subtargetP);
                                break;
                            }
                            #endif
                            LOG_ARG("[LWM2M] lwm2m_gettime=%d",lwm2m_gettime()); 
                            connP->last_conn_time = lwm2m_gettime();//add by joe for vzw
                            connP->dlts_resumption = 0;
                            LOG_ARG("[LWM2M] mbedtls_ssl_read ret=%d",ret); 
						    if( ret != MBEDTLS_ERR_SSL_WANT_READ &&
    							ret != MBEDTLS_ERR_SSL_WANT_WRITE ){
    							break;
                            }     

                            read_retry++;
                            if(read_retry > 5){
    							break;
                            }
    					}
    				}
                    LOG_ARG( "%d bytes read\n\n%s\n", ret, buffer);
                output_buffer(stderr, buffer, ret, 0);
				lwm2m_handle_packet(lwm2mH, buffer, ret, connP);
			}
            }

            subtargetP = subtargetP->next; //add by joe. go to next server
        }
        if(lwm2m_check_is_vzw_netwrok()){
           #ifdef DTLS_SESSION_FREQUENTLY_RESUMPTION
           lwm2m_client_dtls_session_resumption(lwm2mH);//try not resumption every minute, resumption behavior TBD
           #endif
           lwm2m_client_checkif_update_all_server_regist(g_lwm2mH_ptr);//check if need server regist update
		}
        g_lwm2mH_ptr = lwm2mH;
        usleep(50*1000);//add to reduce CPU usage
        //LOG("while (0 == g_quit) E");//joe del because too many logs
    }

    LOGD("while done");

    /*
     * Finally when the loop is left smoothly - asked by user in the command line interface - we unregister our client from it
     */

    free(pskBuffer);
#ifdef LWM2M_BOOTSTRAP
    close_backup_object();
#endif
    lwm2m_close(lwm2mH);

    close(data.sock);
    connection_free(data.connList);

    clean_security_object(objArray[0]);
    lwm2m_free(objArray[0]);
    clean_server_object(objArray[1]);
    lwm2m_free(objArray[1]);
    acl_ctrl_free_object(objArray[2]);
    free_object_device(objArray[3]);
    free_object_conn_m(objArray[4]);
    free_object_firmware(objArray[5]);
    free_object_location(objArray[6]);
    //free_test_object(objArray[5]);//jambo delete
    free_object_conn_s(objArray[7]);
    if(/*lwm2m_check_is_vzw_netwrok()*/0){
        free_vzw_object_host_device(objArray[8]);
    }else{
        free_object_host_device(objArray[8]);
    }
    free_object_apn_conn(objArray[9]);
    free_object_conn_ex(objArray[10]);
	free_object_devicecap(objArray[11]);
	#ifdef ENABLE_SOFTWARE_MGNT_OBJ
	free_object_softwaremgnt(objArray[12]);
	#endif
#ifdef MEMORY_TRACE
    if (g_quit == 1)
    {
        trace_print(0, 1);
    }
#endif
    lwm2m_release_log_filefd();

    return 0;
}

#else
int main(int argc, char *argv[])
{
    LOGD("mango-lwm2m deamon start");
    client_data_t data;
    int result;
    lwm2m_context_t * lwm2mH = NULL;
    int i;
    const char * localPort = "56830";
    const char * server = NULL;
    const char * serverPort = LWM2M_STANDARD_PORT_STR;
    char * name = "testlwm2mclient";
    int lifetime = 300;
    int batterylevelchanging = 0;
    time_t reboot_time = 0;
    int opt;
    bool bootstrapRequested = false;
    bool serverPortChanged = false;

    bool is_receive_started = false;


#ifdef LWM2M_BOOTSTRAP
    lwm2m_client_state_t previousState = STATE_INITIAL;
#endif

    char * pskId = NULL;
#ifdef WITH_TLS
    char * psk = NULL;
#endif
    uint16_t pskLen = -1;
    char * pskBuffer = NULL;

    /*
     * The function start by setting up the command line interface (which may or not be useful depending on your project)
     *
     * This is an array of commands describes as { name, description, long description, callback, userdata }.
     * The firsts tree are easy to understand, the callback is the function that will be called when this command is typed
     * and in the last one will be stored the lwm2m context (allowing access to the server settings and the objects).
     */
    command_desc_t commands[] =
            {
                    {"list", "List known servers.", NULL, prv_output_servers, NULL},
                    {"change", "Change the value of resource.", " change URI [DATA]\r\n"
                                                                "   URI: uri of the resource such as /3/0, /3/0/2\r\n"
                                                                "   DATA: (optional) new value\r\n", prv_change, NULL},
                    {"update", "Trigger a registration update", " update SERVER\r\n"
                                                                "   SERVER: short server id such as 123\r\n", prv_update, NULL},
#ifdef LWM2M_BOOTSTRAP
            {"bootstrap", "Initiate a DI bootstrap process", NULL, prv_initiate_bootstrap, NULL},
            {"dispb", "Display current backup of objects/instances/resources\r\n"
                    "\t(only security and server objects are backupped)", NULL, prv_display_backup, NULL},
#endif
                    {"ls", "List Objects and Instances", NULL, prv_object_list, NULL},
                    {"disp", "Display current objects/instances/resources", NULL, prv_display_objects, NULL},
                    {"dump", "Dump an Object", "dump URI"
                                               "URI: uri of the Object or Instance such as /3/0, /1\r\n", prv_object_dump, NULL},
                    {"add", "Add support of object 31024", NULL, prv_add, NULL},
                    {"rm", "Remove support of object 31024", NULL, prv_remove, NULL},
                    {"quit", "Quit the client gracefully.", NULL, prv_quit, NULL},
                    {"^C", "Quit the client abruptly (without sending a de-register message).", NULL, NULL, NULL},

                    COMMAND_END_LIST
            };

    memset(&data, 0, sizeof(client_data_t));
    data.addressFamily = AF_INET6;

    opt = 1;
    while (opt < argc)
    {
        if (argv[opt] == NULL
            || argv[opt][0] != '-'
            || argv[opt][2] != 0)
        {
            print_usage();
            return 0;
        }
        switch (argv[opt][1])
        {
            case 'b':
                bootstrapRequested = true;
                if (!serverPortChanged) serverPort = LWM2M_BSSERVER_PORT_STR;
                break;
            case 'c':
                batterylevelchanging = 1;
                break;
            case 't':
                opt++;
                if (opt >= argc)
                {
                    print_usage();
                    return 0;
                }
                if (1 != sscanf(argv[opt], "%d", &lifetime))
                {
                    print_usage();
                    return 0;
                }
                break;
#ifdef WITH_TLS
            case 'i':
            opt++;
            if (opt >= argc)
            {
                print_usage();
                return 0;
            }
            pskId = argv[opt];
            break;
        case 's':
            opt++;
            if (opt >= argc)
            {
                print_usage();
                return 0;
            }
            psk = argv[opt];
            break;
#endif
            case 'n':
                opt++;
                if (opt >= argc)
                {
                    print_usage();
                    return 0;
                }
                name = argv[opt];
                break;
            case 'l':
                opt++;
                if (opt >= argc)
                {
                    print_usage();
                    return 0;
                }
                localPort = argv[opt];
                break;
            case 'h':
                opt++;
                if (opt >= argc)
                {
                    print_usage();
                    return 0;
                }
                server = argv[opt];
                break;
            case 'p':
                opt++;
                if (opt >= argc)
                {
                    print_usage();
                    return 0;
                }
                serverPort = argv[opt];
                serverPortChanged = true;
                break;
            case '4':
                data.addressFamily = AF_INET;
                break;
            default:
                print_usage();
                return 0;
        }
        opt += 1;
    }

    if (!server)
    {
        server = (AF_INET == data.addressFamily ? DEFAULT_SERVER_IPV4 : DEFAULT_SERVER_IPV6);
    }

    /*
     *This call an internal function that create an IPV6 socket on the port 5683.
     */
    fprintf(stderr, "Trying to bind LWM2M Client to port %s\r\n", localPort);

    data.sock = create_socket(localPort, data.addressFamily);
    if (data.sock < 0)
    {
        fprintf(stderr, "Failed to open socket: %d %s\r\n", errno, strerror(errno));
        return -1;
    }
    LOGD("create_socket done");


    /*
     * Now the main function fill an array with each object, this list will be later passed to liblwm2m.
     * Those functions are located in their respective object file.
     */
#ifdef WITH_TLS
    if (psk != NULL)
    {
        pskLen = strlen(psk) / 2;
        pskBuffer = malloc(pskLen);

        if (NULL == pskBuffer)
        {
            fprintf(stderr, "Failed to create PSK binary buffer\r\n");
            return -1;
        }
        // Hex string to binary
        char *h = psk;
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
#endif

    char serverUri[50];
    int serverId = 123;
#ifdef WITH_TLS
    sprintf (serverUri, "coaps://%s:%s", server, serverPort);
#else
    sprintf (serverUri, "coap://%s:%s", server, serverPort);
#endif

    LOGD("serverUri = %s, serverPort = %s, psk = %s, pskLen = %d", serverUri, serverPort, psk, pskLen);


#ifdef LWM2M_BOOTSTRAP
    LOGD("LWM2M_BOOTSTRAP defined");
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, bootstrapRequested);
#else
    LOGD("LWM2M_BOOTSTRAP not defined");
    objArray[0] = get_security_object(serverId, serverUri, pskId, pskBuffer, pskLen, false);
#endif

    LOGD("get_security_object done");


    if (NULL == objArray[0])
    {
        fprintf(stderr, "Failed to create security object\r\n");
        return -1;
    }
    data.securityObjP = objArray[0];

    objArray[1] = get_server_object(serverId, "U", lifetime, false);
    if (NULL == objArray[1])
    {
        fprintf(stderr, "Failed to create server object\r\n");
        return -1;
    }

    int instId = 0;
    objArray[2] = acc_ctrl_create_object();
    if (NULL == objArray[2])
    {
        fprintf(stderr, "Failed to create Access Control object\r\n");
        return -1;
    }
    else if (acc_ctrl_obj_add_inst(objArray[2], instId, 3, 0, serverId)==false)
    {
        fprintf(stderr, "Failed to create Access Control object instance\r\n");
        return -1;
    }
    else if (acc_ctrl_oi_add_ac_val(objArray[2], instId, 0, 0b000000000001111)==false)
    {
        fprintf(stderr, "Failed to create Access Control ACL default resource\r\n");
        return -1;
    }
    else if (acc_ctrl_oi_add_ac_val(objArray[2], instId, 999, 0b000000000000001)==false)
    {
        fprintf(stderr, "Failed to create Access Control ACL resource for serverId: 999\r\n");
        return -1;
    }
    
    objArray[3] = get_object_device();
    if (NULL == objArray[3])
    {
        fprintf(stderr, "Failed to create Device object\r\n");
        return -1;
    }

    objArray[4] = get_object_conn_m();
    if (NULL == objArray[4])
    {
        fprintf(stderr, "Failed to create connectivity monitoring object\r\n");
        return -1;
    }

    objArray[5] = get_object_firmware();
    if (NULL == objArray[5])
    {
        fprintf(stderr, "Failed to create Firmware object\r\n");
        return -1;
    }

    objArray[6] = get_object_location();
    if (NULL == objArray[6])
    {
        fprintf(stderr, "Failed to create location object\r\n");
        return -1;
    }
/*
    objArray[5] = get_test_object();
    if (NULL == objArray[5])
    {
        fprintf(stderr, "Failed to create test object\r\n");
        return -1;
    }
*/
//jambo delete and modify array

    objArray[7] = get_object_conn_s();
    if (NULL == objArray[7])
    {
        fprintf(stderr, "Failed to create connectivity statistics object\r\n");
        return -1;
    }

    LOGD("init all objArray done");

    /*
     * The liblwm2m library is now initialized with the functions that will be in
     * charge of communication
     */
    //klein: add client_data_t to lwm2m_context_t
    lwm2mH = lwm2m_init(&data);
    if (NULL == lwm2mH)
    {
        fprintf(stderr, "lwm2m_init() failed\r\n");
        return -1;
    }

#ifdef WITH_TLS
    data.lwm2mH = lwm2mH;
#endif

    /*
     * We configure the liblwm2m library with the name of the client - which shall be unique for each client -
     * the number of objects we will be passing through and the objects array
     */
    //klein: add endpointName, objectList to lwm2m_context_t
    result = lwm2m_configure(lwm2mH, name, NULL, NULL, OBJ_COUNT, objArray);
    if (result != 0)
    {
        fprintf(stderr, "lwm2m_configure() failed: 0x%X\r\n", result);
        return -1;
    }

    signal(SIGINT, handle_sigint);

    /**
     * Initialize value changed callback.
     */
    init_value_change(lwm2mH);

    /*
     * As you now have your lwm2m context complete you can pass it as an argument to all the command line functions
     * precedently viewed (first point)
     */
    for (i = 0 ; commands[i].name != NULL ; i++)
    {
        commands[i].userData = (void *)lwm2mH;
    }
    fprintf(stdout, "LWM2M Client \"%s\" started on port %s\r\n", name, localPort);
    fprintf(stdout, "> "); fflush(stdout);

    /*
     * We now enter in a while loop that will handle the communications from the server
     */
    while (0 == g_quit)
    {
        LOGD("while (0 == g_quit) X");
        struct timeval tv;
        fd_set readfds;

        if (g_reboot == 1)
        {
            time_t tv_sec;

            tv_sec = lwm2m_gettime();

            if (0 == reboot_time)
            {
                reboot_time = tv_sec + 5;
            }
            if (reboot_time < tv_sec)
            {
                /*
                 * Message should normally be lost with reboot ...
                 */
                fprintf(stderr, "reboot time expired, rebooting ...");
                system_reboot();
            }
            else
            {
                tv.tv_sec = reboot_time - tv_sec;
            }
        }else if(g_reboot == 2){//factoryreset
            time_t tv_sec;

            tv_sec = lwm2m_gettime();

            if (0 == reboot_time)
            {
                reboot_time = tv_sec + 5;
            }
            if (reboot_time < tv_sec)
            {
                /*
                 * Message should normally be lost with reboot ...
                 */
                fprintf(stderr, "reboot time expired, rebooting ...");
                system_factoryreset();
            }
            else
            {
                tv.tv_sec = reboot_time - tv_sec;
            }
        }
        else if (batterylevelchanging)
        {
            update_battery_level(lwm2mH);
            tv.tv_sec = 5;
        }
        else
        {
            tv.tv_sec = 60;
        }
        tv.tv_usec = 0;



        /*
         * This function does two things:
         *  - first it does the work needed by liblwm2m (eg. (re)sending some packets).
         *  - Secondly it adjusts the timeout value (default 60s) depending on the state of the transaction
         *    (eg. retransmission) and the time between the next operation
         */
        result = lwm2m_step(lwm2mH, &(tv.tv_sec));
        fprintf(stdout, " -> State: ");
        LOGD("lwm2mH->state = %d", lwm2mH->state);
        switch (lwm2mH->state)
        {
            case STATE_INITIAL:
                fprintf(stdout, "STATE_INITIAL\r\n");
                break;
            case STATE_BOOTSTRAP_REQUIRED:
                fprintf(stdout, "STATE_BOOTSTRAP_REQUIRED\r\n");
                break;
            case STATE_BOOTSTRAPPING:
                fprintf(stdout, "STATE_BOOTSTRAPPING\r\n");
                break;
            case STATE_REGISTER_REQUIRED:
                fprintf(stdout, "STATE_REGISTER_REQUIRED\r\n");
                break;
            case STATE_REGISTERING:
                fprintf(stdout, "STATE_REGISTERING\r\n");
                break;
            case STATE_READY:
                fprintf(stdout, "STATE_READY\r\n");
                break;
            default:
                fprintf(stdout, "Unknown...\r\n");
                break;
        }
        if (result != 0)
        {
            fprintf(stderr, "lwm2m_step() failed: 0x%X\r\n", result);
            LOGD("lwm2m_step() failed: 0x%X", result);
            if(previousState == STATE_BOOTSTRAPPING)
            {
#ifdef WITH_LOGS
                fprintf(stdout, "[BOOTSTRAP] restore security and server objects\r\n");
#endif
                LOGD("[BOOTSTRAP] restore security and server objects");
                prv_restore_objects(lwm2mH);
                lwm2mH->state = STATE_INITIAL;
            }
            else return -1;
        }
#ifdef LWM2M_BOOTSTRAP
        update_bootstrap_info(&previousState, lwm2mH);
#endif

        LOGD("data.sock = %d", data.sock);


        FD_ZERO(&readfds);
        FD_SET(data.sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        LOGD("----tv.tv_sec = %d", tv.tv_sec);
        /*
         * This part will set up an interruption until an event happen on SDTIN or the socket until "tv" timed out (set
         * with the precedent function)
         */
        result = select(FD_SETSIZE, &readfds, NULL, NULL, &tv);

        if (result < 0)
        {
            if (errno != EINTR)
            {
              fprintf(stderr, "Error in select(): %d %s\r\n", errno, strerror(errno));
            }
        }
        else if (result > 0)
        {
            uint8_t buffer[MAX_PACKET_SIZE];
            int numBytes;

            /*
             * If an event happens on the socket
             */
            if (FD_ISSET(data.sock, &readfds))
            {
                struct sockaddr_storage addr;
                socklen_t addrLen;

                addrLen = sizeof(addr);

                /*
                 * We retrieve the data received
                 */
                numBytes = recvfrom(data.sock, buffer, MAX_PACKET_SIZE, 0, (struct sockaddr *)&addr, &addrLen);

                if (0 > numBytes)
                {
                    fprintf(stderr, "Error in recvfrom(): %d %s\r\n", errno, strerror(errno));
                }
                else if (0 < numBytes)
                {
                    char s[INET6_ADDRSTRLEN];
                    in_port_t port;

#ifdef WITH_TINYDTLS
                    dtls_connection_t * connP;
#else
                    connection_t * connP;
#endif
                    if (AF_INET == addr.ss_family)
                    {
                        struct sockaddr_in *saddr = (struct sockaddr_in *)&addr;
                        inet_ntop(saddr->sin_family, &saddr->sin_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin_port;
                    }
                    else if (AF_INET6 == addr.ss_family)
                    {
                        struct sockaddr_in6 *saddr = (struct sockaddr_in6 *)&addr;
                        inet_ntop(saddr->sin6_family, &saddr->sin6_addr, s, INET6_ADDRSTRLEN);
                        port = saddr->sin6_port;
                    }
                    fprintf(stderr, "%d bytes received from [%s]:%hu\r\n", numBytes, s, ntohs(port));
                    LOGD("%d bytes received  from [%s]:%hu", numBytes, s, ntohs(port));

                    /*
                     * Display it in the STDERR
                     */
                    output_buffer(stderr, buffer, numBytes, 0);
                    LOGD("connection_find out");
                    connP = connection_find(data.connList, &addr, addrLen);
                    if (connP != NULL)
                    {
                        /*
                         * Let liblwm2m respond to the query depending on the context
                         */
#ifdef WITH_TINYDTLS
                        LOGD("connection_handle_packet out");
                        int result = connection_handle_packet(connP, buffer, numBytes);
                        if (0 != result)
                        {
                             LOG_ARG("error handling message %d\n",result);
                        }
#else
                        lwm2m_handle_packet(lwm2mH, buffer, numBytes, connP);
#endif
                        conn_s_updateRxStatistic(objArray[7], numBytes, false);
                    }
                    else
                    {
                        fprintf(stderr, "received bytes ignored!\r\n");
                    }
                }
            }

            /*
             * If the event happened on the SDTIN
             */
            if (FD_ISSET(STDIN_FILENO, &readfds))
            {
                numBytes = read(STDIN_FILENO, buffer, MAX_PACKET_SIZE - 1);

                if (numBytes > 1)
                {
                    buffer[numBytes] = 0;
                    /*
                     * We call the corresponding callback of the typed command passing it the buffer for further arguments
                     */
                    handle_command(commands, (char*)buffer);
                }
                if (g_quit == 0)
                {
                    fprintf(stdout, "\r\n> ");
                    fflush(stdout);
                }
                else
                {
                    fprintf(stdout, "\r\n");
                }
            }
        }

        LOGD("while (0 == g_quit) E");
    }

    LOGD("while done");

    /*
     * Finally when the loop is left smoothly - asked by user in the command line interface - we unregister our client from it
     */
    if (g_quit == 1)
    {
#ifdef WITH_TLS
        free(pskBuffer);
#endif

#ifdef LWM2M_BOOTSTRAP
        close_backup_object();
#endif
        lwm2m_close(lwm2mH);
    }
    close(data.sock);
    connection_free(data.connList);

    clean_security_object(objArray[0]);
    lwm2m_free(objArray[0]);
    clean_server_object(objArray[1]);
    lwm2m_free(objArray[1]);
    acl_ctrl_free_object(objArray[2]);
    free_object_device(objArray[3]);
    free_object_conn_m(objArray[4]);
    free_object_firmware(objArray[5]);
    free_object_location(objArray[6]);
    //free_test_object(objArray[5]);//jambo delete
    free_object_conn_s(objArray[7]);


#ifdef MEMORY_TRACE
    if (g_quit == 1)
    {
        trace_print(0, 1);
    }
#endif

    return 0;
}
#endif


int lwm2m_client_write_object_resource(lwm2m_context_t * lwm2mH, int obj_id, int instance_id, int res_id, void *res_data)
{
	//lwm2m_client  *client_ptr = (lwm2m_client *)client_hdl;
	lwm2m_object_data_t *lwm2m_dataP = NULL;
	lwm2m_context_t * contextP = NULL;
	lwm2m_object_t  *targetP = NULL;
	uint8_t result = 0;

//	MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] %s", __FUNCTION__);
	if(/*client_ptr == NULL || client_ptr->lwm2m_ctx == NULL || */res_data == NULL)
		return LWM2M_CLIENT_ERR_INVALID_PARAM;
	contextP = lwm2mH;

	if (!LWM2M_IS_STANDARD_OBJECT(obj_id))
	{
		return LWM2M_CLIENT_ERR_INVALID_PARAM;
	}

	targetP = (lwm2m_object_t *)LWM2M_LIST_FIND(contextP->objectList, obj_id);
	if ((NULL == targetP) || (!targetP->setValueFunc))
	{
		return LWM2M_CLIENT_ERR_INVALID_PARAM;
	}

	if(res_id < 0)
	{
		return LWM2M_CLIENT_ERR_INVALID_PARAM;
	}

	lwm2m_dataP = lwm2m_malloc(sizeof(lwm2m_object_data_t));

	if(lwm2m_dataP == NULL)
		return LWM2M_CLIENT_ERR_OUT_MEM;
	
	memset(lwm2m_dataP, 0, sizeof(lwm2m_object_data_t));
	lwm2m_dataP->instance_info = lwm2m_malloc(sizeof(lwm2m_instance_info_t));

	if(lwm2m_dataP->instance_info == NULL)
	{
		lwm2m_free_lwm2m_data(lwm2m_dataP);
		return LWM2M_CLIENT_ERR_OUT_MEM;
	}

	memset(lwm2m_dataP->instance_info, 0, sizeof(lwm2m_instance_info_t));
	lwm2m_dataP->instance_info->resource_info = lwm2m_malloc(sizeof(lwm2m_resource_info_t));

	if(lwm2m_dataP->instance_info->resource_info == NULL)
	{
		lwm2m_free_lwm2m_data(lwm2m_dataP);
		return LWM2M_CLIENT_ERR_OUT_MEM;
	}

	memset(lwm2m_dataP->instance_info->resource_info, 0, sizeof(lwm2m_resource_info_t));
	lwm2m_dataP->object_ID = obj_id;
	lwm2m_dataP->no_instances++;
    lwm2m_dataP->instance_info->instance_ID = instance_id;
    lwm2m_dataP->instance_info->no_resources++;
    lwm2m_dataP->instance_info->resource_info->resource_ID = res_id;

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] set obj_id=%d", obj_id);

	// if(obj_id == LWM2M_CONN_MONITOR_OBJECT_ID && (res_id == RES_O_LINK_QUALITY || res_id == RES_M_RADIO_SIGNAL_STRENGTH))
	// {
  		// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_INTEGER;
		// lwm2m_dataP->instance_info->resource_info->value.asInteger = *((int8_t*)res_data);
		// lwm2m_set_object_info(client_ptr->lwm2m_ctx, lwm2m_dataP,obj_id);
		// lwm2m_free_lwm2m_data(lwm2m_dataP);
		// return LWM2M_CLIENT_OK;
	// }
	// if(obj_id == LWM2M_LOCATION_OBJECT_ID && (res_id == RES_O_RADIUS || res_id == RES_M_LATITUDE || res_id == RES_M_LONGITUDE))
	// {
		// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_FLOAT;
		// lwm2m_dataP->instance_info->resource_info->value.asFloat = *((double*)res_data);
		// lwm2m_set_object_info(client_ptr->lwm2m_ctx, lwm2m_dataP,obj_id);
		// lwm2m_free_lwm2m_data(lwm2m_dataP);
		// return LWM2M_CLIENT_OK;
	// }
	// if(obj_id == LWM2M_FIRMWARE_UPDATE_OBJECT_ID &&(res_id == 3/*RES_M_STATE*/||res_id == 5)){
	//	 lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_INTEGER;
	//	 lwm2m_dataP->instance_info->resource_info->value.asInteger = *((uint8_t*)res_data);
	//	 lwm2m_set_object_info(lwm2mH, lwm2m_dataP,obj_id);
	//	 lwm2m_free_lwm2m_data(lwm2m_dataP);
	//	 return LWM2M_CLIENT_OK;
	// }
	// if(obj_id == LWM2M_APN_CONN_PROFILE_ID)
	// {
		// if(res_id == 9||res_id == 10 ||res_id == 11||res_id== 12)
		// {
			// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_INTEGER;
			// lwm2m_dataP->instance_info->resource_info->value.asInteger = *((int*)res_data);
		// }
		// else if(res_id == 0 || res_id == 1)
		// {
			// char *value = (char *)res_data;
			// int len = strlen(value);
			// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_STRING;
			// lwm2m_dataP->instance_info->resource_info->value.asBuffer.length = len;
			// lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer = lwm2m_malloc(len+1);
			// memset(lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer , 0, len+1);
            // memcpy(lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer , value, len);
		// }
		// lwm2m_set_object_info(client_ptr->lwm2m_ctx, lwm2m_dataP,obj_id);
		// lwm2m_free_lwm2m_data(lwm2m_dataP);
		// return LWM2M_CLIENT_OK;
	// }
	if(obj_id == LWM2M_HOST_DEVICE_OBJECT_ID && res_id == 0){
		lwm2m_hostdevice_info_t *host_dev_info = (lwm2m_hostdevice_info_t *)res_data;
		lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_MULTIPLE_RESOURCE;
		lwm2m_dataP->instance_info->resource_info->value.asChildren.count = 4;
		lwm2m_dataP->instance_info->resource_info->value.asChildren.array = lwm2m_data_new(4);
		lwm2m_data_encode_string(host_dev_info->unique_id, &(lwm2m_dataP->instance_info->resource_info->value.asChildren.array[0]));
		lwm2m_data_encode_string(host_dev_info->manufacture, &(lwm2m_dataP->instance_info->resource_info->value.asChildren.array[1]));
		lwm2m_data_encode_string(host_dev_info->model, &(lwm2m_dataP->instance_info->resource_info->value.asChildren.array[2]));
		lwm2m_data_encode_string(host_dev_info->sw_version, &(lwm2m_dataP->instance_info->resource_info->value.asChildren.array[3]));

		lwm2m_set_object_info(lwm2mH, lwm2m_dataP,obj_id);
        lwm2m_save_hostdevice_info(instance_id,host_dev_info);
		//lwm2m_client_object_update(client_hdl, int opt,int LWM2M_HOST_DEVICE_OBJECT_ID);
		//lwm2m_client_update(0, true);
		lwm2m_free_lwm2m_data(lwm2m_dataP);
		return LWM2M_CLIENT_OK;
	}
	// if(obj_id == LWM2M_VZW_HOST_DEVICE_OBJECT_ID){
		// if(res_id == 0 || res_id == 1 || res_id == 2 ||res_id == 3 || res_id == 4 || res_id == 5 ){
			// char *value = (char *)res_data;
			// int len = strlen(value);
			// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_STRING;
			// lwm2m_dataP->instance_info->resource_info->value.asBuffer.length = len;
			// lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer = lwm2m_malloc(len+1);
			// memset(lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer , 0, len+1);
            // memcpy(lwm2m_dataP->instance_info->resource_info->value.asBuffer.buffer , value, len);
			// lwm2m_set_object_info(client_ptr->lwm2m_ctx, lwm2m_dataP,obj_id);
			// lwm2m_free_lwm2m_data(lwm2m_dataP);
			// return LWM2M_CLIENT_OK;
		// }else if(res_id == 6){
			// lwm2m_dataP->instance_info->resource_info->type = LWM2M_TYPE_INTEGER;
			// lwm2m_dataP->instance_info->resource_info->value.asInteger = *((int*)res_data);
			// lwm2m_set_object_info(client_ptr->lwm2m_ctx, lwm2m_dataP,obj_id);
			// lwm2m_free_lwm2m_data(lwm2m_dataP);
			// return LWM2M_CLIENT_OK;
		// }
	// }

	lwm2m_free_lwm2m_data(lwm2m_dataP);
	return LWM2M_CLIENT_ERR_INVALID_PARAM;
}
