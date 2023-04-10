
/*===========================================================================
	Copyright(c) 2019 Quectel Wierless Solution,Co.,Ltd. All Rights Reserved.
	Quectel Wireless Solution Proprietary and Confidential.
============================================================================*/
/*===========================================================================
							EDIT HISTORY FOR MODULE
This section contains comments describing changees made to the module.
Notice that changes are listed in reverse chronological order.
WHEN		 WHO		WHAT,WHERE,WHY
----------	-----		----------------------------------------------------
10/05/2019	Herry	  	Modify file for supporting lwm2m object.
============================================================================*/

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lwm2m_configure.h"
#include "object_apn_connection_profile.h"
#include "object_access_control.h"
//#include "quectel_apptcpip_netmgr.h"
#include "lwm2mclient.h"
#include "internals.h"
#define RES_M_PROFILE_NAME                        0
#define RES_M_APN                                 1
#define RES_M_ENABLE_STATUS                       3
#define RES_M_CONN_ESTABLISH_TIME                 9
#define RES_M_CONN_ESTABLISH_RESULT               10
#define RES_M_CONN_ESTABLISH_REJECT_CAUSE         11
#define RES_M_CONN_END_TIME                       12

typedef struct  apn_conn_profile_data_{
    struct apn_conn_profile_data_ *next;
    uint16_t instanceId;
	char      profile_name[128];
	char      apn[128];
	uint8_t   apn_status;
    char      profile_id[128];
	// resource_instance_int_list_t*    establish_time;
	// resource_instance_int_list_t*    establish_result; // 1: true  0:false
	// resource_instance_int_list_t*    establish_rej_cause;
	// resource_instance_int_list_t*    establish_end_time;	
}apn_conn_profile_data_t;

