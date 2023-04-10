/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <string.h>
#include "qapi_zb.h"
#include "qsCommon.h"
#include "qsPack.h"
#include "qapi_zb_cl_level_control_common.h"
#include "qapi_zb_cl_common.h"

uint32_t CalcPackedSize_qapi_ZB_CL_LevelControl_Client_Event_Data_t(qapi_ZB_CL_LevelControl_Client_Event_Data_t *Structure)
{
    uint32_t qsResult;

    if(Structure == NULL)
    {
        qsResult = 0;
    }
    else
    {
        qsResult = QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_DATA_T_MIN_PACKED_SIZE;

        switch(Structure->Event_Type)
        {
            case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Attr_Custom_Read_t((qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Attr_Custom_Write_t((qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Default_Response_t((qapi_ZB_CL_Default_Response_t *)&Structure->Data.Default_Response);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_COMMAND_COMPLETE_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Command_Complete_t((qapi_ZB_CL_Command_Complete_t *)&Structure->Data.Command_Complete);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_UNPARSED_RESPONSE_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Unparsed_Response_t((qapi_ZB_CL_Unparsed_Response_t *)&Structure->Data.Unparsed_Response);
                break;
            default:
                break;
        }
    }

    return(qsResult);
}

uint32_t CalcPackedSize_qapi_ZB_CL_LevelControl_Server_Transition_t(qapi_ZB_CL_LevelControl_Server_Transition_t *Structure)
{
    uint32_t qsResult;

    if(Structure == NULL)
    {
        qsResult = 0;
    }
    else
    {
        qsResult = QAPI_ZB_CL_LEVELCONTROL_SERVER_TRANSITION_T_MIN_PACKED_SIZE;
    }

    return(qsResult);
}

uint32_t CalcPackedSize_qapi_ZB_CL_LevelControl_Server_Event_Data_t(qapi_ZB_CL_LevelControl_Server_Event_Data_t *Structure)
{
    uint32_t qsResult;

    if(Structure == NULL)
    {
        qsResult = 0;
    }
    else
    {
        qsResult = QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_DATA_T_MIN_PACKED_SIZE;

        switch(Structure->Event_Type)
        {
            case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Attr_Custom_Read_t((qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Attr_Custom_Write_t((qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_UNPARSED_DATA_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_Unparsed_Data_t((qapi_ZB_CL_Unparsed_Data_t *)&Structure->Data.Unparsed_Data);
                break;
            case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_TRANSITION_E:
                qsResult += CalcPackedSize_qapi_ZB_CL_LevelControl_Server_Transition_t((qapi_ZB_CL_LevelControl_Server_Transition_t *)&Structure->Data.Transition);
                break;
            default:
                break;
        }
    }

    return(qsResult);
}

SerStatus_t PackedWrite_qapi_ZB_CL_LevelControl_Client_Event_Data_t(PackedBuffer_t *Buffer, qapi_ZB_CL_LevelControl_Client_Event_Data_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;

    if(Buffer->Remaining >= CalcPackedSize_qapi_ZB_CL_LevelControl_Client_Event_Data_t(Structure))
    {
        if(Structure != NULL)
        {
         if(qsResult == ssSuccess)
             qsResult = PackedWrite_int(Buffer, (int *)&Structure->Event_Type);

         if(qsResult == ssSuccess)
         {
             switch(Structure->Event_Type)
             {
                 case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Attr_Custom_Read_t(Buffer, (qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Attr_Custom_Write_t(Buffer, (qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Default_Response_t(Buffer, (qapi_ZB_CL_Default_Response_t *)&Structure->Data.Default_Response);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_COMMAND_COMPLETE_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Command_Complete_t(Buffer, (qapi_ZB_CL_Command_Complete_t *)&Structure->Data.Command_Complete);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_UNPARSED_RESPONSE_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Unparsed_Response_t(Buffer, (qapi_ZB_CL_Unparsed_Response_t *)&Structure->Data.Unparsed_Response);
                     break;
                 default:
                     break;
             }
         }

        }
    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedWrite_qapi_ZB_CL_LevelControl_Server_Transition_t(PackedBuffer_t *Buffer, qapi_ZB_CL_LevelControl_Server_Transition_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;

    if(Buffer->Remaining >= CalcPackedSize_qapi_ZB_CL_LevelControl_Server_Transition_t(Structure))
    {
        if(Structure != NULL)
        {
         if(qsResult == ssSuccess)
             qsResult = PackedWrite_8(Buffer, (uint8_t *)&Structure->Level);

         if(qsResult == ssSuccess)
             qsResult = PackedWrite_16(Buffer, (uint16_t *)&Structure->TransitionTime);

         if(qsResult == ssSuccess)
             qsResult = PackedWrite_32(Buffer, (uint32_t *)&Structure->WithOnOff);

        }
    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedWrite_qapi_ZB_CL_LevelControl_Server_Event_Data_t(PackedBuffer_t *Buffer, qapi_ZB_CL_LevelControl_Server_Event_Data_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;

    if(Buffer->Remaining >= CalcPackedSize_qapi_ZB_CL_LevelControl_Server_Event_Data_t(Structure))
    {
        if(Structure != NULL)
        {
         if(qsResult == ssSuccess)
             qsResult = PackedWrite_int(Buffer, (int *)&Structure->Event_Type);

         if(qsResult == ssSuccess)
         {
             switch(Structure->Event_Type)
             {
                 case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Attr_Custom_Read_t(Buffer, (qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Attr_Custom_Write_t(Buffer, (qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_UNPARSED_DATA_E:
                  qsResult = PackedWrite_qapi_ZB_CL_Unparsed_Data_t(Buffer, (qapi_ZB_CL_Unparsed_Data_t *)&Structure->Data.Unparsed_Data);
                     break;
                 case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_TRANSITION_E:
                  qsResult = PackedWrite_qapi_ZB_CL_LevelControl_Server_Transition_t(Buffer, (qapi_ZB_CL_LevelControl_Server_Transition_t *)&Structure->Data.Transition);
                     break;
                 default:
                     break;
             }
         }

        }
    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedRead_qapi_ZB_CL_LevelControl_Client_Event_Data_t(PackedBuffer_t *Buffer, BufferListEntry_t **BufferList, qapi_ZB_CL_LevelControl_Client_Event_Data_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;
    Boolean_t   qsPointerValid = FALSE;

    UNUSED(qsPointerValid);

    if(Buffer->Remaining >= QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_DATA_T_MIN_PACKED_SIZE)
    {
        if(qsResult == ssSuccess)
            qsResult = PackedRead_int(Buffer, BufferList, (int *)&Structure->Event_Type);

        if(qsResult == ssSuccess)
        {
            switch(Structure->Event_Type)
            {
                case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                    qsResult = PackedRead_qapi_ZB_CL_Attr_Custom_Read_t(Buffer, BufferList, (qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                    qsResult = PackedRead_qapi_ZB_CL_Attr_Custom_Write_t(Buffer, BufferList, (qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_DEFAULT_RESPONSE_E:
                    qsResult = PackedRead_qapi_ZB_CL_Default_Response_t(Buffer, BufferList, (qapi_ZB_CL_Default_Response_t *)&Structure->Data.Default_Response);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_COMMAND_COMPLETE_E:
                    qsResult = PackedRead_qapi_ZB_CL_Command_Complete_t(Buffer, BufferList, (qapi_ZB_CL_Command_Complete_t *)&Structure->Data.Command_Complete);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_CLIENT_EVENT_TYPE_UNPARSED_RESPONSE_E:
                    qsResult = PackedRead_qapi_ZB_CL_Unparsed_Response_t(Buffer, BufferList, (qapi_ZB_CL_Unparsed_Response_t *)&Structure->Data.Unparsed_Response);
                    break;
                default:
                    break;
            }
        }

    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedRead_qapi_ZB_CL_LevelControl_Server_Transition_t(PackedBuffer_t *Buffer, BufferListEntry_t **BufferList, qapi_ZB_CL_LevelControl_Server_Transition_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;
    Boolean_t   qsPointerValid = FALSE;

    UNUSED(qsPointerValid);

    if(Buffer->Remaining >= QAPI_ZB_CL_LEVELCONTROL_SERVER_TRANSITION_T_MIN_PACKED_SIZE)
    {
        if(qsResult == ssSuccess)
            qsResult = PackedRead_8(Buffer, BufferList, (uint8_t *)&Structure->Level);

        if(qsResult == ssSuccess)
            qsResult = PackedRead_16(Buffer, BufferList, (uint16_t *)&Structure->TransitionTime);

        if(qsResult == ssSuccess)
            qsResult = PackedRead_32(Buffer, BufferList, (uint32_t *)&Structure->WithOnOff);

    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}

SerStatus_t PackedRead_qapi_ZB_CL_LevelControl_Server_Event_Data_t(PackedBuffer_t *Buffer, BufferListEntry_t **BufferList, qapi_ZB_CL_LevelControl_Server_Event_Data_t *Structure)
{
    SerStatus_t qsResult = ssSuccess;
    Boolean_t   qsPointerValid = FALSE;

    UNUSED(qsPointerValid);

    if(Buffer->Remaining >= QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_DATA_T_MIN_PACKED_SIZE)
    {
        if(qsResult == ssSuccess)
            qsResult = PackedRead_int(Buffer, BufferList, (int *)&Structure->Event_Type);

        if(qsResult == ssSuccess)
        {
            switch(Structure->Event_Type)
            {
                case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_READ_E:
                    qsResult = PackedRead_qapi_ZB_CL_Attr_Custom_Read_t(Buffer, BufferList, (qapi_ZB_CL_Attr_Custom_Read_t *)&Structure->Data.Attr_Custom_Read);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_ATTR_CUSTOM_WRITE_E:
                    qsResult = PackedRead_qapi_ZB_CL_Attr_Custom_Write_t(Buffer, BufferList, (qapi_ZB_CL_Attr_Custom_Write_t *)&Structure->Data.Attr_Custom_Write);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_UNPARSED_DATA_E:
                    qsResult = PackedRead_qapi_ZB_CL_Unparsed_Data_t(Buffer, BufferList, (qapi_ZB_CL_Unparsed_Data_t *)&Structure->Data.Unparsed_Data);
                    break;
                case QAPI_ZB_CL_LEVELCONTROL_SERVER_EVENT_TYPE_TRANSITION_E:
                    qsResult = PackedRead_qapi_ZB_CL_LevelControl_Server_Transition_t(Buffer, BufferList, (qapi_ZB_CL_LevelControl_Server_Transition_t *)&Structure->Data.Transition);
                    break;
                default:
                    break;
            }
        }

    }
    else
    {
        qsResult = ssInvalidLength;
    }

    return(qsResult);
}
