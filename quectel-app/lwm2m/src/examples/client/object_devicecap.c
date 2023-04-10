/******************************************************************************

  @file    object_devicecap.c
  @brief   File contains the implementation for the operations related to object 
           Device capability management
  ---------------------------------------------------------------------------

  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

 *****************************************************************************/

/*
 *  Object                         |      | Multiple  |     | Description                   |
 *  Name                           |  ID  | Instances |Mand.|                               |
 *---------------------------------+------+-----------+-----+-------------------------------+
 *  Device Capability Management   |  15  |    Yes    |  No |                               |
 *
 *  Resources:
 *  Name        | ID  | Oper.|Instances|Mand.|  Type   | Range | Units | Description                                                                      |
 * -------------+-----+------+---------+-----+---------+-------+-------+----------------------------------------------------------------------------------+
 *  Property    |  0  |  R   | Single  | Yes | String  |       |       | List of Device Capabilites inside a given group. The format is a free                                                                          list ASCII - represented integers seperated by semi-colon e.g - 0;1;10
 *  Group       |  1  |  R   | Single  | Yes | Integer |       |       | Group name of device Capabilites 0-SENSOR, 1-CONTROL, 2-CONNECTIVITY,                                                                          3-NAVIGATION, 4-STORAGE, 5-VISION, 6-SOUND, 7-ANALOG_INPUT, 
 8-ANALOG_OUTPUT.
 *  Description |  2  |  R   | Single  | No  | String  |       |       | Device capability description.                                        
 *  Attached    |  3  |  R   | Single  | No  | Boolean |       |       | When the resource doesn’t exist, it means the associated Device 
 Capability is not removable. When this resource is “False”, it means 
 the associated Device Capability is removable and is currently not 
 attached to the device. When this resource is “True”, it means the 
 associated Device Capability – if removable – is currently attached 
 to the Device. When a Device Capability is not removable, and the 
 “Attached” Resource is present, the “Attached” value but be set 
 to “True”.                                                            
 *  Enabled     |  4  |  R   | Single  | Yes | Boolean |       |       | This resource indicates whether the Device Capability is enabled 
 regardless whether the Device Capability is attached or not. If the 
 value of this resource is “True” the Device Capability is in Enabled 
 State. If the value is “False” the Device Capability is in Disabled 
 State; The ‘Attached’ property is independent of ‘Enabled’ property. 
 A Device Capability MAY have ‘True’ as value for ‘Enabled’ node 
 while having ‘False’ as value for the ‘Attached’ node. That means the 
 Device Capability is still not available and can’t be used until it 
 is attached to the Device, but will be useable once the Device 
 Capability is attached.                                               
 *  opEnable   |  5  |  E   | Single  | Yes |          |       |       | This command is used to enable the Device Capability to transfer 
 the  Device Capability from Disabled State to Enabled state. 
 In Enabled State, the Device Capability is allowed to work when it 
 is attached to the Device.
 * opDisable   |  6  |  E   | Single  | Yes |          |       |       | This command is used to disable the Device Capability to transfer 
 the Device Capability from Enabled State to Disabled State. 
 In Disabled state the Device Capability is not allowed to work.  
 * NotifyEn    |  7  |  RW  | Single  | No  | Boolean  |       |       | This resource specifies whether a LWM2M Server may be notified when 
 the “Attached” Resource is under “Observation” and the value is 
 changing. If the resource is not present or the value is ‘False’, 
 the LWM2M Server will be not notified about this change. If this 
 Resource  is present and the value is ‘True’,the LWM2M Server will 
 */

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "internals.h"

#define ENABLE_DEVCAP_OBJ
#ifdef ENABLE_DEVCAP_OBJ

#define PRV_PROPERTY  "0;1;10" /**< Default value for resource "property". */
#define PRV_GROUP   0 /**< Default value for resource "group". */
#define PRV_DESCRIPTION "Device Capability" /**< Default value for resource
                                              "description". */ 
#define PRV_ATTACHED_TRUE 1 /**< True value for resource "attached". */ 
#define PRV_ATTACHED_FALSE  0 /**< False value for resource "attached". */  
#define PRV_ENABLED_TRUE    1 /**< True value for resource "enabled". */
#define PRV_ENABLED_FALSE   0 /**< False value for resource "enabled". */
#define PRV_NOTIFY_EN_TRUE    1 /**< True value for resource "Notify En".*/
#define PRV_NOTIFY_EN_FALSE   0 /**< False value for resource "Notify En".*/

