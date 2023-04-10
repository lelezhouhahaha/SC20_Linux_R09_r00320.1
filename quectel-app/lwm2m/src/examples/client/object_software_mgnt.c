/******************************************************************************

  @file    object_software_mgnt.c
  @brief   File contains the implementation for the operations related to object
           Software management
  ---------------------------------------------------------------------------

  Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

 *****************************************************************************/

/*
 *  Object                         |      | Multiple  |     | Description
 *  |
 *  Name                           |  ID  | Instances |Mand.|
 *  |
 *---------------------------------+------+-----------+-----+-------------------------------+
 *  Software Management            |  9   |    Yes    |  No | urn:oma:lwm2m:oma:9
 *  |
 *
 *  Resources:
 *  Name        | ID  | Oper.|Instances|Mand.|  Type   | Range | Units |
 *  Description
 *  |
 * -------------+-----+------+---------+-----+---------+-------+-------+----------------------------------------------------------------------------------+
 *  PkgName     |  0  |  R   | Single  | Yes | String  | 0-255 |       | Name of the software package
 *  PkgVersion  |  1  |  R   | Single  | Yes | String  | 0-255 |       | Version of the software package
 *  Package     |  2  |  W   | Single  | No  | Opaque  |       |       | Software package The package can be in one single software component,
 or any delivery material used by the Device to determine the
 software component(s) and proceed to their installation. Can be 
 archive file, executable, manifest. This resource to be used when it 
 is single block of delivery.
 * Package URI  |  3  |  W   | Single  | No  | String  | 0-255 |       | URI from where the device can download the software package by an 
 alternative mechanism. As soon the device has received the Package 
 URI it performs the download at the next practical opportunity.
 Can be direct link to a single software component or link to archive 
 file, executable, or manifest, used by the Device to determine, 
 then access to the software component(s). This resource to be used 
 when it is single block of delivery.
 * Install      |  4  |  E   | Single  | Yes |         |       |       | Installs software from the package either stored in Package 
 resource, or,downloaded from the Package URI. This Resource is only
 executable when the value of the State Resource is DELIVERED.
 *


*/
#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef ENABLE_SOFTWARE_MGNT_OBJ
#define PRV_PKG_NAME    "gesl software"     /**< Default value for package name */
#define PRV_PKG_VERSION "1.0"       /**< Default value for pacakge version */
//#define PRV_PACKAGE   
//#define PRV_PACKAGE_URI   "http://172.16.7.95"
//#define PRV_CHECKPOINT    
#define PRV_UPDATE_STATE    0 /**< Default value for update state */
#define PRV_UPDATE_SUPP_OBJ_TRUE    1 /**< True value for update supported
                                        objects */
#define PRV_UPDATE_SUPP_OBJ_FALSE   0 /**< False value for update supported
                                        objects*/
#define PRV_UPDATE_RES      0 /**< Default update result */
#define PRV_ACTIVATION_STATE_TRUE   1 /**< True value for activation state */
#define PRV_ACTIVATION_STATE_FALSE  0 /**< False value for activation state */
//#define PRV_PACKAGE_SETTINGS
#define PRV_USER_NAME   "gesl" /**< Default value for username */
#define PRV_PASSWORD    "password" /**< Default value for password */
#define PRV_STATUS_REASON   "no software update" /**< Default value for status
                                                   reason */
//#define PRV_SOFTWARE_COMP_LINK
//#define PRV_SOFTWARE_COMP_TREE_LEN    0


#define RES_M_PKG_NAME      0 /**< Resource ID for "PkgName" */
#define RES_M_PKG_VERSION       1 /**< Resource ID for "PkgVersion" */
#define RES_O_PACKAGE           2 /**< Resource ID for "Package" */
#define RES_O_PACKAGE_URI       3 /**< Resource ID for "Package URI" */
#define RES_M_INSTALL           4 /**< Resource ID for "Install" */
#define RES_O_CHECKPOINT        5 /**< Resource ID for "Checkpoint" */
#define RES_M_UNINSTALL     6 /**< Resource ID for "Uninstall" */
#define RES_M_UPDATE_STATE  7 /**< Resource ID for "Update State" */
#define RES_O_UPDATE_SUPPORTED_OBJ  8 /**< Resource ID for "Update
                                        Supported Objects" */
#define RES_M_UPDATE_RESULT 9 /**< Resource ID for "Update
                                Result" */   
#define RES_M_ACTIVATE      10 /**< Resource ID for "Activate" */
#define RES_M_DEACTIVATE        11 /**< Resource ID for "Deactivate" */
#define RES_M_ACTIVATION_STATE  12 /**< Resource ID for "Activation
                                     State" */
#define RES_O_PACKAGE_SETTINGS  13 /**< Resource ID for "Package
                                     Settings" */
#define RES_O_USERNAME          14 /**< Resource ID for "User Name" */
#define RES_O_PASSWORD          15 /**< Resource ID for "Password" */
#define RES_O_STATUS_REASON     16 /**< Resource ID for "Status
                                     Reason" */