static uint8_t prv_apn_conn_set_value
(
lwm2m_data_t * dataP,
apn_conn_profile_data_t * connDataP
)
{
    switch (dataP->id)
    {
    case RES_M_PROFILE_NAME:
    	{
			lwm2m_data_encode_string(connDataP->profile_name, dataP);
    	}
		return COAP_205_CONTENT;

    case RES_M_APN:
	    {
			// extern int lwm2m_client_get_pdpcid(int client_hdl);
			// apptcpip_pdp_profile_param_struct  pdp_profile;
			
			// memset((void *)&pdp_profile, 0, sizeof(apptcpip_pdp_profile_param_struct));
			// apptcpip_netif_get_pdp_profile(lwm2m_client_get_pdpcid((int)(((lwm2m_context_t *)ctx_p)->userData)), &pdp_profile);
			
			// if(pdp_profile.apn[0])
			// {
		       	// lwm2m_data_encode_string((const char *)pdp_profile.apn, dataP);
			// }
			// else
			// {
				lwm2m_data_encode_string(connDataP->apn, dataP);
			//}
	    }
	    return COAP_205_CONTENT;

    case RES_M_ENABLE_STATUS: //s-int
    	{
        	lwm2m_data_encode_int(connDataP->apn_status, dataP);
    	}
		return COAP_205_CONTENT;

    case RES_M_CONN_ESTABLISH_TIME: //s-int
    	{
			// resource_instance_int_list_t *estab_time =NULL;
			// int estab_time_cnt = 0;
			 lwm2m_data_t* subTlvP = NULL;
			
			// for (estab_time = connDataP->establish_time, estab_time_cnt=0; estab_time!=NULL; estab_time = estab_time->next, estab_time_cnt++);
			// if(estab_time_cnt == 0)
			// {
				// if(TRUE == lwm2m_check_is_att_netwrok())
				// {
					 subTlvP = lwm2m_data_new(1);
					 subTlvP->id = 0;
					 lwm2m_data_encode_int(0, subTlvP);
					 lwm2m_data_encode_instances(subTlvP, 1, dataP);
					 return COAP_205_CONTENT ;
				// }
				// else
            		// return COAP_404_NOT_FOUND;
			// }

			// subTlvP = lwm2m_data_new(estab_time_cnt);
			// estab_time_cnt = 0;
			// for (estab_time =  connDataP->establish_time; estab_time!= NULL;estab_time = estab_time->next)
            // {
            	// subTlvP[estab_time_cnt].id = (uint16_t)estab_time->resInstId;
            	// lwm2m_data_encode_int(estab_time->InstValue, &subTlvP[estab_time_cnt]);
				// estab_time_cnt++;
			// }
			 //lwm2m_data_encode_instances(subTlvP, 0, dataP);
    	 }
		//return COAP_205_CONTENT ;

    case RES_M_CONN_ESTABLISH_RESULT: //s-int
    	 {
			// resource_instance_int_list_t *estab_ret =NULL;
			// int estab_ret_cnt = 0;
			 lwm2m_data_t* subTlvP = NULL;
			
			// for (estab_ret = connDataP->establish_result, estab_ret_cnt=0; estab_ret!=NULL; estab_ret = estab_ret->next, estab_ret_cnt++);
			// if(estab_ret_cnt == 0)
			// {
				// if(TRUE == lwm2m_check_is_att_netwrok())
				// {
					 subTlvP = lwm2m_data_new(1);
					 subTlvP->id = 0;
					 lwm2m_data_encode_int(0, subTlvP);
					 lwm2m_data_encode_instances(subTlvP, 1, dataP);
					 return COAP_205_CONTENT ;
				// }
				// else
            		// return COAP_404_NOT_FOUND;
			// }

			// subTlvP = lwm2m_data_new(estab_ret_cnt);
			// estab_ret_cnt = 0;
			// for (estab_ret =  connDataP->establish_result; estab_ret!= NULL;estab_ret = estab_ret->next)
            // {
            	// subTlvP[estab_ret_cnt].id = (uint16_t)estab_ret->resInstId;
            	// lwm2m_data_encode_int(estab_ret->InstValue, &subTlvP[estab_ret_cnt]);
				// estab_ret_cnt++;
			// }
			// lwm2m_data_encode_instances(subTlvP, 0, dataP);
    	 }
		//return COAP_205_CONTENT ;
	 case RES_M_CONN_ESTABLISH_REJECT_CAUSE: //s-int
    	 {
			// resource_instance_int_list_t *estab_rej =NULL;
			// int estab_rej_cnt = 0;
			 lwm2m_data_t* subTlvP = NULL;
			
			// for (estab_rej = connDataP->establish_rej_cause, estab_rej_cnt=0; estab_rej!=NULL; estab_rej = estab_rej->next, estab_rej_cnt++);
			// if(estab_rej_cnt == 0)
			// {
				// if(TRUE == lwm2m_check_is_att_netwrok())
				// {
					 subTlvP = lwm2m_data_new(1);
					 subTlvP->id = 0;
					 lwm2m_data_encode_int(0, subTlvP);
					 lwm2m_data_encode_instances(subTlvP, 1, dataP);
					 return COAP_205_CONTENT ;
				// }
				// else
					// return COAP_404_NOT_FOUND;
			// }

			// subTlvP = lwm2m_data_new(estab_rej_cnt);
			// estab_rej_cnt = 0;
			// for (estab_rej =  connDataP->establish_rej_cause; estab_rej!= NULL;estab_rej = estab_rej->next)
            // {
            	// subTlvP[estab_rej_cnt].id = (uint16_t)estab_rej->resInstId;
            	// lwm2m_data_encode_int(estab_rej->InstValue, &subTlvP[estab_rej_cnt]);
				// estab_rej_cnt++;
			// }
			 //lwm2m_data_encode_instances(subTlvP, 0, dataP);
    	 }
		//return COAP_205_CONTENT ;
	  case RES_M_CONN_END_TIME: //s-int
    	 {
			// resource_instance_int_list_t *end_time =NULL;
			// int end_time_cnt = 0;
			 lwm2m_data_t* subTlvP = NULL;
			
			// for (end_time = connDataP->establish_end_time, end_time_cnt=0; end_time!=NULL; end_time = end_time->next, end_time_cnt++);
			// if(end_time_cnt == 0)
			// {
				// if(TRUE == lwm2m_check_is_att_netwrok())
				// {
					 subTlvP = lwm2m_data_new(1);
					 subTlvP->id = 0;
					 lwm2m_data_encode_int(0, subTlvP);
					 lwm2m_data_encode_instances(subTlvP, 1, dataP);
					 return COAP_205_CONTENT ;
				// }
				// else
					// return COAP_404_NOT_FOUND;
			// }

			// subTlvP = lwm2m_data_new(end_time_cnt);
			// end_time_cnt = 0;
			// for (end_time =  connDataP->establish_end_time; end_time!= NULL;end_time = end_time->next)
            // {
            	// subTlvP[end_time_cnt].id = (uint16_t)end_time->resInstId;
            	// lwm2m_data_encode_int(end_time->InstValue, &subTlvP[end_time_cnt]);
				// end_time_cnt++;
			// }
			 //lwm2m_data_encode_instances(subTlvP, 0, dataP);
    	 }
		//return COAP_205_CONTENT ;

    default:
        return COAP_404_NOT_FOUND ;
    }
}


