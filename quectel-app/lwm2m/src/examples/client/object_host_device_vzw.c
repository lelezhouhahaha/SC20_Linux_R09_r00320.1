#include "liblwm2m.h"
	
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "object_host_device_vzw.h"
#include "lwm2m_configure.h"
#include "quectel_getinfo.h"
#include "quectel_info_util.h"

#define PRV_DEVICE_TYPE      "MODULE"
#define PRV_HARDWARE_VERSION  "R1.0"

void free_vzw_object_host_device(lwm2m_object_t * object);
/**
 * @fn static uint8_t prv_res2tlv()
 *
 * @brief This fucntion is used to populate the values of the resource
 *        to be provided to the server during the read operation on the object.
 * @param dataP data array for the resource 
 * @param host_dev_dataP Host Device object data
 * @return LWM2M response type
 */
static uint8_t prv_vzw_res2tlv(lwm2m_data_t* dataP,
    object_host_dev_t* host_dev_dataP)
{
  uint8_t ret = COAP_205_CONTENT;
  if( dataP == NULL || host_dev_dataP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  
#ifndef LWM2M_SUPPORT_QLRIL_API
  ql_module_about_info_s about;
  QL_LWM2M_GetAbout(&about);
#endif
  switch (dataP->id)     
  {
    case RES_M_HOST_DEV_MANUFACTURER:
    {
      //lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_manufacturer, dataP);
#ifdef LWM2M_SUPPORT_QLRIL_API
    lwm2m_data_encode_string(QL_LWM2M_GetManufacture(), dataP);
#else
    lwm2m_data_encode_string(about.manufacturer, dataP);
#endif

    }  
      break;
    case RES_M_HOST_DEV_MODEL:
    {
#ifdef LWM2M_SUPPORT_QLRIL_API
        lwm2m_data_encode_string(QL_LWM2M_GetProductname(), dataP);
#else
       lwm2m_data_encode_string(about.product_name, dataP);
#endif
    }
      //lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_model, dataP);
      break;
    case RES_M_HOST_DEV_ID:
        {
      //lwm2m_data_encode_string((const char *)host_dev_dataP->host_dev_id, dataP);
        char serialnumber[64] = {0};
        QL_LWM2M_GetImei(&serialnumber);
        lwm2m_data_encode_string(serialnumber, dataP);
        }
      break;
	case RES_M_HOST_DEV_FW_VER:
        #ifdef LWM2M_SUPPORT_QLRIL_API
        lwm2m_data_encode_string(GetSoftwareVersion(), dataP);
        #else
        lwm2m_data_encode_string(about.firmware_version, dataP);
        #endif        

	  break;
	case RES_M_HOST_DEV_SW_VER:
	  //lwm2m_data_encode_string((const char *)host_dev_dataP->sw_ver, dataP);
    {   
        char version[64] = {0};
        char *svn = GetSoftwareVersion();
        if(strlen(svn) == 0){
            get_sw_version(&version);
        }else{
            strcpy(version,svn);
        }
        lwm2m_data_encode_string(version, dataP);
    }    
	  break;
	case RES_M_HOST_DEV_HW_VER:
        lwm2m_data_encode_string(PRV_HARDWARE_VERSION, dataP);
	  break;	
	case RES_M_HOST_DEV_UPDATE_TM:
    {   
      host_dev_dataP->upgrade_time = lwm2m_gettime()+GPS_TO_UNIX_TIME_OFFSET;
	  lwm2m_data_encode_int(host_dev_dataP->upgrade_time, dataP);
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
static uint8_t prv_vzw_host_dev_read(uint16_t objInstId,
                                 int*  numDataP,
                                 lwm2m_data_t** tlvArrayP,
                                 lwm2m_object_t*  objectP)
{
  int     i = 0;
  uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
  object_host_dev_t* host_dev_dataP = NULL;
 // if( tlvArrayP == NULL || objectP == NULL || numDataP == NULL) 
  //{
  //  return COAP_400_BAD_REQUEST;
 // }

  host_dev_dataP = (object_host_dev_t*) lwm2m_list_find(objectP->instanceList, objInstId);

  if (host_dev_dataP == NULL) 
  {
    return COAP_404_NOT_FOUND;
  }
  if (*numDataP == 0)     // full object, readable resources!
  {
    uint16_t readResIds[] = {
      RES_M_HOST_DEV_MANUFACTURER,
      RES_M_HOST_DEV_MODEL,
      RES_M_HOST_DEV_ID
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
    result = prv_vzw_res2tlv ((*tlvArrayP)+i, host_dev_dataP);
    if (result!= COAP_205_CONTENT) 
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
static uint8_t prv_vzw_host_dev_discover(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t ** dataArrayP,
lwm2m_object_t * objectP
)
{
  uint8_t result = COAP_205_CONTENT;
  int i = 0;
  //if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
  //{
  //  return COAP_500_INTERNAL_SERVER_ERROR;
  //}

  if(*numDataP == 0)
  {
    uint16_t resList[] = {
      RES_M_HOST_DEV_MANUFACTURER,
      RES_M_HOST_DEV_MODEL,
      RES_M_HOST_DEV_ID       
    };
    int nbRes = sizeof(resList) / sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;
    *numDataP = nbRes;
    for (i = 0; i < nbRes; i++)
    {
      (*dataArrayP)[i].id = resList[i];
    }
  }
  else
  {
    for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
    {
      switch ((*dataArrayP)[i].id)
      {
        case RES_M_HOST_DEV_MANUFACTURER:
        case RES_M_HOST_DEV_MODEL:
        case RES_M_HOST_DEV_ID:
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
static uint8_t prv_vzw_host_dev_create(uint16_t instanceId,
                                         int numData,
                                         lwm2m_data_t * dataArray,
                                         lwm2m_object_t * objectP)
{
  object_host_dev_t *host_dev_data = NULL;
   lwm2m_hostdevice_info_t  hostdev_info;

  if( dataArray == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  if(instanceId > 1){
	 return COAP_400_BAD_REQUEST;
  }

  host_dev_data = (object_host_dev_t*) lwm2m_malloc(sizeof(object_host_dev_t));
  if( NULL == host_dev_data)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  host_dev_data->instanceId = instanceId;
 
  memset(&hostdev_info, 0, sizeof(lwm2m_hostdevice_info_t));
  memset (host_dev_data, 0 , sizeof (object_host_dev_t));
#if 0//add by joe
  if(lwm2m_load_hostdevice_info(instanceId, &hostdev_info) == FALSE){
	  memset(host_dev_data->host_dev_manufacturer, 0, 128);
  	  lwm2m_get_device_manufacturer_id((char *)host_dev_data->host_dev_manufacturer);

	  memset(host_dev_data->host_dev_model, 0, 128);
	  lwm2m_get_device_model_id((char *)host_dev_data->host_dev_model);

	  memset(host_dev_data->host_dev_id, 0, 128);
	  lwm2m_get_device_serial_no((char *)host_dev_data->host_dev_id ,128);

	  memset(host_dev_data->sw_ver, 0, 128);
	  lwm2m_get_device_sw_ver(host_dev_data->sw_ver);

	  memset(host_dev_data->fw_ver, 0, 128);
	  lwm2m_get_device_sw_ver(host_dev_data->fw_ver);

	  memset(host_dev_data->hw_ver, 0, 128);
	  lwm2m_get_device_hw_ver(host_dev_data->hw_ver);

	  host_dev_data->upgrade_time = lwm2m_gettime()+GPS_TO_UNIX_TIME_OFFSET;
   }else
#endif //add by joe
   {
  	  memset(host_dev_data->host_dev_manufacturer, 0, 128);
  	  memcpy(host_dev_data->host_dev_manufacturer, hostdev_info.manufacture, strlen(hostdev_info.manufacture));

	  memset(host_dev_data->host_dev_model, 0, 128);
	  memcpy(host_dev_data->host_dev_model, hostdev_info.model, strlen(hostdev_info.model));

	  memset(host_dev_data->host_dev_id, 0, 128);
	  memcpy(host_dev_data->host_dev_id, hostdev_info.unique_id, strlen(hostdev_info.unique_id));

	  memset(host_dev_data->sw_ver, 0, 128);
	  memcpy(host_dev_data->sw_ver, hostdev_info.sw_version, strlen(hostdev_info.sw_version));

	  memset(host_dev_data->fw_ver, 0, 128);
	  memcpy(host_dev_data->fw_ver, hostdev_info.fw_version, strlen(hostdev_info.fw_version));

	  memset(host_dev_data->hw_ver, 0, 128);
	  memcpy(host_dev_data->hw_ver, hostdev_info.hw_version, strlen(hostdev_info.hw_version));
			   
	  host_dev_data->upgrade_time = hostdev_info.upgrade_time+GPS_TO_UNIX_TIME_OFFSET;    
  }

  objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, host_dev_data);

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
static uint8_t prv_vzw_host_dev_delete(void *ctx_p,
	uint16_t id,
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
static int32_t prv_vzw_host_dev_setvalue
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
	case RES_M_HOST_DEV_MANUFACTURER:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->host_dev_manufacturer, 0, 128);
				memcpy(obj_host_dev->host_dev_manufacturer, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_MODEL:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->host_dev_model, 0, 128);
				memcpy(obj_host_dev->host_dev_model, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_ID:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->host_dev_id, 0, 128);
				memcpy(obj_host_dev->host_dev_id, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_FW_VER:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->fw_ver, 0, 128);
				memcpy(obj_host_dev->fw_ver, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_SW_VER:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->sw_ver, 0, 128);
				memcpy(obj_host_dev->sw_ver, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_HW_VER:
		{
			if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING){
				memset(obj_host_dev->hw_ver, 0, 128);
				memcpy(obj_host_dev->hw_ver, lwm2m_data->instance_info->resource_info->value.asBuffer.buffer,
				       lwm2m_data->instance_info->resource_info->value.asBuffer.length);
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
	case RES_M_HOST_DEV_UPDATE_TM:
		{
			 if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				obj_host_dev->upgrade_time = lwm2m_data->instance_info->resource_info->value.asInteger;
				uriP->objectId	 = lwm2m_data->object_ID;
				uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				result = 1;
		    }
		}
		break;
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
lwm2m_object_t * get_vzw_object_host_dev(void)
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

    host_dev_obj->objID = LWM2M_VZW_HOST_DEVICE_OBJECT_ID;

    // And the private function that will access the object.
    // Those function will be called when a read query is made by the server.
    // In fact the library don't need to know the resources of the object, only the server does.

    host_dev_obj->readFunc    = prv_vzw_host_dev_read;
    host_dev_obj->discoverFunc= prv_vzw_host_dev_discover;
    host_dev_obj->createFunc  = prv_vzw_host_dev_create;
	host_dev_obj->setValueFunc = prv_vzw_host_dev_setvalue;
    host_dev_obj->deleteFunc  = prv_vzw_host_dev_delete; 

    host_dev_data = (object_host_dev_t*)lwm2m_malloc(sizeof(object_host_dev_t));

    if (NULL != host_dev_data)
    {
    	 lwm2m_hostdevice_info_t  hostdev_info;

		  memset(&hostdev_info, 0, sizeof(lwm2m_hostdevice_info_t));
		  memset (host_dev_data, 0 , sizeof (object_host_dev_t));
          #if 0 //add by joe
	      if(lwm2m_load_hostdevice_info(instance_id, &hostdev_info) == FALSE){
			  /** fetch the device manufacturer */
			  memset(host_dev_data->host_dev_manufacturer, 0, 128);
		  	  lwm2m_get_device_manufacturer_id((char *)host_dev_data->host_dev_manufacturer);
		      /** Fetch the model number of the device */
			  memset(host_dev_data->host_dev_model, 0, 128);
			  lwm2m_get_device_model_id((char *)host_dev_data->host_dev_model);

		      /** Fetch the device id of the device */
			  memset(host_dev_data->host_dev_id, 0, 128);
			  lwm2m_get_device_serial_no((char *)host_dev_data->host_dev_id ,128);

			  memset(host_dev_data->sw_ver, 0, 128);
			  lwm2m_get_device_sw_ver(host_dev_data->sw_ver);

			  memset(host_dev_data->fw_ver, 0, 128);
			  lwm2m_get_device_sw_ver(host_dev_data->fw_ver);

			  memset(host_dev_data->hw_ver, 0, 128);
			  lwm2m_get_device_hw_ver(host_dev_data->hw_ver);

			  host_dev_data->upgrade_time = lwm2m_gettime()+GPS_TO_UNIX_TIME_OFFSET;
		  }else
          #endif //add by joe
          {
		  	  memset(host_dev_data->host_dev_manufacturer, 0, 128);
		  	  memcpy(host_dev_data->host_dev_manufacturer, hostdev_info.manufacture, strlen(hostdev_info.manufacture));

			  memset(host_dev_data->host_dev_model, 0, 128);
			  memcpy(host_dev_data->host_dev_model, hostdev_info.model, strlen(hostdev_info.model));

			  memset(host_dev_data->host_dev_id, 0, 128);
			  memcpy(host_dev_data->host_dev_id, hostdev_info.unique_id, strlen(hostdev_info.unique_id));

			  memset(host_dev_data->sw_ver, 0, 128);
			  memcpy(host_dev_data->sw_ver, hostdev_info.sw_version, strlen(hostdev_info.sw_version));

			  memset(host_dev_data->fw_ver, 0, 128);
			  memcpy(host_dev_data->fw_ver, hostdev_info.fw_version, strlen(hostdev_info.fw_version));

			  memset(host_dev_data->hw_ver, 0, 128);
			  memcpy(host_dev_data->hw_ver, hostdev_info.hw_version, strlen(hostdev_info.hw_version));
			   
			  host_dev_data->upgrade_time = hostdev_info.upgrade_time+GPS_TO_UNIX_TIME_OFFSET;    
		  }
	   
      host_dev_data->instanceId = instance_id;

      host_dev_obj->instanceList = LWM2M_LIST_ADD(host_dev_obj->instanceList, host_dev_data); 

    }

    host_dev_obj->userData = NULL;
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
void free_vzw_object_host_device(lwm2m_object_t * object)
{
  object_host_dev_t *instanceP =  NULL;
  if( object == NULL)  
  {
    return ;
  }

  if (NULL != object->instanceList)
  {
    lwm2m_free(object->instanceList);
    object->instanceList = NULL;
  }

  lwm2m_free(object);
}

