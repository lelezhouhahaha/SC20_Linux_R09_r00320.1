/*
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *
 *       Filename:  ril_utils.h
 *    Description:  This head file 
 *
 *        Version:  1.0.0(2020年05月29日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2020年05月29日 14时31分19秒"
 */
#ifndef __RIL_COMMON_H__
#define __RIL_COMMON_H__

#include <telephony/ril.h>
#include <binder/Parcel.h>

/*  Constants for response types */
#define RESPONSE_SOLICITED 0
#define RESPONSE_UNSOLICITED 1
#define RESPONSE_SOLICITED_ACK 2
#define RESPONSE_SOLICITED_ACK_EXP 3
#define RESPONSE_UNSOLICITED_ACK_EXP 4

namespace android {

const char * requestToString(int request);
const char * failCauseToString(RIL_Errno);
const char * callStateToString(RIL_CallState);
const char * radioStateToString(RIL_RadioState);

int getResponseRilSignalStrength(Parcel &p, RIL_SignalStrength_v10 *p_cur);

}

#endif