static uint8_t prv_apn_conn_read
(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t ** dataArrayP,
lwm2m_object_t * objectP,
uint16_t resInstId
)
{
	uint8_t result = 0;
	int i;
    apn_conn_profile_data_t* apn_conn_profile_dataP = NULL;
		// this is a single instance object
		// if (instanceId != 0)
		// {
			// return COAP_404_NOT_FOUND ;
		// }

		// is the server asking for the full object ?
    apn_conn_profile_dataP = (apn_conn_profile_data_t*) lwm2m_list_find(objectP->instanceList, instanceId);
    if (apn_conn_profile_dataP == NULL) 
    {
        return COAP_404_NOT_FOUND;
    }
		if (*numDataP == 0)
		{
			uint16_t resList[] = {
			 RES_M_PROFILE_NAME,
  			 RES_M_APN,
  			 RES_M_ENABLE_STATUS,
  			 RES_M_CONN_ESTABLISH_TIME,
  			 RES_M_CONN_ESTABLISH_RESULT,
  			 RES_M_CONN_ESTABLISH_REJECT_CAUSE,
  			 RES_M_CONN_END_TIME,
			};
			int nbRes = sizeof(resList) / sizeof(uint16_t);

			// if(false == lwm2m_check_is_att_netwrok())
			// {
				// nbRes -= 4;
			// }

			*dataArrayP = lwm2m_data_new(nbRes);
			if (*dataArrayP == NULL){
            return COAP_500_INTERNAL_SERVER_ERROR ;
            }
			*numDataP = nbRes;
			for (i = 0; i < nbRes; i++)
			{
				(*dataArrayP)[i].id = resList[i];
			}
		}

		i = 0;
		do
		{
			result = prv_apn_conn_set_value((*dataArrayP) + i, apn_conn_profile_dataP);
			i++;
		} while (i < *numDataP && result == COAP_205_CONTENT );

	return result;
}

