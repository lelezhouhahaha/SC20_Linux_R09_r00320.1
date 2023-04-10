/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __MNL_QAPI_BLE_LNS_COMMON_H__
#define __MNL_QAPI_BLE_LNS_COMMON_H__
#include "qsCommon.h"
#include "qapi_ble.h"
#include "qapi_ble_lns.h"
#include "qapi_ble_btapityp_common.h"
#include "qapi_ble_bttypes_common.h"
#include "qapi_ble_gatt_common.h"
#include "qapi_ble_lnstypes_common.h"

/* Packed structure size definitions. */
uint32_t Mnl_CalcPackedSize_qapi_BLE_LNS_LN_Control_Point_Response_Data_t(qapi_BLE_LNS_LN_Control_Point_Response_Data_t *Structure);

/* Pack structure function definitions. */
SerStatus_t Mnl_PackedWrite_qapi_BLE_LNS_LN_Control_Point_Response_Data_t(PackedBuffer_t *Buffer, qapi_BLE_LNS_LN_Control_Point_Response_Data_t *Structure);

/* Unpack structure function definitions. */
SerStatus_t Mnl_PackedRead_qapi_BLE_LNS_LN_Control_Point_Response_Data_t(PackedBuffer_t *Buffer, BufferListEntry_t **BufferList, qapi_BLE_LNS_LN_Control_Point_Response_Data_t *Structure);

#endif // __MNL_QAPI_BLE_LNS_COMMON_H__
