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
 *    Pascal Rieux - please refer to git log
 *    
 ******************************************************************************/

/*
 * This "Access Control" object is optional and multiple instantiated
 * 
 *  Resources:
 *
 *          Name         | ID | Oper. | Inst. | Mand.|  Type   | Range | Units |
 *  ---------------------+----+-------+-------+------+---------+-------+-------+
 *  Object ID            |  0 |   R   | Single|  Yes | Integer |1-65534|       |
 *  Object instance ID   |  1 |   R   | Single|  Yes | Integer |0-65535|       |
 *  ACL                  |  2 |   RW  | Multi.|  No  | Integer | 16bit |       |
 *  Access Control Owner |  3 |   RW  | Single|  Yes | Integer |0-65535|       |
 */

#include "liblwm2m.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include "internals.h"
#include "object_access_control.h"
// Resource Id's:
#define RES_M_OBJECT_ID             0
#define RES_M_OBJECT_INSTANCE_ID    1
#define RES_O_ACL                   2
#define RES_M_ACCESS_CONTROL_OWNER  3

#if 0
typedef struct acc_ctrl_ri_s
{   // linked list:
    struct acc_ctrl_ri_s*   next;       // matches lwm2m_list_t::next
    uint16_t                resInstId;  // matches lwm2m_list_t::id, ..serverID
    // resource data:
    uint16_t                accCtrlValue;
} acc_ctrl_ri_t;

typedef struct acc_ctrl_oi_s
{   //linked list:
    struct acc_ctrl_oi_s*   next;       // matches lwm2m_list_t::next
    uint16_t                objInstId;  // matches lwm2m_list_t::id
    // resources
    uint16_t                objectId;
    uint16_t                objectInstId;
    uint16_t                accCtrlOwner;
    acc_ctrl_ri_t*          accCtrlValList;
} acc_ctrl_oi_t;
#endif
//add by joe start

#define MAX_ACL_RES_INSTANCES   4

typedef struct _aclres_peristent_info_
{
  bool                    isValid;
  uint16_t                resInstId; 
  uint16_t                accCtrlValue;
} aclres_persistent_info_t;

typedef struct _ac_persistent_info_
{
  uint16_t                  instanceId;
  uint16_t                  objectId;
  uint16_t                  objectInstId;
  uint16_t                  accCtrlOwner;
  bool                      isValid;
  aclres_persistent_info_t  accCtrlVal[MAX_ACL_RES_INSTANCES];
} ac_persistent_info_t;

int load_ac_persistent_info(lwm2m_object_t *aclObjP)
{
  int fd = 0, ret = 0, index = 0;
  acc_ctrl_oi_t *targetP = NULL;
  acc_ctrl_ri_t *aclP = NULL;
  ac_persistent_info_t info;

  if (NULL == aclObjP)
  {
    return EFAILURE;
  }

  fd = open(ACL_PERSISTENCE_FILE, O_RDONLY);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }
  
  //acl_ctrl_free_object(aclObjP);//call by joe to delete auto create accesscontrol object first

  while (1)
  {
    memset(&info, 0x00, sizeof(info));
    targetP = NULL;

    ret = read(fd, &info, sizeof(info));
    if (ret <= 0)
    {
      break;
    }
    if (LWM2M_FALSE == info.isValid)
    {
      continue;
    }

    targetP = lwm2m_malloc(sizeof(acc_ctrl_oi_t));
    
    if (NULL == targetP)
    {
      ret = -1;
      break;
    }
    memset(targetP, 0, sizeof(acc_ctrl_oi_t));//add by joe
    targetP->objInstId = info.instanceId;
    targetP->objectId = info.objectId;
    targetP->objectInstId = info.objectInstId;
    targetP->accCtrlOwner = info.accCtrlOwner;
    for (index = 0; index < MAX_ACL_RES_INSTANCES; index++)
    {
        LOG("load_ac_persistent_info 1\n" );
      aclP = NULL;
      if (LWM2M_FALSE == info.accCtrlVal[index].isValid)
        continue;

      aclP = lwm2m_malloc(sizeof(acc_ctrl_ri_t));
      LOG("load_ac_persistent_info 2\n" );
      if (NULL == aclP)
      {
        ret = -1;
        break;
      }
      memset(aclP,0,sizeof(acc_ctrl_ri_t));
      aclP->resInstId = info.accCtrlVal[index].resInstId;
      aclP->accCtrlValue = info.accCtrlVal[index].accCtrlValue;
      LOG_ARG("aclP->resInstId= %d aclP->resInstId=%d\n", aclP->resInstId,aclP->accCtrlValue );
      if(targetP->accCtrlValList == NULL){
          LOG("load_ac_persistent_info 3\n" );
      }
      targetP->accCtrlValList = (acc_ctrl_ri_t *)LWM2M_LIST_ADD(targetP->accCtrlValList, aclP);
      LOG("load_ac_persistent_info 4\n" );
    }
    if (-1 == ret)
      break;
    aclObjP->instanceList = LWM2M_LIST_ADD(aclObjP->instanceList, targetP);
  }
  if (ret < 0)
  {
    close(fd);
    acl_ctrl_free_object(aclObjP);
    return EFAILURE;
  }
  close(fd);
  return ESUCCESS;
}

