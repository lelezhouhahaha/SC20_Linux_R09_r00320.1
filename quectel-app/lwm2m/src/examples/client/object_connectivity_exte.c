/******************************************************************************

  @file    object_connectivity_exte.c
  @brief   File contains the implementation for Connectivity Extension object
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
#include "quectel_getinfo.h"

// Resource Id's:
#define RES_O_ICCID            			1
#define RES_O_IMSI						2
#define RES_O_MSISDN					3
#define RES_O_APN_RETRIES				4
#define RES_O_APN_RETRY_PERIOD			5
#define RES_O_APN_RETRY_BACKOFF			6
#define RES_O_SINR						7
#define RES_O_SRXLEV					8
#define RES_O_CE_MODE					9

//#define RES_O_CONN_EXTE         		4

typedef struct conn_ex_data_
{
  uint16_t instanceId;
  char   	connExICCID[64];
  char   	connExIMSI[64];
  char   	connExMSISDN[64];
  int16_t   connExApnRetries;
  int64_t   connExApnRetryPeriod;
  int64_t   connExApnRetryBackoff;
  int16_t   connExSINR;
  int64_t   connExSRXLEV;
  uint8_t   connExCeMode;
} conn_ex_data_t;


void free_object_conn_ex(lwm2m_object_t * object);

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
    conn_ex_data_t* conn_ex_dataP)
{
	uint8_t ret = COAP_205_CONTENT; 
  
	if( dataP == NULL || conn_ex_dataP == NULL ) 
	{
		return COAP_500_INTERNAL_SERVER_ERROR;
	}

	switch (dataP->id)     
	{
		case RES_O_ICCID:
			lwm2m_data_encode_string((const char *)conn_ex_dataP->connExICCID, dataP);
			break;
		case RES_O_IMSI:
			lwm2m_data_encode_string((const char *)conn_ex_dataP->connExIMSI, dataP);
			break;
		case RES_O_MSISDN:
			lwm2m_data_encode_string((const char *)conn_ex_dataP->connExMSISDN, dataP);
			break;
		case RES_O_APN_RETRIES:
			//lwm2m_data_encode_int(conn_ex_dataP->connExApnRetries, dataP);
                {
                    lwm2m_data_t * subTlvP = NULL;
                    subTlvP = lwm2m_data_new(2);
                    subTlvP[0].id = 0;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetries, &subTlvP[0]);
                    subTlvP[1].id = 1;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetries, &subTlvP[1]);
                    lwm2m_data_encode_instances(subTlvP, 2, dataP);
                }
			break;
		case RES_O_APN_RETRY_PERIOD:
                {
                    lwm2m_data_t * subTlvP = NULL;
                    subTlvP = lwm2m_data_new(2);
                    subTlvP[0].id = 0;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetryPeriod, &subTlvP[0]);
                    subTlvP[1].id = 1;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetryPeriod, &subTlvP[1]);
                    lwm2m_data_encode_instances(subTlvP, 2, dataP);
                }
			break;
		case RES_O_APN_RETRY_BACKOFF:
                {
                    lwm2m_data_t * subTlvP = NULL;
                    subTlvP = lwm2m_data_new(2);
                    subTlvP[0].id = 0;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetryBackoff, &subTlvP[0]);
                    subTlvP[1].id = 1;
                    lwm2m_data_encode_int(conn_ex_dataP->connExApnRetryBackoff, &subTlvP[1]);
                    lwm2m_data_encode_instances(subTlvP, 2, dataP);
                }
			break;
		case RES_O_SINR:
			lwm2m_data_encode_int(conn_ex_dataP->connExSINR, dataP);
			break;
		case RES_O_SRXLEV:
			lwm2m_data_encode_int(conn_ex_dataP->connExSRXLEV, dataP);
			break;
		case RES_O_CE_MODE:
			lwm2m_data_encode_int(conn_ex_dataP->connExCeMode, dataP);
			break;
           // case RES_O_CONN_EXTE:
                // {
                    // lwm2m_data_t * subTlvP = NULL;
                    // subTlvP = lwm2m_data_new(4);
                    // subTlvP[0].id = 0;
                    // lwm2m_data_encode_int(conn_ex_dataP->connExApnRetryPeriod, &subTlvP[0]);
                    // subTlvP[1].id = 1;
                    // lwm2m_data_encode_int(conn_ex_dataP->connExSINR, &subTlvP[1]);
                    // subTlvP[2].id = 2;
                    // lwm2m_data_encode_int(conn_ex_dataP->connExSRXLEV, &subTlvP[2]);
                    // subTlvP[3].id = 3;
                    // lwm2m_data_encode_int(conn_ex_dataP->connExCeMode, &subTlvP[3]);
                    // lwm2m_data_encode_instances(subTlvP, 4, dataP);
                // }
                // break;
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
static uint8_t prv_conn_ex_read
(
uint16_t instanceId,
int*  numDataP,
lwm2m_data_t** dataArrayP,
lwm2m_object_t*  objectP,
uint16_t resInstId
)
{
  int     i = 0;
  uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
  
  if( dataArrayP == NULL || objectP == NULL || numDataP == NULL) 
  {
    return  COAP_400_BAD_REQUEST;
  }
  
  // this is a single instance object
  if (instanceId != 0)
  {
    return COAP_404_NOT_FOUND ;
  }
  
  if (*numDataP == 0)     // full object, readable resources!
  {
    uint16_t readResIds[] = {
		RES_O_ICCID,
		RES_O_IMSI,
		RES_O_MSISDN,
		RES_O_APN_RETRIES,
		RES_O_APN_RETRY_PERIOD,
        RES_O_APN_RETRY_BACKOFF,
		RES_O_SINR,
		RES_O_SRXLEV,
		RES_O_CE_MODE
    }; // readable resources!

    *numDataP  = sizeof(readResIds)/sizeof(uint16_t);
    *dataArrayP = lwm2m_data_new(*numDataP);
    if (*dataArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;

    // init readable resource id's
    for (i = 0 ; i < *numDataP ; i++)
    {
      (*dataArrayP)[i].id = readResIds[i];
    }
  }

  for (i = 0 ; i < *numDataP ; i++)
  {
    result = prv_res2tlv ((*dataArrayP)+i, (conn_ex_data_t*)(objectP->userData));
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
static uint8_t prv_conn_ex_discover
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
	
	// this is a single instance object
	if (instanceId != 0)
	{
		return COAP_404_NOT_FOUND;
	}

	if(*numDataP == 0)
	{
		uint16_t resList[] =
		{
			RES_O_ICCID,
			RES_O_IMSI,
			RES_O_MSISDN,
			RES_O_APN_RETRIES,
			RES_O_APN_RETRY_PERIOD,
            RES_O_APN_RETRY_BACKOFF,
			RES_O_SINR,
			RES_O_SRXLEV,
			RES_O_CE_MODE
		};
		*numDataP = sizeof(resList) / sizeof(uint16_t);

		*dataArrayP = lwm2m_data_new(*numDataP);
		if (*dataArrayP == NULL) 
		return COAP_500_INTERNAL_SERVER_ERROR;

		for (i = 0; i < *numDataP; i++)
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
				case RES_O_ICCID:
				case RES_O_IMSI:
				case RES_O_MSISDN:
				case RES_O_APN_RETRIES:
				case RES_O_APN_RETRY_PERIOD:
                case RES_O_APN_RETRY_BACKOFF:
				case RES_O_SINR:
				case RES_O_SRXLEV:
				case RES_O_CE_MODE:
					break;
				default:
					result = COAP_404_NOT_FOUND;
			}
		}
	}

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
static uint8_t prv_conn_ex_write 
(
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray, 
lwm2m_object_t * objectP, 
uint16_t resInstId, 
uint8_t write_method
)
{
	conn_ex_data_t *conn_ex_dataP = NULL;
	int i = 0;
	int64_t val_i = 0;
	uint8_t val_s[64] = {0};

	if (NULL == dataArray || NULL == objectP)
	{
		return  COAP_500_INTERNAL_SERVER_ERROR;
	}

	// this is a single instance object
	if (instanceId != 0)
	{
		return  COAP_404_NOT_FOUND ;
	}

	conn_ex_dataP = (conn_ex_data_t *)objectP->userData;
	if (NULL == conn_ex_dataP) 
		return  COAP_404_NOT_FOUND ;

	for (i = 0; i < numData; i++)
	{
		switch (dataArray[i].id)
		{
			case RES_O_ICCID:
			case RES_O_IMSI:
			case RES_O_MSISDN:
				/*If create/write have one more one resource to write we write on reablable resource*/
				if(numData > 1)
					break;
				else
					return COAP_405_METHOD_NOT_ALLOWED;
				
			case RES_O_APN_RETRIES:
				if (1 == lwm2m_data_decode_int(dataArray + i, &val_i))
				{
					conn_ex_dataP->connExApnRetries = val_i;
				}
				else
				{
					return COAP_400_BAD_REQUEST;
				}
				break;
			case RES_O_APN_RETRY_PERIOD:
				if (1 == lwm2m_data_decode_int(dataArray + i, &val_i))
				{
					conn_ex_dataP->connExApnRetryPeriod = val_i;
				}
				else
				{
					return COAP_400_BAD_REQUEST;
				}
				break;
            //case RES_O_APN_RETRY_BACKOFF:
			case RES_O_SINR:
				if (1 == lwm2m_data_decode_int(dataArray + i, &val_i))
				{
					conn_ex_dataP->connExSINR = val_i;
				}
				else
				{
					return COAP_400_BAD_REQUEST;
				}
				break;
			case RES_O_SRXLEV:
				if (1 == lwm2m_data_decode_int(dataArray + i, &val_i))
				{
					conn_ex_dataP->connExSRXLEV = val_i;
				}
				else
				{
					return COAP_400_BAD_REQUEST;
				}
				break;
			case RES_O_CE_MODE:
				if (1 == lwm2m_data_decode_int(dataArray + i, &val_i))
				{
					conn_ex_dataP->connExCeMode = val_i;
				}
				else
				{
					return COAP_400_BAD_REQUEST;
				}
				break;

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