#define RES_O_SOFTWARE_COMP_LINK    17 /**< Resource ID for "Software
                                         Component Link" */
#define RES_O_SOFTWARE_COMP_TREE_LEN    18 /**< Resource ID for "Software
                                             Component tree length" */

#define PRV_UPDATE_STATE_INITIAL         0/**< Update state "initial" */
#define PRV_UPDATE_STATE_DOWNLOAD_STARTED   1 /**< Update state "Download
                                                started" */
#define PRV_UPDATE_STATE_DOWNLOADED    2 /**< Update state "Downloaded" */
#define PRV_UPDATE_STATE_DELIVERED      3 /**< Update state "Delivered " */
#define PRV_UPDATE_STATE_INSTALLED      4 /**< Update state "State installed" */


#define PRV_UPDATE_RES_INITIAL      0 /**< Update result "Initial" */
#define PRV_UPDATE_RES_DOWNLOADING  1 /**< Update result "Downloading" */
#define PRV_UPDATE_RES_INSTALL_SUCCESS  2 /**< Update result "install success" */
#define PRV_UPDATE_RES_DOWNLOAD_SUCCESS 3 /**< Update result "Download success" */
#define PRV_UPDATE_RES_NO_ENOUGH_STORAGE    50 /**< Update result "Not enough
                                                 storage" */
#define PRV_UPDATE_RES_OUT_OF_MEMORY    51 /**< Update result "Device is out of
                                             memory" */
#define PRV_UPDATE_RES_CONNECTION_LOST  52 /**< Update result "Connection lost" */
#define PRV_UPDATE_RES_PKG_CHECK_FAILURE    53 /**< Update result "Package check
                                                 failure" */
#define PRV_UPDATE_RES_PKG_UNSUPPORTED      54 /**< Update result "package
                                                 unsupported" */
#define PRV_UPDATE_RES_INVALID_URI      56 /**< Update result "URI is invalid" */
#define PRV_UPDATE_RES_UPDATE_ERROR     57 /**< Update result "Update error" */
#define PRV_UPDATE_RES_INSTALL_ERROR    58 /**< Update result "Install error" */
#define PRV_UPDATE_RES_UNINSTALL_ERROR  59 /**< Update result "uninstall error" */
#define PRV_UPDATE_RES_IMPROPER_STATE 60 /**< Out of state events, Eg: Package written 
                                           when state is neither initial nor downloading */

#define PRV_MAX_STRSIZE 255

#define PRV_EMPTY   " "

bool gpackage_running = false;
/**
 * @brief Describes the Software management object resources
 *
 */
typedef struct _software_mgnt_t
{
  struct _software_mgnt_t *next;  /**< corresponding to lwm2m_list_t::next */
  uint16_t    id;                 /**< corresponding to lwm2m_list_t::id  */
  char pkg_name[PRV_MAX_STRSIZE];     /**< Package name (Mandatory) */
  char pkg_version[PRV_MAX_STRSIZE];  /**< Package version (Mandatory) */
  char package[PRV_MAX_STRSIZE];      /**< Package (optional) */
  char package_uri[PRV_MAX_STRSIZE];  /**< Pacakge URI (optional) */
  int8_t update_state;    /**< Update state (Mandatory) */
  bool update_supp_obj;   /**< Update supported objects (Optional) */
  int16_t update_result;  /**< Update result (Mandatory) */
  bool activation_state;  /**< Activation state (Mandatory) */
  char username[PRV_MAX_STRSIZE];     /**< User name (optional) */
  char password[PRV_MAX_STRSIZE];     /**< Password (optional) */
  char status_reason[PRV_MAX_STRSIZE];    /**< Status reason (optional) */
} software_mgnt_t;

/**
 * @brief This function is used to write the resources into object instance
 * while creating the object instance. Note: not to be called to perform write
 * operation
 * @param   instanceP (INOUT)   handle to object instance
 * @param   numData (IN)        number of resources to be written
 * @param   dataArray (IN)      array of resource values
 * @return  LWM2M_NO_ERROR on success
 */
static uint8_t prv_write_smresources(software_mgnt_t *instanceP, int numData,
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
      case RES_M_PKG_NAME:
        LWM2M_STRCPY(instanceP->pkg_name, (const char *) ((dataArray + i)->value.asBuffer.buffer), 
            PRV_MAX_STRSIZE);
        break;
      case RES_M_PKG_VERSION:
        LWM2M_STRCPY(instanceP->pkg_version, (const char *)((dataArray + i)->value.asBuffer.buffer),
            PRV_MAX_STRSIZE);
        break;
      case RES_M_UPDATE_STATE:
        {
          int64_t value = 0;
          if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t*)&value))
            return COAP_400_BAD_REQUEST;
          instanceP->update_state = (int8_t)value;
        }
        break;
      case RES_M_UPDATE_RESULT:
        {
          int64_t value = 0;
          if (1 != lwm2m_data_decode_int(dataArray + i, (int64_t*)&value))
            return COAP_400_BAD_REQUEST;
          instanceP->update_result = value;
        }
        break;
      case RES_M_ACTIVATION_STATE:
        if (1 != lwm2m_data_decode_bool(dataArray + i, &instanceP->activation_state))
          return COAP_400_BAD_REQUEST;
        break;
      default:
        //Ignoring any optional resource
        break;
    }
  }
  return COAP_NO_ERROR;
}
/**
 * @fn static uint8_t prv_softwaremgnt_create()
 * @brief   This function implements the create operation performed
 * on the Software Management object
 * @param   instanceId (IN) Instance ID of the object instance to be created
 * @param   numData (IN)    number of resources in the array i.e. dataArray
 * @param   dataArray (IN)  resource value array
 * @param   objectP (IN)    Object whose instance is to be created
 * return   COAP response type
 */
