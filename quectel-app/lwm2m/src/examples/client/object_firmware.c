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
 *    Julien Vermillard - initial implementation
 *    Fabien Fleutot - Please refer to git log
 *    David Navarro, Intel Corporation - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Gregory Lemercier - Please refer to git log
 *    
 *******************************************************************************/

/*
 * This object is single instance only, and provide firmware upgrade functionality.
 * Object ID is 5.
 */

/*
 * resources:
 * 0 package                   write
 * 1 package url               write
 * 2 update                    exec
 * 3 state                     read
 * 5 update result             read
 * 6 package name              read
 * 7 package version           read
 * 8 update protocol support   read
 * 9 update delivery method    read
 */

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "quectel_info_util.h"
#include "internals.h"

// ---- private object "Firmware" specific defines ----
// Resource Id's:
#define RES_M_PACKAGE                   0
#define RES_M_PACKAGE_URI               1
#define RES_M_UPDATE                    2
#define RES_M_STATE                     3
#define RES_M_UPDATE_RESULT             5
#define RES_O_PKG_NAME                  6
#define RES_O_PKG_VERSION               7
#define RES_O_UPDATE_PROTOCOL           8
#define RES_M_UPDATE_METHOD             9

#define LWM2M_FIRMWARE_PROTOCOL_NUM     2
#define LWM2M_FIRMWARE_PROTOCOL_NULL    ((uint8_t)-1)

#define LWM2M_FIRMWARE_PROTOCOL_COAP    0
#define LWM2M_FIRMWARE_PROTOCOL_COAPS   1
#define LWM2M_FIRMWARE_PROTOCOL_HTTP    2
#define LWM2M_FIRMWARE_PROTOCOL_HTTPS   3


#define LWM2M_FIRMWARE_PROTOCOL_MAX   4


#define LWM2M_FIRMWARE_DELIVERY_PULL    1

#define LWM2M_FIRMWARE_DELIVERY_PUSH    2

#define LWM2M_FIRMWARE_PROTOCOL_NULL    ((uint8_t)-1)

#define BITMASK(x) (0x1 << x)