#define RES_M_PROPERTY    0 /**< Resource ID for resource "property". */
#define RES_M_GROUP     1 /**< Resource ID for resource "group". */
#define RES_O_DESCRIPTION 2 /**< Resource ID for resource "description". */
#define RES_O_ATTACHED    3 /**< Resource ID for resource "attached". */
#define RES_M_ENABLED   4 /**< Resource ID for resource "enabled". */
#define RES_M_OP_ENABLE   5 /**< Resource ID for resource "op enable". */
#define RES_M_OP_DISBALE  6 /**< Resource ID for resource "op disable". */
#define RES_O_NOTIFY_EN   7 /**< Resource ID for resource "notify en". */

#define PRV_GROUP_SENSOR    11 /**< Group category Sensor. */
#define PRV_GROUP_CONTROL   12 /**< Group category Control. */
#define PRV_GROUP_CONNECTIVITY  13 /**< Group category Connectivity. */
#define PRV_GROUP_NAVIGATION  14 /**< Group category Navigation. */
#define PRV_GROUP_STORAGE   15 /**< Group category Storage. */
#define PRV_GROUP_VISION    16 /**< Group category Vision. */
#define PRV_GROUP_SOUND     17 /**< Group category Sound. */
#define PRV_GROUP_ANALOG_INPUT  18 /**< Group category Analog input. */
#define PRV_GROUP_ANALOG_OUTPUT 19 /**< Group category Analog output. */

#define PRV_PROPERTY_MAXLEN   128 /**< "property" resource length. */
#define PRV_DESCRIPTION_MAXLEN  255 /**< "description" resource length. */

/**
 * @brief Describes the Device capability management object resources
 *
 */
typedef struct _device_cap_manangement_t
{
  struct _device_cap_manangement_t *next;  /**< corresponding to lwm2m_list_t::next */
  uint16_t    instanceId;                 /**< corresponding to lwm2m_list_t::id  */
  char property[PRV_PROPERTY_MAXLEN];  /**< Device capability list (Mandatory) */
  int8_t group; /**< Group name of device capability (Mandatory) */
  char description[PRV_DESCRIPTION_MAXLEN]; /**< Device capability
                                              description (Optional) */
  bool attached; /**< Device capability is removeable or not (optional) */
  bool enabled; /**< Device capability is enabled or not (Mandatory) */
  bool notifyen; /**< notification is enabled or nor (optional) */
  bool openable;
  bool opdisable;
} device_cap_manangement_t;


/**
 * @brief This function is used to write the resources into object instance
 * while creating the object instance. Note: not to be called to perform write
 * operation
 * @param   instanceP (INOUT)   handle to object instance
 * @param   numData (IN)        number of resources to be written
 * @param   dataArray (IN)      array of resource values
 * @return  LWM2M_NO_ERROR on success
 */
static uint8_t prv_devicecap_write_resources(device_cap_manangement_t *instanceP,
    int numData,
    lwm2m_data_t *dataArray)
{
  int i = 0;
  if( instanceP == NULL  || dataArray == NULL )
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  for (i = 0; i < numData; i++)
  {
    switch((dataArray + i)->id)
    {
      case RES_M_PROPERTY:
        strlcpy(instanceP->property, (const char *)((dataArray + i)->value.asBuffer.buffer),
            ((dataArray + i)->value.asBuffer.length) + 1);
        break;

      case RES_M_GROUP:
        {
          int64_t value = 0;
          if (1 != lwm2m_data_decode_int(dataArray+i, (int64_t*)&value))
          {
            return COAP_400_BAD_REQUEST;
          }
          instanceP->group= (int8_t)value;
        }
        break;

      case RES_M_ENABLED:
        if(1 != lwm2m_data_decode_bool(dataArray+i, &instanceP->enabled))
        {
          return COAP_400_BAD_REQUEST;
        }
        break;

      case RES_O_DESCRIPTION:
        {
          strlcpy(instanceP->description, (const char *)((dataArray + i)->value.asBuffer.buffer),
              ((dataArray + i)->value.asBuffer.length) + 1);
        }
        break;

      case RES_O_ATTACHED:
        if(1 != lwm2m_data_decode_bool(dataArray+i, &instanceP->attached))
        {
          return COAP_400_BAD_REQUEST;
        }
        break;

      case RES_O_NOTIFY_EN:
        if(1 != lwm2m_data_decode_bool(dataArray+i, &instanceP->notifyen))
        {
          return COAP_400_BAD_REQUEST;
        }
        break;

      default:
        //Ignoring any optional resource
        break;
    }
  }
  return COAP_NO_ERROR;
}

/**
 * @fn static uint8_t prv_devicecap_create()
 * @brief   This function implements the create operation performed
 * on the Device Capability object
 * @param   instanceId (IN) Instance ID of the object instance to be created
 * @param   numData (IN)    number of resources in the array i.e. dataArray
 * @param   dataArray (IN)  resource value array
 * @param   objectP (IN)    Object whose instance is to be created
 * return   COAP response type
 */