static uint8_t prv_softwaremgnt_create(uint16_t instanceId, int numData, 
    lwm2m_data_t * dataArray, 
    lwm2m_object_t * objectP)
{
  software_mgnt_t *instanceP = NULL;
  uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
  if( dataArray == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  instanceP = (software_mgnt_t *)lwm2m_malloc(sizeof(software_mgnt_t));
  if (instanceP != NULL)
  {
    memset(instanceP, 0, sizeof(software_mgnt_t));
    instanceP->id = instanceId;
    result = prv_write_smresources(instanceP, numData, dataArray);

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
 * @fn static uint8_t prv_softwaremgnt_delete(uint16_t instanceId, 
 *                          lwm2m_object_t * objectP)
 * @brief   This function implements the delete operation performed
 * on the Software Management object
 * @param   instanceId (IN) Instance ID of the object instance to be deleted
 * @param   objectP (IN)    Object whose instance is to be deleted
 * @return  LWM2M response type
 */
static uint8_t prv_softwaremgnt_delete(uint16_t instanceId,
    lwm2m_object_t *objectP)
{
  software_mgnt_t *instanceP = NULL;
  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  objectP->instanceList = LWM2M_LIST_RM(objectP->instanceList,instanceId, &instanceP);
  if (instanceP == NULL) 
  	return COAP_404_NOT_FOUND;

  lwm2m_free(instanceP);

  return COAP_202_DELETED;
}

/**
 * @brief This fucntion is used to populate the values of the resource
 * to be provided to the server during the read operation on the object.
 * @param dataP (OUT) data array for the resource 
 * @param swMgntDataP (IN) Software Management object data
 * @return LWM2M response type
 */
static uint8_t prv_set_value(lwm2m_data_t * dataP,
    software_mgnt_t * swMgntDataP)
{
  if( dataP == NULL || swMgntDataP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  switch (dataP->id)
  {
    case RES_M_PKG_NAME:
      lwm2m_data_encode_string(swMgntDataP->pkg_name, dataP); 
      return COAP_205_CONTENT;
    case RES_M_PKG_VERSION:
      lwm2m_data_encode_string(swMgntDataP->pkg_version, dataP);
      return COAP_205_CONTENT;
    case RES_M_UPDATE_STATE:
      lwm2m_data_encode_int(swMgntDataP->update_state, dataP);
      return COAP_205_CONTENT;
    case RES_M_UPDATE_RESULT:
      lwm2m_data_encode_int(swMgntDataP->update_result, dataP);
      return COAP_205_CONTENT;
    case RES_M_ACTIVATION_STATE:
      lwm2m_data_encode_bool(swMgntDataP->activation_state, dataP);
      return COAP_205_CONTENT;
    case RES_O_STATUS_REASON:
      lwm2m_data_encode_string(swMgntDataP->status_reason, dataP);
      return COAP_205_CONTENT;
    case RES_O_PACKAGE:
    case RES_O_PACKAGE_URI:
    case RES_M_INSTALL:
    case RES_M_UNINSTALL:
    case RES_M_ACTIVATE:
    case RES_M_DEACTIVATE:
    case RES_O_USERNAME:
    case RES_O_PASSWORD:
      return COAP_405_METHOD_NOT_ALLOWED;
    case RES_O_CHECKPOINT:
    case RES_O_PACKAGE_SETTINGS:
    case RES_O_SOFTWARE_COMP_LINK:
    case RES_O_SOFTWARE_COMP_TREE_LEN:
    case RES_O_UPDATE_SUPPORTED_OBJ:
    default:
      return COAP_404_NOT_FOUND;
  }
}

/**
 * @brief This function implements the read operation performed on Software
 * management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */
static uint8_t prv_softwaremgnt_read(uint16_t instanceId,
    int * numDataP,
    lwm2m_data_t ** dataArrayP,
    lwm2m_object_t * objectP,
    uint16_t resInstId)
{
  uint8_t result = COAP_404_NOT_FOUND;
  int i = 0;
  software_mgnt_t *instanceP = NULL;
  if( dataArrayP == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }



  instanceP = (software_mgnt_t *)LWM2M_LIST_FIND(objectP->instanceList, instanceId);
  if (NULL == instanceP)
  	 return result;

  if(*numDataP == 0)
  {
    uint16_t resList[] = {
      RES_M_PKG_NAME,
      RES_M_PKG_VERSION,
      RES_M_UPDATE_STATE,
      RES_M_UPDATE_RESULT,
      RES_M_ACTIVATION_STATE,
      RES_O_STATUS_REASON
    };

    int nbRes = sizeof(resList)/sizeof(uint16_t);

    *dataArrayP = lwm2m_data_new(nbRes);
    if(*dataArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;
    *numDataP = nbRes;
    for(i = 0; i < nbRes; i++)
    {
      (*dataArrayP)[i].id = resList[i];
    }       
  }

  i = 0;
  do
  {
    result = prv_set_value((*dataArrayP) + i, instanceP);
    i++;
  } while(i < *numDataP && result == COAP_205_CONTENT);

  return result;
}

/**
 * @brief This fucntion implements the discover operation performed on the
 * Software management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */
static uint8_t prv_softwaremgnt_discover(uint16_t instanceId,
    int * numDataP,
    lwm2m_data_t ** dataArrayP,
    lwm2m_object_t * objectP)
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
      RES_M_PKG_NAME,
      RES_M_PKG_VERSION,
      RES_O_PACKAGE,
      RES_O_PACKAGE_URI,
      RES_M_INSTALL,
      //Uncomment below line when checkpoint resource is supported
      //RES_O_CHECKPOINT, 
      RES_M_UNINSTALL,
      RES_M_UPDATE_STATE,
      RES_M_UPDATE_RESULT,
      RES_M_ACTIVATE,
      RES_M_DEACTIVATE,
      RES_M_ACTIVATION_STATE,  
      //Uncomment below line when package setting resource is supported
      //RES_O_PACKAGE_SETTINGS,
      RES_O_USERNAME,
      RES_O_PASSWORD,
      RES_O_STATUS_REASON,
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
        case RES_M_PKG_NAME:
        case RES_M_PKG_VERSION:
        case RES_O_PACKAGE:
        case RES_O_PACKAGE_URI: 
        case RES_M_INSTALL:
          //Uncomment below line when checkpoint resource is supported
          //case RES_O_CHECKPOINT:
        case RES_M_UNINSTALL:
        case RES_M_UPDATE_STATE:
        case RES_M_UPDATE_RESULT:
        case RES_M_ACTIVATE:
        case RES_M_DEACTIVATE:
        case RES_M_ACTIVATION_STATE:
          //Uncomment below line when package setting resource is supported
          //case RES_O_PACKAGE_SETTINGS:
        case RES_O_USERNAME:
        case RES_O_PASSWORD:
        case RES_O_STATUS_REASON:
          break;
        default:
          result = COAP_404_NOT_FOUND;
      }
    }
  }

  return result;
}

/**
 * @brief This fucntion implements the write operation performed on software
 * management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param numDataP (OUT) number of resources in the array
 * @param dataArrayP (OUT) resource value array
 * @param objectP (IN) Object to be read
 * @return LWM2M response type
 */
static uint8_t prv_softwaremgnt_write(uint16_t instanceId,
    int numData,
    lwm2m_data_t * dataArray,
    lwm2m_object_t * objectP,
    uint16_t resInstId, uint8_t write_method)
{
  int i = 0;
  uint8_t result = COAP_404_NOT_FOUND;

  lwm2m_object_data_t lwm2m_data;
  uint8_t id_set = 0;
  bool ret_value  = false ;
  software_mgnt_t *instanceP = NULL;
  if( dataArray == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  } 

  instanceP = (software_mgnt_t *)LWM2M_LIST_FIND(objectP->instanceList, instanceId);

  if (NULL == instanceP) 
  	return result;

  do
  {
    switch(dataArray[i].id)
    {
      case RES_O_PACKAGE:
        if ((PRV_UPDATE_STATE_DOWNLOAD_STARTED != instanceP->update_state
              && PRV_UPDATE_STATE_INITIAL != instanceP->update_state ) || (gpackage_running == true))
        {
          return COAP_400_BAD_REQUEST;
        }
        memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
        id_set = 0;
        id_set = id_set | LWM2M_OBJECT_ID_E;
        id_set = id_set | LWM2M_INSTANCE_ID_E;
        id_set = id_set | LWM2M_RESOURCE_ID_E;

        lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
            lwm2m_data.instance_info->resource_info->resource_ID = RES_O_PACKAGE;
            lwm2m_data.instance_info->resource_info->type = (lwm2m_data_type_t)dataArray[i].type;
            lwm2m_data.instance_info->resource_info->value.asBuffer.length  = dataArray[i].value.asBuffer.length;
           // lwm2m_data.instance_info->resource_info->value.asBuffer.buffer  = dataArray[i].value.asBuffer.buffer;
            if (lwm2m_data.instance_info->resource_info->value.asBuffer.length )
            {
              lwm2m_data.instance_info->resource_info->value.asBuffer.buffer = malloc (lwm2m_data.instance_info->resource_info->value.asBuffer.length);
              if (lwm2m_data.instance_info->resource_info->value.asBuffer.buffer == NULL )
              {
                return COAP_500_INTERNAL_SERVER_ERROR;

              }
              memscpy (lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,lwm2m_data.instance_info->resource_info->value.asBuffer.length, \
                  dataArray[i].value.asBuffer.buffer ,lwm2m_data.instance_info->resource_info->value.asBuffer.length);                
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

        ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_O_PACKAGE, &lwm2m_data);
        lwm2m_free_instance_info(lwm2m_data.instance_info);

        if (ret_value == false ) {
          result = COAP_400_BAD_REQUEST;
        }
        else
        {
          instanceP->update_state  = PRV_UPDATE_STATE_DOWNLOAD_STARTED; 
          instanceP->update_result = PRV_UPDATE_RES_DOWNLOADING;
          result = COAP_204_CHANGED;
          if ( dataArray[i].value.asBuffer.block1_more == false) {
            instanceP->update_state  = PRV_UPDATE_STATE_DOWNLOADED;
            instanceP->update_result = PRV_UPDATE_RES_INITIAL;
            memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
            id_set = 0;
            id_set = id_set | LWM2M_OBJECT_ID_E;
            id_set = id_set | LWM2M_INSTANCE_ID_E;
            id_set = id_set | LWM2M_RESOURCE_ID_E;

            lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
                lwm2m_data.instance_info->resource_info->resource_ID = RES_M_UPDATE_STATE;
                lwm2m_data.instance_info->resource_info->type = (lwm2m_data_type_t)LWM2M_TYPE_INTEGER;
                lwm2m_data.instance_info->resource_info->value.asInteger = PRV_UPDATE_STATE_DOWNLOADED;
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

            ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_M_UPDATE_STATE, &lwm2m_data);
            lwm2m_free_instance_info(lwm2m_data.instance_info);
            if (ret_value == false ) {
              result = COAP_400_BAD_REQUEST;
            }
            else
            {

              result = COAP_204_CHANGED;
            }        
          }
        }
        break;
      case RES_O_PACKAGE_URI:
        if (PRV_UPDATE_STATE_INITIAL != instanceP->update_state)
        {
          return COAP_400_BAD_REQUEST;
        }
        memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
        id_set = 0;
        id_set = id_set | LWM2M_OBJECT_ID_E;
        id_set = id_set | LWM2M_INSTANCE_ID_E;
        id_set = id_set | LWM2M_RESOURCE_ID_E;

        lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
            lwm2m_data.instance_info->resource_info->resource_ID = RES_O_PACKAGE_URI;
            lwm2m_data.instance_info->resource_info->type = (lwm2m_data_type_t)dataArray[i].type;
            lwm2m_data.instance_info->resource_info->value.asBuffer.length  = dataArray[i].value.asBuffer.length;
//            lwm2m_data.instance_info->resource_info->value.asBuffer.buffer  = dataArray[i].value.asBuffer.buffer;

            if (lwm2m_data.instance_info->resource_info->value.asBuffer.length )
            {
              lwm2m_data.instance_info->resource_info->value.asBuffer.buffer = malloc (lwm2m_data.instance_info->resource_info->value.asBuffer.length);
              if (lwm2m_data.instance_info->resource_info->value.asBuffer.buffer == NULL )
              {
                return COAP_500_INTERNAL_SERVER_ERROR;

              }
              memscpy (lwm2m_data.instance_info->resource_info->value.asBuffer.buffer,lwm2m_data.instance_info->resource_info->value.asBuffer.length, \
                  dataArray[i].value.asBuffer.buffer ,lwm2m_data.instance_info->resource_info->value.asBuffer.length);                
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

        ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_O_PACKAGE_URI, &lwm2m_data);
        lwm2m_free_instance_info(lwm2m_data.instance_info);
        if (ret_value == false ) {
          result = COAP_400_BAD_REQUEST;
        }
        else
        {
          gpackage_running = true;                 
          result = COAP_204_CHANGED;
        }
        break;
        /* commented the break to address the dead code, as this case is currently not supported 
           break; */
      case RES_O_USERNAME:
        if (PRV_MAX_STRSIZE < (dataArray + i)->value.asBuffer.length)
          return COAP_400_BAD_REQUEST;
        LWM2M_STRCPY(instanceP->username, (const char *) ((dataArray + i)->value.asBuffer.buffer), 
            PRV_MAX_STRSIZE); 
        result = COAP_204_CHANGED;
        break;
      case RES_O_PASSWORD:
        if (PRV_MAX_STRSIZE < (dataArray + i)->value.asBuffer.length)
          return COAP_400_BAD_REQUEST;
        LWM2M_STRCPY(instanceP->password, (const char *) ((dataArray + i)->value.asBuffer.buffer), 
            PRV_MAX_STRSIZE); 
        result = COAP_204_CHANGED;
        break;
        case RES_M_PKG_NAME:
        case RES_M_PKG_VERSION:
        case RES_M_INSTALL:
          //Uncomment below line when checkpoint resource is supported
          //case RES_O_CHECKPOINT:
        case RES_M_UNINSTALL:
        case RES_M_UPDATE_STATE:
        case RES_M_UPDATE_RESULT:
        case RES_M_ACTIVATE:
        case RES_M_DEACTIVATE:
        case RES_M_ACTIVATION_STATE:
          //Uncomment below line when package setting resource is supported
          //case RES_O_PACKAGE_SETTINGS:
        case RES_O_STATUS_REASON:
        /*If create/write have one more one resource to write we write on reablable resource*/
        if(numData > 1)
          result = COAP_204_CHANGED;
        else
          result = COAP_405_METHOD_NOT_ALLOWED;
          break;

      default:
        /*If create/write have one more one resource to write we write on unknown/notsupported resource*/
        if(numData > 1)
          result = COAP_204_CHANGED;
        else
          result = COAP_404_NOT_FOUND;
    }
    i++;
  } while (i < numData); // && result == LWM2M_204_CHANGED); commented to fix KW

  return result;

}