/**
 * @fn lwm2m_object_t * get_object_conn_ex()
 *
 * @brief This function is used to get the information regarding the Host
 *        Device object during client registartion
 *
 * @param void
 *
 * @return Host Device object  
 *
 */
// extern boolean FUNC_SIM_GetIccid(uint8 * pICCID, uint16 uIccidMaxLen);
// extern boolean FUNC_SIM_GetImsiGsmS2(uint8 * pImsi, uint16 uImsiMaxLen);
// extern boolean FUNC_SIM_GetMsisdnGsmNum(char *pNum,uint16 uNumMaxLen);

lwm2m_object_t * get_object_conn_ex(void)
{
  lwm2m_object_t * conn_ex_objP = NULL;
  int instance_id = 0;

  conn_ex_objP = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
  if(NULL != conn_ex_objP)
  {
    memset(conn_ex_objP, 0, sizeof(lwm2m_object_t));

    // It assigns its unique ID
    // The 10255 is the standard ID for the optional object "Host Device".

    conn_ex_objP->objID = LWM2M_CONN_EXTEN_OBJECT_ID;

    // And the private function that will access the object.
    // Those function will be called when a read query is made by the server.
    // In fact the library don't need to know the resources of the object, only the server does.

    conn_ex_objP->readFunc    	= prv_conn_ex_read;
    conn_ex_objP->discoverFunc	= prv_conn_ex_discover;
    conn_ex_objP->writeFunc  	= prv_conn_ex_write;


	conn_ex_objP->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
	if (NULL != conn_ex_objP->instanceList)
	{
	  memset(conn_ex_objP->instanceList, 0, sizeof(lwm2m_list_t));
	}
	else
	{
	  lwm2m_free(conn_ex_objP);
	  return NULL;
	}
	
	conn_ex_objP->userData = lwm2m_malloc(sizeof(conn_ex_data_t));
	if (NULL != conn_ex_objP->userData)
	{
//		uint16 msisdnLen = 0;
		
		conn_ex_data_t *conn_ex_dataP = (conn_ex_data_t*)conn_ex_objP->userData;
		memset(conn_ex_dataP, 0, sizeof(conn_ex_data_t));

		// FUNC_SIM_GetIccid((uint8 *)conn_ex_dataP->connExICCID, 64);
		// FUNC_SIM_GetImsiGsmS2((uint8 *)conn_ex_dataP->connExIMSI, 64);
		// FUNC_SIM_GetMsisdnGsmNum(conn_ex_dataP->connExMSISDN, 64);
//		/*		data init
		memset(conn_ex_dataP->connExICCID, 0, 64);
        //strcpy(conn_ex_dataP->connExICCID, "89014104279202589792");
        //strcpy((char *)conn_ex_dataP->connExICCID, "89860082191449603747");
        QL_LWM2M_GetIccid((char *)conn_ex_dataP->connExICCID);
		memset(conn_ex_dataP->connExIMSI, 0, 64);
        //strcpy(conn_ex_dataP->connExIMSI, "083901142077418812");
        //strcpy((char *)conn_ex_dataP->connExIMSI, "310410123456789");
        QL_LWM2M_GetImsi((char *)conn_ex_dataP->connExIMSI);
		memset(conn_ex_dataP->connExMSISDN, 0, 64);
        //strcpy(conn_ex_dataP->connExMSISDN, "4566688");
        //strcpy((char *)conn_ex_dataP->connExMSISDN, "310410123456789");
        GetDevicePhoneNumber((char *)conn_ex_dataP->connExMSISDN);
//		*/

        // conn_ex_dataP->connExApnRetries = 1;
        // conn_ex_dataP->connExApnRetryPeriod = 2;
        // conn_ex_dataP->connExSINR = 33;
        // conn_ex_dataP->connExSRXLEV = 44;
        // conn_ex_dataP->connExCeMode = 55;


	}
	else
	{
		lwm2m_free(conn_ex_objP);
		conn_ex_objP = NULL;
	}
  }
  return conn_ex_objP;
}

/**
 * @fn void free_object_host_device()
 *
 * @brief This fucntion frees the Host Device object allocated
 * @param ObjectP Object to free
 *
 * @return void
 */
void free_object_conn_ex(lwm2m_object_t * objectP)
{
	if (objectP == NULL) 
	return;

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