static uint8_t prv_devicecap_create        (uint16_t instanceId,
                                         int numData,
                                         lwm2m_data_t * dataArray,
                                         lwm2m_object_t * objectP)

{
  device_cap_manangement_t *instanceP = NULL;
  uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;

  //if( dataArray == NULL || objectP == NULL ) 
  //{
  //  return COAP_500_INTERNAL_SERVER_ERROR;
  //}

  instanceP = (device_cap_manangement_t *)lwm2m_malloc(sizeof(device_cap_manangement_t));

  if (instanceP != NULL)
  {
    memset(instanceP, 0, sizeof(device_cap_manangement_t));
    instanceP->instanceId = instanceId;
    result = prv_devicecap_write_resources(instanceP, numData, dataArray);

    if (result == COAP_NO_ERROR)
    {
      objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, instanceP);
      result = COAP_201_CREATED;
    }
    else
    {
      lwm2m_free(instanceP);
      // Issue in creating object instanace so retuning Bad request error.
      result = COAP_400_BAD_REQUEST;
    }
  }
  return result;

}

/**
 * @fn static uint8_t prv_set_value()
 * @brief This fucntion is used to populate the values of the resource
 * to be provided to the server during the read operation on the object.
 * @param dataP (OUT) data array for the resource 
 * @param devDataP (IN) device capability object data
 * @return LWM2M response type
 */
static uint8_t prv_set_value(lwm2m_data_t * dataP,
    device_cap_manangement_t * devDataP)
{
  if( dataP == NULL || devDataP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  } 
  LOG_ARG("dataP->id=%d",dataP->id);

  switch (dataP->id)
  {
    case RES_M_PROPERTY:
      lwm2m_data_encode_string(devDataP->property, dataP);
      return COAP_205_CONTENT;

    case RES_M_GROUP:
      lwm2m_data_encode_int(devDataP->group, dataP);
      return COAP_205_CONTENT;

    case RES_O_DESCRIPTION:
      lwm2m_data_encode_string(devDataP->description, dataP);
      return COAP_205_CONTENT;

    case RES_O_ATTACHED:
        lwm2m_data_encode_bool(devDataP->attached, dataP);
        return COAP_205_CONTENT;

    case RES_M_ENABLED:
        lwm2m_data_encode_bool(devDataP->enabled, dataP);
        return COAP_205_CONTENT;

    case RES_O_NOTIFY_EN:
        lwm2m_data_encode_bool(devDataP->notifyen, dataP);
        return COAP_205_CONTENT;

    case RES_M_OP_DISBALE://add by joe
        lwm2m_data_encode_bool(devDataP->opdisable, dataP);
        return COAP_205_CONTENT;
    
    case RES_M_OP_ENABLE://add by joe
        lwm2m_data_encode_bool(devDataP->openable, dataP);

        return COAP_205_CONTENT;
    default:
      return COAP_404_NOT_FOUND;
  }
}

/**
 * @brief This function implements the read operation performed on device
 * capability management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */ 
static uint8_t prv_devicecap_read        (uint16_t instanceId,
                                       int * numDataP,
                                       lwm2m_data_t ** dataArrayP,
                                       lwm2m_object_t * objectP)

{
  uint8_t result = COAP_404_NOT_FOUND;
  int i = 0;
  device_cap_manangement_t *instanceP = NULL;
  LOG_ARG("read enter instanceId=%d",instanceId);

  // this is a single instance object
  if (instanceId != 0)
  {
      return COAP_404_NOT_FOUND;
  }


  instanceP = (device_cap_manangement_t *)LWM2M_LIST_FIND(objectP->instanceList, 
      instanceId);

  //MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] instanceP=%p", instanceP);		  
  if (NULL == instanceP)
  {
    return result;
  }

  if(*numDataP == 0)
  {
    uint16_t resList[] = {
      RES_M_PROPERTY,
      RES_M_GROUP,
      RES_O_DESCRIPTION,
      RES_O_ATTACHED,
      RES_M_ENABLED,
      RES_O_NOTIFY_EN
    };
    if(lwm2m_check_is_vzw_netwrok()){
        uint16_t resList[] = {
          RES_M_PROPERTY,
          RES_M_GROUP,
          RES_O_DESCRIPTION,
          RES_O_ATTACHED,
          RES_M_ENABLED,
          RES_M_OP_ENABLE,
          RES_M_OP_DISBALE,
          RES_O_NOTIFY_EN
        };
    }
    int nbRes = sizeof(resList)/sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL)
    {
      return COAP_500_INTERNAL_SERVER_ERROR;
    }

    *numDataP = nbRes;
    for (i = 0 ; i < nbRes ; i++)
    {
      (*dataArrayP)[i].id = resList[i];
    }
  }

  i = 0;
  do
  {
    result = prv_set_value((*dataArrayP)+i, instanceP);
    i++;
  }while (i < *numDataP && result == COAP_205_CONTENT);

  return result;
}

