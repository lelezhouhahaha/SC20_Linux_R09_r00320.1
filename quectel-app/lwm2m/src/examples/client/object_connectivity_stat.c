/*******************************************************************************
 *
 * Copyright (c) 2015 Bosch Software Innovations GmbH Germany.
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
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    
 *******************************************************************************/

/*
 * This connectivity statistics object is optional and single instance only
 * 
 *  Resources:
 *
 *          Name         | ID | Oper. | Inst. | Mand.|  Type   | Range | Units | Description |
 *  SMS Tx Counter       |  0 |   R   | Single|  No  | Integer |       |       |             |
 *  SMS Rx Counter       |  1 |   R   | Single|  No  | Integer |       |       |             |
 *  Tx Data              |  2 |   R   | Single|  No  | Integer |       | kByte |             |
 *  Rx Data              |  3 |   R   | Single|  No  | Integer |       | kByte |             |
 *  Max Message Size     |  4 |   R   | Single|  No  | Integer |       | Byte  |             |
 *  Average Message Size |  5 |   R   | Single|  No  | Integer |       | Byte  |             |
 *  StartOrReset         |  6 |   E   | Single|  Yes | Integer |       |       |             |
 */

#include "liblwm2m.h"
#include "lwm2mclient.h"
#include "fcntl.h"
#include "internals.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Resource Id's:
#define RES_O_SMS_TX_COUNTER            0
#define RES_O_SMS_RX_COUNTER            1
#define RES_O_TX_DATA                   2
#define RES_O_RX_DATA                   3
#define RES_O_MAX_MESSAGE_SIZE          4
#define RES_O_AVERAGE_MESSAGE_SIZE      5
#define RES_M_START_OR_RESET            6
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
#define RES_M_STOP                      7
#define RES_O_COLLECTION_PERIOD         8
#endif

typedef struct
{
  uint16_t instanceId;
  int64_t   smsTxCounter;
  int64_t   smsRxCounter;
  int64_t   maxMessageSize;
  int64_t   prvTxDataByte;
  int64_t   txDataByte;             // report in kByte!
  int64_t   prvRxDataByte;
  int64_t   rxDataByte;             // report in kByte!
  int64_t   avrMessageSize;
  int64_t   prvMessageCount;
  int64_t   messageCount;           // private for incremental average calc.
  time_t    collectionPeriod;
  bool      collectDataStarted;
  bool      collectDataStopped;
} conn_s_data_t;

//static rex_timer_type collPeriodTimer;
static bool collectDataStopped;
static bool resetCollection;
bool conn_stat_create = FALSE;


/**
 * @fn static uint8_t prv_set_tlv()
 *
 * @brief This fucntion is used to populate the values of the resource
 *        to be provided to the server during the read operation on the object.
 * @param dataP data array for the resource
 * @param connStDataP connectivity statistics object data
 * @return LWM2M response type
 */