/**
 * @brief This fucntion implements the execute operation performed on software
 * management object
 * @param instanceId (IN) Instance ID of the object to be read
 * @param resourceId (IN) Resource ID of the resource to be executed
 * @param buffer (IN) message payload from server fro execute operation
 * @param length (IN) message payload length
 * @param objectP Object on which the operation needs to be executed
 * @return LWM2M response type
 */
static uint8_t prv_softwaremgnt_execute(uint16_t instanceId,
    uint16_t resourceId,
    uint8_t * buffer,
    int length,
    lwm2m_object_t * objectP)
{
  software_mgnt_t *instanceP = NULL;
  bool ret_value = false;
  lwm2m_object_data_t lwm2m_data;
  uint8_t id_set = 0 ;
  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  instanceP = (software_mgnt_t *)LWM2M_LIST_FIND(objectP->instanceList, instanceId);
  if (NULL == instanceP) 
  	 return COAP_404_NOT_FOUND;

  switch(resourceId)
  {
    case RES_M_INSTALL:
      if (PRV_UPDATE_STATE_DELIVERED != instanceP->update_state)
      {
        return COAP_405_METHOD_NOT_ALLOWED; 
      }

      memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
      id_set = 0;
      id_set = id_set | LWM2M_OBJECT_ID_E;
      id_set = id_set | LWM2M_INSTANCE_ID_E;
      id_set = id_set | LWM2M_RESOURCE_ID_E;

      lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
          lwm2m_data.instance_info->resource_info->resource_ID = RES_M_INSTALL;
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

      ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_M_INSTALL, &lwm2m_data);
      lwm2m_free_instance_info(lwm2m_data.instance_info);
      if (ret_value == false ) {
        return COAP_500_INTERNAL_SERVER_ERROR;
      }
      else
      {    
        return COAP_204_CHANGED;
      }      

    case RES_M_UNINSTALL:
      if (PRV_UPDATE_STATE_INSTALLED != instanceP->update_state  && PRV_UPDATE_STATE_DELIVERED != instanceP->update_state)
      {
        return COAP_405_METHOD_NOT_ALLOWED; 
      }
      else 
      {
        if (PRV_UPDATE_STATE_INSTALLED == instanceP->update_state)
        {
          if (length != 0 && buffer != NULL && buffer[0] == '1')
          {
          }
          else //if (length != 0 && buffer != NULL && buffer[0] != '1')
          {
            /* TODO: Call package manager api to uninstall */
          }
        }
        else if (PRV_UPDATE_STATE_DELIVERED == instanceP->update_state)
        {
        }

        memset(instanceP->package, 0, PRV_MAX_STRSIZE);

        memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
        id_set = 0;
        id_set = id_set | LWM2M_OBJECT_ID_E;
        id_set = id_set | LWM2M_INSTANCE_ID_E;
        id_set = id_set | LWM2M_RESOURCE_ID_E;

        lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
            lwm2m_data.instance_info->resource_info->resource_ID = RES_M_UNINSTALL;
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

        ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_M_UNINSTALL, &lwm2m_data);
        lwm2m_free_instance_info(lwm2m_data.instance_info);
        if (ret_value == false ) {
          return COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {    
          return COAP_204_CHANGED;
        }        

      }

    case RES_M_ACTIVATE: 
      if ( (PRV_UPDATE_STATE_INSTALLED != instanceP->update_state )
          || (false != instanceP->activation_state) )
      {
        return COAP_405_METHOD_NOT_ALLOWED; 
      } 
      else 
      {
        memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
        id_set = 0;
        id_set = id_set | LWM2M_OBJECT_ID_E;
        id_set = id_set | LWM2M_INSTANCE_ID_E;
        id_set = id_set | LWM2M_RESOURCE_ID_E;

        lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
            lwm2m_data.instance_info->resource_info->resource_ID = RES_M_ACTIVATE;
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

        ret_value =  lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_M_ACTIVATE, &lwm2m_data);
        lwm2m_free_instance_info(lwm2m_data.instance_info);
        if (ret_value == false ) {
          return COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {    
          return COAP_204_CHANGED;
        }    
      }
    case RES_M_DEACTIVATE:
      if (true != instanceP->activation_state)
      {
        return COAP_405_METHOD_NOT_ALLOWED; 
      } 
      else
      { 
        memset(&lwm2m_data, 0, sizeof( lwm2m_object_data_t));
        id_set = 0;
        id_set = id_set | LWM2M_OBJECT_ID_E;
        id_set = id_set | LWM2M_INSTANCE_ID_E;
        id_set = id_set | LWM2M_RESOURCE_ID_E;

        lwm2m_data.object_ID = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
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
            lwm2m_data.instance_info->resource_info->resource_ID = RES_M_DEACTIVATE;
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

        ret_value = lwm2m_check_if_observe_and_update_app(id_set, LWM2M_SOFTWARE_MGNT_OBJECT_ID, instanceId, RES_M_DEACTIVATE, &lwm2m_data);
        lwm2m_free_instance_info(lwm2m_data.instance_info);
        if (ret_value == false ) {
          return COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {    
          return COAP_204_CHANGED;
        }        
      }
    case RES_M_PKG_NAME:
    case RES_M_PKG_VERSION:
    case RES_O_PACKAGE:
    case RES_O_PACKAGE_URI: 
      //Uncomment below line when checkpoint resource is supported
      //case RES_O_CHECKPOINT:
    case RES_M_UPDATE_STATE:
    case RES_M_UPDATE_RESULT:
    case RES_M_ACTIVATION_STATE:
          //Uncomment below line when package setting resource is supported
          //case RES_O_PACKAGE_SETTINGS:
    case RES_O_USERNAME:
    case RES_O_PASSWORD:
    case RES_O_STATUS_REASON:
      return COAP_405_METHOD_NOT_ALLOWED;

    default:
      return COAP_404_NOT_FOUND;         
  }
}