/**
 * @brief This fucntion implements the discover operation performed on the 
 *        Device capability management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */ 
static uint8_t prv_devicecap_discover(uint16_t instanceId,
                                          int * numDataP,
                                          lwm2m_data_t ** dataArrayP,
                                          lwm2m_object_t * objectP)

{

  uint8_t result = COAP_205_CONTENT;
  int i = 0;
 // if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
  //{
  //  return COAP_500_INTERNAL_SERVER_ERROR;
  //}

  if(*numDataP == 0)
  {
    uint16_t resList[] = {
      RES_M_PROPERTY,
      RES_M_GROUP,
      RES_O_DESCRIPTION,
      RES_O_ATTACHED,
      RES_M_ENABLED,
      RES_M_OP_ENABLE,
      RES_M_OP_DISBALE,
      RES_O_NOTIFY_EN
    };
    int nbRes = sizeof(resList) / sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL)
    {
      return  COAP_500_INTERNAL_SERVER_ERROR;
    }

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
        case RES_M_PROPERTY:
        case RES_M_GROUP:
        case RES_O_DESCRIPTION:
        case RES_O_ATTACHED:
        case RES_M_ENABLED:
        case RES_M_OP_ENABLE:
        case RES_M_OP_DISBALE:
        case RES_O_NOTIFY_EN:
          break;
        default:
          result = COAP_404_NOT_FOUND;
      }
    }
  }

  return result;
}