static uint8_t prv_set_tlv(lwm2m_data_t * dataP, conn_s_data_t * connStDataP)
{
  int64_t txDataByte = 0, rxDataByte = 0;

  // Do not fetch value and send until collection is started by server

  switch (dataP->id)
  {
    //  VZ_REQ_OTADMLWM2M_41068 Requirement
    case RES_O_SMS_TX_COUNTER:
      lwm2m_data_encode_int(connStDataP->smsTxCounter, dataP);
      return COAP_205_CONTENT;
      //commented the break to address the dead code, as this case is currently not supported
      //break;
    case RES_O_SMS_RX_COUNTER:
      lwm2m_data_encode_int(connStDataP->smsRxCounter, dataP);
      return COAP_205_CONTENT;
      // commented the break to address the dead code, as this case is currently not supported
      //break;

    case RES_O_TX_DATA:
      lwm2m_data_encode_int(connStDataP->txDataByte/1024, dataP);
      return COAP_205_CONTENT;
      // commented the break to address the dead code, as this case is currently not supported
      //break;
    case RES_O_RX_DATA:
      lwm2m_data_encode_int(connStDataP->rxDataByte/1024, dataP);
      return COAP_205_CONTENT;
      // commented the break to address the dead code, as this case is currently not supported
      // break
      // As interface is not available , so disabling , once interface available need to enable
    case RES_O_MAX_MESSAGE_SIZE:
    {
		if(lwm2m_check_is_att_netwrok() || lwm2m_check_is_vzw_netwrok())
		{
			lwm2m_data_encode_int(connStDataP->maxMessageSize, dataP);
			return COAP_205_CONTENT;
		}
		else
		{
			return	COAP_404_NOT_FOUND ;
		}
    }
    case RES_O_AVERAGE_MESSAGE_SIZE:
      lwm2m_data_encode_int(connStDataP->avrMessageSize, dataP);
      return COAP_205_CONTENT;
      // commented the break to address the dead code, as this case is currently not supported
      // break;
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
    case RES_O_COLLECTION_PERIOD:
      lwm2m_data_encode_int(connStDataP->collectionPeriod, dataP);
      return  COAP_205_CONTENT;
   case RES_M_START_OR_RESET :
   case RES_M_STOP  :
 #endif
      return  COAP_405_METHOD_NOT_ALLOWED;
    default:
      return  COAP_404_NOT_FOUND ;
  }
}

/**
 * @fn static void prv_resetCounter()
 *
 * @brief This function is used to reset counter
 *
 * @param objectP handle to connectivity statistics Object
 * @param start bool to indicate start/stop data collection
 *
 * @return void
 */
static void prv_resetCounter(lwm2m_object_t* objectP, bool start)
{
  conn_s_data_t *myData = (conn_s_data_t*) objectP->userData;

  if (NULL == myData)
  {
    return ;
  }


  if(start) //When start is executed
  {

    resetCollection = LWM2M_FALSE;
  }
  else // When reset is executed
  {
    myData->smsTxCounter        = 0;
    myData->smsRxCounter        = 0;
    myData->maxMessageSize      = 0;
    myData->prvTxDataByte       = 0;
    myData->txDataByte          = 0;
    myData->prvRxDataByte       = 0;
    myData->rxDataByte          = 0;
    myData->avrMessageSize      = 0;
    myData->prvMessageCount     = 0;
    myData->messageCount        = 0;
    myData->collectDataStarted  = start;
  }
}