#define MAX_FIRMWARE_URI_LEN 128
#define FIRMWARE_STR_LEN     128
#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) \
        (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif

/*
VZW:
0: Idle (before downloading or after successful updating)
1: Downloading (The data sequence is on the way) 
2: Downloaded 
3: Updating
*/
typedef enum {
  LWM2M_FIRWARE_STATE_IDLE = 0,
  LWM2M_FIRWARE_STATE_DOWNLOADING =1,
  LWM2M_FIRWARE_STATE_DOWNLOADED =2,
  LWM2M_FIRWARE_STATE_UPDATING =3,
}lwm2m_firmware_state;

typedef enum  {
  LWM2M_FIRWARE_UPDATE_DEFAULT ,
  LWM2M_FIRWARE_UPDATE_SUCCESSFUL,
  LWM2M_FIRWARE_UPDATE_NOT_ENOUGH_STORAGE,
  LWM2M_FIRWARE_UPDATE_OUT_OF_MEMORY,
  LWM2M_FIRWARE_UPDATE_CONNECTION_LOST,
  LWM2M_FIRWARE_UPDATE_CRC_FAILED,
  LWM2M_FIRWARE_UPDATE_UNSUPPORTED_TYPE,
  LWM2M_FIRWARE_UPDATE_INVALID_URI,
  LWM2M_FIRMWARE_UPDATE_FAILED,
}lwm2m_firmware_uptate_result;

typedef struct
{
	struct firmware_data_ *next;
  	uint16_t  instanceId;
  	bool      updateExecuted;
    bool      supported;
    uint8_t state;
    uint8_t result;
    char pkg_name[FIRMWARE_STR_LEN];
    char pkg_version[FIRMWARE_STR_LEN];
	char      pkg_url[MAX_FIRMWARE_URI_LEN];
    resource_instance_int_list_t   *protocol_support;
    uint8_t delivery_method;
	int       last_block_num;
} firmware_data_t;

bool updateExecuted;

static firmware_data_t *firmwareInstance = NULL;
bool is_firmware_create = FALSE;
int is_firmware_update_downloading = LWM2M_FIRWARE_STATE_IDLE;
bool is_firmware_update_execute = false;
bool firmware_update_state_successful = false;
bool is_firmware_update_result_reported = true;

static void prv_change_protocol_support(firmware_data_t * data, uint8_t protocol_value, bool supported)
{

  resource_instance_int_list_t *prev = NULL;
  resource_instance_int_list_t *protocolList = NULL;
  uint8_t ri = 0;
  bool found = false;

  for(protocolList = data->protocol_support, prev = data->protocol_support, ri = 0;
      protocolList != NULL; protocolList = protocolList->next, ri++)
  {
    if(protocol_value == protocolList->InstValue)
    {
      if(supported == true)
        return;
      else {
        found = true;
        break;
      }
    }
    prev = protocolList;
  }

  if(supported)
  {
    resource_instance_int_list_t *new = NULL;

    new = lwm2m_malloc(sizeof(resource_instance_int_list_t));
    if(new != NULL) {
      memset(new, 0, sizeof(resource_instance_int_list_t));
      if(prev == NULL) {
        data->protocol_support = new;
      } else {
        prev->next = new;
      }
      new->resInstId = ri;
      new->InstValue = protocol_value;
      return;
    }else
      return;
  } 
  else 
  {
    if(found == true) {
      if(data->protocol_support == protocolList)
        data->protocol_support = protocolList->next;
      else {
        prev->next = protocolList->next;
      }     
      lwm2m_free(protocolList); 
    }
  }

}

static uint8_t prv_set_value(lwm2m_data_t * dataP, firmware_data_t * firmwareDataP)
{
  if( dataP == NULL || firmwareDataP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  } 
  
  switch (dataP->id)
  {
    case RES_M_PACKAGE_URI:
      lwm2m_data_encode_string(firmwareDataP->pkg_url, dataP);
      return COAP_205_CONTENT;
    case RES_M_STATE:
      lwm2m_data_encode_int(firmwareDataP->state, dataP);
      return COAP_205_CONTENT;
    case RES_M_UPDATE_RESULT :
      lwm2m_data_encode_int(firmwareDataP->result, dataP);
      return COAP_205_CONTENT;
    case RES_O_PKG_NAME:
      lwm2m_data_encode_string(firmwareDataP->pkg_name, dataP);
      return COAP_205_CONTENT;
    case RES_O_PKG_VERSION:
      lwm2m_data_encode_string(firmwareDataP->pkg_version, dataP);
      return COAP_205_CONTENT; 
    case RES_M_UPDATE_METHOD:
      lwm2m_data_encode_int(firmwareDataP->delivery_method, dataP);
      return COAP_205_CONTENT;    
    case RES_M_PACKAGE:
    case RES_M_UPDATE :
    case RES_O_UPDATE_PROTOCOL:
      return COAP_405_METHOD_NOT_ALLOWED;
    default:
      return COAP_404_NOT_FOUND;
  }
}


/** 
 * @fn     store_device_persistent_info()
 * @brief  This function is used to dump resource value set from App into persistence file
 * @param  fmDataP - pointer to Dev object
 * @return on success  - LWM2M_TRUE
           on error    - LWM2M_FALSE
 */ 
bool store_firmware_persistent_info(firmware_data_t* fmDataP)
{
  int fd = 0;
  bool res = LWM2M_FALSE;

  if (NULL == fmDataP)
  {
    return res;
  }
  
  fd = open(FIRMWARE_PERSISTENCE_FILE, O_CREAT | O_WRONLY | O_TRUNC, 00644);
  if (EFAILURE == fd)
  {
    return res;
  }

  if(!write_Tlv_to_file(fd, RES_O_PKG_NAME, strlen(fmDataP->pkg_name), (uint8_t*)fmDataP->pkg_name))
  {
    goto write_error;
  }
  if(!write_Tlv_to_file(fd, RES_M_PACKAGE_URI, strlen(fmDataP->pkg_url), (uint8_t*)fmDataP->pkg_url))
  {
    goto write_error;
  }
  if(!write_Tlv_to_file(fd, RES_O_PKG_VERSION, strlen(fmDataP->pkg_version), (uint8_t*)fmDataP->pkg_version))
  {
    goto write_error;
  }
  if(!write_Tlv_to_file(fd, RES_M_STATE, sizeof(fmDataP->state), (uint8_t*)&fmDataP->state))
  {
    goto write_error;
  }
  if(!write_Tlv_to_file(fd, RES_M_UPDATE_RESULT, sizeof(fmDataP->result), (uint8_t*)&fmDataP->result))
  {
    goto write_error;  
  }
  if(!write_Tlv_to_file(fd, RES_M_UPDATE_METHOD, sizeof(fmDataP->delivery_method), (uint8_t*)&fmDataP->delivery_method))
  {
    goto write_error;
  }

  if (fmDataP->protocol_support)
  {
    uint8_t ri = 0;
    resource_instance_int_list_t* protocolList = NULL;
	int support_protocol[LWM2M_FIRMWARE_PROTOCOL_MAX];
    
	for(protocolList = fmDataP->protocol_support;protocolList != NULL; protocolList = protocolList->next, ri++)
	{
	  if(ri == LWM2M_FIRMWARE_PROTOCOL_MAX)
	  {
	    break;
	  }
	  support_protocol[ri] = protocolList->InstValue;
	}
	if(ri > 0)
	{
	  if(!write_Tlv_to_file(fd, RES_O_UPDATE_PROTOCOL, ri*sizeof(int), (uint8_t*)support_protocol))
	  {
		 goto write_error;	  
	  }
	}
  }
  
  
  res = LWM2M_TRUE;

  write_error:
	close(fd);
    if(res == LWM2M_FALSE)
    {//if any error happen when delete the file
      unlink(FIRMWARE_PERSISTENCE_FILE);
    }

    return res;	
}


/** 
 * @fn     load_firmware_persistent_info()
 * @brief  This function is used to load resource value set from App from persistence file
 * @param  fmDataP - pointer to Dev object
 * @return on success  - LWM2M_TRUE
           on error    - LWM2M_FALSE
 */ 
bool load_firmware_persistent_info(firmware_data_t* fmDataP)
{
  int fd = 0;
  int    resource_id   = 0;
  uint8_t  resource_data_length = 0;
  uint8_t* resource_data = NULL;
  
  int   res = EFAILURE;

  if (NULL == fmDataP)
  {
    return LWM2M_FALSE;
  }
  fd = open(FIRMWARE_PERSISTENCE_FILE, 00);
  if (EFAILURE == fd)
  {
    return LWM2M_FALSE;
  }

  res = read_Tlv_from_file(fd, &resource_id, &resource_data_length, &resource_data);
  while(res == 1)
  {
    switch(resource_id)
    {
	  case RES_O_PKG_NAME:
		strlcpy(fmDataP->pkg_name, (const char*)resource_data, FIRMWARE_STR_LEN);
		break;
      case RES_O_PKG_VERSION:
	  	strlcpy(fmDataP->pkg_version, (const char*)resource_data, FIRMWARE_STR_LEN);
		break;
	  case RES_M_PACKAGE_URI:
	    strlcpy(fmDataP->pkg_url, (const char*)resource_data, MAX_FIRMWARE_URI_LEN);
		break;
	  case RES_M_STATE:
	  	fmDataP->state = *resource_data; 
		break;
	  case RES_M_UPDATE_RESULT:
	  	fmDataP->result = *resource_data; 
		break;
	  case RES_M_UPDATE_METHOD:
		fmDataP->delivery_method = *resource_data; 
		break;
	  case RES_O_UPDATE_PROTOCOL:
	  	{
	      int i = 0;
		  for(i=0; i < resource_data_length; i = i+4)
		  {		    
			prv_change_protocol_support(fmDataP, *(resource_data+i), true);
		  }
	  	}
		break;
	  default:
	  	break;
    }		
	lwm2m_free(resource_data);
	resource_data = NULL;
	resource_data_length = 0;
	res = read_Tlv_from_file(fd, &resource_id, &resource_data_length, &resource_data);
  }

  close(fd);

  if(res == EFAILURE)
  {
    unlink(FIRMWARE_PERSISTENCE_FILE);
	return LWM2M_FALSE;
  }

  return LWM2M_TRUE;
}

static uint8_t prv_firmware_read(uint16_t instanceId,
                                 int * numDataP,
                                 lwm2m_data_t ** dataArrayP,
                                 lwm2m_object_t * objectP)
{
    int i;
    uint8_t result;
    firmware_data_t * data = (firmware_data_t*)(objectP->userData);

    // this is a single instance object
    if (instanceId != 0)
    {
        return COAP_404_NOT_FOUND;
    }

    // is the server asking for the full object ?
    #if 0
    if (*numDataP == 0)
    {
        *dataArrayP = lwm2m_data_new(3);
        if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = 6;
        (*dataArrayP)[0].id = 3;
        (*dataArrayP)[1].id = 5;
        (*dataArrayP)[2].id = 6;
        (*dataArrayP)[3].id = 7;
        (*dataArrayP)[4].id = 8;
        (*dataArrayP)[5].id = 9;
    }
    #endif
      // is the server asking for the full object ?
      if (*numDataP == 0)
      {
        uint16_t resList[] = {
          RES_M_PACKAGE_URI,
          RES_M_STATE,
          RES_M_UPDATE_RESULT ,
          RES_O_PKG_NAME ,
          RES_O_PKG_VERSION ,
          RES_O_UPDATE_PROTOCOL,
          RES_M_UPDATE_METHOD
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);
        int num = nbRes;

        if(data->protocol_support == NULL)
          nbRes--;

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL) 
    		return COAP_500_INTERNAL_SERVER_ERROR;
        *numDataP = nbRes;
        for (i = 0; i < nbRes; i++)
        { 
          if(resList[i] == RES_O_UPDATE_PROTOCOL) {
            if(data->protocol_support != NULL)
              (*dataArrayP)[i].id = resList[i];
            else if(i < (num-1))
              (*dataArrayP)[i].id = resList[i+1];
          } else
            (*dataArrayP)[i].id = resList[i];  
        }
      }
  
    i = 0;
    do
    {
        switch ((*dataArrayP)[i].id)
        {
        case RES_M_PACKAGE:
        case RES_M_UPDATE:
            result = COAP_405_METHOD_NOT_ALLOWED;
            break;
        case RES_M_PACKAGE_URI:
            lwm2m_data_encode_string(data->pkg_url, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
        case RES_M_STATE:
            // firmware update state (int)
            lwm2m_data_encode_int(data->state, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
        case RES_O_PKG_NAME:
          lwm2m_data_encode_string(data->pkg_name, *dataArrayP + i);
          result = COAP_205_CONTENT;
          break;
        case RES_O_PKG_VERSION:
          if(data->pkg_version[0] == '\0'){
              char *svn = GetSoftwareVersion();
              if(strlen(svn) == 0){
                  get_sw_version(&data->pkg_version);
              }else{
                  strcpy(data->pkg_version,svn);              
              }            
              //lwm2m_get_device_sw_ver(data->pkg_version);
          }
          lwm2m_data_encode_string(data->pkg_version, *dataArrayP + i);
          result = COAP_205_CONTENT;
          break;      
        case RES_M_UPDATE_RESULT:
            lwm2m_data_encode_int(data->result, *dataArrayP + i);
            result = COAP_205_CONTENT;
            //add by joe ,mark fota result has reported to server
            is_firmware_update_result_reported = true;
            break;
        case RES_M_UPDATE_METHOD:
            lwm2m_data_encode_int(data->delivery_method, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;          
        case RES_O_UPDATE_PROTOCOL:
            #if 0
        {
            int ri;
            int num = 0;
            lwm2m_data_t* subTlvP = NULL;

            while ((num < LWM2M_FIRMWARE_PROTOCOL_NUM) &&
                    (data->protocol_support[num] != LWM2M_FIRMWARE_PROTOCOL_NULL))
                num++;

            if (num) {
                subTlvP = lwm2m_data_new(num);
                for (ri = 0; ri<num; ri++)
                {
                    subTlvP[ri].id = ri;
                    lwm2m_data_encode_int(data->protocol_support[ri], subTlvP + ri);
                }
            } else {
                /* If no protocol is provided, use CoAP as default (per spec) */
                num = 1;
                subTlvP = lwm2m_data_new(num);
                subTlvP[0].id = 0;
                lwm2m_data_encode_int(0, subTlvP);
            }
            lwm2m_data_encode_instances(subTlvP, num, *dataArrayP + i);
            result = COAP_205_CONTENT;
            break;
        }
        #endif
        {
          resource_instance_int_list_t* protocolList = NULL;
          #if 0 //not support
          if(resInstId != INVALID_RES_INST_ID)
              {
                resource_instance_int_list_t *resInst;
                resInst = find_resource_inst(data->protocol_support, resInstId);
                if(resInst != NULL)
                {
                  lwm2m_data_encode_int(resInst->InstValue, *dataArrayP + i);
                  result = COAP_205_CONTENT;
                }
                else
                {
                  result = COAP_404_NOT_FOUND;
                }
              }
              else
             #endif
              {
                uint8_t ri = 0;
                if (data->protocol_support)
                {
                   ri++;
                   for(protocolList = data->protocol_support;protocolList->next != NULL; protocolList = protocolList->next, ri++);
                }
                
                if(ri == 0) // no values!
                {
                  result = COAP_404_NOT_FOUND;
                }
                else
                {
                  lwm2m_data_t *subTlvP = lwm2m_data_new(ri);
                  if(NULL == subTlvP)
                    return COAP_500_INTERNAL_SERVER_ERROR;
    
                  for(protocolList = data->protocol_support, ri = 0; protocolList != NULL;
                      protocolList = protocolList->next, ri++)
                  {
                    subTlvP[ri].id = (uint16_t)protocolList->resInstId;
                    lwm2m_data_encode_int(protocolList->InstValue, &subTlvP[ri]);
                  }
                  lwm2m_data_encode_instances(subTlvP, ri, *dataArrayP + i);
                  result = COAP_205_CONTENT;
                }
              }
            }
        break;

        default:
            result = COAP_404_NOT_FOUND;
        }

        i++;
    } while (i < *numDataP && result == COAP_205_CONTENT);

    return result;
}

 /*
 * @fn       static int32_t prv_firmware_setvalue ()
 * @brief    This function is used to set the particular resource value from pllication
 * @param    lwm2m_data - pointer to lwm2m object information
    targetP  - pointer to lwm2m object 
 * @return   on success - 1
              on error   - 0    
 */
 static int32_t prv_firmware_setvalue (void *ctx_p, lwm2m_object_data_t * lwm2m_data, lwm2m_object_t * targetP, lwm2m_uri_t *uriP)
 {
     int result = 0;
     if(lwm2m_data == NULL || targetP == NULL || lwm2m_data->instance_info == NULL || lwm2m_data->instance_info->resource_info == NULL) 
     {
         return 0;
     }
     
     if(((firmware_data_t *)targetP->userData) == NULL) 
         return 0;
     
     switch(lwm2m_data->instance_info->resource_info->resource_ID) 
     {
     case RES_M_UPDATE_RESULT: 
          {
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER)
             {  
     
               ((firmware_data_t *)targetP->userData)->result =
                 lwm2m_data->instance_info->resource_info->value.asInteger;
     
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
     
               result = 1;
             } 
             else
             {
               result = 0;
             }
          }
          break;
     case RES_M_STATE: 
          {
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER)
             {
               ((firmware_data_t *)targetP->userData)->state = lwm2m_data->instance_info->resource_info->value.asInteger;
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID |
                 LWM2M_URI_FLAG_OBJECT_ID;
               result = 1;
             } 
             else 
             {
               result = 0;
             }
          }
          break;
     case RES_O_PKG_NAME:
          {
     
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING)
             {
               strlcpy(((firmware_data_t *)targetP->userData)->pkg_name, 
                   (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer, 
                   FIRMWARE_STR_LEN);
     
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID |LWM2M_URI_FLAG_OBJECT_ID;
               result = 1;
             }
             else
             {
               result = 0;
             }
         }
         break;
      case RES_O_PKG_VERSION:
         {
 
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING)
             {  
               strlcpy(((firmware_data_t *)targetP->userData)->pkg_version, 
                   (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer, 
                   FIRMWARE_STR_LEN);
     
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
               result = 1;
             }
             else
             {
               result = 0;
             }
           }
           break;
         case RES_M_UPDATE_METHOD:
           {
 
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER)
             {
               ((firmware_data_t *)targetP->userData)->delivery_method = lwm2m_data->instance_info->resource_info->value.asInteger;
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
               result = 1;
             }
             else
             {
               result = 0;
             }
           }
           break;
         case RES_O_UPDATE_PROTOCOL:
           {
             int protocol_bitmask = 0;
             bool coap_support, coaps_support, http_support, https_support;
     
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER)
             {
               protocol_bitmask = lwm2m_data->instance_info->resource_info->value.asInteger;
     
               coap_support  = protocol_bitmask & BITMASK(LWM2M_FIRMWARE_PROTOCOL_COAP)  ?TRUE:FALSE;
               coaps_support = protocol_bitmask & BITMASK(LWM2M_FIRMWARE_PROTOCOL_COAPS) ?TRUE:FALSE;
               http_support  = protocol_bitmask & BITMASK(LWM2M_FIRMWARE_PROTOCOL_HTTP)  ?TRUE:FALSE;
               https_support = protocol_bitmask & BITMASK(LWM2M_FIRMWARE_PROTOCOL_HTTPS) ?TRUE:FALSE;
     
               prv_change_protocol_support((firmware_data_t *)targetP->userData, LWM2M_FIRMWARE_PROTOCOL_COAP, coap_support);
               prv_change_protocol_support((firmware_data_t *)targetP->userData, LWM2M_FIRMWARE_PROTOCOL_COAPS, coaps_support);
               prv_change_protocol_support((firmware_data_t *)targetP->userData, LWM2M_FIRMWARE_PROTOCOL_HTTP, http_support);
               prv_change_protocol_support((firmware_data_t *)targetP->userData, LWM2M_FIRMWARE_PROTOCOL_HTTPS, https_support);
     
               result = 1;
             } 
             else 
             {
               result = 0;
             }
           }
           break;
         case RES_M_PACKAGE_URI:
           {
 
             if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING
                || lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_OPAQUE)
             {  
               strlcpy(((firmware_data_t *)targetP->userData)->pkg_url, 
                   (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer, 
                   MAX_FIRMWARE_URI_LEN);
     
               uriP->objectId = lwm2m_data->object_ID;
               uriP->instanceId = lwm2m_data->instance_info->instance_ID;
               uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
               uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID |LWM2M_URI_FLAG_OBJECT_ID;
               result = 1;
             }
             else
             {
               result = 0;
             }
           }
           break;
         default :
           result = 0;
           break;
       }
     
       if(result == 1)
       {
         store_firmware_persistent_info(firmwareInstance);  
       }
       return result;
     }