int store_ac_persistent_info(acc_ctrl_oi_t *targetP, bool store)
{
  int fd = 0, index = 0;
  ac_persistent_info_t info;
  acc_ctrl_ri_t *aclP;

  if (NULL == targetP)
  {
    return EFAILURE;
  }

  fd = open(ACL_PERSISTENCE_FILE, O_CREAT | O_WRONLY, 00644);
  if (EFAILURE == fd)
  {
    return EFAILURE;
  }

  memset(&info, 0x00, sizeof(info));
  
  if(store)
  {
    info.isValid = LWM2M_TRUE; 
  }
  else
  {
    info.isValid = LWM2M_FALSE; 
  }
  info.instanceId = targetP->objInstId;
  info.objectId = targetP->objectId;
  info.objectInstId = targetP->objectInstId;
  info.accCtrlOwner = targetP->accCtrlOwner;
  aclP = targetP->accCtrlValList;
  while (aclP && index < MAX_ACL_RES_INSTANCES)
  {
    info.accCtrlVal[index].isValid = LWM2M_TRUE;
    info.accCtrlVal[index].resInstId = aclP->resInstId;
    info.accCtrlVal[index].accCtrlValue = aclP->accCtrlValue;
    aclP = aclP->next;
	index++;
  }
  while (index < MAX_ACL_RES_INSTANCES)
  {
    info.accCtrlVal[index++].isValid = LWM2M_FALSE;
  }

  lseek(fd, info.instanceId * sizeof(info), SEEK_SET);
  if (EFAILURE == write(fd, &info, sizeof(info)))
  {
    close(fd);
    return EFAILURE;
  }
  close(fd);
  return ESUCCESS;
}

int dump_ac_persistent_info(lwm2m_object_t *aclObjP)
{
  acc_ctrl_oi_t *targetP = NULL;

  if (NULL == aclObjP)
  {
    return EFAILURE;
  }

  targetP = (acc_ctrl_oi_t *)aclObjP->instanceList;
  while (targetP)
  {
    if (EFAILURE == store_ac_persistent_info(targetP, LWM2M_TRUE))
    {
      return EFAILURE;
    }
    targetP = targetP->next;
  }
  return ESUCCESS;
}


//add by joe end

static uint8_t prv_set_tlv(lwm2m_data_t* dataP, acc_ctrl_oi_t* accCtrlOiP)
{
    switch (dataP->id) {
    case RES_M_OBJECT_ID:
        lwm2m_data_encode_int(accCtrlOiP->objectId, dataP);
        return COAP_205_CONTENT;
        break;
    case RES_M_OBJECT_INSTANCE_ID:
        lwm2m_data_encode_int(accCtrlOiP->objectInstId, dataP);
        return COAP_205_CONTENT;
        break;
    case RES_O_ACL:
    {
        int ri;
        acc_ctrl_ri_t* accCtrlRiP;
        for (accCtrlRiP =accCtrlOiP->accCtrlValList, ri=0;
             accCtrlRiP!=NULL;
             accCtrlRiP = accCtrlRiP->next, ri++);

        if (ri==0)  // no values!
        {
            return COAP_404_NOT_FOUND;
        }
        else
        {
            lwm2m_data_t* subTlvP = lwm2m_data_new(ri);
            for (accCtrlRiP = accCtrlOiP->accCtrlValList, ri = 0;
                 accCtrlRiP!= NULL;
                 accCtrlRiP = accCtrlRiP->next, ri++)
            {
                subTlvP[ri].id = accCtrlRiP->resInstId;
                lwm2m_data_encode_int(accCtrlRiP->accCtrlValue, &subTlvP[ri]);
            }
            lwm2m_data_encode_instances(subTlvP, ri, dataP);//modify by joe
            return COAP_205_CONTENT;
        }
    }   break;
    case RES_M_ACCESS_CONTROL_OWNER:
        lwm2m_data_encode_int(accCtrlOiP->accCtrlOwner, dataP);
        return COAP_205_CONTENT;
        break;
    default:
        return COAP_404_NOT_FOUND ;
    }
}

