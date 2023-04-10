/******************************************************************************

  @file    object_host_device.c
  @brief   File contains the implementation for Host device object
  ---------------------------------------------------------------------------

  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

 *****************************************************************************/
#include "liblwm2m.h"
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lwm2m_configure.h"

/*
0: Host Device Unique ID = HUID0
1: Host Device Manufacturer = HMAN0
2: Host Device Model = HMOD0
3: Host Device Software Version = HSW0
*/  
/*
#define RES_M_HOST_DEV_MANUFACTURER     5905//0
#define RES_M_HOST_DEV_MODEL            5906//1
#define RES_M_HOST_DEV_ID               5907//2
#define RES_M_HOST_DEV_SWV              5908//3
*/
#define RES_M_HOST_DEV               0

#define TEMP_LWM2M_MALLOC_SIZE          30   //Temporory Array size
#define PRV_EMPTY                       " "
typedef struct object_host_dev_
{
  struct object_host_dev_ *next;
  uint16_t instanceId;
  uint8_t  host_dev_manufacturer[128];
  uint8_t  host_dev_model[128];
  uint8_t  host_dev_id[128];
  uint8_t  host_dev_swv[128];
}object_host_dev_t;


void free_object_host_device(lwm2m_object_t * object);
/**
 * @fn static uint8_t prv_res2tlv()
 *
 * @brief This fucntion is used to populate the values of the resource
 *        to be provided to the server during the read operation on the object.
 * @param dataP data array for the resource 
 * @param host_dev_dataP Host Device object data
 * @return LWM2M response type
 */
static uint8_t prv_res2tlv(lwm2m_data_t* dataP,
    object_host_dev_t* host_dev_dataP)
{
  uint8_t ret = COAP_205_CONTENT; 
  if( dataP == NULL || host_dev_dataP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  switch (dataP->id)     
  {
  /*
    case RES_M_HOST_DEV_MANUFACTURER:
      lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_manufacturer, dataP);
      break;
    case RES_M_HOST_DEV_MODEL:
      lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_model, dataP);
      break;
    case RES_M_HOST_DEV_ID:
      lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_id, dataP);
      break;
	case RES_M_HOST_DEV_SWV:
	  lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_swv, dataP);
      break;
  */
    case RES_M_HOST_DEV:
    	{
    		lwm2m_data_t * subTlvP = NULL;
			subTlvP = lwm2m_data_new(4);
			subTlvP[0].id = 0;
            lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_id, &subTlvP[0]);
			subTlvP[1].id = 1;
            lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_manufacturer, &subTlvP[1]);
			subTlvP[2].id = 2;
            lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_model, &subTlvP[2]);
			subTlvP[3].id = 3;
            lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_swv, &subTlvP[3]);
			lwm2m_data_encode_instances(subTlvP, 4, dataP);
    	}
		break;
    default:
      ret = COAP_404_NOT_FOUND;
      break;
  }

  return ret;
}

