/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include "qapi_zb_cl_ballast.h"

qapi_Status_t _qapi_ZB_CL_Ballast_Create_Server(uint8_t TargetID, qapi_ZB_Handle_t ZB_Handle, qapi_ZB_Cluster_t *Cluster, uint8_t PhysicalMin, uint8_t PhysicalMax, uint8_t MaxFactorAdj, qapi_ZB_CL_Cluster_Info_t *Cluster_Info, qapi_ZB_CL_Ballast_Server_CB_t Event_CB, uint32_t CB_Param);

qapi_Status_t _qapi_ZB_CL_Ballast_Populate_Attributes(uint8_t TargetID, qbool_t Server, uint8_t *AttributeCount, qapi_ZB_CL_Attribute_t *AttributeList);