static uint8_t prv_apn_conn_discover
(
uint16_t instanceId,
int * numDataP,
lwm2m_data_t ** dataArrayP,
lwm2m_object_t * objectP
)
{
	uint8_t result = 0;
	apn_conn_profile_data_t * targetP = NULL;
	int i = 0;
	int estab_time_cnt = 0, estab_ret_cnt = 0, estab_rej_cause_cnt = 0, end_time_cnt = 0;
	// resource_instance_int_list_t *estab_time = NULL, *estab_ret = NULL, *estab_rej = NULL,*end_time = NULL;

	if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
	{
		return COAP_500_INTERNAL_SERVER_ERROR;
    }

	// if (instanceId != 0)
	// {
		// return COAP_404_NOT_FOUND;
	// }

	result = COAP_205_CONTENT;
    targetP = (apn_conn_profile_data_t*) (objectP->userData);
	// if(targetP != NULL)
	// {
		 // for (estab_time = targetP->establish_time, estab_time_cnt=0; estab_time!=NULL; estab_time = estab_time->next, estab_time_cnt++);

		 // for (estab_ret= targetP->establish_result, estab_ret_cnt=0; estab_ret!=NULL; estab_ret = estab_ret->next, estab_ret_cnt++);

		 // for (estab_rej = targetP->establish_rej_cause, estab_rej_cause_cnt=0; estab_rej!=NULL; estab_rej = estab_rej->next, estab_rej_cause_cnt++);

		 // for (end_time= targetP->establish_end_time, end_time_cnt=0; end_time!=NULL; end_time = end_time->next, end_time_cnt++);
	// }
	if (*numDataP == 0)
	{
		uint16_t resList[] = {
			 RES_M_PROFILE_NAME,
  			 RES_M_APN,
  			 RES_M_ENABLE_STATUS,
  			 RES_M_CONN_ESTABLISH_TIME,
  			 RES_M_CONN_ESTABLISH_RESULT,
  			 RES_M_CONN_ESTABLISH_REJECT_CAUSE,
  			 RES_M_CONN_END_TIME
			};
		int nbRes = sizeof(resList) / sizeof(uint16_t);
		*dataArrayP = lwm2m_data_new(nbRes);

		if (*dataArrayP == NULL) 
			return COAP_500_INTERNAL_SERVER_ERROR;

		*numDataP = nbRes;
		for (i = 0; i < nbRes; i++)
		{
			(*dataArrayP)[i].id = resList[i];
			if((*dataArrayP)[i].id == RES_M_CONN_ESTABLISH_TIME )
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = estab_time_cnt;
			}
		 	else if ((*dataArrayP)[i].id == RES_M_CONN_ESTABLISH_RESULT )
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = estab_ret_cnt;
			}
			else if((*dataArrayP)[i].id == RES_M_CONN_ESTABLISH_REJECT_CAUSE)
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = estab_rej_cause_cnt;
			} 
			else if((*dataArrayP)[i].id == RES_M_CONN_END_TIME)
			{
				(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				(*dataArrayP)[i].value.asChildren.count = end_time_cnt;
			} 
		}
	}
	else 
	{
		for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
		{
		 	switch ((*dataArrayP)[i].id)
			{
			case RES_M_PROFILE_NAME:
			case RES_M_APN:
			case RES_M_ENABLE_STATUS:
				break;
			case RES_M_CONN_ESTABLISH_TIME:
				{
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				 	(*dataArrayP)[i].value.asChildren.count = estab_time_cnt;
				}
				break;
			case RES_M_CONN_ESTABLISH_RESULT:
				{
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				 	(*dataArrayP)[i].value.asChildren.count = estab_ret_cnt;
				}
				break;
			case RES_M_CONN_ESTABLISH_REJECT_CAUSE:
				{
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				 	(*dataArrayP)[i].value.asChildren.count = estab_rej_cause_cnt;
				}
				break;
			case RES_M_CONN_END_TIME:
					{
					(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
				 	(*dataArrayP)[i].value.asChildren.count = end_time_cnt;
				}
				break;
			default:
				result = COAP_404_NOT_FOUND;
			}
		}
	}
	return result;
}