static uint8_t prv_firmware_write(uint16_t instanceId,
                                  int numData,
                                  lwm2m_data_t * dataArray,
                                  lwm2m_object_t * objectP)
 {
   int i = 0;
   uint8_t result = 0;
   lwm2m_object_data_t lwm2m_data;
   firmware_data_t *firmwareObj = NULL;
 
   uint8_t id_set = 0;
   bool ret_value = false;
   firmware_data_t *data = NULL;
 
   
   if(dataArray == NULL || objectP == NULL)
   {
     return COAP_500_INTERNAL_SERVER_ERROR;
   }
   data = (firmware_data_t *)(objectP->userData);
   firmwareObj = (firmware_data_t *)LWM2M_LIST_FIND(objectP->instanceList, 
                                                    instanceId);
 
   if(firmwareObj == NULL)
   {
     return COAP_404_NOT_FOUND;
   }
 
   memset(&lwm2m_data, 0, sizeof(lwm2m_object_data_t));
 
   // this is a single instance object
   if(instanceId != 0)
   {
     return COAP_404_NOT_FOUND;
   }
 
   i = 0;
 
   do
   {
     switch(dataArray[i].id)
     {
       case RES_M_PACKAGE:

         if(LWM2M_FIRWARE_STATE_DOWNLOADING != data->state &&
             LWM2M_FIRWARE_STATE_IDLE != data->state)
         {
           return COAP_405_METHOD_NOT_ALLOWED;
         }
         else
         {
             LOG_ARG("data->last_block_num=%d dataArray[i].value.asBuffer.block1_num=%d",data->last_block_num,dataArray[i].value.asBuffer.block1_num);
           if(data->last_block_num == dataArray[i].value.asBuffer.block1_num)
           {
             return COAP_NO_ERROR;
           }
 
           
           data->last_block_num = dataArray[i].value.asBuffer.block1_num;
           id_set = 0;
           id_set = id_set | LWM2M_OBJECT_ID_E;
           id_set = id_set | LWM2M_INSTANCE_ID_E;
           id_set = id_set | LWM2M_RESOURCE_ID_E;
 
           lwm2m_data.object_ID = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;
           lwm2m_data.no_instances++;
           lwm2m_data.instance_info = (lwm2m_instance_info_t *)lwm2m_malloc(sizeof(lwm2m_instance_info_t));
           
           if(NULL != lwm2m_data.instance_info)
           {
             memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
             lwm2m_data.instance_info->instance_ID = 0;
             lwm2m_data.instance_info->no_resources++;
             lwm2m_data.instance_info->resource_info =
               (lwm2m_resource_info_t *)lwm2m_malloc(
                   sizeof(lwm2m_resource_info_t));
             if(NULL != lwm2m_data.instance_info->resource_info)
             {
               memset(lwm2m_data.instance_info->resource_info, 0,
                   sizeof(lwm2m_resource_info_t));
               lwm2m_data.instance_info->resource_info->resource_ID =
                 RES_M_PACKAGE;
               lwm2m_data.instance_info->resource_info->type = (lwm2m_data_type_t)dataArray[i].type;
               lwm2m_data.instance_info->resource_info->value.asBuffer.length = dataArray[i].value.asBuffer.length;
               if(lwm2m_data.instance_info->resource_info->value.asBuffer.length)
               {
                 lwm2m_data.instance_info->resource_info->value.asBuffer.buffer =
                   lwm2m_malloc(lwm2m_data.instance_info->resource_info->value.asBuffer.length);
                 if(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer == NULL)
                 {
                   lwm2m_free_instance_info(lwm2m_data.instance_info);
                   return COAP_500_INTERNAL_SERVER_ERROR;
                 }
                 memset(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,
                     0, lwm2m_data.instance_info->resource_info->value.asBuffer.length);
 
                 memscpy(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,
                     lwm2m_data.instance_info->resource_info->value.asBuffer.length,
                     dataArray[i].value.asBuffer.buffer,
                     lwm2m_data.instance_info->resource_info->value.asBuffer.length);
                lwm2m_data.instance_info->resource_info->value.asBuffer.block1_more=
                dataArray[i].value.asBuffer.block1_more;
                lwm2m_data.instance_info->resource_info->value.asBuffer.block1_size=
                dataArray[i].value.asBuffer.block1_size;
                lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num=
                dataArray[i].value.asBuffer.block1_num;
                lwm2m_data.instance_info->resource_info->value.asBuffer.block1_offset=
                dataArray[i].value.asBuffer.block1_offset;
                lwm2m_data.instance_info->resource_info->value.asBuffer.size1=
                dataArray[i].value.asBuffer.size1;
                LOG_ARG("block1_more=%d block1_size=%d block1_num=%d block1_offset=%d",lwm2m_data.instance_info->resource_info->value.asBuffer.block1_more
                    ,lwm2m_data.instance_info->resource_info->value.asBuffer.block1_size,lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num,
                    lwm2m_data.instance_info->resource_info->value.asBuffer.size1
                    );
                if(lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num%50 == 0){
                    char inband_download[64]={0};
                    snprintf(inband_download, 64, "Inband download %d blocks!", lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num);
                    quec_lwm2m_client_info_notify(inband_download);
                }else if(lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num == 1){
                    char inband_download_start[64]={0};
                    snprintf(inband_download_start, 64, "Inband fota package download start!", lwm2m_data.instance_info->resource_info->value.asBuffer.block1_num);
                    quec_lwm2m_client_info_notify(inband_download_start);
                }
               }
             }
             else
             {
               lwm2m_free_instance_info(lwm2m_data.instance_info);
               return COAP_500_INTERNAL_SERVER_ERROR;
             }
           }
           else
           {
             return COAP_500_INTERNAL_SERVER_ERROR;
           }
           LOG("call lwm2m_check_if_observe_and_update_app");
           ret_value = lwm2m_check_if_observe_and_update_app (id_set, LWM2M_FIRMWARE_UPDATE_OBJECT_ID, 0, RES_M_PACKAGE, &lwm2m_data);
           lwm2m_free_instance_info(lwm2m_data.instance_info);
           LOG_ARG("call lwm2m_check_if_observe_and_update_app ret_value=%d",ret_value);
           if(ret_value == false)
           {
             data->state = LWM2M_FIRWARE_STATE_IDLE;
             result = COAP_400_BAD_REQUEST;
             LOG("result = COAP_400_BAD_REQUEST");
           }
           else
           {
             result = COAP_NO_ERROR;
             data->last_block_num = dataArray[i].value.asBuffer.block1_num;
             data->state = LWM2M_FIRWARE_STATE_DOWNLOADING;
             data->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
             firmwareInstance->state = LWM2M_FIRWARE_STATE_DOWNLOADING;
             firmwareInstance->updateExecuted = LWM2M_FALSE;
             is_firmware_update_downloading = LWM2M_FIRWARE_STATE_DOWNLOADING;
             lwm2m_indicate_firmware_upgrade_status("fotadownloading");
             LOG_ARG("call data->last_block_num=%d data->state=%d",data->last_block_num,data->state);
             if(dataArray[i].value.asBuffer.block1_more == LWM2M_FALSE)
             {
               if(dataArray[i].value.asBuffer.block1_offset == 0
                && dataArray[i].value.asBuffer.block1_size == 0
                && dataArray[i].value.asBuffer.block1_num == 0){
                   data->last_block_num = -1;
                   data->state = LWM2M_FIRWARE_STATE_IDLE;
                   data->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
                   firmwareInstance->state = LWM2M_FIRWARE_STATE_IDLE;
                   is_firmware_update_downloading = LWM2M_FIRWARE_STATE_IDLE;
                   firmwareInstance->updateExecuted = LWM2M_FALSE;
                   result = COAP_204_CHANGED;
                   lwm2m_indicate_firmware_upgrade_status("idle");
                   LOG("block1_more == -1 reset inband download state\n");

               }else{
                   data->last_block_num = -1;
                   data->state = LWM2M_FIRWARE_STATE_DOWNLOADED;
                   firmwareInstance->state = LWM2M_FIRWARE_STATE_DOWNLOADED;
                   is_firmware_update_downloading = LWM2M_FIRWARE_STATE_DOWNLOADED;
                   firmwareInstance->updateExecuted = LWM2M_FALSE;
                   data->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
                   result = COAP_204_CHANGED;
                   lwm2m_indicate_firmware_upgrade_status("fotadownloaded");
                   // This update would interrupt and influence FOTA progress , remove by joe 20220309
                   //if(lwm2m_check_is_vzw_netwrok()){
                   //    quec_queue_update_register_for_vzw_dm_server();
                   //}
                   LOG_ARG("block1_more == LWM2M_FALSE call data->last_block_num=%d data->state=%d",data->last_block_num,data->state);
               }

             }
             
           }
           store_firmware_persistent_info(firmwareInstance);//save update info before fota update procedure
           return result;
         }
 
         break;
 
       case RES_M_PACKAGE_URI:
         if(LWM2M_FIRWARE_STATE_IDLE != data->state)
         {
           if(data->state == LWM2M_FIRWARE_STATE_DOWNLOADING || data->state == LWM2M_FIRWARE_STATE_DOWNLOADED || data->state == LWM2M_FIRWARE_STATE_UPDATING){
               result = COAP_204_CHANGED;
           }else
           result = COAP_405_METHOD_NOT_ALLOWED;
         }
         else
         {
           id_set = 0;
           id_set = id_set | LWM2M_OBJECT_ID_E;
           id_set = id_set | LWM2M_INSTANCE_ID_E;
           id_set = id_set | LWM2M_RESOURCE_ID_E;
 
           lwm2m_data.object_ID = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;
           lwm2m_data.no_instances++;
           lwm2m_data.instance_info = (lwm2m_instance_info_t *)lwm2m_malloc(sizeof(lwm2m_instance_info_t));
           if(NULL != lwm2m_data.instance_info)
           {
             memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
             lwm2m_data.instance_info->instance_ID = 0;
             lwm2m_data.instance_info->no_resources = 1;
             lwm2m_data.instance_info->resource_info = (lwm2m_resource_info_t *)lwm2m_malloc(sizeof(lwm2m_resource_info_t));
             if(NULL != lwm2m_data.instance_info->resource_info)
             {
               memset(lwm2m_data.instance_info->resource_info, 0,
                   sizeof(lwm2m_resource_info_t));
               lwm2m_data.instance_info->resource_info->resource_ID = RES_M_PACKAGE_URI;
               lwm2m_data.instance_info->resource_info->type = (lwm2m_data_type_t)dataArray[i].type;
               lwm2m_data.instance_info->resource_info->value.asBuffer.length = dataArray[i].value.asBuffer.length;
               if(lwm2m_data.instance_info->resource_info->value.asBuffer.length)
               {
                 lwm2m_data.instance_info->resource_info->value.asBuffer.buffer =
                   lwm2m_malloc(lwm2m_data.instance_info->resource_info->value.asBuffer.length);
                 if(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer == NULL)
                 {
                    lwm2m_free_instance_info(lwm2m_data.instance_info);
                   return COAP_500_INTERNAL_SERVER_ERROR;
                 }
                 memset(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,
                     0, lwm2m_data.instance_info->resource_info->value.asBuffer.length);
                 
                 memscpy(lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,
                     lwm2m_data.instance_info->resource_info->value.asBuffer.length,
                     dataArray[i].value.asBuffer.buffer,
                     lwm2m_data.instance_info->resource_info->value.asBuffer.length);
                 
                 memset(data->pkg_url,0, MAX_FIRMWARE_URI_LEN);
                 memscpy(data->pkg_url,MAX_FIRMWARE_URI_LEN,
                     dataArray[i].value.asBuffer.buffer,
                     lwm2m_data.instance_info->resource_info->value.asBuffer.length);
               }
             }
             else
             {
                lwm2m_free_instance_info(lwm2m_data.instance_info);
               return COAP_500_INTERNAL_SERVER_ERROR;
             }
           }
           else
           {
             return COAP_500_INTERNAL_SERVER_ERROR;
           }
            is_firmware_update_downloading = LWM2M_FIRWARE_STATE_DOWNLOADING;
           data->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
           lwm2m_indicate_firmware_upgrade_status("fotadownloading");
           //data->state = LWM2M_FIRWARE_STATE_DOWNLOADING;
           ret_value = lwm2m_check_if_observe_and_update_app(id_set, 5, 0, RES_M_PACKAGE_URI, &lwm2m_data);
           lwm2m_free_instance_info(lwm2m_data.instance_info);

           if(ret_value == FALSE)
           {
               firmwareInstance->updateExecuted = LWM2M_FALSE;
               firmwareInstance->state = LWM2M_FIRWARE_STATE_IDLE;
               result = COAP_400_BAD_REQUEST;
               is_firmware_update_downloading =LWM2M_FIRWARE_STATE_IDLE;
               lwm2m_indicate_firmware_upgrade_status("idle");
           }
           else
           {
               firmwareInstance->updateExecuted = LWM2M_FALSE;
               //firmwareInstance->state = LWM2M_FIRWARE_STATE_DOWNLOADED;
               //is_firmware_update_downloading =false;
               if(lwm2m_data.instance_info->resource_info->value.asBuffer.length == 0){
                   LOG("lwm2m_data.instance_info->resource_info->value.asBuffer.length == 0 reset fota outband state\n");
                   firmwareInstance->state = LWM2M_FIRWARE_STATE_IDLE;
                   is_firmware_update_downloading =LWM2M_FIRWARE_STATE_IDLE;
                   data->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
                   data->state = LWM2M_FIRWARE_STATE_IDLE;
                   lwm2m_indicate_firmware_upgrade_status("idle");
               }
               result = COAP_204_CHANGED;
           }
           store_firmware_persistent_info(firmwareInstance);//save update info before fota update procedure
         }
         break;
       case RES_M_UPDATE:
       case RES_M_STATE:
       case RES_M_UPDATE_RESULT:
       case RES_O_PKG_NAME:
       case RES_O_PKG_VERSION:
       case RES_O_UPDATE_PROTOCOL:
       case RES_M_UPDATE_METHOD:
         /*If create/write have one more one resource to write we write on reablable resource*/
         if(numData > 1)
           result = COAP_204_CHANGED;
         else
           result = COAP_405_METHOD_NOT_ALLOWED;
       default:
         /*If create/write have one more one resource to write we write on unknown/notsupported resource*/
         if(numData > 1)
           result = COAP_204_CHANGED;
         else
           result = COAP_404_NOT_FOUND;
     }
 
     i++;
   } while(i < numData && result == COAP_204_CHANGED);
 
   return result;
 }