/**
 * @fn static uint8_t prv_read()
 *
 * @brief This function implements the read operation performed on Connectivity
 * statistics object
 *
 * @param instanceId Instance ID of the object to be read
 * @param numDataP number of resources in the array
 * @param dataArrayP resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_read
(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t** dataArrayP,
lwm2m_object_t * objectP,
uint16_t resInstId
)
{
  uint8_t result = 0;
  int i = 0;

  if( numDataP == NULL || dataArrayP == NULL || objectP == NULL )
  {
    return  COAP_500_INTERNAL_SERVER_ERROR;
  }

  // this is a single instance object
  if (instanceId != 0)
  {
    return COAP_404_NOT_FOUND ;
  }

  if (resetCollection)
    prv_resetCounter(objectP, ((conn_s_data_t *)objectP->userData)->collectDataStarted);

  // is the server asking for the full object ?
  if (*numDataP == 0)
  {
    uint16_t resList[7] = {
      RES_O_TX_DATA,
      RES_O_RX_DATA,
      RES_O_AVERAGE_MESSAGE_SIZE,
      //  VZ_REQ_OTADMLWM2M_41068 Requirement
 #if 0
      RES_O_SMS_TX_COUNTER,
      RES_O_SMS_RX_COUNTER,
      //  VZ_REQ_OTADMLWM2M_41068 Requirement
      RES_O_MAX_MESSAGE_SIZE,
 #endif
 #ifdef CONNECTIVITY_STATS_ENHANCEMENT
      RES_O_COLLECTION_PERIOD,
  #endif
    };
    int nbRes = 5;		//= sizeof(resList) / sizeof(uint16_t);

	if(lwm2m_check_is_att_netwrok() || lwm2m_check_is_vzw_netwrok())
	{
		resList[0] = RES_O_SMS_TX_COUNTER;
		resList[1] = RES_O_SMS_RX_COUNTER;
		resList[2] = RES_O_TX_DATA;
		resList[3] = RES_O_RX_DATA;
		resList[4] = RES_O_MAX_MESSAGE_SIZE;
		resList[5] = RES_O_AVERAGE_MESSAGE_SIZE;
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
		resList[6] = RES_O_COLLECTION_PERIOD;

		nbRes = 7;
#else
		nbRes = 6;
#endif
	}

    *dataArrayP = lwm2m_data_new(nbRes);
    if (*dataArrayP == NULL)
      return COAP_500_INTERNAL_SERVER_ERROR ;
    *numDataP = nbRes;
    for (i = 0; i < nbRes; i++)
    {
      (*dataArrayP)[i].id = resList[i];
    }
  }

  i = 0;
  do
  {
    result = prv_set_tlv((*dataArrayP) + i, (conn_s_data_t*) (objectP->userData));
    i++;
  } while (i < *numDataP && result == COAP_205_CONTENT);

  return result;
}

/**
 * @fn static uint8_t prv_write()
 *
 * @brief This fucntion implements the write operation performed on Connectivity
 *        Statistics object
 *
 * @param instanceId Instance ID of the object to be read
 * @param numData number of resources in the array
 * @param dataArray resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_write
(
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP,
uint16_t resInstId,
uint8_t write_method
)
{
  conn_s_data_t *connStDataP = NULL;
  int i = 0;
  int64_t val = 0;

  if (NULL == dataArray || NULL == objectP)
  {
    return  COAP_500_INTERNAL_SERVER_ERROR;
  }

  // this is a single instance object
  if (instanceId != 0)
  {
    return  COAP_404_NOT_FOUND ;
  }

  connStDataP = (conn_s_data_t *)objectP->userData;
  if (NULL == connStDataP)
  	 return  COAP_404_NOT_FOUND ;

  for (i = 0; i < numData; i++)
  {
    switch (dataArray[i].id)
    {
  #ifdef CONNECTIVITY_STATS_ENHANCEMENT
      case RES_O_COLLECTION_PERIOD:
        if (1 == lwm2m_data_decode_int(dataArray + i, &val))
        {
          if (val >= 0 || val <= 0xFFFFFFFF)
          {
            connStDataP->collectionPeriod = (time_t)val;
            break;
          }
          else
          {
            return COAP_400_BAD_REQUEST;
          }
        }
        else
        {
          return COAP_400_BAD_REQUEST;
        }
#endif
      case RES_O_RX_DATA:
      case RES_O_TX_DATA:
      case RES_O_AVERAGE_MESSAGE_SIZE:
      case RES_M_START_OR_RESET:
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
      case RES_M_STOP:
        /*If create/write have one more one resource to write we write on reablable resource*/
        if(numData > 1)
          break;
        else
          return COAP_405_METHOD_NOT_ALLOWED;
#endif
      default:
        /*If create/write have one more one resource to write we write on unknown/notsupported resource*/
        if(numData > 1)
          break;
        else
          return COAP_404_NOT_FOUND;
    }
  }
  return COAP_204_CHANGED;
}

#ifdef CONNECTIVITY_STATS_ENHANCEMENT
/**
 * @fn static uint8_t prv_discover()
 *
 * @brief This fucntion implements the discover operation performed on the
 *     Connectivity Statistics object
 *
 * @param instanceId Instance ID of the object to be read
 * @param numDataP number of resources in the array
 * @param dataArrayP resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 *
 */
