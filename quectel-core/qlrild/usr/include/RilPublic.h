/*
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *
 *       Filename:  RilPublic.h
 *    Description:  Ril public head file for API.
 *
 *        Version:  1.0.0
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on 2020.06.06
 */

#ifndef __RIL_PUBLIC_H__
#define __RIL_PUBLIC_H__

#include <telephony/ril.h>
#include <gnss/ql_loc_v01.h>
#include <ims/imsIF.pb.h>

#ifdef __cplusplus
extern "C" {
#endif

int QLRIL_Init(int *handle);
int QLRIL_Exit(int *handle);

int QLRIL_GetVersion(char *version, size_t size);
int QLRIL_ResetQcrild(int which, int timeout);

int QLRIL_GetDefaultTimeout(int *handle, int *timeout);
int QLRIL_SetDefaultTimeout(int *handle, int timeout);

int QLRIL_GetSimCardSlotId(int *handle, int *slotId);
int QLRIL_SetSimCardSlotId(int *handle, int slotId);

int QLRIL_GetOperator(int *handle, char ***resp);
int QLRIL_GetCellInfo(int *handle, void **resp);
int QLRIL_Dial(int *handle, char *address, int clirMode);
int QLRIL_Dial_WithUUSInfo(int *handle, char *address, int clirMode, void *uusInfo);

int QLRIL_GetRadioCapability(int *handle, void **resp);
int QLRIL_SetRadioCapability(int *handle, void *data, void **resp);

int QLRIL_AcceptCall(int *handle);
int QLRIL_RejectCall(int *handle);
int QLRIL_HangupConnection(int *handle, int gsmIndex);
int QLRIL_HangupWaitingOrBackground(int *handle);
int QLRIL_HangupForegroundResumeBackground(int *handle);
int QLRIL_SwitchWaitingOrHoldingAndActive(int *handle);
int QLRIL_Conference(int *handle);
int QLRIL_GetMute(int *handle, int *muteState);
int QLRIL_SetMute(int *handle, int enableMute);

int QLRIL_SendSms(int *handle, char *address, char *message, void **resp);
int QLRIL_SendSmsByPDU(int *handle, char *smscPDU, char *pdu, void **resp);
int QLRIL_SendImsGsmSms(int *handle, char *address, char *msg, void **resp);
int QLRIL_SendImsGsmSmsByPDU(int *handle, char *smscPDU, char *pdu, char retry, int msgRef, void **resp);
int QLRIL_AcknowledgeLastIncomingGsmSms(int *handle, int slotId, int success, int cause);
int QLRIL_DeleteSmsOnSim(int *handle, int index);
int QLRIL_DeleteSmsOnRuim(int *handle, int index);

int QLRIL_GetSignalStrength(int *handle, void **resp);

int QLRIL_SetupDataCall(int *handle, int radioTechnology, int profile, char *apn,
		char *user, char *password, int authType, char *protocol, void **resp);
int QLRIL_GetCurrentCalls(int *handle, int slotId, void **resp);
int QLRIL_GetCurrentCallsByAtCmd(char *dev, int slotId, void **resp);
int QLRIL_GetDataCallList(int *handle, void **resp);
int QLRIL_DeactivateDataCall(int *handle, int cid, int reason);
int QLRIL_SetDataAllowed(int *handle, int allowed);

/* Set IP route or add DNS */
int QLRIL_ClearResolvConfFile(void);
int QLRIL_AppendDNSForIPV4V6(char *dns);
int QLRIL_SetDNSForIPV4V6(char *dns);
int QLRIL_SetRouteForIPV4(char *ifname);
int QLRIL_SetRouteForIPV6(char *ifname);
int QLRIL_SetRouteForIPV4V6(char *ifname);
int QLRIL_SetRouteAndDNS(char *ifname, char *dns1, char *dns2);
int QLRIL_SetRouteAndDNSForIPV6(char *ifname, char *dns1, char *dns2);

int QLRIL_SetRadioPower(int *handle, int on, int timeout);
int QLRIL_SetScreenState(int *handle, int on);
int QLRIL_RequestShutdown(int *handle);
int QLRIL_ResetRadio(int *handle);

int QLRIL_GetVoiceRegistrationState(int *handle, int slotId, char ***resp);
int QLRIL_GetDataRegistrationState(int *handle, char ***resp);

int QLRIL_GetPreferredNetworkType(int *handle, int *type);
int QLRIL_SetPreferredNetworkType(int *handle, int type);

int QLRIL_GetNetworkSelectionMode(int *handle, int *mode);
int QLRIL_SetNetworkSelectionModeAutomatic(int *handle);
int QLRIL_GetAvailableNetworks(int *handle, char ***resp, int timeout);
int QLRIL_SetNetworkSelectionModeManual(int *handle, int mccmnc);

int QLRIL_GetIccCardStatus(int *handle, void **resp);
int QLRIL_QueryFacilityLockForApp(int *handle, char *facility, char *password,
		int serviceClass, char *aid, int *scbv);
int QLRIL_SetFacilityLockForApp(int *handle, char *facility, int lockState,
		char *password, int serviceClass, char *aid, int *retries);

int QLRIL_SupplyIccPin(int *handle, char *pin, int *retries);
int QLRIL_SupplyIccPinForApp(int *handle, char *pin, char *aid, int *retries);
int QLRIL_SupplyIccPuk(int *handle, char *puk, char *newPin, int *retries);
int QLRIL_SupplyIccPukForApp(int *handle, char *puk, char *newPin, char *aid, int *retries);
int QLRIL_ChangeIccPin(int *handle, char *oldPin, char *newPin, int *retries);
int QLRIL_ChangeIccPinForApp(int *handle, char *oldPin, char *newPin, char *aid, int *retries);

int QLRIL_GetIMSI(int *handle, char *imsi, size_t size);
int QLRIL_GetIMSIForApp(int *handle, char *aid, char *imsi, size_t size);
int QLRIL_GetIMSIForUSIM(int *handle, char *imsi, size_t size);
int QLRIL_GetIMEI(int *handle, char *imei, size_t size);
int QLRIL_GetVoiceRadioTechnology(int *handle, int *radioTech);
int QLRIL_GetImsRegistrationState(int *handle, int *state, size_t size);

int QLRIL_IccIO(int *handle, int command, int fileid, char *path, int p1, int p2,
		int p3, char *data, char *pin2, void **resp);
int QLRIL_IccIOForApp(int *handle, int command, int fileid, char *path, int p1, int p2,
		int p3, char *data, char *pin2, char *aid, void **resp);

int QLRIL_GetPhoneNumber(int *handle, int appType, char *aid, char *number, size_t size);
int QLRIL_GetDeviceIdentity(int *handle, char ***resp);
int QLRIL_GetBasebandVersion(int *handle, char *resp, size_t size);
int QLRIL_GetSimIccId(int *handle, char *aid, char *iccid, size_t size);
int QLRIL_GetNITZTime(int *handle, char *nitz, size_t size);

int QLRIL_SendAtCmd(int *handle, char *cmd, char *resp, size_t size);/* DEPRECATED */
int QLRIL_SendAT(char *cmd, char *resp, size_t size);

int QLRIL_EnableRNDIS(char *ifname);
int QLRIL_DisableRNDIS(void);

typedef enum {
	RIL_UNSOL_IND_LISTENER_ENTER = 1049,
	RIL_UNSOL_IND_LISTENER_EXIT = 1050,
	IMS_UNSOL_RESPONSE_CALL_STATE_CHANGED = 1051, /* 201 */
	IMS_UNSOL_CALL_RING,
	IMS_UNSOL_IMS_NETWORK_STATE_CHANGED = 1054, /* 204 */
	IMS_UNSOL_SRV_STATUS_UPDATE = 1060, /* 210 */
	IMS_UNSOL_RADIO_STATE_CHANGED = 1063, /* 213 */ 
}UnsolInd_V01_e;

typedef void (*UnsolIndCallback)(int *handle, int slotId, int event, void *data, size_t size);

int QLRIL_AddUnsolEventsListener(int *handle, UnsolIndCallback ind_cb, void *privData);
int QLRIL_DelUnsolEventsListener(int *handle);

int QLRIL_GetPrivateData(int *handle, void **privData);

/**
 * Unsolicited response ID are reported in a band of
 * RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED (1000) ~
 * RIL_UNSOL_RESPONSE_ADN_RECORDS (1048) */
int QLRIL_RegisterUnsolEvents(int *handle, int start, int end);
int QLRIL_UnregisterUnsolEvents(int *handle, int start, int end);

int QLRIL_ParseEvent_NITZTime(char **dest, void *src, size_t size);
int QLRIL_ParseEvent_SignalStrength(void *dest, size_t dsize, void *src, size_t ssize);
int QLRIL_ParseEvent_RadioState(char *dest, size_t dsize, void *src, size_t ssize);
int QLRIL_ParseEvent_NewSMS(void *dest, size_t dsize, void *src, size_t ssize);
int QLRIL_ParseEvent_DataCallList(void **dest, void *src, size_t size);
int QLRIL_ParseEvent_CallRing(char *dest, size_t dsize, void *src, size_t ssize);
int QLRIL_ParseEvent_String(char **dest, void *src, size_t size);
int QLRIL_ParseEvent_Strings(char ***dest, void *src, size_t size);
int QLRIL_ParseEvent_Int(int *dest, void *src, size_t size);
int QLRIL_ParseEvent_Ints(int **dest, void *src, size_t size);
int QLRIL_ParseEvent_RawBytes(char **dest, void *src, size_t size);
int QLRIL_ParseEvent_CBS7BitGsm(char *pdu, size_t size, int *msgid, char **msg);
int QLRIL_ParseEvent_CdmaSMS(void **dest, void *src, size_t size);
int QLRIL_DecodeBearerData(void **out, uint8_t *in, size_t size);

/* GNSS API */
typedef void (*GNSSIndCallback)(int id, void *data, size_t size, void *priv);

/* Indication Messages Definition */
typedef enum {
	LOC_LOCATION_INFO_IND = 0,
	LOC_STATUS_INFO_IND,
	LOC_SV_INFO_IND,
	LOC_NMEA_INFO_IND,
	LOC_CAPABILITIES_INFO_IND,
	LOC_UTC_TIME_REQ_IND,
	LOC_XTRA_DATA_REQ_IND,//6
	LOC_AGPS_STATUS_IND,
	LOC_NI_NOTIFICATION_IND,
	LOC_XTRA_REPORT_SERVER_IND,//9
} LOC_INFO_IND_V01_e;

int QLRIL_GNSS_AddListener(int *handle, GNSSIndCallback gnsscb, void *privData);
int QLRIL_GNSS_DelListener(int *handle);
int QLRIL_GNSS_SetAttribute(int *handle, int mode, int recurrence, int minInterval, int prefAccuracy, int prefTime);
int QLRIL_GNSS_RegisterEvents(int *handle, int start, int end);
int QLRIL_GNSS_UnregisterEvents(int *handle, int start, int end);
int QLRIL_GNSS_StartNavigation(int *handle);
int QLRIL_GNSS_StopNavigation(int *handle);
int QLRIL_GNSS_GetLocation(int *handle, double *longitude, double *latitude, double *altitude, float *accuracy, int timeout);
int QLRIL_GNSS_StartServer(void);
int QLRIL_GNSS_StopServer(void);

/* IMS API */
int QLRIL_IMSServerState(int *on);
int QLRIL_IMSServerEnable(int *handle, int on); /* on: 1 - start IMS server, 0 - stop IMS server */
int QLRIL_IMSSendImsRegistrationState(int *handle, int state);
int QLRIL_IMSSendRoamingRegistrationState(int *handle, int state);
int QLRIL_IMSAcceptCall(int *handle, int type);
int QLRIL_IMSDial(int *handle, char *addr, int clir, int type, int domain, int pres, int encrypted);
int QLRIL_IMSGetCurrentCalls(int *handle, int slotId, int *id, void **resp);

int QLRIL_IMSParseUnsolMsg(char *msg, size_t size, void **resp); /* alloc memory*/
int QLRIL_IMSFreeResponse(int id, void **resp); /* free memory */

#ifdef __cplusplus
}
#endif

#endif