static uint8_t prv_firmware_execute(uint16_t instanceId,
                                    uint16_t resourceId,
                                    uint8_t * buffer,
                                    int length,
                                    lwm2m_object_t * objectP)
{
  firmware_data_t *data = NULL;
  bool ret_value = false;
  lwm2m_object_data_t lwm2m_data;
  uint8_t id_set = 0;
  
  if(objectP == NULL)
  {
      return COAP_500_INTERNAL_SERVER_ERROR;
  }
  
  data = (firmware_data_t *)(objectP->userData);
  
  // this is a single instance object
  if((instanceId != 0) || (data == NULL))
  {
      return COAP_404_NOT_FOUND;
  }
  
  // for execute callback, resId is always set.
  switch(resourceId)
  {
  case RES_M_UPDATE:
      if(data->state == LWM2M_FIRWARE_STATE_DOWNLOADED)
        {
          // trigger your firmware update logic

          firmwareInstance->updateExecuted = LWM2M_TRUE;
          
          data->state = LWM2M_FIRWARE_STATE_UPDATING;
          is_firmware_update_downloading = LWM2M_FIRWARE_STATE_UPDATING;
          store_firmware_persistent_info(firmwareInstance);//save update info before fota update procedure
          updateExecuted = LWM2M_TRUE;
  
          memset(&lwm2m_data, 0, sizeof(lwm2m_object_data_t));
          id_set = 0;
          id_set = id_set | LWM2M_OBJECT_ID_E;
          id_set = id_set | LWM2M_INSTANCE_ID_E;
          id_set = id_set | LWM2M_RESOURCE_ID_E;
  
          lwm2m_data.object_ID = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;
          lwm2m_data.no_instances++;
          lwm2m_data.instance_info = (lwm2m_instance_info_t *)lwm2m_malloc(sizeof(lwm2m_instance_info_t));
          if(NULL != lwm2m_data.instance_info)
          {
            memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
            lwm2m_data.instance_info->instance_ID = 0;
            lwm2m_data.instance_info->no_resources++;
            lwm2m_data.instance_info->resource_info = (lwm2m_resource_info_t *)lwm2m_malloc(sizeof(lwm2m_resource_info_t));
            if(NULL != lwm2m_data.instance_info->resource_info)
            {
              memset(lwm2m_data.instance_info->resource_info, 0,
                  sizeof(lwm2m_resource_info_t));
              lwm2m_data.instance_info->resource_info->resource_ID = RES_M_UPDATE;
            }
          }
          
          ret_value = lwm2m_check_if_observe_and_update_app(id_set, 5, 0, RES_M_UPDATE, &lwm2m_data);
          lwm2m_free_instance_info(lwm2m_data.instance_info);
          if(ret_value == false)
          {
            lwm2m_indicate_firmware_upgrade_status("idle");
            return COAP_500_INTERNAL_SERVER_ERROR;
          }
          else
          {
            lwm2m_indicate_firmware_upgrade_status("fotaupgrading");
            return COAP_204_CHANGED;
          }
        }
        else
        {
          // firmware update already running
          return COAP_405_METHOD_NOT_ALLOWED;
        }
  case RES_M_PACKAGE:
  case RES_M_PACKAGE_URI:
  case RES_M_STATE:
  case RES_M_UPDATE_RESULT:
  case RES_O_PKG_NAME:
  case RES_O_PKG_VERSION:
  case RES_O_UPDATE_PROTOCOL:
  case RES_M_UPDATE_METHOD:
       return COAP_405_METHOD_NOT_ALLOWED;
  default:
       return COAP_404_NOT_FOUND;
 }
}