static uint8_t prv_apn_conn_write
(
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP,
uint16_t resInstId, 
uint8_t write_method
)
{
	uint8_t result = 0;
	int i = 0;
    int64_t value;
    apn_conn_profile_data_t* apn_conn_profile_dataP = NULL;
    apn_conn_profile_data_t* apn_conn_profile_sec_dataP = NULL;

    apn_conn_profile_dataP = (apn_conn_profile_data_t*) lwm2m_list_find(objectP->instanceList, instanceId);

    apn_conn_profile_sec_dataP = (apn_conn_profile_data_t*) lwm2m_list_find(objectP->instanceList, (instanceId == 0)?1:0);
//	MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] %s", __FUNCTION__);
	if( dataArray == NULL || objectP == NULL ) 
	{
	    return COAP_500_INTERNAL_SERVER_ERROR;
	}

	// this is a single instance object
	// if (instanceId != 0)
	// {
	    // return COAP_404_NOT_FOUND;
	// }

	i = 0;

	do
	  {
	    switch (dataArray[i].id)
	    {
	    case RES_M_APN:
            result = COAP_405_METHOD_NOT_ALLOWED;
		case RES_M_ENABLE_STATUS:
            if (1 == lwm2m_data_decode_int(dataArray + i, &value)){
                if (value >= 0 && value <= 0xFFFF)
                    apn_conn_profile_dataP -> apn_status = value;
            }
            fprintf(stderr, "=======>  RES_M_ENABLE_STATUS value = %d \r\n",value);
            if(value == 0){
                fprintf(stderr, "=======> check value == 0,apn=%s \r\n",apn_conn_profile_dataP->apn);
                char at[128];
                memset(at, 0, 128);
                sprintf(at, "at+qcfg=\"%s\",\"%s\"", "SetBlockAPN", apn_conn_profile_dataP->apn);
                //enableApn (false, apn_conn_profile_dataP->profile_id, apn_conn_profile_dataP->apn);
                lwm2m_enableApn (true, apn_conn_profile_sec_dataP->profile_id, apn_conn_profile_sec_dataP->apn);
                send_AT_Command(&at,NULL);
                lwm2m_setApnEnable(0,apn_conn_profile_dataP->profile_id);
                fprintf(stderr, "=======> sec,apn=%s \r\n",apn_conn_profile_sec_dataP->apn);
            }else if(value == 1){
                lwm2m_enableApn (true, apn_conn_profile_dataP->profile_id, apn_conn_profile_dataP->apn);
            }
            result = COAP_204_CHANGED;
            break;
		case RES_M_CONN_ESTABLISH_TIME:
		case RES_M_CONN_ESTABLISH_RESULT:
		case RES_M_CONN_ESTABLISH_REJECT_CAUSE:
		case RES_M_CONN_END_TIME:
	     	result = COAP_405_METHOD_NOT_ALLOWED;
	      default:
	          result = COAP_404_NOT_FOUND;
			  break;
	    }

	    i++;
	 } while (i < numData && result == COAP_204_CHANGED);

	return result;

}
static uint8_t prv_apn_conn_execute
(
uint16_t instanceId,
uint16_t resourceId,
uint8_t * buffer,
int length,
lwm2m_object_t * objectP
)
{
	uint8_t result = 0;

	if( objectP == NULL ) 
	{
	    return COAP_500_INTERNAL_SERVER_ERROR;
	}

	// if (instanceId != 0)
	// {
	   // return COAP_404_NOT_FOUND;
	// }

	switch (resourceId)
	{
		case RES_M_ENABLE_STATUS:

		 	return COAP_204_CHANGED;
	    case RES_M_APN:
	
		case RES_M_CONN_ESTABLISH_TIME:
		case RES_M_CONN_ESTABLISH_RESULT:
		case RES_M_CONN_ESTABLISH_REJECT_CAUSE:
		case RES_M_CONN_END_TIME:
	     	result = COAP_405_METHOD_NOT_ALLOWED;
	      default:
	          result = COAP_404_NOT_FOUND;
			  break;
	    }
	return result;

}

static uint8_t prv_apn_conn_delete
(
uint16_t id,
lwm2m_object_t * objectP
)
{
	uint8_t result = 0;

	return result;

}

static uint8_t prv_apn_conn_create
(
uint16_t instanceId,
int numData,
lwm2m_data_t * dataArray,
lwm2m_object_t * objectP
)
{
	uint8_t result = 0;

	return result;
}