static uint8_t prv_discover
(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t ** dataArrayP,
lwm2m_object_t * objectP
)
{
  uint8_t result;
  int i;

  // this is a single instance object
  if (instanceId != 0)
  {
    return COAP_404_NOT_FOUND;
  }

  result = COAP_205_CONTENT;

  // is the server asking for the full object ?
  if (*numDataP == 0)
  {
    uint16_t resList[] = {
      //  VZ_REQ_OTADMLWM2M_41068 Requirement
      RES_O_SMS_TX_COUNTER,
      RES_O_SMS_RX_COUNTER,
      RES_O_TX_DATA,
      RES_O_RX_DATA,
      //RES_O_MAX_MESSAGE_SIZE is not supported in TX
      //  VZ_REQ_OTADMLWM2M_41068 Requirement
      RES_O_MAX_MESSAGE_SIZE,
      RES_O_AVERAGE_MESSAGE_SIZE,
      RES_M_START_OR_RESET,
 #ifdef CONNECTIVITY_STATS_ENHANCEMENT
      RES_M_STOP,
      RES_O_COLLECTION_PERIOD
  #endif
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
        //  VZ_REQ_OTADMLWM2M_41068 Requirement
        case RES_O_SMS_TX_COUNTER:
        case RES_O_SMS_RX_COUNTER:
        case RES_O_TX_DATA:
        case RES_O_RX_DATA:
          //  VZ_REQ_OTADMLWM2M_41068 Requirement
        case RES_O_MAX_MESSAGE_SIZE:
        case RES_O_AVERAGE_MESSAGE_SIZE:
        case RES_M_START_OR_RESET:
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
        case RES_M_STOP:
        case RES_O_COLLECTION_PERIOD:
#endif
          break;
        default:
          result = COAP_404_NOT_FOUND;
      }
    }
  }

  return result;
}
#endif


/**
 * @fn static uint8_t prv_exec()
 *
 * @brief This fucntion implements the execute operation performed on Connectivity
 *        Statistics object
 * @param instanceId Instance ID of the object to be read
 * @param resourceId Resource ID of the resource to be executed
 * @param buffer message payload from server fro execute operation
 * @param length message payload length
 * @param objectP Object on which the operation needs to be executed
 *
 * @return LWM2M response type
 *
 */
static uint8_t prv_exec
(
uint16_t instanceId,
uint16_t resourceId,
uint8_t * buffer,
int length,
lwm2m_object_t * objectP
)
{
  // this is a single instance object
  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  if (instanceId != 0)
  {
    return COAP_404_NOT_FOUND;
  }

  switch (resourceId)
  {

    case RES_M_START_OR_RESET:
      {
        conn_s_data_t *dataP = (conn_s_data_t *)objectP->userData;
        prv_resetCounter(objectP, LWM2M_TRUE);
        collectDataStopped = LWM2M_FALSE;

        if (0 != dataP->collectionPeriod)
          //timer_set(&collPeriodTimer, dataP->collectionPeriod, 0, T_SEC);

        resetCollection = LWM2M_FALSE;
        return COAP_204_CHANGED;
      }
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
    case RES_M_STOP:

      collectDataStopped = LWM2M_TRUE;
      //rex_clr_timer(&collPeriodTimer);
      return COAP_204_CHANGED;
    case RES_O_COLLECTION_PERIOD:
#endif
    case RES_O_RX_DATA:
    case RES_O_TX_DATA:
    case RES_O_AVERAGE_MESSAGE_SIZE:
      return COAP_405_METHOD_NOT_ALLOWED;

    default:
      return COAP_404_NOT_FOUND;
  }
}