/**
 * @fn static uint8_t prv_firmware_discover()
 *
 * @brief This fucntion implements the discover operation performed on the
 *     firmware object
 *
 * @param instanceId Instance ID of the object to be read
 * @param numDataP number of resources in the array
 * @param dataArrayP resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 *
 */
static uint8_t prv_firmware_discover(uint16_t instanceId,
    int * numDataP,
    lwm2m_data_t ** dataArrayP,
    lwm2m_object_t * objectP)
{
  uint8_t result = 0;
  int i = 0;
  int ri = 0;
  resource_instance_int_list_t* protocolList = NULL;
  firmware_data_t * data = NULL;

  if (numDataP == NULL || dataArrayP == NULL ) 
  {
    return  COAP_400_BAD_REQUEST;
  }
  // this is a single instance object
  if (instanceId != 0)
  {
    return COAP_404_NOT_FOUND;
  }

  result = COAP_205_CONTENT;
  data = (firmware_data_t*)(objectP->userData);
  for(protocolList = data->protocol_support, ri = 0; protocolList != NULL;
      protocolList = protocolList->next, ri++);

  // is the server asking for the full object ?
  if (*numDataP == 0)
  {
    uint16_t resList[] = {

      RES_M_PACKAGE  ,
      RES_M_PACKAGE_URI ,               
      RES_M_UPDATE ,
      RES_M_STATE  ,
      RES_M_UPDATE_RESULT ,
      RES_O_PKG_NAME ,
      RES_O_PKG_VERSION ,
      RES_O_UPDATE_PROTOCOL,
      RES_M_UPDATE_METHOD
    };
    int nbRes = sizeof(resList) / sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL) 
        return COAP_500_INTERNAL_SERVER_ERROR;
    *numDataP = nbRes;
    for (i = 0; i < nbRes; i++)
    {
      (*dataArrayP)[i].id = resList[i];
      if((*dataArrayP)[i].id == RES_O_UPDATE_PROTOCOL )
      {
        (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
        (*dataArrayP)[i].value.asChildren.count = ri;
      }
    }
  }
  else
  {
    for(i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
    {
      switch((*dataArrayP)[i].id)
      {
        case RES_O_UPDATE_PROTOCOL:
        {
              (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
              (*dataArrayP)[i].value.asChildren.count = ri;
              break;
        }
        case RES_M_PACKAGE:
        case RES_M_PACKAGE_URI:
        case RES_M_UPDATE:
        case RES_M_STATE:
        case RES_M_UPDATE_RESULT:
        case RES_O_PKG_NAME:
        case RES_O_PKG_VERSION:
        case RES_M_UPDATE_METHOD:
          break;
        default:
          result = COAP_404_NOT_FOUND;
      }
    }
  }

  return result;
}


void display_firmware_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    firmware_data_t * data = (firmware_data_t *)object->userData;
    fprintf(stdout, "  /%u: Firmware object:\r\n", object->objID);
    if (NULL != data)
    {
        fprintf(stdout, "    state: %u, result: %u\r\n", data->state,
                data->result);
    }
#endif
}


/**
 * @fn static uint8_t prv_firmware_create()
 *
 * @brief This fucntion is used to create Firmware Object
 *
 * @param instanceId Instance ID of the object to be created
 * @param numData number of resources in the array
 * @param dataArray resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_firmware_create(void *ctx_p, uint16_t instanceId,
    int numData,
    lwm2m_data_t * dataArray,
    lwm2m_object_t * objectP)
{
  firmware_data_t *firmwareObj = NULL;
  uint8_t result = 0;

  if( objectP == NULL ) 
  {
     return COAP_500_INTERNAL_SERVER_ERROR;
  }

  /*check if already one instance is created */
  if(objectP->instanceList != NULL)
  {
    return COAP_400_BAD_REQUEST;
  }

  firmwareObj = (firmware_data_t*) lwm2m_malloc(sizeof(firmware_data_t));
  if( NULL == firmwareObj)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  //TODO: Do  I need to call load_firmware_persistent_info() function?
  firmwareObj->state      = LWM2M_FIRWARE_STATE_IDLE;
  firmwareObj->supported  = false;
  firmwareObj->result     = LWM2M_FIRWARE_UPDATE_DEFAULT;

  firmwareObj->instanceId = instanceId;
  objectP->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
  if (NULL != objectP->instanceList)
  {
    memset(objectP->instanceList, 0x00, sizeof(lwm2m_list_t));
  }
  objectP->userData = firmwareObj;
  firmwareInstance = objectP->userData;

  /* We instansiated the FW object */
  if(dataArray == NULL)
  {
    goto end;
  }

  result = prv_firmware_write(instanceId, numData, dataArray, objectP);
  if (result != COAP_204_CHANGED)
  {
    // Issue in creating object instanace so retuning Bad request error.
    result = COAP_400_BAD_REQUEST;
    lwm2m_free(objectP->instanceList);
    objectP->instanceList = NULL;
    lwm2m_free(firmwareObj);
    objectP->userData = NULL;
    return result;
  }

end:
  unlink(LWM2M_FIRMWARE_OBJ_PERSISTENCE_PATH);
  return COAP_201_CREATED;
}