static uint8_t prv_read(uint16_t instanceId, int * numDataP,
                        lwm2m_data_t** dataArrayP, lwm2m_object_t * objectP)
{
    uint8_t result;
    int     ri, ni;
    LOG_ARG("instanceId=%d\n", instanceId );
    // multi-instance object: search instance
    acc_ctrl_oi_t* accCtrlOiP =
        (acc_ctrl_oi_t *)lwm2m_list_find(objectP->instanceList, instanceId);
    if (accCtrlOiP == NULL)
    {
        return COAP_404_NOT_FOUND ;
    }

    // is the server asking for the full object ?
    if (*numDataP == 0)
    {
        uint16_t resList[] = {
                RES_M_OBJECT_ID,
                RES_M_OBJECT_INSTANCE_ID,
                RES_O_ACL,  // prv_set_tlv will return COAP_404_NOT_FOUND w/o values!
                RES_M_ACCESS_CONTROL_OWNER
        };
        int nbRes = sizeof(resList) / sizeof(uint16_t);

        *dataArrayP = lwm2m_data_new(nbRes);
        if (*dataArrayP == NULL)
            return COAP_500_INTERNAL_SERVER_ERROR ;
        *numDataP = nbRes;
        for (ri = 0; ri < nbRes; ri++)
        {
            (*dataArrayP)[ri].id = resList[ri];
            LOG_ARG("resList[%d]=%d\n", ri,resList[ri] );
            
        }
    }

    ni = ri = 0;
    do
    {
        result = prv_set_tlv((*dataArrayP) + ni, accCtrlOiP);
        if (result==COAP_404_NOT_FOUND) {
            ri++;
            if (*numDataP>1) result = COAP_205_CONTENT;
        }
        else if (ri > 0)    // copy new one by ri skipped ones in front
        {
            (*dataArrayP)[ni-ri] = (*dataArrayP)[ni];
            LOG_ARG("dataArrayP[%d]=%d\n", ni,(*dataArrayP)[ni] );
        }
        ni++;
    } while (ni < *numDataP && result == COAP_205_CONTENT);
    *numDataP = ni-ri;

    return result;
}

static bool prv_add_ac_val(acc_ctrl_oi_t* accCtrlOiP,
                           uint16_t acResId, uint16_t acValue)
{
    bool ret = false;
    acc_ctrl_ri_t *accCtrlRiP;
    accCtrlRiP = (acc_ctrl_ri_t *)lwm2m_malloc(sizeof(acc_ctrl_ri_t));
    if (accCtrlRiP==NULL)
    {
        return ret;
    }
    else
    {
        memset(accCtrlRiP, 0, sizeof(acc_ctrl_ri_t));
        accCtrlRiP->resInstId      = acResId;
        accCtrlRiP->accCtrlValue   = acValue;

        accCtrlOiP->accCtrlValList = (acc_ctrl_ri_t*)
                LWM2M_LIST_ADD(accCtrlOiP->accCtrlValList, accCtrlRiP);
        ret = true;
    }
    return ret;
}