/**
 * @brief This fucntion implements the write operation performed on Device
 * capability management object.
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */ 
static uint8_t prv_devicecap_write(uint16_t instanceId,
                                int numData,
                                lwm2m_data_t * dataArray,
                                lwm2m_object_t * objectP)
{

  int i = 0;
  uint8_t result = COAP_404_NOT_FOUND;
  device_cap_manangement_t *instanceP = NULL;
  lwm2m_object_data_t lwm2m_data;
  uint8_t id_set = 0;

  /**< No instance id check as its a multiple instance object */
  if( dataArray == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  instanceP = (device_cap_manangement_t *)LWM2M_LIST_FIND(objectP->instanceList, 
      instanceId);
  if (NULL == instanceP)
  {
    return result;
  }

  do
  {
   switch(dataArray[i].id) {
    case RES_O_NOTIFY_EN:
    {
      memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
      id_set = 0;
      id_set = id_set | LWM2M_OBJECT_ID_E;
      id_set = id_set | LWM2M_INSTANCE_ID_E;
      id_set = id_set | LWM2M_RESOURCE_ID_E;

      lwm2m_data.object_ID = LWM2M_DEVICE_CAP_OBJECT_ID;
      lwm2m_data.no_instances++;

      lwm2m_data.instance_info = (lwm2m_instance_info_t *) lwm2m_malloc(sizeof(lwm2m_instance_info_t));
      if(NULL != lwm2m_data.instance_info)
      {
        memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
        lwm2m_data.instance_info->instance_ID = instanceId;
        lwm2m_data.instance_info->no_resources++;

        lwm2m_data.instance_info->resource_info = (lwm2m_resource_info_t *) lwm2m_malloc(sizeof(lwm2m_resource_info_t));
        if(NULL != lwm2m_data.instance_info->resource_info)
        {
          memset(lwm2m_data.instance_info->resource_info, 0, sizeof(lwm2m_resource_info_t));
          lwm2m_data.instance_info->resource_info->resource_ID = RES_O_NOTIFY_EN;
          lwm2m_data.instance_info->resource_info->type = LWM2M_TYPE_BOOLEAN;
		  if(1 != lwm2m_data_decode_bool(dataArray+i, &lwm2m_data.instance_info->resource_info->value.asBoolean))
          {
            return COAP_400_BAD_REQUEST;
          }
		   
          lwm2m_data.instance_info->resource_info->next = NULL;
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

      #if 0 //add by joe start
      result = lwm2m_check_if_observe_and_update_app(ctx_p,id_set, LWM2M_DEVICE_CAP_OBJECT_ID, instanceId, RES_O_NOTIFY_EN, &lwm2m_data);
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      if (result == FALSE )
      {
        return COAP_400_BAD_REQUEST;
      }
      else
      {
        return COAP_204_CHANGED;
      }

      #else
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      return COAP_204_CHANGED;

      #endif //add by joe end

      
    }
    case RES_M_PROPERTY:
    case RES_M_GROUP:
    case RES_O_DESCRIPTION:
    case RES_O_ATTACHED:
    case RES_M_ENABLED:
    case RES_M_OP_ENABLE:
    case RES_M_OP_DISBALE:
      return COAP_405_METHOD_NOT_ALLOWED;
   }
   i++;
  } while (i < numData); //&& result == LWM2M_204_CHANGED); commented to fix KW

  return result;
}

/**
 * @brief This fucntion is used to change the value of resource "enabled" based
 * on the execute operation performed on resource "op_enable" or "op_disable"
 * @param objectP (IN/OUT) Object value to be changed
 * @param status (IN) status of the object 
 * @return LWM2M response type
 */ 
static uint8_t prv_change_device_status(lwm2m_object_t * objectP, bool status, 
    uint16_t instanceId)
{

  device_cap_manangement_t* device_cap_obj = NULL;

  if( objectP == NULL ) 
  {
    return  COAP_500_INTERNAL_SERVER_ERROR;
  }

  device_cap_obj = (device_cap_manangement_t *)LWM2M_LIST_FIND(objectP->instanceList, 
      instanceId);
  if (NULL == device_cap_obj)
  {
    return COAP_404_NOT_FOUND;
  }

  if(status == false)
  {
    if(device_cap_obj->enabled == true)
    {
      device_cap_obj->enabled = false;
      return COAP_204_CHANGED;
    }
    else
    {
      return COAP_406_NOT_ACCEPTABLE;
    }
  }

  else
  {
    if(device_cap_obj->enabled == false)
    {
      device_cap_obj->enabled = true;
      return COAP_204_CHANGED;
    }
    else
    {
      return COAP_406_NOT_ACCEPTABLE;
    }
  }
}

/**
 * @brief This fucntion implements the execute operation performed on Device
 * capability management object.
 * @param instanceId (IN) Instance ID of the object to be read
 * @param resourceId (IN) Resource ID of the resource to be executed
 * @param buffer (IN) message payload from server fro execute operation
 * @param length (IN) message payload length
 * @param objectP Object on which the operation needs to be executed
 * @return LWM2M response type
 */ 
static uint8_t prv_devicecap_execute(uint16_t instanceId,
                                  uint16_t resourceId,
                                  uint8_t * buffer,
                                  int length,
                                  lwm2m_object_t * objectP)
{
  device_cap_manangement_t *instanceP = NULL;
  uint8_t ret_val = COAP_405_METHOD_NOT_ALLOWED;
  bool result = false;
  lwm2m_object_data_t lwm2m_data;
  uint8_t id_set = 0;

  if ( objectP == NULL )
  {
    return COAP_400_BAD_REQUEST;
  }

  instanceP = (device_cap_manangement_t *)LWM2M_LIST_FIND(objectP->instanceList, instanceId);

  if (NULL == instanceP)
  {
    return COAP_404_NOT_FOUND;
  }

  switch(resourceId)
  {
    case RES_M_OP_ENABLE:
      ret_val = prv_change_device_status(objectP, true, instanceId);


      //if(LWM2M_204_CHANGED == ret_val)

      //Intimate the device capability app on value change
      memset (&lwm2m_data , 0 , sizeof (lwm2m_object_data_t));
      id_set = 0;
      id_set = id_set | LWM2M_OBJECT_ID_E;
      id_set = id_set | LWM2M_INSTANCE_ID_E;
      id_set = id_set | LWM2M_RESOURCE_ID_E;

      lwm2m_data.object_ID = LWM2M_DEVICE_CAP_OBJECT_ID;
      lwm2m_data.no_instances++;

      lwm2m_data.instance_info = (lwm2m_instance_info_t *) lwm2m_malloc(sizeof(lwm2m_instance_info_t));
      if(NULL != lwm2m_data.instance_info)
      {
        memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
        lwm2m_data.instance_info->instance_ID = instanceId;
        lwm2m_data.instance_info->no_resources++;

        lwm2m_data.instance_info->resource_info = (lwm2m_resource_info_t *) lwm2m_malloc(sizeof(lwm2m_resource_info_t));
        if(NULL != lwm2m_data.instance_info->resource_info)
        {
          memset(lwm2m_data.instance_info->resource_info, 0, sizeof(lwm2m_resource_info_t));
          lwm2m_data.instance_info->resource_info->resource_ID = RES_M_OP_ENABLE;
          lwm2m_data.instance_info->resource_info->type = LWM2M_TYPE_BOOLEAN;
          lwm2m_data.instance_info->resource_info->value.asBoolean  = 1;
        }
        else
        {
          lwm2m_free_instance_info(lwm2m_data.instance_info);
          return COAP_500_INTERNAL_SERVER_ERROR;
        }
      }
      else
      {
        return  COAP_500_INTERNAL_SERVER_ERROR;
      }
      //add by joe start
#if 0
      result = lwm2m_check_if_observe_and_update_app(ctx_p,id_set, LWM2M_DEVICE_CAP_OBJECT_ID, instanceId, RES_M_OP_ENABLE, &lwm2m_data);
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      if (result == false )
      {
        ret_val = COAP_500_INTERNAL_SERVER_ERROR;
      }
      else
      {
        ret_val = COAP_204_CHANGED;
      }
#else
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      ret_val = COAP_204_CHANGED;
#endif
      //add by joe end

      break;

    case RES_M_OP_DISBALE:
      ret_val = prv_change_device_status(objectP, false, instanceId);
      //Intimate the device capability app on value change
      memset (&lwm2m_data , 0 , sizeof (lwm2m_object_data_t));
      id_set = 0;
      id_set = id_set | LWM2M_OBJECT_ID_E;
      id_set = id_set | LWM2M_INSTANCE_ID_E;
      id_set = id_set | LWM2M_RESOURCE_ID_E;

      lwm2m_data.object_ID = LWM2M_DEVICE_CAP_OBJECT_ID;
      lwm2m_data.no_instances++;

      lwm2m_data.instance_info = (lwm2m_instance_info_t *) lwm2m_malloc(sizeof(lwm2m_instance_info_t));
      if(NULL != lwm2m_data.instance_info)
      {
        memset(lwm2m_data.instance_info, 0, sizeof(lwm2m_instance_info_t));
        lwm2m_data.instance_info->instance_ID = instanceId;
        lwm2m_data.instance_info->no_resources++;

        lwm2m_data.instance_info->resource_info = (lwm2m_resource_info_t *) lwm2m_malloc(sizeof(lwm2m_resource_info_t));
        if(NULL != lwm2m_data.instance_info->resource_info)
        {
          memset(lwm2m_data.instance_info->resource_info, 0, sizeof(lwm2m_resource_info_t));    
          lwm2m_data.instance_info->resource_info->resource_ID = RES_M_OP_DISBALE;
          lwm2m_data.instance_info->resource_info->type = LWM2M_TYPE_BOOLEAN;
          lwm2m_data.instance_info->resource_info->value.asBoolean  = 0;
        }
        else
        {
          lwm2m_free_instance_info(lwm2m_data.instance_info);
          return  COAP_500_INTERNAL_SERVER_ERROR;
        }
      }
      else
      {
        return  COAP_500_INTERNAL_SERVER_ERROR;
      }

      #if 0 //add by joe start
      result = lwm2m_check_if_observe_and_update_app(ctx_p,id_set, LWM2M_DEVICE_CAP_OBJECT_ID, instanceId, RES_M_OP_DISBALE, &lwm2m_data);
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      if (result == FALSE )
      {
        ret_val = COAP_500_INTERNAL_SERVER_ERROR;
      }
      else
      {
        ret_val = COAP_204_CHANGED;
      }
      #else
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      ret_val = COAP_204_CHANGED;
      #endif
        //add by joe end
     break;
    case RES_M_PROPERTY:
    case RES_M_GROUP:
    case RES_O_DESCRIPTION:
    case RES_O_ATTACHED:
    case RES_M_ENABLED:
    case RES_O_NOTIFY_EN:
      return  COAP_405_METHOD_NOT_ALLOWED;
    default:
      return  COAP_404_NOT_FOUND;
  }
  return ret_val;
}


/**
 * @fn      static uint8_t prv_devicecap_delete()
 * @brief   This function implements the delete operation performed
 *           on the Device cap object
 * @param   instanceId (IN) Instance ID of the object instance to be deleted
 * @param   objectP (IN)    Object whose instance is to be deleted
 * @return  LWM2M response type
 */
static uint8_t prv_devicecap_delete(uint16_t id,
                                 lwm2m_object_t * objectP)
{
  device_cap_manangement_t *instanceP = NULL;
  if( objectP == NULL )
  {
    return  COAP_500_INTERNAL_SERVER_ERROR;
  }

  objectP->instanceList = LWM2M_LIST_RM(objectP->instanceList,id, &instanceP);
  if (instanceP == NULL)
  {
    return COAP_404_NOT_FOUND;
  }
  lwm2m_free(instanceP);

  return  COAP_202_DELETED;
}

/*
 * @fn      static int32_t prv_devicecap_setvalue ()
 * @brief   This function is used to  set the particular resource values for given instance 
 * @param   lwm2m_data - pointer to lwm2m object information
 *          targetP    - pointer to lwm2m object 
 * @return  on success - 1
 on failure - 0
 */

static int32_t prv_devicecap_setvalue
(
void *ctx_p,
lwm2m_object_data_t * lwm2m_data,
lwm2m_object_t * targetP,
lwm2m_uri_t *uriP
)
{

  int result = 0;
  device_cap_manangement_t* instanceP = NULL;

  if (lwm2m_data == NULL || targetP == NULL)
  {
    return 0;
  }
  instanceP = (device_cap_manangement_t *)LWM2M_LIST_FIND(targetP->instanceList, lwm2m_data->instance_info->instance_ID);

  if (NULL == instanceP)
  {
    return 0;
  }

  switch (lwm2m_data->instance_info->resource_info->resource_ID) 
  {
    case RES_M_PROPERTY:
      {
        strlcpy((instanceP->property),(const char *)(lwm2m_data->instance_info->resource_info->value.asBuffer.buffer),
            (lwm2m_data->instance_info->resource_info->value.asBuffer.length) + 1);

        uriP->objectId   = lwm2m_data->object_ID;
        uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
        uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
        uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

        result = 1;
      }
      break;

    case RES_M_GROUP:
      {
        int64_t value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asInteger = lwm2m_data->instance_info->resource_info->value.asInteger;
        if (1 == lwm2m_data_decode_int(&dataP , &value))
        {
          instanceP->group= value;
          uriP->objectId   = lwm2m_data->object_ID;
          uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
          uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
          uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID ;

          result = 1;
        }
        else
        {
          result = 0;
        }
      }
      break;

    case RES_O_DESCRIPTION:
      {
        strlcpy((instanceP->description),(const char *)(lwm2m_data->instance_info->resource_info->value.asBuffer.buffer),
            (lwm2m_data->instance_info->resource_info->value.asBuffer.length) + 1);

        uriP->objectId   = lwm2m_data->object_ID;
        uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
        uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
        uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

        result = 1;
      }
      break;

    case RES_O_ATTACHED:
      {
        bool value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asBoolean = lwm2m_data->instance_info->resource_info->value.asBoolean;
        if(1 == lwm2m_data_decode_bool(&dataP, &value))
        {
          instanceP->attached = value;
          uriP->objectId   = lwm2m_data->object_ID;
          uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
          uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
          uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

          result = 1;
        }
        else
        {
          result = 0;
        }
      }
      break;

    case RES_M_ENABLED:
      {
        bool value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asBoolean = lwm2m_data->instance_info->resource_info->value.asBoolean;
        if(1 == lwm2m_data_decode_bool(&dataP, &value))
        {
          instanceP->enabled = value;
          uriP->objectId   = lwm2m_data->object_ID;
          uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
          uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
          uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

          result = 1;
        }
        else
        {
          result = 0;
        }
      }
      break;

    case RES_O_NOTIFY_EN:
      {
        bool value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asBoolean = lwm2m_data->instance_info->resource_info->value.asBoolean;
        if(1 == lwm2m_data_decode_bool(&dataP, &value))
        {
          instanceP->notifyen = value;
          uriP->objectId   = lwm2m_data->object_ID;
          uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
          uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
          uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

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

  return result;

}


static uint8_t prv_devicecap_getValue
(
void *ctx_p,
uint16_t instanceId, 
lwm2m_object_t *objectP, 
void * getval
)
{
  device_cap_manangement_t *device_cap_obj = NULL;
  bool *retval;

  if((objectP == NULL) || (NULL == getval)) 
  {
    return  COAP_500_INTERNAL_SERVER_ERROR;
  }

  device_cap_obj = (device_cap_manangement_t *)LWM2M_LIST_FIND(objectP->instanceList, 
      instanceId);
  if (NULL == device_cap_obj)
  {
    return COAP_404_NOT_FOUND;
  }

  retval = getval;
  *retval = device_cap_obj->notifyen;

  return COAP_204_CHANGED;
}

/*
 * @fn      static int32_t prv_devicecap_getvalue ()
 * @brief   This function is used to  get the particular resource values for given instance 
 * @param   id_info      - indicates what values need to be filled whether resource or instance or object level 
 lwm2m_dataP  - pointer to lwm2m object information to be filled 
 *          targetP      - pointer to lwm2m object 
 * @return  on successs  -  0
 on failure   - -1
 */


int32_t prv_devicecap_getvalue
(
void *ctx_p,
lwm2m_id_info_t id_info,
lwm2m_object_data_t * lwm2m_dataP ,
lwm2m_object_t * targetP
)
{
  device_cap_manangement_t *instanceP = NULL;
  int8_t result = -1;
  lwm2m_data_t *dataArrayP = NULL;
  lwm2m_data_t *dataArray  = NULL;
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
    
    resource_info = (lwm2m_resource_info_t *)lwm2m_malloc (sizeof (lwm2m_resource_info_t)); 
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

  instanceP = (device_cap_manangement_t *)LWM2M_LIST_FIND(targetP->instanceList, id_info.instance_ID);
  if (NULL == instanceP)
    return result;
 
  if (numData == 0)
  {
    uint16_t resList[] = {
      RES_M_PROPERTY,
      RES_M_GROUP,
      RES_O_DESCRIPTION,
      RES_O_ATTACHED,
      RES_M_ENABLED,
      RES_O_NOTIFY_EN
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

      case LWM2M_TYPE_BOOLEAN :
        resource_info->value.asBoolean = dataArrayP->value.asBoolean;
        break;

      default :
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
 * @brief This function is used to get the information regarding the Device
 * capability management object during client registration.
 * @param void
 * @return Device capability management object  
 */ 
lwm2m_object_t * get_object_deviceCap()
{

  /* The get_object_deviceCap function create the object itself and return a 
     pointer to the structure that represent it.*/
  lwm2m_object_t * deviceCapObj = NULL;
  device_cap_manangement_t *devcap_data = NULL;

  deviceCapObj = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));

  if(NULL != deviceCapObj)
  {
    memset(deviceCapObj, 0, sizeof(lwm2m_object_t));

    /* It assigns his unique ID. The 15 is the standard ID for the mandatory 
       object "Object device".*/
    deviceCapObj->objID = LWM2M_DEVICE_CAP_OBJECT_ID;

    /*
     * And the private function that will access the object.
     * Those function will be called when a read/write/execute query is made by the server. In fact the library don't need to
     * know the resources of the object, only the server does.
     */
    deviceCapObj->readFunc     = prv_devicecap_read;
    deviceCapObj->discoverFunc = prv_devicecap_discover;
    deviceCapObj->writeFunc    = prv_devicecap_write;
    deviceCapObj->executeFunc  = prv_devicecap_execute;
    deviceCapObj->createFunc   = prv_devicecap_create;
    deviceCapObj->deleteFunc   = prv_devicecap_delete;
    //deviceCapObj->getValueFunc = prv_devicecap_getValue;
    //deviceCapObj->get_Value_Func = prv_devicecap_getvalue;
    deviceCapObj->setValueFunc   = prv_devicecap_setvalue;
	 /*
   * and its unique instance
   *
   */
	devcap_data = (device_cap_manangement_t*)lwm2m_malloc(sizeof(device_cap_manangement_t));
	  
	if (NULL != devcap_data)
	{
		  memset(devcap_data, 0, sizeof(device_cap_manangement_t));

		  memset(devcap_data->description, 0, PRV_DESCRIPTION_MAXLEN);
		  strcpy(devcap_data->description, "SD storage function");

		  memset(devcap_data->property, 0, PRV_PROPERTY_MAXLEN);
		  strcpy(devcap_data->property, "SD storage function");
          
		  LOG_ARG("devcap_data->property=%s",devcap_data->property);
		  devcap_data->group = PRV_GROUP_CONNECTIVITY;
	   	  devcap_data->attached = false;
		  devcap_data->enabled = false;
		  devcap_data->notifyen = false;
          devcap_data->openable = false;
          devcap_data->openable = false;

	      devcap_data->instanceId = 0;

	      deviceCapObj->instanceList = LWM2M_LIST_ADD(deviceCapObj->instanceList, devcap_data); 
		  deviceCapObj->userData = NULL;
	}
	else
	{
		lwm2m_free(deviceCapObj);
		return NULL;
	}
  }
  
 
 // MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] deviceCapObj=%p", deviceCapObj);	
  
  return deviceCapObj;
}

/**
 * @brief This fucntion frees the Device capability management object allocated
 * @param ObjectP (IN) Object to free
 * @return void
 */ 
void free_object_devicecap(lwm2m_object_t * objectP)
{

  if( objectP == NULL )
  { 
    return ;
  }

  if(NULL != objectP->userData)
  {
    lwm2m_free(objectP->userData);
    objectP->userData = NULL;
  }

  if(NULL != objectP->instanceList)
  {
    lwm2m_list_free(objectP->instanceList);
    objectP->instanceList = NULL;
  }

  lwm2m_free(objectP);
}

#endif //ENABLE_DEVCAP_OBJ
