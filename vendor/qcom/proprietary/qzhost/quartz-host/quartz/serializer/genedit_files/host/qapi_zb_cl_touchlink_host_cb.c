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
#include "qapi_zb_cl_touchlink_common.h"
#include "qapi_zb_cl_touchlink_host_cb.h"

void Host_qapi_ZB_CL_Touchlink_Client_CB_t_Handler(uint32_t CallbackID, CallbackInfo_t *Info, uint16_t BufferLength, uint8_t *Buffer)
{
    SerStatus_t        qsResult = ssSuccess;
    BufferListEntry_t *qsBufferList = NULL;
    PackedBuffer_t     qsInputBuffer = { NULL, 0, 0, 0, NULL, NULL };
    CallbackInfo_t     qsCallbackInfo;
    Boolean_t          qsPointerValid = FALSE;


    UNUSED(qsPointerValid);

    /* Function parameters. */
    qapi_ZB_Handle_t ZB_Handle;
    qapi_ZB_Cluster_t Cluster;
    qapi_ZB_CL_Touchlink_Client_Event_Data_t *Event_Data = NULL;
    uint32_t CB_Param;

    InitializePackedBuffer(&qsInputBuffer, Buffer, BufferLength);

    /* Read function parameters. */
    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&ZB_Handle);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&Cluster);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&CB_Param);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_PointerHeader(&qsInputBuffer, &qsBufferList, &qsPointerValid);

    if((qsResult == ssSuccess) && (qsPointerValid == TRUE))
    {
        Event_Data = AllocateBufferListEntry(&qsBufferList, sizeof(qapi_ZB_CL_Touchlink_Client_Event_Data_t));

        if(Event_Data == NULL)
        {
            qsResult = ssAllocationError;
        }
        else
        {
            qsResult = PackedRead_qapi_ZB_CL_Touchlink_Client_Event_Data_t(&qsInputBuffer, &qsBufferList, (qapi_ZB_CL_Touchlink_Client_Event_Data_t *)Event_Data);
        }
    }
    else
        Event_Data = NULL;

    /* Find the callback and issue it. */
    if(qsResult == ssSuccess)
    {
        qsResult = Callback_Retrieve(CB_Param, &qsCallbackInfo);

        if((qsResult == ssSuccess) && (qsCallbackInfo.AppFunction != NULL))
            (*(qapi_ZB_CL_Touchlink_Client_CB_t)qsCallbackInfo.AppFunction)(ZB_Handle, Cluster, Event_Data, (uint32_t)qsCallbackInfo.AppParam);
    }

    FreeBufferList(&qsBufferList);
}
void Host_qapi_ZB_CL_Touchlink_Server_CB_t_Handler(uint32_t CallbackID, CallbackInfo_t *Info, uint16_t BufferLength, uint8_t *Buffer)
{
    SerStatus_t        qsResult = ssSuccess;
    BufferListEntry_t *qsBufferList = NULL;
    PackedBuffer_t     qsInputBuffer = { NULL, 0, 0, 0, NULL, NULL };
    CallbackInfo_t     qsCallbackInfo;
    Boolean_t          qsPointerValid = FALSE;


    UNUSED(qsPointerValid);

    /* Function parameters. */
    qapi_ZB_Handle_t ZB_Handle;
    qapi_ZB_Cluster_t Cluster;
    qapi_ZB_CL_Touchlink_Server_Event_Data_t *Event_Data = NULL;
    uint32_t CB_Param;

    InitializePackedBuffer(&qsInputBuffer, Buffer, BufferLength);

    /* Read function parameters. */
    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&ZB_Handle);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&Cluster);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_32(&qsInputBuffer, &qsBufferList, (uint32_t *)&CB_Param);

    if(qsResult == ssSuccess)
        qsResult = PackedRead_PointerHeader(&qsInputBuffer, &qsBufferList, &qsPointerValid);

    if((qsResult == ssSuccess) && (qsPointerValid == TRUE))
    {
        Event_Data = AllocateBufferListEntry(&qsBufferList, sizeof(qapi_ZB_CL_Touchlink_Server_Event_Data_t));

        if(Event_Data == NULL)
        {
            qsResult = ssAllocationError;
        }
        else
        {
            qsResult = PackedRead_qapi_ZB_CL_Touchlink_Server_Event_Data_t(&qsInputBuffer, &qsBufferList, (qapi_ZB_CL_Touchlink_Server_Event_Data_t *)Event_Data);
        }
    }
    else
        Event_Data = NULL;

    /* Find the callback and issue it. */
    if(qsResult == ssSuccess)
    {
        qsResult = Callback_Retrieve(CB_Param, &qsCallbackInfo);

        if((qsResult == ssSuccess) && (qsCallbackInfo.AppFunction != NULL))
            (*(qapi_ZB_CL_Touchlink_Server_CB_t)qsCallbackInfo.AppFunction)(ZB_Handle, Cluster, Event_Data, (uint32_t)qsCallbackInfo.AppParam);
    }

    FreeBufferList(&qsBufferList);
}