/**
 * @fn static int32_t prv_firmware_getvalue
 * @brief   This function is used to set the particular resource value from application
 * @param   lwm2m_data - pointer to lwm2m object information
           targetP    - pointer to lwm2m object 
 * @return  on success - 1
           on error   - 0      
 */

static int32_t prv_firmware_getvalue(void *ctx_p, lwm2m_id_info_t id_info, 
                                      lwm2m_object_data_t * lwm2m_dataP , 
                                      lwm2m_object_t * targetP)
{
  firmware_data_t *instanceP = NULL;
  int8_t result = -1;
  lwm2m_data_t * dataArrayP = NULL;
  lwm2m_data_t * dataArray  = NULL;
  lwm2m_resource_info_t *resource_info = NULL;
  lwm2m_resource_info_t *resource_infoP = NULL;
  int i = 0;
  int numData = 0;

  if (lwm2m_dataP == NULL || targetP == NULL)
  {
    return result;
  }

  if (id_info.id_set & LWM2M_RESOURCE_ID_E)
  {
    numData = 1;

    resource_info = lwm2m_dataP->instance_info->resource_info;
    if (resource_info == NULL)
    {
      return result;
    }

    dataArrayP = lwm2m_data_new(numData);
    if (dataArrayP == NULL)
      return result;    
    
    dataArray = dataArrayP;
    dataArrayP->id = resource_info->resource_ID;
  }
  else if (id_info.id_set & LWM2M_INSTANCE_ID_E)
  {

    numData = 0;

    if (lwm2m_dataP->instance_info == NULL)
    {
      return result;
    }

    resource_info = (lwm2m_resource_info_t *)lwm2m_malloc(sizeof(lwm2m_resource_info_t)); 
    if (resource_info == NULL)
    {
      return result;
    } 
    
    memset(resource_info, 0x00, sizeof(lwm2m_resource_info_t));
    lwm2m_dataP->instance_info->resource_info = resource_info;
  }

  else
  {
    // need to handle
    return result;
  }

  instanceP = (firmware_data_t *)targetP->userData;
  if (NULL == instanceP)
    return result;
  
  if (numData == 0)
  {
    uint16_t resList[] = {
      RES_M_PACKAGE_URI,
      RES_M_STATE  ,
      RES_M_UPDATE_RESULT ,
      RES_O_PKG_NAME ,
      RES_O_PKG_VERSION ,
      RES_M_UPDATE_METHOD
    };    

    int nbRes = sizeof(resList)/sizeof(uint16_t);

    dataArrayP = lwm2m_data_new(nbRes);
    if (dataArrayP == NULL)
      return result;
    
    dataArray = dataArrayP;
    numData = nbRes;

    for (i = 0; i < nbRes; i++)
      (dataArrayP)[i].id = resList[i];
  }

  i = 0;
  do
  {
    result = prv_set_value((dataArrayP) + i, instanceP);
    i++;
  } while (i < numData && result == COAP_205_CONTENT);

  if (result != COAP_205_CONTENT)
  {
    result = -1;
    goto GET_ERROR;
  }

  for (i = 0 ; i < numData ; i++)
  {
    resource_info->resource_ID = dataArrayP->id;
    resource_info->type = (lwm2m_data_type_t)dataArrayP->type;

    switch (dataArrayP->type)
    {
      case LWM2M_TYPE_STRING:
        resource_info->value.asBuffer.buffer = dataArrayP->value.asBuffer.buffer;
        resource_info->value.asBuffer.length = dataArrayP->value.asBuffer.length;
        break;

      case LWM2M_TYPE_INTEGER:
        resource_info->value.asInteger = dataArrayP->value.asInteger;
        break;

      case LWM2M_TYPE_BOOLEAN:
        resource_info->value.asBoolean = dataArrayP->value.asBoolean;
        break;

      default:
        break;
    }
    
    resource_infoP = resource_info;

    /* Allocation for one of the resource node is done prior to entering the loop. Skip the last allocation. */
    if (i < numData - 1)
    {
      resource_info = (lwm2m_resource_info_t *)lwm2m_malloc (sizeof (lwm2m_resource_info_t));
      if (resource_info == NULL)
      {
        result = -1;

        goto GET_ERROR;
      }

      memset(resource_info, 0x00, sizeof(lwm2m_resource_info_t)); 
    }

    resource_infoP->next = resource_info;
    dataArrayP = dataArrayP + 1;     
  }

  resource_infoP->next = NULL;
  lwm2m_dataP->instance_info->no_resources = numData;
  result = 0;

GET_ERROR:
  /* Resources allocated for lwm2m_dataP are cleaned-up by the caller */
  if (dataArray)
    lwm2m_free(dataArray);
  
  return result;
}