/**
 * @fn static uint8_t prv_host_dev_read()
 *
 * @brief This function implements the read operation performed on Host
 * Device object
 *
 * @param objInstId Object ID of the object to be read
 * @param numDataP number of resources in the array
 * @param tlvArrayP resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_host_dev_read
(
uint16_t objInstId,
int*  numDataP,
lwm2m_data_t** tlvArrayP,
lwm2m_object_t*  objectP,
uint16_t resInstId
)
{
  int     i = 0;
  uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
  object_host_dev_t* host_dev_dataP = NULL;
  if( tlvArrayP == NULL || objectP == NULL || numDataP == NULL) 
  {
    return  COAP_400_BAD_REQUEST;
  }

  host_dev_dataP = (object_host_dev_t*) lwm2m_list_find(objectP->instanceList, objInstId);

  if (host_dev_dataP == NULL) 
  {
    return COAP_404_NOT_FOUND;
  }
  if (*numDataP == 0)     // full object, readable resources!
  {
/*
0: Host Device Unique ID = HUID0
1: Host Device Manufacturer = HMAN0
2: Host Device Model = HMOD0
3: Host Device Software Version = HSW0
*/  
    uint16_t readResIds[] = {
    /*
	  RES_M_HOST_DEV_ID,
      RES_M_HOST_DEV_MANUFACTURER,
      RES_M_HOST_DEV_MODEL,      
      RES_M_HOST_DEV_SWV
   */
      RES_M_HOST_DEV
    }; // readable resources!

    *numDataP  = sizeof(readResIds)/sizeof(uint16_t);
    *tlvArrayP = lwm2m_data_new(*numDataP);
    if (*tlvArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;

    // init readable resource id's
    for (i = 0 ; i < *numDataP ; i++)
    {
      (*tlvArrayP)[i].id = readResIds[i];
    }
  }

  for (i = 0 ; i < *numDataP ; i++)
  {
    result = prv_res2tlv ((*tlvArrayP)+i, host_dev_dataP);
    if (result != COAP_205_CONTENT) 
		break;
  }
  return result;
}

/**
 * @fn static uint8_t prv_host_dev_discover()
 *
 * @brief This fucntion implements the discover operation performed on the
 *     Host Device object
 *
 * @param instanceId Instance ID of the object to be read
 * @param numDataP number of resources in the array
 * @param dataArrayP resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 *
 */