// static int32_t prv_apn_conn_setvalue 
// (
// lwm2m_object_data_t * lwm2m_data,
// lwm2m_object_t * targetP,
// lwm2m_uri_t *uriP
// )
// {
	// uint8_t result = 0;
	// apn_conn_profile_data_t* connDataP = NULL;
	// int estab_time_cnt = 0, estab_ret_cnt = 0, estab_rej_cause_cnt = 0, end_time_cnt = 0;
	// resource_instance_int_list_t *estab_time = NULL, *estab_ret = NULL, *estab_rej = NULL,*end_time = NULL;

	//MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] %s", __FUNCTION__);

	// if (lwm2m_data == NULL || targetP == NULL)
	// {
		// return 0;
	// }

	// connDataP = targetP->userData;

	// if (NULL == connDataP)
	// {
		// return 0;
    // }

	// for (estab_time = connDataP->establish_time, estab_time_cnt=0; estab_time!=NULL; estab_time = estab_time->next, estab_time_cnt++);
						
	// for (estab_ret= connDataP->establish_result, estab_ret_cnt=0; estab_ret!=NULL; estab_ret = estab_ret->next, estab_ret_cnt++);

	// for (estab_rej = connDataP->establish_rej_cause, estab_rej_cause_cnt=0; estab_rej!=NULL; estab_rej = estab_rej->next, estab_rej_cause_cnt++);
						
	// for (end_time= connDataP->establish_end_time, end_time_cnt=0; end_time!=NULL; end_time = end_time->next, end_time_cnt++);



	//MSG_SPRINTF_4(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] %d,%d,%d,%d", estab_time_cnt,estab_ret_cnt,estab_rej_cause_cnt,end_time_cnt);

	// switch (lwm2m_data->instance_info->resource_info->resource_ID) 
	// {
	// case RES_M_PROFILE_NAME:
		// if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING)
		// {
			// memset(connDataP->profile_name, 0, 128);
 			// strcpy(connDataP->profile_name, (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer);
			// uriP->objectId	 = lwm2m_data->object_ID;
			// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
			// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
			// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

			// result = 1;
		// }
		// break;

	// case RES_M_APN:
		// if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_STRING)
		// {
			// memset(connDataP->apn, 0, 128);
 			// strcpy(connDataP->apn, (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer);
			//MSG_SPRINTF_2(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M] connDataP->apn:%s,buffer:%s", connDataP->apn, 
				// (const char *)lwm2m_data->instance_info->resource_info->value.asBuffer.buffer);
			// uriP->objectId	 = lwm2m_data->object_ID;
			// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
			// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
			// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;

			// result = 1;
		// }
		// break;
			
	// case RES_M_CONN_ESTABLISH_TIME:
		  // {
		  	// /*
		  	// previous:
		  	// connect start------------connect fail(rej)
		  	// connect start-------------connect success(end)
		  	// connect start = connect fail+connect success
		  	// */
		  	// if(estab_time_cnt != (estab_rej_cause_cnt+end_time_cnt)){
				// return 0;
			// }
		    // if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				
				// if(estab_time_cnt >= 5)
					// estab_time_cnt = 0;
				// prv_write_resource_inst_val(&(connDataP->establish_time),estab_time_cnt, lwm2m_data->instance_info->resource_info->value.asInteger);
				// uriP->objectId	 = lwm2m_data->object_ID;
				// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				// result = 1;
		    // }
		  // }
		  // break;
	
	// case RES_M_CONN_ESTABLISH_RESULT: 
		// {
			// if((estab_ret_cnt +1) != estab_time_cnt){
				// return 0;
			// }
		    // if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				
				// if(estab_ret_cnt >= 5)
					// estab_ret_cnt = 0;
				// prv_write_resource_inst_val(&(connDataP->establish_result),estab_ret_cnt, lwm2m_data->instance_info->resource_info->value.asInteger);
				// uriP->objectId	 = lwm2m_data->object_ID;
				// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				// result = 1;
		    // }
		  // }
		  // break;
	// case RES_M_CONN_ESTABLISH_REJECT_CAUSE:
		// {
		    // if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				// if(estab_ret_cnt > 0)
					// estab_ret_cnt -= 1;
			
				// prv_write_resource_inst_val(&(connDataP->establish_rej_cause),estab_ret_cnt, lwm2m_data->instance_info->resource_info->value.asInteger);
				// uriP->objectId	 = lwm2m_data->object_ID;
				// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				// result = 1;
		    // }
		  // }
		  // break;
	// case RES_M_CONN_END_TIME:
		// {
		    // if(lwm2m_data->instance_info->resource_info->type == LWM2M_TYPE_INTEGER){
				// if(estab_time_cnt > 0)
					// estab_time_cnt -= 1;
				// prv_write_resource_inst_val(&(connDataP->establish_end_time),estab_time_cnt, lwm2m_data->instance_info->resource_info->value.asInteger);
				// uriP->objectId	 = lwm2m_data->object_ID;
				// uriP->instanceId = lwm2m_data->instance_info->instance_ID ;
				// uriP->resourceId = lwm2m_data->instance_info->resource_info->resource_ID;
				// uriP->flag		 = LWM2M_URI_FLAG_RESOURCE_ID | LWM2M_URI_FLAG_INSTANCE_ID | LWM2M_URI_FLAG_OBJECT_ID;
		
				// result = 1;
		    // }
		  // }
		  // break;
		 // break;
		// default :
		  // result = 0;
		  // break;
	  // }
	// return result;
// }

// int32_t prv_apn_conn_getvalue
// (
// lwm2m_id_info_t id_info,
// lwm2m_object_data_t *lwm2m_dataP,
// lwm2m_object_t *targetP
// )
// {
	// uint8_t result = 0;

	// return result;
// }