/**
 * @fn static uint8_t prv_firmware_delete()
 *
 * @brief This fucntion is used to delete Firmware object
 *
 * @param id Instance ID of the object to be deleted
 * @param objectP Object to be deleted
 *
 * @return LWM2M response type
 */
static uint8_t prv_firmware_delete(void *ctx_p, uint16_t id,
    lwm2m_object_t * objectP)
{
  int fd = 0;
  firmware_data_t * firmwareObj = NULL;

  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }


  objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&firmwareObj);
  if (NULL == firmwareObj) 
    return COAP_404_NOT_FOUND;

  lwm2m_free(firmwareObj);
  firmwareObj = NULL;

  /*  Update single instance object persistence */

  fd = open( LWM2M_FIRMWARE_OBJ_PERSISTENCE_PATH, O_CREAT, 00744);
  if( EFAILURE == fd)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  close(fd);

  is_firmware_create = TRUE;
  free_object_firmware(objectP);
  return COAP_202_DELETED;

}

lwm2m_object_t * get_object_firmware(void)
{
    /*
     * The get_object_firmware function create the object itself and return a pointer to the structure that represent it.
     */
    lwm2m_object_t * firmwareObj;
    int fd = 0;
    LOG("get_object_firmware enter");
    firmwareObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != firmwareObj)
    {
        memset(firmwareObj, 0, sizeof(lwm2m_object_t));
        /*
         * It assigns its unique ID
         * The 5 is the standard ID for the optional object "Object firmware".
         */
        firmwareObj->objID = LWM2M_FIRMWARE_UPDATE_OBJECT_ID;


        /*
         * And the private function that will access the object.
         * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
         * know the resources of the object, only the server does.
         */
        firmwareObj->discoverFunc= prv_firmware_discover;
        firmwareObj->readFunc    = prv_firmware_read;
        firmwareObj->writeFunc   = prv_firmware_write;
        firmwareObj->executeFunc = prv_firmware_execute;

        /*  Check if firmware_obj_single_instance_pr file exist */
        fd = open(LWM2M_FIRMWARE_OBJ_PERSISTENCE_PATH, O_RDONLY);
        if(fd != EFAILURE)
        {
          close(fd);
          return firmwareObj;
        }
        /*
         * and its unique instance
         *
         */
        firmwareObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != firmwareObj->instanceList)
        {
            memset(firmwareObj->instanceList, 0, sizeof(lwm2m_list_t));
        }
        else
        {
            lwm2m_free(firmwareObj);
            return NULL;
        }
        firmwareObj->userData    = lwm2m_malloc(sizeof(firmware_data_t));
        /*
         * Also some user data can be stored in the object with a private structure containing the needed variables
         */
        if (NULL != firmwareObj->userData)
        {
            memset(firmwareObj->userData, 0, sizeof(firmware_data_t));
    		firmwareInstance = firmwareObj->userData;

            
            //check firmware version change
            if(quec_check_firmware_version_changed(GetSoftwareVersion())){
                set_fw_update_state_successful();
            }
            quec_load_fw_update_execute_flag();

             ((firmware_data_t*)firmwareObj->userData)->last_block_num        = -1;
    		 ((firmware_data_t*)firmwareObj->userData)->state		= LWM2M_FIRWARE_STATE_IDLE;
    		 ((firmware_data_t*)firmwareObj->userData)->supported	= TRUE;
             if(firmware_update_state_successful == true && is_firmware_update_execute == true){
                 ((firmware_data_t*)firmwareObj->userData)->result  = LWM2M_FIRWARE_UPDATE_SUCCESSFUL;
                 quec_save_fw_update_execute_flag(false);
                 char* fota_update_success = "Firmware upgrade successful! Current verison:";
                 quec_lwm2m_client_info_notify(fota_update_success);
                 quec_lwm2m_client_info_notify(GetSoftwareVersion());
                 lwm2m_indicate_firmware_upgrade_status("fotaupgradesucceful");
                 is_firmware_update_result_reported = false;
             }else if(firmware_update_state_successful == false && is_firmware_update_execute == true){
                 ((firmware_data_t*)firmwareObj->userData)->result  = LWM2M_FIRMWARE_UPDATE_FAILED;
                 quec_save_fw_update_execute_flag(false);
                  char* fota_update_fail = "Firmware upgrade failed!";
                 quec_lwm2m_client_info_notify(fota_update_fail);
                 lwm2m_indicate_firmware_upgrade_status("fotaupgradefailed");
                 is_firmware_update_result_reported = false;
             }else{
                 ((firmware_data_t*)firmwareObj->userData)->result  = LWM2M_FIRWARE_UPDATE_DEFAULT;
                 lwm2m_indicate_firmware_upgrade_status("idle");
             }
             remove(QUEC_LWM2M_FOTA_UPDATE_FILE);
             LOG_ARG("firmwareObj userData result=%d",((firmware_data_t*)firmwareObj->userData)->result);
             #if 0
    		 if(load_firmware_persistent_info(firmwareInstance) == LWM2M_FALSE)
    		 {
    		     memset(firmwareInstance->pkg_name, 0, FIRMWARE_STR_LEN);
    			 strcpy(firmwareInstance->pkg_name, QUEC_LWM2M_FOTA_UPDATE_FILE);
    			 memset(firmwareInstance->pkg_version, 0, FIRMWARE_STR_LEN);
                 char* firmware_version = GetSoftwareVersion();
                 strcpy(firmwareInstance->pkg_version, firmware_version);
    			 memset(firmwareInstance->pkg_url, 0, MAX_FIRMWARE_URI_LEN);
    			 firmwareInstance->state = LWM2M_FIRWARE_STATE_IDLE;
    			 firmwareInstance->result = LWM2M_FIRWARE_UPDATE_DEFAULT;
    			 firmwareInstance->delivery_method = 2;
    			// prv_change_protocol_support(firmwareInstance, LWM2M_FIRMWARE_PROTOCOL_COAP, TRUE);
    			// prv_change_protocol_support(firmwareInstance, LWM2M_FIRMWARE_PROTOCOL_COAPS, TRUE);
    			// prv_change_protocol_support(firmwareInstance, LWM2M_FIRMWARE_PROTOCOL_HTTP, TRUE);
    			// prv_change_protocol_support(firmwareInstance, LWM2M_FIRMWARE_PROTOCOL_HTTPS, TRUE);
                 LOG("get_object_firmware enter14");
    		 }
             #endif 
             if((lwm2m_check_is_att_netwrok())){
                 ((firmware_data_t*)firmwareObj->userData)->delivery_method = 0;//only outband
             }else{
                 ((firmware_data_t*)firmwareObj->userData)->delivery_method = 2; //both inband and outband
             }
              prv_change_protocol_support((firmware_data_t*)firmwareObj->userData, LWM2M_FIRMWARE_PROTOCOL_COAP, false);
              prv_change_protocol_support((firmware_data_t*)firmwareObj->userData, LWM2M_FIRMWARE_PROTOCOL_COAPS, false);
              prv_change_protocol_support((firmware_data_t*)firmwareObj->userData, LWM2M_FIRMWARE_PROTOCOL_HTTP, TRUE);
              prv_change_protocol_support((firmware_data_t*)firmwareObj->userData, LWM2M_FIRMWARE_PROTOCOL_HTTPS, TRUE);

    		 firmwareInstance->last_block_num = -1;
             #if 0
    		 if(firmwareInstance->state == LWM2M_FIRWARE_STATE_UPDATING){
    			  int fd;
    			  fd = open("/data/lwm2m/fota_upgrade_result", O_RDONLY);
    		 	  if(fd < 0){
    			  	 firmwareInstance->result = LWM2M_FIRMWARE_UPDATE_FAILED;
    		 	  }else{
    				 char code[10] = {0};
    				 int code_val = -1;
    				 read(fd, code, 10);

    				 close(fd);
    				 code_val = atoi(code);
    				 if(code_val == 200)
    				 	 firmwareInstance->result = LWM2M_FIRWARE_UPDATE_SUCCESSFUL;
    				 else if(code_val == 400)
    				 	 firmwareInstance->result = LWM2M_FIRMWARE_UPDATE_FAILED;
    				 else
    				 	 firmwareInstance->result = LWM2M_FIRWARE_UPDATE_CRC_FAILED;
    			  }
    			 
    		      firmwareInstance->state = LWM2M_FIRWARE_STATE_IDLE;
    			  unlink(FIRMWARE_PERSISTENCE_FILE);
    		 }
             #endif
    	}
        else
        {
            lwm2m_free(firmwareObj);
            firmwareObj = NULL;
        }
    }

    return firmwareObj;
}
	