static uint8_t prv_write_resources(uint16_t instanceId, int numData,
               lwm2m_data_t* tlvArray, lwm2m_object_t* objectP, bool doCreate)
{
    int i;
    uint8_t result = COAP_500_INTERNAL_SERVER_ERROR;
    int64_t value;

    acc_ctrl_oi_t* accCtrlOiP = (acc_ctrl_oi_t *)
            lwm2m_list_find(objectP->instanceList, instanceId);
    if (NULL == accCtrlOiP)
        return COAP_404_NOT_FOUND ;

    i = 0;
    do
    {
        switch (tlvArray[i].id)
        {
        case RES_M_OBJECT_ID:
            if (doCreate==false)
            {
                result = COAP_405_METHOD_NOT_ALLOWED;
            }
            else
            {
                if (1 != lwm2m_data_decode_int(&tlvArray[i], &value))
                {
                    result = COAP_400_BAD_REQUEST;
                }
                else if (value < 1 || value > 65534)
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
                else
                {
                    accCtrlOiP->objectId = value;
                    result = COAP_204_CHANGED;
                }
            }
            break;
        case RES_M_OBJECT_INSTANCE_ID:
            if (doCreate==false)
            {
                result = COAP_405_METHOD_NOT_ALLOWED;
            }
            else
            {
                if (1 != lwm2m_data_decode_int(&tlvArray[i], &value))
                {
                    result = COAP_400_BAD_REQUEST;
                }
                else if (value < 0 || value > 65535)
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
                else
                {
                    accCtrlOiP->objectInstId = value;
                    result = COAP_204_CHANGED;
                }
            }
            break;
        case RES_O_ACL:
        {
            if (tlvArray[i].type!= LWM2M_TYPE_MULTIPLE_RESOURCE)
            {
                result = COAP_400_BAD_REQUEST;
            }
            else
            {
                // MR-Write: Replace-implementation variant only
                // see LWM2M-TS:5.4.3 (wakaama has no part-update switch)

                // 1st: save accValueList!
                acc_ctrl_ri_t* acValListSave = accCtrlOiP->accCtrlValList;
                accCtrlOiP->accCtrlValList = NULL;

                int ri;
                lwm2m_data_t* subTlvArray = tlvArray[i].value.asChildren.array;

                if (tlvArray[i].value.asChildren.count == 0)
                {
                    result = COAP_204_CHANGED;
                }
                else if (subTlvArray==NULL)
                {
                    result = COAP_400_BAD_REQUEST;
                }
                else
                {
                    for (ri=0; ri < tlvArray[i].value.asChildren.count; ri++)
                    {
                        if (1 != lwm2m_data_decode_int(&subTlvArray[ri], &value))
                        {
                            result = COAP_400_BAD_REQUEST;
                            break;
                        }
                        else if (value < 0 || value > 0xFFFF)
                        {
                            result = COAP_406_NOT_ACCEPTABLE;
                            break;
                        }
                        else if (!prv_add_ac_val(accCtrlOiP, subTlvArray[ri].id,
                                                             (uint16_t)value))
                        {
                            result = COAP_500_INTERNAL_SERVER_ERROR;
                            break;
                        }
                        else
                        {
                            result = COAP_204_CHANGED;
                        }
                    }
                }

                if (result != COAP_204_CHANGED)
                {
                    // free pot. partial created new ones
                    LWM2M_LIST_FREE(accCtrlOiP->accCtrlValList);
                    // restore old values:
                    accCtrlOiP->accCtrlValList = acValListSave;
                }
                else
                {
                    // final free saved value list
                    LWM2M_LIST_FREE(acValListSave);
                }
            }
        }   break;
        case RES_M_ACCESS_CONTROL_OWNER: {
            if (1 == lwm2m_data_decode_int(tlvArray + i, &value))
            {
                if (value >= 0 && value <= 0xFFFF)
                {
                    accCtrlOiP->accCtrlOwner = value;
                    result = COAP_204_CHANGED;
                }
                else
                {
                    result = COAP_406_NOT_ACCEPTABLE;
                }
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
            break;
        default:
            return COAP_404_NOT_FOUND ;
        }
        i++;
    } while (i < numData && result == COAP_204_CHANGED);
    //add by joe start
    if (COAP_204_CHANGED == result && lwm2m_check_is_vzw_netwrok())
      store_ac_persistent_info(accCtrlOiP, LWM2M_TRUE);
//add by joe end
    return result;
}

static uint8_t prv_write(uint16_t instanceId, int numData,
                         lwm2m_data_t* tlvArray, lwm2m_object_t* objectP)
{
//add by joe for vzw start
if(lwm2m_check_is_vzw_netwrok()){
   lwm2m_client_state_t lwm2m_state = (lwm2m_client_state_t)0;
   lwm2m_context_t *contextP = (lwm2m_context_t *)objectP->ctxP;
   lwm2m_state = lwm2m_get_client_state(contextP);
   LOG_ARG("lwm2m_state %d\n", lwm2m_state);
   if( (lwm2m_state == STATE_BOOTSTRAP_REQUIRED) || ( lwm2m_state == STATE_BOOTSTRAPPING ) )
       return prv_write_resources(instanceId, numData, tlvArray, objectP, true);
   else
       return prv_write_resources(instanceId, numData, tlvArray, objectP, false);
}else{
       return prv_write_resources(instanceId, numData, tlvArray, objectP, false);
}	   
   //add by joe for vzw end
}

static uint8_t prv_delete(uint16_t id, lwm2m_object_t * objectP)
{
    acc_ctrl_oi_t* targetP;

    objectP->instanceList = lwm2m_list_remove(objectP->instanceList, id,
                                              (lwm2m_list_t**)&targetP);
    //add by joe start
	if(lwm2m_check_is_vzw_netwrok()){
    store_ac_persistent_info(targetP,LWM2M_FALSE);
	}
    //add by joe end
    if (NULL == targetP) return COAP_404_NOT_FOUND;

    LWM2M_LIST_FREE(targetP->accCtrlValList);
    lwm2m_free(targetP);

    return COAP_202_DELETED;
}

static uint8_t prv_create(uint16_t objInstId, int numData,
                          lwm2m_data_t * tlvArray, lwm2m_object_t * objectP)
{
    acc_ctrl_oi_t * targetP;
    uint8_t result;

    targetP = (acc_ctrl_oi_t *)lwm2m_malloc(sizeof(acc_ctrl_oi_t));
    if (NULL == targetP) return COAP_500_INTERNAL_SERVER_ERROR;
    memset(targetP, 0, sizeof(acc_ctrl_oi_t));

    targetP->objInstId    = objInstId;
    objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, targetP);

    result = prv_write_resources(objInstId, numData, tlvArray, objectP, true);

    if (result != COAP_204_CHANGED)
    {
        prv_delete(objInstId, objectP);
    }
    else
    {
        result = COAP_201_CREATED;
    }
    return result;
}

	/* @fn		static uint8_t prv_discover()
	 * @brief	This function is used to discover resources of a access control object
	 * @param	instanceId - instance id of object 
	 *			numdataP   - pointer to store number of data array elements 
	 dataArrayP - pointer to store data 
	 objectP	- pointer to lwm2m_object 
	 * @return	on success - LWM2M_404_NOT_FOUND
	 on error	- LWM2M_404_NOT_FOUND
	 */   

	static uint8_t prv_discover(uint16_t instanceId,
		int * numDataP,
		lwm2m_data_t ** dataArrayP,
		lwm2m_object_t * objectP)
	{
	  uint8_t result = 0;
	  int i = 0;
	  acc_ctrl_oi_t* accCtrlOiP = NULL;
	  int ri = 0;
	  acc_ctrl_ri_t* accCtrlRiP = NULL;
	printf("->->->-> ->->->-> 02 prv_discover  \n");
	  if( numDataP == NULL || dataArrayP == NULL || objectP == NULL ) 
	  {
		return COAP_500_INTERNAL_SERVER_ERROR;
	  }
	
	  result = COAP_205_CONTENT;
	  if(lwm2m_check_is_vzw_netwrok()){
	   accCtrlOiP = (acc_ctrl_oi_t *)lwm2m_list_find(objectP->instanceList, instanceId);
	   if (accCtrlOiP != NULL)
	   {
		 for (accCtrlRiP =accCtrlOiP->accCtrlValList, ri=0;
				   accCtrlRiP!=NULL;
				   accCtrlRiP = accCtrlRiP->next, ri++);
	   }
	   }
	  if (*numDataP == 0)
	  {
		 uint16_t resList[] = {
		   RES_M_OBJECT_ID,
		   RES_M_OBJECT_INSTANCE_ID,
		   RES_O_ACL,
		   RES_M_ACCESS_CONTROL_OWNER
	 };
	
		int nbRes = sizeof(resList) / sizeof(uint16_t);
	
		*dataArrayP = lwm2m_data_new(nbRes);
		if (*dataArrayP == NULL) 
			return COAP_500_INTERNAL_SERVER_ERROR;
		  *numDataP = nbRes;
		for (i = 0; i < nbRes; i++)
		{
		  (*dataArrayP)[i].id = resList[i];
		  if((*dataArrayP)[i].id == RES_O_ACL )
		  {
			(*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
			(*dataArrayP)[i].value.asChildren.count = ri;
		  }
		}
	  }
	  else
	  {
		for (i = 0; i < *numDataP && result == COAP_205_CONTENT; i++)
		{
		  switch ((*dataArrayP)[i].id)
		  { 					   
			case RES_O_ACL:
			{
			  (*dataArrayP)[i].type = LWM2M_TYPE_MULTIPLE_RESOURCE;
			  (*dataArrayP)[i].value.asChildren.count = ri;
			  break;
			}
			case RES_M_OBJECT_ID:
			case RES_M_OBJECT_INSTANCE_ID:
			case RES_M_ACCESS_CONTROL_OWNER:
			  break;
			default:
			  result = COAP_404_NOT_FOUND;
		  }
		}
	  }
	
	  return result;
	}

void display_acl_object(lwm2m_object_t * object)
{
#ifdef WITH_LOGS
    acc_ctrl_oi_t * data = (acc_ctrl_oi_t *)object->userData;
    fprintf(stdout, "  /%u: acl object:\r\n", object->objID);
#endif
}
/*
 * Create an empty multiple instance LWM2M Object: Access Control
 */
lwm2m_object_t * acc_ctrl_create_object(void)
{
    /*
     * The acc_ctrl_create_object() function creates an empty object
     * and returns a pointer to the structure that represents it.
     */
    lwm2m_object_t* accCtrlObj = NULL;

    accCtrlObj = (lwm2m_object_t *) lwm2m_malloc(sizeof(lwm2m_object_t));

    if (NULL != accCtrlObj)
    {
        memset(accCtrlObj, 0, sizeof(lwm2m_object_t));
        /*
         * It assign his unique object ID
         * The 2 is the standard ID for the optional object "Access Control".
         */
        accCtrlObj->objID = 2;
        // Init callbacks, empty instanceList!
        accCtrlObj->readFunc    = prv_read;
        accCtrlObj->writeFunc   = prv_write;
        accCtrlObj->createFunc  = prv_create;
        accCtrlObj->deleteFunc  = prv_delete;
        accCtrlObj->discoverFunc = prv_discover;
    }
    return accCtrlObj;
}

void acl_ctrl_free_object(lwm2m_object_t * objectP)
{
    acc_ctrl_oi_t *accCtrlOiT;
    acc_ctrl_oi_t *accCtrlOiP = (acc_ctrl_oi_t*)objectP->instanceList;
    while (accCtrlOiP != NULL)
    {
        // first free acl (multiple resource!):
        LWM2M_LIST_FREE(accCtrlOiP->accCtrlValList);
        accCtrlOiT = accCtrlOiP;
        accCtrlOiP = accCtrlOiP->next;
        lwm2m_free(accCtrlOiT);
    }
    lwm2m_free(objectP);
}

bool  acc_ctrl_obj_add_inst (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                uint16_t acObjectId, uint16_t acObjInstId, uint16_t acOwner)
{
    bool ret = false;
    //add by joe start
    LOG("acc_ctrl_obj_add_inst enter\n");
    if(lwm2m_check_is_vzw_netwrok()){
        LOG("acc_ctrl_obj_add_inst vzw return\n");
        return true;
    }
    //add by joe end
    if (NULL == accCtrlObjP)
    {
        return ret;
    }
    else
    {
        // create an access control object instance
        acc_ctrl_oi_t* accCtrlOiP;
        accCtrlOiP = (acc_ctrl_oi_t *)lwm2m_malloc(sizeof(acc_ctrl_oi_t));
        if (NULL == accCtrlOiP)
        {
            return ret;
        }
        else
        {
            memset(accCtrlOiP, 0, sizeof(acc_ctrl_oi_t));
            // list: key
            accCtrlOiP->objInstId    = instId;
            // object instance data:
            accCtrlOiP->objectId     = acObjectId;
            accCtrlOiP->objectInstId = acObjInstId;
            accCtrlOiP->accCtrlOwner = acOwner;

            accCtrlObjP->instanceList =
                    LWM2M_LIST_ADD(accCtrlObjP->instanceList, accCtrlOiP);
            ret = true;
        }
    }
    return ret;
}

bool acc_ctrl_oi_add_ac_val (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                             uint16_t acResId, uint16_t acValue)
{
    bool ret = false;
    //add by joe start
    
    LOG("acc_ctrl_oi_add_ac_val enter\n");
    if(lwm2m_check_is_vzw_netwrok()){
        LOG("acc_ctrl_oi_add_ac_val vzw return\n");
        return true;
    }
    //add by joe end

    acc_ctrl_oi_t* accCtrlOiP = (acc_ctrl_oi_t *)
            lwm2m_list_find(accCtrlObjP->instanceList, instId);
    if (NULL == accCtrlOiP)
        return ret;

    return prv_add_ac_val (accCtrlOiP, acResId, acValue);
}