lwm2m_object_t * get_object_apn_conn()
{
  lwm2m_object_t * apn_obj = NULL;
  int fd = 0;
  unsigned int ret_status = 0;
  lwm2m_apn_conn_info_t  apn_conn_info;
  int instance_id = 0;
						
  apn_obj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));
  if(NULL != apn_obj)
  {
		apn_conn_profile_data_t *instanceP = NULL;
		memset(apn_obj, 0, sizeof(lwm2m_object_t));

		/*
		 * It assigns his unique ID
		 */
		apn_obj->objID = LWM2M_APN_CONN_PROFILE_ID;


		apn_obj->readFunc = prv_apn_conn_read;
		apn_obj->discoverFunc = prv_apn_conn_discover;
		apn_obj->executeFunc = prv_apn_conn_execute;
		//apn_obj->createFunc = prv_apn_conn_create;
		//apn_obj->deleteFunc = prv_apn_conn_delete;
		apn_obj->writeFunc = prv_apn_conn_write;
		//apn_obj->setValueFunc = prv_apn_conn_setvalue;
		//apn_obj->getValueFunc = prv_apn_conn_getvalue;
		// apn_obj->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
		// if (NULL != apn_obj->instanceList)
		// {
		  // memset(apn_obj->instanceList, 0, sizeof(lwm2m_list_t));
		// }
		// else
		// {
		  // lwm2m_free(apn_obj);
		  // return NULL;
		// }
    while(instance_id < 2){
		instanceP = (apn_conn_profile_data_t *)lwm2m_malloc(sizeof(apn_conn_profile_data_t));
	    if (NULL == instanceP)
	    {
	      free_object_apn_conn(apn_obj);
	      lwm2m_free(apn_obj);
	      return NULL;
	    }



		// apptcpip_pdp_profile_param_struct  pdp_profile;
		// memset((void *)&pdp_profile, 0, sizeof(apptcpip_pdp_profile_param_struct));
		// apptcpip_netif_get_pdp_profile(1, &pdp_profile);

		memset(instanceP,0,sizeof(apn_conn_profile_data_t));
	    //apn_obj->userData   = instanceP;

		memset(instanceP->profile_name,0,128);
        memset(instanceP->profile_id,0,128);
		memset(instanceP->apn,0,128);
        //init_apn_info();
        att_apn_id *apn_id = lwm2m_get_apnid();
        lwm2m_getApnInfo(apn_id);
        LOG_ARG("global_id = %s",apn_id->global_id);
        LOG_ARG("attz_id = %s",apn_id->attz_id);
        if(instance_id == 0){
            strcpy(instanceP->profile_name, "AT&T LWM2M APN");
            strcpy(instanceP->apn, ATT_M2M_GLOBAL);
            if(apn_id->global_id != NULL){
                strcpy(instanceP->profile_id, apn_id->global_id);
            }else{
                LOG("error: attm2mglobal apn not fount");
            }
            instanceP->apn_status = 1;
        }else{
            strcpy(instanceP->profile_name, "AT&T LWM2M FOTA");
            strcpy(instanceP->apn, M2M_COM_ATTZ);
            if(apn_id->attz_id != NULL){
                strcpy(instanceP->profile_id, apn_id->attz_id);
            }else{
                LOG("error: m2m.com.attz apn not fount");
            }
            instanceP->apn_status = 1;
        }
//		if(lwm2m_load_apn_conn_info(&apn_conn_info) == true){
//			MSG_SPRINTF_1(MSG_SSID_DS_ATCOP, MSG_LEGACY_HIGH,"[LWM2M]  mask=0x%x", apn_conn_info.mask);
			// if(apn_conn_info.mask & 0x0002)
				// prv_write_resource_inst_val(&(instanceP->establish_time),0, apn_conn_info.estab_time);
			// if(apn_conn_info.mask & 0x0004)
		    	// prv_write_resource_inst_val(&(instanceP->establish_result),0, apn_conn_info.estab_result);
			// if(apn_conn_info.mask & 0x0008)
		    	// prv_write_resource_inst_val(&(instanceP->establish_rej_cause),0, apn_conn_info.estab_rej_cause);
			// if(apn_conn_info.mask & 0x0010)
		    	// prv_write_resource_inst_val(&(instanceP->establish_end_time),0, apn_conn_info.release_time);
//		}
        instanceP->instanceId = instance_id;
        apn_obj->instanceList = LWM2M_LIST_ADD(apn_obj->instanceList, instanceP); 
        apn_obj->userData = NULL;
		instance_id++;
    }
  }

  return apn_obj;
}


void free_object_apn_conn(lwm2m_object_t * objectP)
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
}