static uint8_t prv_host_dev_discover
(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t ** dataArrayP,
lwm2m_object_t * objectP
)
{
  uint8_t result = COAP_205_CONTENT;
  int i = 0;
  if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  if(*numDataP == 0)
  {
    uint16_t resList[] = {
	  /*
      RES_M_HOST_DEV_MANUFACTURER,
      RES_M_HOST_DEV_MODEL,
      RES_M_HOST_DEV_ID,
      RES_M_HOST_DEV_SWV
      */
      RES_M_HOST_DEV
     };
    int nbRes = sizeof(resList) / sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;
    *numDataP = nbRes;
    for (i = 0; i < nbRes; i++)
    {
      (*dataArrayP)[i].id = resList[i];
	  if(resList[i] == RES_M_HOST_DEV)
	  {
	  	(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
		(*dataArrayP)[i].value.asChildren.count = 4;
	  }
    }
  }
  else
  {
    for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
    {
      switch ((*dataArrayP)[i].id)
      {
      	/*
        case RES_M_HOST_DEV_MANUFACTURER:
        case RES_M_HOST_DEV_MODEL:
        case RES_M_HOST_DEV_ID:
		case RES_M_HOST_DEV_SWV:
        */
        case RES_M_HOST_DEV:
        	{
			  (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
			  (*dataArrayP)[i].value.asChildren.count = 4;
        	}
          break;
        default:
          result = COAP_404_NOT_FOUND;
      }
    }
  }

  return result;                  
}

/**
 * @fn static uint8_t prv_host_dev_create()
 *
 * @brief This fucntion is used to create Host Device object
 *
 * @param instanceId Instance ID of the object to be created
 * @param numData number of resources in the array
 * @param dataArray resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_host_dev_create(uint16_t instanceId,
    int numData,
    lwm2m_data_t * dataArray,
    lwm2m_object_t * objectP)
{
  object_host_dev_t *host_dev = NULL;
  lwm2m_hostdevice_info_t  hostdev_info;

		

  if(objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  if(instanceId > 1){
	 return COAP_400_BAD_REQUEST;
  }

  host_dev = (object_host_dev_t*) lwm2m_malloc(sizeof(object_host_dev_t));
  if( NULL == host_dev)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  if(lwm2m_load_hostdevice_info(instanceId, &hostdev_info) == false){
			snprintf((char *)hostdev_info.manufacture, 128,"HMAN%d",instanceId);
			snprintf((char *)hostdev_info.model, 128,"HMOD%d", instanceId);
			snprintf((char *)hostdev_info.unique_id, 128,"HUID%d", instanceId);
			snprintf((char *)hostdev_info.sw_version, 128,"HSW%d", instanceId);
 }
  memset(host_dev, 0, sizeof(object_host_dev_t));
  
  host_dev->instanceId = instanceId;

  memset(host_dev->host_dev_manufacturer,0, 128);
  strcpy((char *)host_dev->host_dev_manufacturer, hostdev_info.manufacture);

  memset(host_dev->host_dev_model,0, 128);
  strcpy((char *)host_dev->host_dev_model, hostdev_info.model);

  memset(host_dev->host_dev_id, 0, 128);
  strcpy((char *)host_dev->host_dev_id, hostdev_info.unique_id);

  memset(host_dev->host_dev_swv, 0,128);
  strcpy((char *)host_dev->host_dev_swv, hostdev_info.sw_version);
 
  objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, host_dev);

  return COAP_201_CREATED;
}

/**
 * @fn static uint8_t prv_host_dev_delete()
 *
 * @brief This fucntion is used to delete Host Device object
 *
 * @param id Instance ID of the object to be deleted
 * @param objectP Object to be deleted
 *
 * @return LWM2M response type
 */
static uint8_t prv_host_dev_delete(uint16_t id,
    lwm2m_object_t * objectP)
{
  object_host_dev_t * host_dev = NULL;

  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&host_dev);
  if (NULL == host_dev) 
  {
    return COAP_404_NOT_FOUND;
  }

  lwm2m_free (host_dev);
  return COAP_202_DELETED;

}
static int32_t prv_host_dev_setvalue
(
void *ctx_p,
lwm2m_object_data_t * lwm2m_data ,
lwm2m_object_t * targetP,
lwm2m_uri_t *uriP
)
{
	int result = 0;
	object_host_dev_t *obj_host_dev = NULL;

	if(lwm2m_data == NULL || targetP == NULL || lwm2m_data->instance_info == NULL || lwm2m_data->instance_info->resource_info == NULL) 
	{
		return 0;
	}
	obj_host_dev = (object_host_dev_t *)LWM2M_LIST_FIND(targetP->instanceList, lwm2m_data->instance_info->instance_ID);
	
	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  obj_host_dev=%p", obj_host_dev);
	if(obj_host_dev == NULL) 
		return 0;
	switch(lwm2m_data->instance_info->resource_info->resource_ID) 
	{
	case RES_M_HOST_DEV:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_MULTIPLE_RESOURCE
			   && lwm2m_data->instance_info->resource_info->value.asChildren.count == 4){
				memset(obj_host_dev->host_dev_id,0, 128);
			    lwm2m_data_decode_string(&(lwm2m_data->instance_info->resource_info->value.asChildren.array[0]), (char *)(obj_host_dev->host_dev_id), 128);
				memset(obj_host_dev->host_dev_manufacturer,0, 128);
			    lwm2m_data_decode_string(&(lwm2m_data->instance_info->resource_info->value.asChildren.array[1]), (char *)(obj_host_dev->host_dev_manufacturer), 128);
				memset(obj_host_dev->host_dev_model,0, 128);
			    lwm2m_data_decode_string(&(lwm2m_data->instance_info->resource_info->value.asChildren.array[2]), (char *)(obj_host_dev->host_dev_model), 128);
				memset(obj_host_dev->host_dev_swv,0, 128);
			    lwm2m_data_decode_string(&(lwm2m_data->instance_info->resource_info->value.asChildren.array[3]), (char *)(obj_host_dev->host_dev_swv), 128);

				uriP->objectId = lwm2m_data->object_ID;
			    uriP->instanceId = lwm2m_data->instance_info->instance_ID;
			    uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
			    //uriP->flag = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID |LWM2M_URI_FLAG_OBJECT_ID;
				result = 1;
				//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  host_dev_id=%s", obj_host_dev->host_dev_id);
			}
            break;
		}
	default:
		return 0;
	}
	return result;
}
/**
 * @fn lwm2m_object_t * get_object_host_dev()
 *
 * @brief This function is used to get the information regarding the Host
 *        Device object during client registartion
 *
 * @param void
 *
 * @return Host Device object  
 *
 */