/*
 * @fn      static int32_t prv_softwaremgnt_getvalue ()
 * @brief   This function is used to  get the particular resource values for given instance 
 * @param   id_info      - indicates what values need to be filled whether resource or instance or object level 
 lwm2m_dataP  - pointer to lwm2m object information to be filled 
 *          targetP      - pointer to lwm2m object 
 * @return  on successs  -  0
 on failure   - -1
 */


int32_t prv_softwaremgnt_getvalue(lwm2m_id_info_t id_info, lwm2m_object_data_t * lwm2m_dataP , lwm2m_object_t * targetP)
{
  software_mgnt_t *instanceP = NULL; 
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

    resource_info = (lwm2m_resource_info_t *)malloc (sizeof (lwm2m_resource_info_t)); 
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

  instanceP = (software_mgnt_t *)LWM2M_LIST_FIND(targetP->instanceList, id_info.instance_ID);
  if (NULL == instanceP) 
    return result;

  if (numData == 0)
  {
    uint16_t resList[] = {
      RES_M_PKG_NAME,
      RES_M_PKG_VERSION,
      RES_M_UPDATE_STATE,
      RES_M_UPDATE_RESULT,
      RES_M_ACTIVATION_STATE,
      RES_O_STATUS_REASON
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
      resource_info = (lwm2m_resource_info_t *)malloc (sizeof (lwm2m_resource_info_t));
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
    free(dataArray);

  return result;
}

/*
 * @fn      static int32_t prv_softwaremgnt_setvalue ()
 * @brief   This function is used to  set the particular resource values for given instance 
 * @param   lwm2m_data - pointer to lwm2m object information
 *          targetP    - pointer to lwm2m object 
 * @return  on success - 1
 on failure - 0
 */

static int32_t prv_softwaremgnt_setvalue 
(
lwm2m_object_data_t * lwm2m_data,
lwm2m_object_t * targetP,
lwm2m_uri_t *uriP
)
{

  int result = 0;
  software_mgnt_t *instanceP = NULL;

  if (lwm2m_data == NULL || targetP == NULL)
  {
    return 0;
  }
  instanceP = (software_mgnt_t *)LWM2M_LIST_FIND(targetP->instanceList, lwm2m_data->instance_info->instance_ID);

  if (NULL == instanceP) return 0;
  switch (lwm2m_data->instance_info->resource_info->resource_ID) 
  {
    case RES_M_UPDATE_RESULT:
      {
        int64_t value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asInteger = lwm2m_data->instance_info->resource_info->value.asInteger;
        if (1 == lwm2m_data_decode_int(&dataP , &value))
        {
          instanceP->update_result = value;
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
    case RES_M_UPDATE_STATE: 
      {
        int64_t value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asInteger = lwm2m_data->instance_info->resource_info->value.asInteger;
        if (1 == lwm2m_data_decode_int(&dataP , &value))
        {
          instanceP->update_state = value;
          uriP->objectId   = lwm2m_data->object_ID;
          uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
          uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
          uriP->flag       = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID ;      

          if (instanceP->update_state == PRV_UPDATE_STATE_INITIAL || instanceP->update_state == PRV_UPDATE_STATE_DELIVERED ) {
            gpackage_running = false;
          }
          result = 1;
        }
        else
        {
          result = 0;
        }
      }
      break;
    case RES_M_ACTIVATION_STATE: 
      {
        int64_t value = 0;
        lwm2m_data_t dataP;
        dataP.type = (lwm2m_data_type_t)lwm2m_data->instance_info->resource_info->type ;
        dataP.value.asInteger = lwm2m_data->instance_info->resource_info->value.asInteger;
        if (1 == lwm2m_data_decode_int(&dataP , &value))
        {
          if(value == 0){
            instanceP->activation_state = 0;
          }else{
            instanceP->activation_state = 1;
          }
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

    default :
      result = 0;
      break;
  }
  return result;

}

/**
 * @brief This function is used to get the information regarding the Software
 * management object during client registartion
 * @param void
 * @return Device capability management object  
 */
lwm2m_object_t * get_object_software_mgnt()
{

  lwm2m_object_t * softwaremgntObj = NULL;
  softwaremgntObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));
  if(NULL != softwaremgntObj)
  {
    memset(softwaremgntObj, 0, sizeof(lwm2m_object_t));

    softwaremgntObj->objID  = LWM2M_SOFTWARE_MGNT_OBJECT_ID;
#if 0
    softwaremgntObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
    if(NULL == softwaremgntObj->instanceList)
    {
      lwm2m_free(softwaremgntObj);
      return NULL;
    }

    memset(softwaremgntObj->instanceList, 0, sizeof(lwm2m_list_t));
#endif
    softwaremgntObj->readFunc = prv_softwaremgnt_read;
  //  softwaremgntObj->discoverFunc = prv_softwaremgnt_discover;
    softwaremgntObj->writeFunc = prv_softwaremgnt_write;
    softwaremgntObj->executeFunc = prv_softwaremgnt_execute;
    softwaremgntObj->createFunc = prv_softwaremgnt_create;
    softwaremgntObj->deleteFunc = prv_softwaremgnt_delete;
    softwaremgntObj->setValueFunc = prv_softwaremgnt_setvalue;
    //softwaremgntObj->get_Value_Func = prv_softwaremgnt_getvalue;  
#if 0
    softwaremgntObj->userData = lwm2m_malloc(sizeof(software_mgnt_t));
    if(NULL == softwaremgntObj->userData )
    {
      lwm2m_free(softwaremgntObj);
      return NULL;
    }
    else
    {
      software_mgnt_t *sw_mgmt_data = (software_mgnt_t *)softwaremgntObj->userData;

      LWM2M_STRCPY(sw_mgmt_data->pkg_name, PRV_EMPTY, PRV_MAX_STRSIZE);
      LWM2M_STRCPY(sw_mgmt_data->pkg_version, PRV_EMPTY, PRV_MAX_STRSIZE);
      LWM2M_STRCPY(sw_mgmt_data->package, PRV_EMPTY, PRV_MAX_STRSIZE);
      LWM2M_STRCPY(sw_mgmt_data->package_uri, PRV_EMPTY, PRV_MAX_STRSIZE);
      sw_mgmt_data->update_state = PRV_UPDATE_STATE_INITIAL;
      sw_mgmt_data->update_supp_obj = PRV_UPDATE_SUPP_OBJ_FALSE;
      sw_mgmt_data->update_result = PRV_UPDATE_RES_INITIAL;
      sw_mgmt_data->activation_state = PRV_ACTIVATION_STATE_FALSE;
      LWM2M_STRCPY(sw_mgmt_data->username, PRV_EMPTY, PRV_MAX_STRSIZE);
      LWM2M_STRCPY(sw_mgmt_data->password, PRV_EMPTY, PRV_MAX_STRSIZE);
      LWM2M_STRCPY(sw_mgmt_data->status_reason, PRV_EMPTY, PRV_MAX_STRSIZE);
    }
#endif
  }
  /*
   * and its unique instance
   *
   */
  softwaremgntObj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
  if (NULL != softwaremgntObj->instanceList)
  {
	memset(softwaremgntObj->instanceList, 0, sizeof(lwm2m_list_t));
  }
  else
  {
	lwm2m_free(softwaremgntObj);
	return NULL;
  }

  return softwaremgntObj;
}

/**
 * @brief This fucntion frees the Software management object allocated
 * @param ObjectP (IN) Object to free
 * @return void
 */
void free_object_softwaremgnt(lwm2m_object_t * objectP)
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

#endif //ENABLE_SOFTWARE_MGNT_OBJ