/**
 * @fn uint8_t firmware_change()
 *
 * @brief This fucntion is used to populate the values of the resource
 *		  to be provided to the server during the read operation on the object.
 * @param dataArray data array for the resource 
 * @param objectP handle to device object 
 * @return LWM2M response type
 */
uint8_t firmware_change(lwm2m_data_t * dataArray,	lwm2m_object_t * objectP)
{
	uint8_t result = COAP_NO_ERROR;
	if( dataArray == NULL || objectP == NULL ) 
	{
		return COAP_500_INTERNAL_SERVER_ERROR;
	}
    LOG("firmware_change enter\n");
	switch (dataArray->id)
	{
	case RES_M_STATE:
		  {
			int64_t value = 0;
			if (1 == lwm2m_data_decode_int(dataArray, &value))
			{
			  if (LWM2M_FIRWARE_STATE_IDLE <= value && LWM2M_FIRWARE_STATE_UPDATING >= value)
			  {
                  LOG("firmware_change value\n");
				((firmware_data_t*)objectP->userData)->state = value;
				result = COAP_204_CHANGED;
			  }
			  else
			  {
				result = COAP_400_BAD_REQUEST;
			  }
			}
			else
			{
			  result = COAP_400_BAD_REQUEST;
			}
		  }
		  break;
	
	case RES_M_UPDATE_RESULT:
		  {
			int64_t value = 0;
			if (1 == lwm2m_data_decode_int(dataArray, &value))
			{
			  if (LWM2M_FIRWARE_UPDATE_DEFAULT <= value && LWM2M_FIRMWARE_UPDATE_FAILED >= value)
			  {
				((firmware_data_t*)objectP->userData)->result = value;
                LOG("firmware_change COAP_204_CHANGED");
				result = COAP_204_CHANGED;
			  }
			  else
			  {
                  LOG("firmware_change COAP_400_BAD_REQUEST");
				result = COAP_400_BAD_REQUEST;
			  }
			}
			else
			{
                LOG("firmware_change COAP_400_BAD_REQUEST");
			  result = COAP_400_BAD_REQUEST;
			}
		  }
		  break;
	
		default:
            LOG("firmware_change COAP_405_METHOD_NOT_ALLOWED");
		  result = COAP_405_METHOD_NOT_ALLOWED;
		  break;
  }
	  return result;
}

int get_fw_update_download_state(){

    return is_firmware_update_downloading;
}

void set_fw_update_download_state(int state){
    is_firmware_update_downloading = state;
}

void set_fw_update_state_successful(){

    firmware_update_state_successful = true;
}

bool get_fw_update_state_successful(){

   return firmware_update_state_successful;
}

bool get_fw_update_result_reported(){

   return is_firmware_update_result_reported;
}

void quec_load_fw_update_execute_flag(){
    char *filename = QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE;
    FILE *fp;
    fp = fopen(filename,"rb");
    char *execute_flag=NULL;
    
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",filename);
        perror("fopen: ");
        return ;
    }
    
    execute_flag = lwm2m_malloc(sizeof(char)*64);
    memset(execute_flag, 0, 64);
    fread(execute_flag, sizeof(char)*64, 1, fp);
    LOG_ARG("load_get execute_flag=%s",execute_flag);
    if(execute_flag == NULL){
        is_firmware_update_execute = false;
        return;
    }
    if(strstr(execute_flag,"true") != 0){
        is_firmware_update_execute = true;

    }else {
        is_firmware_update_execute = false;
    }
    
    LOG_ARG("load_get execute_flag=%d",is_firmware_update_execute);
    fclose(fp);
    lwm2m_free(execute_flag);
}

void quec_save_fw_update_execute_flag(bool state){
    FILE *fp;
    int i;
    char* execute_flag_state = "false";
    char *filename = QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE;
    LOG_ARG("quec_save_fw_update_execute_flag enter state=%d",state);

    if (access(filename, F_OK) == 0)
    {
        LOG("if QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE exist, remove it");
		remove(QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE);
    }

    fp = fopen(QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE,"wb");
    if(fp == NULL)
    {
        LOG("file QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE error: \n");
        perror("fopen: ");
        return -1;
    }
    if(state){
        execute_flag_state = "true";
    }else{
        execute_flag_state = "false";
    }
    fwrite(execute_flag_state, strlen(execute_flag_state), 1, fp);
    fclose(fp);
    LOG("quec_save_fw_update_execute_flag exit");

}

char * get_fw_dload_uri(lwm2m_object_t * objectP)
{
	firmware_data_t *user_data  = (firmware_data_t *)(objectP->userData);
	
    if (NULL != user_data)
    {
        return lwm2m_strdup(user_data->pkg_url);
    }

    return NULL;
}

void free_object_firmware(lwm2m_object_t * objectP)
{
    if (NULL != objectP->userData)
    {
        lwm2m_free(objectP->userData);
        objectP->userData = NULL;
    }
    if (NULL != objectP->instanceList)
    {
        lwm2m_free(objectP->instanceList);
        objectP->instanceList = NULL;
    }
    lwm2m_free(objectP);
}

void lwm2m_indicate_firmware_upgrade_status(char* status){
    char cmd[64] = {0};
    sprintf(cmd, "echo %s > /data/lwm2m/fotastatus",status);
    LOG_ARG("lwm2m chagng fota status to: %s",status);
    system(cmd);
}

