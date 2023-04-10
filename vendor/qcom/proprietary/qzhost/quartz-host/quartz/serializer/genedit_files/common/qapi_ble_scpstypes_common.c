/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <string.h>
#include "qapi_ble.h"
#include "qsCommon.h"
#include "qsPack.h"
#include "qapi_ble_scpstypes_common.h"
#include "qapi_ble_bttypes_common.h"

uint32_t CalcPackedSize_qapi_BLE_SCPS_Scan_Interval_Window_t(qapi_BLE_SCPS_Scan_Interval_Window_t *Structure)
{
    uint32_t qsResult;

    if(Structure == NULL)
    {
        qsResult = 0;
    }
    else
    {
        qsResult = QAPI_BLE_SCPS_SCAN_INTERVAL_WINDOW_T_MIN_PACKED_SIZE;

        qsResult += CalcPackedSize_16((uint16_t *)&Structure->LE_Scan_Interval);

        qsResult += CalcPackedSize_16((uint16_t *)&Structure->LE_Scan_Window);
    }

    return(qsResult);
}

SerStatus_t PackedWrite_qapi_BLE_SCPS_Scan_Interval_Window_t(PackedBuffer_t *Buffer, qapi_BLE_SCPS_Scan_Interval_Window_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;

    if(Buffer->Remaining >= CalcPackedSize_qapi_BLE_SCPS_Scan_Interval_Window_t(Structure))
    {
        if(Structure != NULL)
        {
         if(qsResult == ssSuccess)
             qsResult = PackedWrite_16(Buffer, (uint16_t *)&Structure->LE_Scan_Interval);

         if(qsResult == ssSuccess)
             qsResult = PackedWrite_16(Buffer, (uint16_t *)&Structure->LE_Scan_Window);

        }
    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedRead_qapi_BLE_SCPS_Scan_Interval_Window_t(PackedBuffer_t *Buffer, BufferListEntry_t **BufferList, qapi_BLE_SCPS_Scan_Interval_Window_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;
    Boolean_t   qsPointerValid = FALSE;

    UNUSED(qsPointerValid);

    if(Buffer->Remaining >= QAPI_BLE_SCPS_SCAN_INTERVAL_WINDOW_T_MIN_PACKED_SIZE)
    {
        if(qsResult == ssSuccess)
            qsResult = PackedRead_16(Buffer, BufferList, (uint16_t *)&Structure->LE_Scan_Interval);

        if(qsResult == ssSuccess)
            qsResult = PackedRead_16(Buffer, BufferList, (uint16_t *)&Structure->LE_Scan_Window);

    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}