lwm2m_object_t * get_object_host_dev(void)
{
  lwm2m_object_t * host_dev_obj = NULL;
  object_host_dev_t *host_dev_data = NULL;
  int instance_id = 0;

  host_dev_obj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
  if(NULL != host_dev_obj)
  {
    memset(host_dev_obj, 0, sizeof(lwm2m_object_t));

    // It assigns its unique ID
    // The 10255 is the standard ID for the optional object "Host Device".

    host_dev_obj->objID = LWM2M_HOST_DEVICE_OBJECT_ID;

    // And the private function that will access the object.
    // Those function will be called when a read query is made by the server.
    // In fact the library don't need to know the resources of the object, only the server does.

    host_dev_obj->readFunc    = prv_host_dev_read;
    host_dev_obj->discoverFunc= prv_host_dev_discover;
    host_dev_obj->createFunc  = prv_host_dev_create;
    host_dev_obj->deleteFunc  = prv_host_dev_delete;
	host_dev_obj->setValueFunc = prv_host_dev_setvalue;


	while(instance_id < 2){
		lwm2m_hostdevice_info_t  hostdev_info;

		memset(&hostdev_info, 0, sizeof(lwm2m_hostdevice_info_t));
		if(lwm2m_load_hostdevice_info(instance_id, &hostdev_info) == false){
			snprintf((char *)hostdev_info.manufacture, 128,"HMAN%d",instance_id);
			snprintf((char *)hostdev_info.model, 128,"HMOD%d", instance_id);
			snprintf((char *)hostdev_info.unique_id, 128,"HUID%d", instance_id);
			snprintf((char *)hostdev_info.sw_version, 128,"HSW%d", instance_id);
		}
	    host_dev_data = (object_host_dev_t*)lwm2m_malloc(sizeof(object_host_dev_t));

	    if (NULL != host_dev_data)
	    {
	      memset (host_dev_data, 0 , sizeof (object_host_dev_t));  
	      memset(host_dev_data->host_dev_manufacturer,0, 128);
		  strcpy((char *)host_dev_data->host_dev_manufacturer, hostdev_info.manufacture);

		  memset(host_dev_data->host_dev_model,0, 128);
          strcpy((char *)host_dev_data->host_dev_model, hostdev_info.model);

		  memset(host_dev_data->host_dev_id, 0, 128);
		  strcpy((char *)host_dev_data->host_dev_id, hostdev_info.unique_id);

		  memset(host_dev_data->host_dev_swv, 0,128);
		  strcpy((char *)host_dev_data->host_dev_swv, hostdev_info.sw_version);
		  host_dev_data->instanceId = instance_id;
		  
	      host_dev_obj->instanceList = LWM2M_LIST_ADD(host_dev_obj->instanceList, host_dev_data); 
	    }

	    host_dev_obj->userData = NULL;
		instance_id++;
	}
  }
  return host_dev_obj;
}

/**
 * @fn void free_object_host_device()
 *
 * @brief This fucntion frees the Host Device object allocated
 * @param ObjectP Object to free
 *
 * @return void
 */
void free_object_host_device(lwm2m_object_t * object)
{
  object_host_dev_t *instanceP =  NULL;
  if( object == NULL)  
  {
    return ;
  }

  if (NULL != object->userData)
  {
    lwm2m_free(object->userData);
    object->userData = NULL;
  }
  
  instanceP = (object_host_dev_t *)object->instanceList;

  while(NULL != instanceP)
  {
    object->instanceList = (lwm2m_list_t *)instanceP->next;
    lwm2m_free(instanceP);
    instanceP = (object_host_dev_t *)object->instanceList;
  }

  lwm2m_free(object);
}