void conn_s_updateTxStatistic(lwm2m_object_t * objectP, uint16_t txDataByte, bool smsBased)
{
    conn_s_data_t* myData = (conn_s_data_t*) (objectP->userData);
    if (myData->collectDataStarted)
    {
        myData->txDataByte += txDataByte;
        myData->messageCount++;
        myData->avrMessageSize = (myData->txDataByte+myData->rxDataByte) /
                                  myData->messageCount;
        if (txDataByte > myData->maxMessageSize)
            myData->maxMessageSize = txDataByte;
        if (smsBased) myData->smsTxCounter++;
    }
}

void conn_s_updateRxStatistic(lwm2m_object_t * objectP, uint16_t rxDataByte, bool smsBased)
{
    conn_s_data_t* myData = (conn_s_data_t*) (objectP->userData);
    if (myData->collectDataStarted)
    {
        myData->rxDataByte += rxDataByte;
        myData->messageCount++;
        myData->avrMessageSize = (myData->txDataByte+myData->rxDataByte) /
                                  myData->messageCount;
        if (rxDataByte > myData->maxMessageSize)
            myData->maxMessageSize = rxDataByte;
        myData->txDataByte += rxDataByte;
        if (smsBased) myData->smsRxCounter++;
    }
}

/**
 * @fn static void prvCollPeriodTmrCB()
 *
 * @brief This function is used to stop collection period
 *
 * @param time_ms time in ms
 * @param userData handle to callback
 *
 * @return void
 *
 */
//static void prvCollPeriodTmrCB(timer_cb_data_type userData )
//{
//  collectDataStopped = TRUE;
//}


/**
 * @fn static uint8_t prv_create()
 *
 * @brief This fucntion is used to create Connectivity Statistics object
 *
 * @param instanceId Instance ID of the object to be created
 * @param numData number of resources in the array
 * @param dataArray resource value array
 * @param objectP Object to be read
 *
 * @return LWM2M response type
 */
static uint8_t prv_create
(
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP
)
{
  conn_s_data_t *connObj = NULL;
  uint8_t result = 0;

  if( dataArray == NULL || objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  /*check if already one instance is created */
  if(objectP->instanceList != NULL)
  {
    return COAP_400_BAD_REQUEST;
  }

  connObj = (conn_s_data_t*) lwm2m_malloc(sizeof(conn_s_data_t));
  if( NULL == connObj)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  connObj->instanceId = instanceId;
  prv_resetCounter(objectP, FALSE);
  

  objectP->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
  if (NULL != objectP->instanceList)
  {
    memset(objectP->instanceList, 0x00, sizeof(lwm2m_list_t));
  }

  objectP->userData = connObj;

  result = prv_write(instanceId, numData, dataArray, objectP, INVALID_RES_INST_ID, COAP_POST);
  if (result != COAP_204_CHANGED)
  {
    // Issue in creating object instanace so retuning Bad request error.
    result = COAP_400_BAD_REQUEST;
    lwm2m_free(objectP->instanceList);
    objectP->instanceList = NULL;
    lwm2m_free(connObj);
    objectP->userData = NULL;
    return result;
  }

  unlink(LWM2M_CONN_STAT_OBJ_PERSISTENCE_PATH);
  return COAP_201_CREATED;
}
/**
 * @fn static uint8_t prv_delete()
 *
 * @brief This fucntion is used to delete Connectivity Statistics object
 *
 * @param id Instance ID of the object to be deleted
 * @param objectP Object to be deleted
 *
 * @return LWM2M response type
 */
static uint8_t prv_delete
(
uint16_t id,
lwm2m_object_t * objectP
)
{
  int fd = 0;
  conn_s_data_t * connObj = NULL;

  if( objectP == NULL ) 
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }


  objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id, (lwm2m_list_t **)&connObj);
  if (NULL == connObj) 
  	return COAP_404_NOT_FOUND;

  lwm2m_free(connObj);
  connObj = NULL;

  /*  Update single instance object persistence */

  fd = open( LWM2M_CONN_STAT_OBJ_PERSISTENCE_PATH, O_CREAT, 00744);
  if( EFAILURE == fd)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }
  close(fd);

  conn_stat_create = TRUE;
  free_object_conn_s(objectP);
  return COAP_202_DELETED;

}

