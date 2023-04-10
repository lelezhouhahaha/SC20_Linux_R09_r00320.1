/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <string.h>
#include "qsCommon.h"
#include "qsHost.h"
#include "qsCallback.h"
#include "qapi_zb.h"
#include "qapi_zb_cl_rel_humid_host_cb_mnl.h"
#include "qapi_zb_cl_rel_humid_common.h"
#include "qapi_zb_cl_rel_humid_host_cb.h"

void Host_qapi_ZB_CL_RelHumid_Server_CB_t_Handler(uint32_t CallbackID, CallbackInfo_t *Info, uint16_t BufferLength, uint8_t *Buffer)
{
    Mnl_Host_qapi_ZB_CL_RelHumid_Server_CB_t_Handler(CallbackID, Info, BufferLength, Buffer);
}