/**
 * @fn lwm2m_object_t * get_object_conn_s()
 *
 * @brief This function is used to get the information about the Connectivity
 *        Statistics object during client registartion
 *
 * @param void
 *
 * @return Connectivity Statistics object  
 *
 */
lwm2m_object_t * get_object_conn_s(void)
{
  /*
   * The get_object_conn_s() function create the object itself and return
   * a pointer to the structure that represent it.
   */
  lwm2m_object_t * connObj = NULL;
  int fd = 0;

  connObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

  if (NULL != connObj)
  {
    memset(connObj, 0, sizeof(lwm2m_object_t));

    /*
     * It assign his unique ID
     * The 7 is the standard ID for the optional object "Connectivity Statistics".
     */
    connObj->objID = LWM2M_CONN_STATS_OBJECT_ID;

    /*
     * And the private function that will access the object.
     * Those function will be called when a read/execute/close
     * query is made by the server or core. In fact the library don't need
     * to know the resources of the object, only the server does.
     */
    connObj->readFunc     = prv_read;
    connObj->writeFunc    = prv_write;
    connObj->executeFunc  = prv_exec;
    connObj->createFunc   = prv_create;
    connObj->deleteFunc   = prv_delete;
#ifdef CONNECTIVITY_STATS_ENHANCEMENT
    connObj->discoverFunc = prv_discover;
#endif

    /*  Check if host_device_obj_single_instance_pr file exist */
    fd = open(LWM2M_CONN_STAT_OBJ_PERSISTENCE_PATH, O_RDONLY);
    if(fd != EFAILURE)
    {
      close(fd);
      return connObj;
    }

    connObj->instanceList = lwm2m_malloc(sizeof(lwm2m_list_t));
    if (NULL != connObj->instanceList)
    {
      memset(connObj->instanceList, 0, sizeof(lwm2m_list_t));
    }
    else 
    {
      lwm2m_free(connObj);
      return NULL;
    }

    connObj->userData     = lwm2m_malloc(sizeof(conn_s_data_t));

    /*
     * Also some user data can be stored in the object with a private
     * structure containing the needed variables.
     */
    if (NULL != connObj->userData)
    {
      memset(connObj->userData, 0, sizeof(conn_s_data_t));
      prv_resetCounter(connObj, FALSE);

	  //rex_def_timer_ex(&collPeriodTimer, prvCollPeriodTmrCB,(unsigned long)connObj);
    }
    else
    {
      lwm2m_free(connObj);
      connObj = NULL;
    }
  }
  return connObj;
}

/**
 * @fn void free_object_conn_s()
 *
 * @brief This fucntion frees the Connectivity Statistics object allocated
 * @param ObjectP Object to free
 *
 * @return void
 */
void free_object_conn_s(lwm2m_object_t * objectP)
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

  if( !conn_stat_create)
    lwm2m_free(objectP);

}


/**
 * @fn uint8_t connectivity_moni_change()
 *
 * @brief This fucntion changes the Connectivity monitoring object 
 *        resource values
 * @param dataArray resource value array
 * @param ObjectP Object to change
 *
 * @return void
 */
uint8_t connectivity_stat_change(lwm2m_data_t * dataArray,
    lwm2m_object_t * objectP)
{
  uint8_t result = 0;
  //conn_m_data_t * data = NULL;

  if (dataArray == NULL || objectP == NULL)
  {
    return COAP_500_INTERNAL_SERVER_ERROR;
  }

  switch (dataArray->id)
  {
    case RES_O_TX_DATA:
    case RES_O_RX_DATA:
    case RES_O_AVERAGE_MESSAGE_SIZE:
      result = COAP_204_CHANGED;
      break;
    default:
      result = COAP_404_NOT_FOUND;
      break;
  }
  return result;
}

