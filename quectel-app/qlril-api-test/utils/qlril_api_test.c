/*
 *      Copyright:  (C) 2020 quectel
 *                  All rights reserved.
 *
 *       Filename:  qlril_api_test.c
 *    Description:  This file is used to test for qlrild daemon.
 *
 *        Version:  1.0.0
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on 2020.05.27
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

#include <RilPublic.h>

#include "c_printf.h"

static int callFromSimCard = -1;
static int CallAcceptIndex = -1;

typedef struct {
	int cmdIdx;
	char *funcName;
} st_api_test_case;

typedef struct {
	char *group_name;
	st_api_test_case *test_cases;
} func_api_test_t;

st_api_test_case api_testlist[] = {
	{0, "Help, Show all the API"},

	{-2, "========================================\n"
		 "General Functions\n"
		 "========================================"},
	{1, "QLRIL_Init"},
	{2, "QLRIL_Exit"},
	{3, "QLRIL_GetVersion"},
	{4, "QLRIL_GetSimCardSlotId"},
	{5, "QLRIL_SetSimCardSlotId"},
	{6, "QLRIL_SendAT"},
	{7, "QLRIL_ResetQcrild"},
	{8, "QLRIL_GetDefaultTimeout"},
	{9, "QLRIL_SetDefaultTimeout"},

	{-2, "========================================\n"
		 "Unsolicited Indication Callback\n"
		 "========================================"},
	{10, "QLRIL_AddUnsolEventsListener"},
	{11, "QLRIL_DelUnsolEventsListener"},
	{12, "QLRIL_RegisterUnsolEvents"},
	{13, "QLRIL_UnregisterUnsolEvents"},

	{-2, "========================================\n"
		 "Query and Settings\n"
		 "========================================"},
	{19, "QLRIL_GetIMSIForUSIM"},
	{20, "QLRIL_GetOperator"},
	{21, "QLRIL_GetIccCardStatus"},
	{22, "QLRIL_SetRadioPower"},
	{23, "QLRIL_SetScreenState"},
	{24, "QLRIL_RequestShutdown - DEPRECATED"},
	{25, "QLRIL_GetDeviceIdentity"},
	{26, "QLRIL_GetIMSI - DEPRECATED"},
	{27, "QLRIL_GetIMSIForApp"},
	{28, "QLRIL_GetIMEI - DEPRECATED"},
	{29, "QLRIL_GetSignalStrength"},
	{30, "QLRIL_GetPhoneNumber"},
	{31, "QLRIL_SupplyIccPin"},
	{32, "QLRIL_SupplyIccPuk"},
	{33, "QLRIL_ChangeIccPin"},
	{34, "QLRIL_QueryFacilityLockForApp"},
	{35, "QLRIL_SetFacilityLockForApp"},
	{36, "QLRIL_IccIOForApp"},
	{37, "QLRIL_GetNetworkSelectionMode"},
	{38, "QLRIL_GetBasebandVersion"},
	{39, "QLRIL_GetSimIccId"},
	{140, "QLRIL_GetNITZTime"},

	{-2, "========================================\n"
		 "Voice Call\n"
		 "========================================"},
	{40, "QLRIL_GetVoiceRegistrationState"},
	{41, "QLRIL_GetVoiceRadioTechnology"},
	{42, "QLRIL_GetImsRegistrationState"},
	{43, "QLRIL_Dial"},
	{44, "QLRIL_AcceptCall"},
	{45, "QLRIL_RejectCall"},
	{46, "QLRIL_GetCurrentCalls"},
	{47, "QLRIL_HangupConnection"},
	{48, "QLRIL_HangupWaitingOrBackground"},
	{49, "QLRIL_HangupForegroundResumeBackground"},
	{50, "QLRIL_GetMute"},
	{51, "QLRIL_SetMute"},
	{52, "QLRIL_SwitchWaitingOrHoldingAndActive"},
	{53, "QLRIL_Conference"},
	{54, "QLRIL_GetCurrentCallsByAtCmd"},
	{56, "QLRIL_IMSServerState"},
	{57, "QLRIL_IMSServerEnable"},
	{58, "QLRIL_IMSSendImsRegistrationState"},
	{59, "QLRIL_IMSSendRoamingRegistrationState"},


	{-2, "========================================\n"
		 "Send SMS\n"
		 "========================================"},
	{60, "QLRIL_SendSms"},
	{61, "QLRIL_SendSmsByPDU"},
	{62, "QLRIL_SendImsGsmSms"},
	{63, "QLRIL_SendImsGsmSmsByPDU"},

	{-2, "========================================\n"
		 "Data Network\n"
		 "========================================"},
	{70, "QLRIL_GetDataRegistrationState"},
	{71, "QLRIL_SetupDataCall"},
	{72, "QLRIL_SetDataAllowed"},
	{73, "QLRIL_GetDataCallList"},
	{74, "QLRIL_DeactivateDataCall"},
	{75, "QLRIL_GetPreferredNetworkType"},
	{76, "QLRIL_SetPreferredNetworkType"},
	{77, "QLRIL_AppendDNSForIPV4V6"},
	{78, "QLRIL_ClearResolvConfFile"},
	{79, "QLRIL_EnableRNDIS"},
	{80, "QLRIL_DisableRNDIS"},

	{-2, "========================================\n"
		 "GNSS Functions\n"
		 "========================================"},
	{90, "QLRIL_GNSS_AddListener"},
	{91, "QLRIL_GNSS_DelListener"},
	{92, "QLRIL_GNSS_RegisterEvents"},
	{93, "QLRIL_GNSS_UnregisterEvents"},
	{94, "QLRIL_GNSS_SetAttribute"},
	{95, "QLRIL_GNSS_StartNavigation"},
	{96, "QLRIL_GNSS_StopNavigation"},
	{97, "QLRIL_GNSS_GetLocation"},
	{98, "Run the GNSS APIs test"},
	{99, "QLRIL_GNSS_StartServer"},
	{100, "QLRIL_GNSS_StopServer"},
	{-1, "Exit!"}
};

func_api_test_t QLRIL_api_test = {"QL RIL API", api_testlist};

static void usage(func_api_test_t *pt_test)
{
	int i;

	printf("Copyright (C) 2020 Quectel, Smart Linux\n");
	printf("Group Name:%s, Supported test cases:\n", pt_test->group_name);

	for (i = 0;; i++) {
		if (pt_test->test_cases[i].cmdIdx == -2) {
			printf ("%s\n", pt_test->test_cases[i].funcName);
			continue;
		}

		printf("%d:\t%s\n", pt_test->test_cases[i].cmdIdx, pt_test->test_cases[i].funcName);
		if (pt_test->test_cases[i].cmdIdx == -1)
			break;
	}
}

void processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
	int i = 0;
	int num = 0;
	int ret = 0;
	char str[512] = {0};
	void *privData = NULL;
	static int times = 0;

	printf ("\n");
	if (times < 3) {
		times++;

		ret = QLRIL_GetPrivateData(handle, &privData);
		if (ret == 0) {
			c_printf("[b]%s%d\n", "QLRIL_GetPrivateData fool = ", *(int *)privData);
		}
	}

	c_printf ("[b]%s[y]%d[b]%s[y]%d\n", "The unsolicited event[", event, "] come from slot ID:", slotId);

	switch (event) {
		case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED://1000
		{
			char radioState[50] = {0};

			ret = QLRIL_ParseEvent_RadioState(radioState, sizeof(radioState), data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_RadioState failed with return:", ret);
			} else {
				/* Radio State:
				 * 0 - RADIO_OFF
				 * 1 - RADIO_UNAVAILABLE
				 * 2 - RADIO_SIM_NOT_READY
				 * 3 - RADIO_SIM_LOCKED_OR_ABSENT
				 * 4 - RADIO_SIM_READY
				 * 5 - RADIO_RUIM_NOT_READY
				 * 6 - RADIO_RUIM_READY
				 * 7 - RADIO_RUIM_LOCKED_OR_ABSENT
				 * 8 - RADIO_NV_NOT_READY
				 * 9 - RADIO_NV_READY
				 * 10 - RADIO_ON
				 */
				c_printf("[y]%s%s%s%d\n", "RadioState: [", radioState, "] ret = ", ret);
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED://1001
		{
			/* RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: Data is NULL */
			RIL_Call *call = NULL;

			/* This function should be moved to other thread
			 * not in the callback function
			 * and QLRIL_GetCurrentCalls query the call state
			 * is not real time, maybe it can be replaced by
			 * QLRIL_GetCurrentCallsByAtCmd
			 */
			ret = QLRIL_GetCurrentCalls(handle, slotId, (void **)&call);

			if (ret >= 0) {
				c_printf("[b]%s%d\n", "QLRIL_GetCurrentCalls items = ", ret);
				if (ret == 0) {
					callFromSimCard = -1;
					CallAcceptIndex = -1;
					c_printf("[b]%s\n", "All of GSM call end");
					break;
				}

				if (call == NULL) {
					c_printf("[y]%s\n", "RIL_Call memory pointer is NULL");
					break;
				}

				if (ret >= 20) {
					c_printf("[r]%s\n", "Warning!!! Calls items is too many");
				}

				for (i = 0; i < ret; i++) {
					c_printf("[y]%s%d\n", "state: ", call[i].state);
					if (call[i].state == 4) {
						callFromSimCard = slotId;
					}

					if (call[i].state == 0 ||
							call[i].state == 2 ||
							call[i].state == 3 ||
							call[i].state == 4) {//FIXME
						if (call[i].isVoice == 1) {//FIXME
							CallAcceptIndex = call[i].index;
						}
					}

					c_printf("[y]%s%d\n", "index: ", call[i].index);
					c_printf("[y]%s%d\n", "isVoice: ", call[i].isVoice);
					/* and so on TODO fix me */
					if (call[i].name) {
						c_printf("[y]%s[g]%s\n", "name: ", call[i].name);
						free(call[i].name);
						call[i].name = NULL;
					}

					if (call[i].number) {
						c_printf("[y]%s[g]%s\n", "Call Id number: ", call[i].number);
						free(call[i].number);
						call[i].number = NULL;
					}
				}
			} else {
				c_printf ("[r]%s%d\n", "QLRIL_GetCurrentCalls failed with return:", ret);
			}

			if (call) {
				free(call);
				call = NULL;
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED://1002
		{
			char **voiceRegistrationState = NULL;

			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED");
			/* Data is NULL */
			/* TODO fix me */
			/*
			 *  Called will invoke the following requests on main thread:
			 *  RIL_REQUEST_VOICE_REGISTRATION_STATE
			 *  RIL_REQUEST_OPERATOR
			 *  For more information refer to include/telephony/ril.h
			 */
			ret = QLRIL_GetVoiceRegistrationState(handle, slotId, &voiceRegistrationState);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetVoiceRegistrationState failed with return:", ret);
			} else {
				/*c_printf("[b]%s\n", "response[0]: e.g. 1 - Registered, home network\n"
				  "response[1]: LAC\n"
				  "response[2]: Cell ID\n"
				  "response[3]: refer to RIL_RadioTechnology in ril.h e.g.:\n"
				  "3  - UMTS; 14 - LTE; 16 - GSM;");*/
				c_printf("[y]%s%d\n", "QLRIL_GetVoiceRegistrationState return:", ret);
				for (i = 0; i < ret && i < 14; i++) {
					if (voiceRegistrationState[i] == NULL)
						continue;

					/* For more information to RIL_REQUEST_VOICE_REGISTRATION_STATE in
					 * include/telephony/ril.h
					 * response[0]:
					 *		1 - Registered, home network
					 *		2 - Not registered, but MT is currently searching a new operator to register
					 *		...
					 * response[1]: LAC
					 * response[2]: Cell ID
					 * response[3]: refer to RIL_RadioTechnology in ril.h e.g.:
					 *	3  - UMTS
					 *	14 - LTE
					 *	16 - GSM
					 */
					if (i == 0)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](1: Registered, home network; 2:Not registered...):",
								voiceRegistrationState[i]);
					else if (i == 2)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](Cell ID):",
								voiceRegistrationState[i]);
					else if (i == 3)
						c_printf("[g]%s%d%s%s\n", "response[", i, "](RadioTechnology: 3: UMTS; 14: LTE; 16:GSM):",
								voiceRegistrationState[i]);
					else
						c_printf("[g]%s%d%s%s\n", "response[", i, "]:", voiceRegistrationState[i]);
					free(voiceRegistrationState[i]);
				}

				if (voiceRegistrationState) {
					free(voiceRegistrationState);
					voiceRegistrationState = NULL;
				}
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS://1003
		{
			char *sms[3] = {0};
			ret = QLRIL_ParseEvent_NewSMS(&sms, sizeof(sms), data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_NewSMS failed with return:", ret);
			} else {
				if (sms[0]) {
					c_printf("[y]%s[g]%s", "Sms details:", sms[0]);
					free(sms[0]);
				}

				if (sms[1]) {
					c_printf("[y]%s[g]%s[y]%s\n", "Sms message: [", sms[1], "]");
					free(sms[1]);
				}

				if (sms[2]) {
					c_printf("[y]%s[g]%s[y]%s\n", "Sms raw data of pdu: [", sms[2], "]");
					free(sms[2]);
				}

				if (sms[0] || sms[1] || sms[2]) {
					ret = QLRIL_AcknowledgeLastIncomingGsmSms(handle, slotId, 1, 0xD3);
					if (ret == 0)
						c_printf("[g]%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
					else
						c_printf("[r]%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms failed with return:", ret);
					break;
				}
			}

			ret = QLRIL_AcknowledgeLastIncomingGsmSms(handle, slotId, 0, 0xD3);
			if (ret == 0)
				c_printf("[g]%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
			else
				c_printf("[r]%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms failed with return:", ret);

			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT://1004
		{
			char *sms_status = NULL;
			ret = QLRIL_ParseEvent_String(&sms_status, data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_String failed with return:", ret);
			} else {
				if (sms_status) {
					c_printf("[b]%s[g]%s\n", "the PDU of an SMS-STATUS-REPORT:", sms_status);
					free(sms_status);
				}
			}

			ret = QLRIL_AcknowledgeLastIncomingGsmSms(handle, slotId, 1, 0xD3);
			if (ret == 0)
				c_printf("[g]%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
			else
				c_printf("[r]%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms failed with return:", ret);
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM://1005
		{
			int *index = NULL;
			ret = QLRIL_ParseEvent_Ints((int **)&index, data, size);
			if (ret < 0)
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_Ints failed with return:", ret);
			else {
				c_printf("[y]%s\n", "The slot index on the SIM that contains the new message:");
				c_printf("[g]%s%d\n", "QLRIL_ParseEvent_Ints return:", ret);
				if (index == NULL)
					break;

				for(i = 0; i < ret; i++) {
					c_printf("[g]%s%d%s%d\n", "index[", i, "] = ", index[i]);
				}

				if (index) {
					free(index);
					index = NULL;
				}
			}

			break;
		}
		case RIL_UNSOL_NITZ_TIME_RECEIVED://1008
		{
			char *NITZTimeData = NULL;

			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_NITZ_TIME_RECEIVED");
			ret = QLRIL_ParseEvent_NITZTime(&NITZTimeData, data, size);
			if (ret < 0)
				c_printf("[r]%s%d\n", "parse NITZ Time failed with return:", ret);
			else {
				if (NITZTimeData) {
					c_printf("[g]%s%s\n", "NITZ Time: ", NITZTimeData);
					free(NITZTimeData);
				}
			}
			break;
		}
		case RIL_UNSOL_SIGNAL_STRENGTH://1009
		{
			RIL_SignalStrength_v10 ss;
			ret = QLRIL_ParseEvent_SignalStrength(&ss, sizeof(ss), data, size);
			if (ret != 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_SignalStrength failed:", ret);
				break;
			}

			c_printf("[y]%s%d%s\n", "SIM card[", slotId, "] signal strength Info:", slotId);
			/**
			 * UINT_MAX = 2147483647
			 */
			ret = 0;
			memset(str, 0, sizeof(str));
			if (ss.GW_SignalStrength.signalStrength != 99) {
				/* signalStrength: Valid values are (0-31, 99) as defined in TS 27.007 8.5
				 *
				 * bitErrorRate: bit error rate (0-7, 99) as defined in TS 27.007 8.5
				 */
				ret += snprintf(str + ret, sizeof(str) - ret, "GW: signalStrength[%d] bitErrorRate[%d]\n",
						ss.GW_SignalStrength.signalStrength, ss.GW_SignalStrength.bitErrorRate);
			}

			if (ss.CDMA_SignalStrength.dbm > 0 || ss.CDMA_SignalStrength.ecio > 0) {
				/* dbm: Valid values are positive integers.  This value is the actual RSSI value
				 *		multiplied by -1.  Example: If the actual RSSI is -75, then this response
				 *		value will be 75.
				 *
				 * ecio: Valid values are positive integers.  This value is the actual Ec/Io multiplied
				 *		by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
				 *		will be 125.
				 */
				ret += snprintf(str + ret, sizeof(str) - ret, "CDMA: dbm[%d] ecio[%d]\n",
						ss.CDMA_SignalStrength.dbm, ss.CDMA_SignalStrength.ecio);
			}

			if (ss.EVDO_SignalStrength.dbm > 0 || ss.EVDO_SignalStrength.ecio > 0 ||
					ss.EVDO_SignalStrength.signalNoiseRatio > 0) {
				/* dbm: Valid values are positive integers.  This value is the actual RSSI value
				 *		multiplied by -1.  Example: If the actual RSSI is -75, then this response
				 *		value will be 75.
				 *
				 * ecio: Valid values are positive integers.  This value is the actual Ec/Io multiplied
				 *		by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
				 *		will be 125.
				 *
				 * signalNoiseRatio: Valid values are 0-8.  8 is the highest signal to noise ratio.
				 */
				ret += snprintf(str + ret, sizeof(str) - ret, "EVDO: dbm[%d] ecio[%d] signalNoiseRatio[%d]\n",
						ss.EVDO_SignalStrength.dbm, ss.EVDO_SignalStrength.ecio,
						ss.EVDO_SignalStrength.signalNoiseRatio);
			}

			if (ss.LTE_SignalStrength.signalStrength != 99 || ss.LTE_SignalStrength.rsrp != INT_MAX) {
				/* signalStrength: Valid values are (0-31, 99) as defined in TS 27.007 8.5
				 *
				 * rsrp: The current Reference Signal Receive Power in dBm multipled by -1.
				 *		 Range: 44 to 140 dbm
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 *
				 * rsrq: The current Reference Signal Receive Quality in dB multiplied by -1.
				 *		 Range: 20 to 3 dB.
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 * 
				 * rssnr: The current reference signal signal-to-noise ratio in 0.1 dB units.
				 *		 Range: -200 to +300 (-200 = -20.0 dB, +300 = 30dB).
				 *		 INT_MAX : 0x7FFFFFFF denotes invalid value.
				 * 
				 * cqi: The current Channel Quality Indicator.
				 *		 Range: 0 to 15.
				 *		 INT_MAX : 0x7FFFFFFF denotes invalid value.
				 * Reference: 3GPP TS 36.101.
				 */
				ret += snprintf(str + ret, sizeof(str) - ret, "LTE: signalStrength[%d] ",
						ss.LTE_SignalStrength.signalStrength);
				ret += snprintf(str + ret, sizeof(str) - ret, "rsrp[%d] rsrq[%d] rssnr[%d]\n",
						ss.LTE_SignalStrength.rsrp, ss.LTE_SignalStrength.rsrq,
						ss.LTE_SignalStrength.rssnr);

				if (ss.LTE_SignalStrength.cqi != INT_MAX) {
					ret += snprintf(str + ret, sizeof(str) - ret, "LTE: cqi[%d]\n",
							ss.LTE_SignalStrength.cqi);
				}
			}

			if (ss.TD_SCDMA_SignalStrength.rscp > 0 && ss.TD_SCDMA_SignalStrength.rscp != INT_MAX) {
				/* rscp: The Received Signal Code Power in dBm multipled by -1.
				 *		 Range : 25 to 120
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 */
				ret += snprintf(str + ret, sizeof(str) - ret, "TDSCDMA: rscp[%d]\n",
						ss.TD_SCDMA_SignalStrength.rscp);
			}

			str[sizeof(str) - 1] = 0;//prevent stack overflows
			if (strlen(str) > 0) {
				c_printf("[g]%s\n", str);
			}
			break;
		}
		case RIL_UNSOL_DATA_CALL_LIST_CHANGED://1010
		{
			RIL_Data_Call_Response_v11 *dc = NULL;
			ret = QLRIL_ParseEvent_DataCallList((void **)&dc, data, size);
			if (ret < 0) {
				c_printf("[b]%s%d\n", "QLRIL_ParseEvent_DataCallList failed with return: ", ret);
			} else {
				c_printf ("[b]%s%d\n", "QLRIL_ParseEvent_DataCallList items = ", ret);

				if (ret == 0)
					break;

				if (dc == NULL) {
					c_printf("[r]%s\n", "Error RIL_Data_Call_Response_v11 memory pointer is NULL");
					break;
				}

				if (ret >= 20) {
					c_printf("[r]%s\n", "Warning!!! Data call list items is too many");
				}

				memset(str, 0, sizeof(str));
				for (i = 0; i < ret; i++) {
					c_printf ("[b]%s\n", "DataCallList response:");
					snprintf (str, sizeof(str), "status:%d, retry:%d, callID:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s, pcscf:%s, mtu:%d\n",
							dc[i].status, dc[i].suggestedRetryTime,
							dc[i].cid, (dc[i].active == 0)?"down":"up",
							dc[i].type, dc[i].ifname, dc[i].addresses,
							dc[i].dnses, dc[i].gateways, dc[i].pcscf, dc[i].mtu);
					str[sizeof(str) - 1] = 0;//prevent stack overflows
					c_printf("[g]%s\n", str);

					if (dc[i].type)
						free(dc[i].type);
					if (dc[i].ifname)
						free(dc[i].ifname);
					if (dc[i].addresses)
						free(dc[i].addresses);
					if (dc[i].dnses)
						free(dc[i].dnses);
					if (dc[i].gateways)
						free(dc[i].gateways);
					if (dc[i].pcscf)
						free(dc[i].pcscf);
				}
			}

			if (dc) {
				free(dc);
				dc = NULL;
			}

			break;
		}
		case RIL_UNSOL_STK_SESSION_END://1012
			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_STK_SESSION_END");
			break;
		case RIL_UNSOL_STK_PROACTIVE_COMMAND://1013
		{
			char *sat_usat = NULL;

			ret = QLRIL_ParseEvent_String(&sat_usat, data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_String failed with return:", ret);
			} else {
				if (sat_usat) {
					c_printf("[b]%s[g]%s\n", "SAT/USAT String: ", sat_usat);
					free(sat_usat);
				}
			}
			break;
		}
		case RIL_UNSOL_CALL_RING://1018
		{
			/*
			 * "data" is NULL for GSM
			 * "data" is const RIL_CDMA_SignalInfoRecord * if CDMA
			 * RIL_CDMA_SignalInfoRecord => char resp[4]
			 */
			char resp[4];
			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_CALL_RING");
			ret = QLRIL_ParseEvent_CallRing(resp, sizeof(resp), data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_CallRing failed with return:", ret);
			} else {
				c_printf("[y]%s\n", "QLRIL_ParseEvent_CallRing success");
				for (i = 0; i < sizeof(resp); i++) {
					c_printf("[g]%s%d%s[y]%d\n", "response[", i, "] = ", resp[i]);
				}
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED://1019
		{
			RIL_CardStatus_v6 *cardStatus = NULL;
			ret = QLRIL_GetIccCardStatus(handle, (void **)&cardStatus);
			if (ret < 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GetIccCardStatus failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "QLRIL_GetIccCardStatus ret = ", ret);
				if (cardStatus == NULL)
					break;
				/**
				 * Card state:
				 * 0: absent;
				 * 1: present
				 * 2: error;
				 * 3: restricted
				 */
				c_printf ("[b]%s[g]%d\n", "card_state(e.g. 0:absent; 1:present;): ", cardStatus->card_state);
				/**
				 * Pin state:
				 * 0: Unknown;
				 * 1: Enable not verified
				 * 2: Enable verified
				 * 3: Disabled
				 * 4: Enable blocked
				 * 5: Enable perm blocked
				 */
				c_printf ("[b]%s[g]%d\n", "universal_pin_state(e.g. 0:unknown;): ",
						cardStatus->universal_pin_state);
				c_printf ("[b]%s[g]%d\n", "gsm_umts_subscription_app_index: ",
						cardStatus->gsm_umts_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "cdma_subscription_app_index: ",
						cardStatus->cdma_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "ims_subscription_app_index: ",
						cardStatus->ims_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "num_applications: ", cardStatus->num_applications);
				for (i = 0; i < cardStatus->num_applications; i++) {
					/**
					 * APP Type:
					 * 0: Unknown;
					 * 1: SIM;
					 * 2: USIM;
					 * 3: RUIM;
					 * 4: CSIM;
					 * 5: ISIM;
					 */
					if (cardStatus->applications[i].app_type > RIL_APPTYPE_ISIM) {
						break;//something wrong
					}

					c_printf ("[b]%s[g]%d\n", "app_type(e.g 1:SIM; 2:USIM 3:RUIM 4:CSIM 5:ISIM): ",
							cardStatus->applications[i].app_type);
					/**
					 * APP state:
					 * 0: Unknown;
					 * 1: Detected;
					 * 2: If PIN1 or UPin is required
					 * 3: If PUK1 or Puk for UPin is required
					 * 4: perso_substate should be look at when app_state is assigned to this value
					 * 5: Ready
					 */
					c_printf ("[b]%s[g]%d\n", "app_state(e.g. 2: Pin req; 3: PUK req; 5:Ready): ",
							cardStatus->applications[i].app_state);
					/**
					 * applicable only if app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO
					 * Perso substate:
					 * 0: Unknown;
					 * 1: In progress;
					 * 2: Ready;
					 * ...
					 * For more information refer to ril.h(RIL_PersoSubstate)
					 */
					if (cardStatus->applications[i].app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO)
						c_printf ("[b]%s[g]%d\n", "perso_substate(e.g. 2: Ready): ",
								cardStatus->applications[i].perso_substate);

					if (cardStatus->applications[i].aid_ptr) {
						c_printf ("[b]%s[g]%s\n", "aid_ptr: ", cardStatus->applications[i].aid_ptr);
						free(cardStatus->applications[i].aid_ptr);
					}

					if (cardStatus->applications[i].app_label_ptr) {
						c_printf ("[b]%s[g]%s\n", "app_label_ptr: ",
								cardStatus->applications[i].app_label_ptr);
						free(cardStatus->applications[i].app_label_ptr);
					}

					if (cardStatus->applications[i].app_type == RIL_APPTYPE_USIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_CSIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_ISIM)
						c_printf ("[b]%s[g]%d\n", "pin1_replaced: ",
								cardStatus->applications[i].pin1_replaced);
					/**
					 * Pin state:
					 * 0: Unknown;
					 * 1: Enable not verified
					 * 2: Enable verified
					 * 3: Disabled
					 * 4: Enabled blocked
					 * 5: Enabled perm blocked
					 */
					c_printf ("[b]%s[g]%d\n", "pin1(e.g. 1:Enable not verified, 2:Enable verified): ",
							cardStatus->applications[i].pin1);
					c_printf ("[b]%s[g]%d\n", "pin2(e.g. 3:Disabled, 4:Enable blocked): ",
							cardStatus->applications[i].pin2);
				}
			}

			if (cardStatus)
				free(cardStatus);
			cardStatus = NULL;
			break;
		}
		case RIL_UNSOL_RESPONSE_CDMA_NEW_SMS: //1020
		{
			RIL_CDMA_SMS_Message *sms = NULL;
			RIL_CDMA_SMS_ClientBd *cbd = NULL;
			ret = QLRIL_ParseEvent_CdmaSMS((void **)&sms, data, size);
			if (ret != 0)
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_CdmaSMS error:", ret);
			else {
				c_printf("[g]%s%s\n", "address:", sms->sAddress.digits);
				c_printf("[g]%s:", "bearer data:");
				for (i = 0; i < sms->uBearerDataLen; i++) {
					printf("%02X ", sms->aBearerData[i]);
				}
				printf ("\n");

				ret = QLRIL_DecodeBearerData((void **)&cbd, sms->aBearerData, sms->uBearerDataLen);
				if (ret != 0)
					c_printf("[r]%s%d\n", "QLRIL_DecodeBearerData error:", ret);
				else {
					c_printf("[g]%s%d\n", "type:", cbd->message_id.type);
					c_printf("[g]%s%d\n", "id_number:", cbd->message_id.id_number);
					c_printf("[g]%s%d\n", "udh_present:", cbd->message_id.udh_present);
					c_printf("[g]%s%d\n", "encoding:", cbd->user_data.encoding);
					c_printf("[g]%s%d\n", "number_of_digits:", cbd->user_data.number_of_digits);
					c_printf("[g]%s%s\n", "content:", cbd->user_data.data);
					c_printf("[g]%s%d\n", "data_len:", cbd->user_data.data_len);
					c_printf("[g]%s%d\n", "padding_bits:", cbd->user_data.padding_bits);
					c_printf("[g]%s", "Current Time:");
					printf("%x-%x-%x-%x-%x-%x\n", cbd->mc_time.year, cbd->mc_time.month, cbd->mc_time.day,
							cbd->mc_time.hour, cbd->mc_time.minute, cbd->mc_time.second);
					c_printf("[g]%s%d\n", "Priority:", cbd->priority);
				}
			}

			if (sms) {
				free(sms);
				sms = NULL;
			}

			if (cbd) {
				free(cbd);
				cbd = NULL;
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS://1021
		{
			int msgid = 0;
			char *msg = NULL;
			char *cbs_pdu = NULL;
			char *warning[] = {
				"Earthquake", //4352
				"Tsunami", //4353
				"Earthquake and Tsunami", //4354
				"Test", //4355
				"Emergency", //4356
				"ALERT", //4370
				NULL
			};
			c_printf("[y]%s%d\n", "Receive event: RIL_UNSOL_RESPONSE_NEW_BROADCAST_SMS size:", size);
			ret = QLRIL_ParseEvent_RawBytes(&cbs_pdu, data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_RawBytes failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "QLRIL_ParseEvent_RawBytes return length:", ret);
				if (ret > 0 && cbs_pdu) {
					for (i = 0; i < ret; i++) {
						c_printf("[g]%02X ", cbs_pdu[i]);
					}
					printf("\n");

					ret = QLRIL_ParseEvent_CBS7BitGsm(cbs_pdu, ret, &msgid, &msg);
					if (ret <= 0)
						if (msgid) {
							c_printf("[r]%s%d\n", "SRV_ID:", msgid);
						} else
							c_printf("[r]%s%d\n", "QLRIL_ParseEvent_CBS7BitGsm failed with return:", ret);
					else {
						if (msgid >= 4352 && msgid <= 4356)
							printf("SRV_ID:%d[%s], Message:%s\n", msgid, warning[msgid - 4352],
									(msg == NULL)?"NONE":msg);
						else
							printf("SRV_ID:%d[ALERT], Message:%s\n", msgid, (msg == NULL)?"Unknown":msg);
						if (msg) {
							free(msg);
							msg = NULL;
						}
					}
					free(cbs_pdu);
					cbs_pdu = NULL;
				}
			}
			break;
		}
		case RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE://1024
		{
			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_ENTER_EMERGENCY_CALLBACK_MODE");
			break;
		}
		case RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE://1033
		{
			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_EXIT_EMERGENCY_CALLBACK_MODE");
			break;
		}
		case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED://1035
		{
			int radioTech;
			ret = QLRIL_ParseEvent_Int(&radioTech, data, size);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_Int failed with return:", ret);
			} else {
				c_printf("[y]%s%d\n", "QLRIL_ParseEvent_Int success with return:", ret);
				c_printf("[g]%s%d\n", "Radio technology(refer to RIL_RadioTechnology):", radioTech);
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED: //1037
		{
			/* To get IMS registration state and IMS SMS format,
			 * callee needs to invoke the following request on main thread */
			int state[3] = {0};
			ret = QLRIL_GetImsRegistrationState(handle, state, sizeof(state));
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetImsRegistrationState failed with return:", ret);
			else {
				c_printf ("[y]%s%d\n", "current IMS registration return:", ret);
				for (i = 0; i < ret; i++) {
					if (i == 0)
						c_printf ("[g]%s%d\n", "Registration state(0 - Not registered; 1 - Registered): ", state[i]);
					else if (i == 1 && state[0] == 1) {
						/**
						 * RIL_RadioTechnologyFamily:
						 * 1: 3GPP Technologies - GSM, WCDMA
						 * 2: 3GPP2 Technologies - CDMA
						 */
						c_printf ("[g]%s%d\n", "type in RIL_RadioTechnologyFamily: ", state[i]);
					}
				}
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_ADN_INIT_DONE://1047
			c_printf("[y]%s\n", "Receive event: RIL_UNSOL_RESPONSE_ADN_INIT_DONE");
			break;
		case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED:
		{
			int *uicc_subscription_status = NULL;
			ret = QLRIL_ParseEvent_Ints((int **)&uicc_subscription_status, data, size);
			if (ret < 0)
				c_printf("[r]%s%d\n", "QLRIL_ParseEvent_Ints failed with ret = ", ret);
			else {
				c_printf("[g]%s%d\n", "QLRIL_ParseEvent_Ints ret = ", ret);
				if (ret == 0)
					break;

				if (uicc_subscription_status == NULL) {
					c_printf("[r]%s\n", "Error uicc_subscription_status is NULL");
					break;
				}

				c_printf("[y]%s\n", "0 - Subscription Deactivated; 1 - Subscription Activated");
				for(i = 0; i < ret; i++) {
					c_printf("[g]%s%d%s[y]%d\n", "status[", i, "] = ", uicc_subscription_status[i]);
				}
			}
			break;
		}
		case RIL_UNSOL_IND_LISTENER_ENTER: //1049
			c_printf("[y]%s\n", "The listener has been started");
			break;
		case RIL_UNSOL_IND_LISTENER_EXIT: //1050
		{
			int reason = *(int *)data;
			c_printf("[y]%s%d\n", "The listener has been stopped, reason:", reason);
			break;
		}
		case IMS_UNSOL_RESPONSE_CALL_STATE_CHANGED: //1051
		{//201 = ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED
			int id = 0;
			void *resp = NULL;

			c_printf("[y]%s%d\n", "unpacked message size:", size);

			if (size <= 0) {
				ret = QLRIL_IMSGetCurrentCalls(handle, slotId, &id, &resp);
			} else {
				ret = size;
				resp = data;
			}

			if (ret <= 0 || resp == NULL) {
				c_printf("[g]%s\n", "Some/All of IMS call end");
				callFromSimCard = -1;
				break;
			}

			callFromSimCard = slotId;

			ims_CallList *call_list = (ims_CallList *)resp;
			ims_CallList_Call **call = (ims_CallList_Call **)call_list->callAttributes.arg;
			if (call != NULL) {
				c_printf("[y]%s\n", "IMS Call state hint: 0-ACTIVE, 1-HOLDING, 2-DIALING,");
				c_printf("[y]%s\n", "3-ALERTING, 4-INCOMING, 5-WAITING, 6-END");
				for (i = 0; call[i] != NULL; i++) {
					ret = 0;
					memset(str, 0, sizeof(str));
					if (call[i]->has_index)
						ret += snprintf(str + ret, sizeof(str) - ret, "IMS Call index[%d]", call[i]->index);
					if (call[i]->number.arg)
						ret += snprintf(str + ret, sizeof(str) - ret, " Number[%s]", (char *)call[i]->number.arg);
					if (call[0]->has_state)
						ret += snprintf(str + ret, sizeof(str) - ret," state[%d]", call[i]->state);
					c_printf("[g]%s\n", str);
				}
			}

			if (id == ims_MsgId_REQUEST_GET_CURRENT_CALLS) {
				/* you must use this to release resp */
				ret = QLRIL_IMSFreeResponse(id, &resp);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLRIL_IMSFreeResponse failed:", ret);
			}

			break;
		}
		case IMS_UNSOL_CALL_RING:
		{//202 = ims_MsgId_UNSOL_CALL_RING
			c_printf("[y]%s%d\n", "Receive event: IMS_UNSOL_CALL_RING, size = ", size);
			break;
		}
		case IMS_UNSOL_IMS_NETWORK_STATE_CHANGED: //1054
		{//204 = ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
			c_printf("[y]%s\n", "Receive event: IMS_UNSOL_MS_NETWORK_STATE_CHANGED");

			c_printf("[y]%s%d\n", "unpacked message size:", size);
			if (size > 0 && data != NULL) {
				ims_Registration *reg = (ims_Registration *)data;
				ret = 0;
				memset(str, 0, sizeof(str));
				if (reg->has_state) {
					c_printf("[y]%s\n", "state: 1:regstered, 2:not regstered, 3:regstering");
					ret += snprintf(str + ret, sizeof(str) - ret, "state:%d ", reg->state);
				}
				if (reg->has_errorCode)
					ret += snprintf(str + ret, sizeof(str) - ret, "errorCode:%d ", reg->errorCode);
				if (reg->has_radioTech)
					ret += snprintf(str + ret, sizeof(str) - ret, "radioTech:%d ", reg->radioTech);
				c_printf("[g]%s\n", str);
			}

			break;
		}
		case IMS_UNSOL_SRV_STATUS_UPDATE:
		{//210 = ims_MsgId_UNSOL_SRV_STATUS_UPDATE
			c_printf("[y]%s\n", "Receive event: ims_MsgId_UNSOL_SRV_STATUS_UPDATE(210)");

			c_printf("[y]%s%d\n", "unpacked message size:", size);
			if (size > 0 && data != NULL) {
				ims_SrvStatusList *sslist = (ims_SrvStatusList *)data;
				ims_Info **info = (ims_Info **)sslist->SrvStatusInfo.arg;
				if (info != NULL) {
					for (i = 0; info[i] != NULL; i++) {
						ret = 0;
						memset(str, 0, sizeof(str));
						if (info[i]->has_isValid)
							ret += snprintf(str + ret, sizeof(str) - ret, "isValid:%d ", info[i]->isValid);
						if (info[i]->has_type)
							ret += snprintf(str + ret, sizeof(str) - ret, "type:%d ", info[i]->type);
						if (info[i]->has_callType)
							ret += snprintf(str + ret, sizeof(str) - ret, "callType:%d ", info[i]->callType);
						if (info[i]->has_status)
							ret += snprintf(str + ret, sizeof(str) - ret, "status:%d ", info[i]->status);
						c_printf("[g]%s\n", str);
					}
				}
			}

			break;
		}
		case IMS_UNSOL_RADIO_STATE_CHANGED: //1063
		{//213 = ims_MsgId_UNSOL_RADIO_STATE_CHANGED
			c_printf("[y]%s\n", "Receive event: ims_MsgId_UNSOL_RADIO_STATE_CHANGED(213)");

			c_printf("[y]%s%d\n", "unpacked message size:", size);
			if (size > 0 && data != NULL) {
				ims_RadioStateChanged *rsc = (ims_RadioStateChanged *)data;
				if (rsc->has_state) {
					c_printf("[g]%s\n", "Radio State: 0 - OFF, 1 - UNAVAILABLE, 10 - ON");
					c_printf("[g]%s%d\n", "Radio State Changed: ", rsc->state);
				}
			}

			break;
		}
		default:
		{
			if (event > 1050 && event < 1070)
				c_printf("[y]%s%d\n", "IMS unsolicited event:", event - 1050 + 200);
			else
				c_printf("[r]%s%d\n", "Unknown unsolicited event:", event);
			break;
		}
	}
}

void gnss_ind_callback(int msgId, void *indData, size_t indLen, void *priv)
{
	int i;
	int *handle = (int *)priv;

	switch(msgId) {
	case LOC_LOCATION_INFO_IND://0
	{
		ql_loc_location_info_ind_msg_v01 *ind = (ql_loc_location_info_ind_msg_v01*)indData;
		ql_gps_location_t_v01* loc = (ql_gps_location_t_v01*)&ind->location;

		c_printf ("[y]%s\n", "Current location:");
		printf("%s%lf\n", "[LOC_LOCATION_INFO] longitude:", loc->longitude);
		printf("%s%lf\n", "[LOC_LOCATION_INFO] latitude:", loc->latitude);
		printf("%s%lf\n", "[LOC_LOCATION_INFO] altitude:", loc->altitude);
		printf("%s%lf\n", "[LOC_LOCATION_INFO] accuracy:", loc->accuracy);
		printf("%s%lf\n", "[LOC_LOCATION_INFO] speed:", loc->speed);
		printf("%s%lf\n", "[LOC_LOCATION_INFO] bearing:", loc->bearing);
		//printf("%s%s\n",  "[LOC_LOCATION_INFO] map_url:", loc->map_url);
		break;
	}
	case LOC_STATUS_INFO_IND://1
	{
		ql_loc_status_info_ind_msg_v01 *status_info = (ql_loc_status_info_ind_msg_v01*)indData;

		c_printf("[y]%s[g]%d\n",  "[LOC_STATUS_INFO] status:", status_info->status.status);
		break;
	}
	case LOC_SV_INFO_IND://2
	{
		ql_loc_sv_info_ind_msg_v01 *ind = (ql_loc_sv_info_ind_msg_v01*)indData;

		c_printf("[y]%s[g]0x%X, 0x%X, 0x%X\n", "[LOC_SV_INFO] mask of ephemeris, almanac, used_in_fix",
				ind->sv_info.ephemeris_mask, ind->sv_info.almanac_mask, ind->sv_info.used_in_fix_mask);
		for (i = 0; i < ind->sv_info.num_svs; i++) {
			printf("[LOC_SV_INFO] prn=%d, snr=%lf, elevation=%lf, azimuth=%lf\n",
					ind->sv_info.sv_list[i].prn, ind->sv_info.sv_list[i].snr,
					ind->sv_info.sv_list[i].elevation, ind->sv_info.sv_list[i].azimuth);
		}
		break;
	}
	case LOC_NMEA_INFO_IND://3
	{
		ql_loc_nmea_info_ind_msg_v01 *ind = (ql_loc_nmea_info_ind_msg_v01*)indData;

		printf("[LOC_NMEA_INFO] timestamp:[%lld]\n", ind->timestamp);
		c_printf("[y]%s[g]%d\n",  "[LOC_NMEA_INFO] length:", ind->length);
		c_printf("[y]%s[g]%s\n",  "[LOC_NMEA_INFO] nmea:", ind->nmea);
		break;
	}
	case LOC_CAPABILITIES_INFO_IND://4
	{
		ql_loc_capabilities_info_ind_msg_v01 *ind = (ql_loc_capabilities_info_ind_msg_v01*)indData;
		c_printf("[y]%s[g]%d\n",  "[LOC_CAPABILITIES_INFO] capabilities:", ind->capabilities);
		break;
	}
	case LOC_AGPS_STATUS_IND://7
	{
		ql_loc_agps_status_ind_msg_v01 *ind = (ql_loc_agps_status_ind_msg_v01*)indData;
		ql_agps_status_t_v01 *status = (ql_agps_status_t_v01 *)&ind->status;

		c_printf("[y]%s[g]%d\n",  "[LOC_AGPS_STATUS_INFO] type:", status->type);
		c_printf("[y]%s[g]%d\n",  "[LOC_AGPS_STATUS_INFO] status:", status->status);
		c_printf("[y]%s[g]%s\n",  "[LOC_AGPS_STATUS_INFO] ipv4 address:", status->ipv4_addr);
		//FIXME
		//status->ipv6_addr
		//status->ssid
		//status->password
		break;
	}
	case LOC_NI_NOTIFICATION_IND://8
	{
		ql_loc_ni_notification_ind_msg_v01 *ind = (ql_loc_ni_notification_ind_msg_v01*)indData;

		c_printf("[y]%s[g]%d\n",  "[LOC_NI_NOTIFICATION_IND] notification_id:", ind->notification.notification_id);
		c_printf("[y]%s[g]%d\n",  "[LOC_NI_NOTIFICATION_IND] ni_type:", ind->notification.ni_type);
		//FIXME
		//ind->notification.notify_flags
		//ind->notification.timeout
		//ind->notification.default_response
		//...
		//refer to gnss/ql_loc_v01.h
		break;
	}
	case LOC_XTRA_REPORT_SERVER_IND://9
	{
		ql_loc_xtra_report_server_ind_msg_v01 *ind = (ql_loc_xtra_report_server_ind_msg_v01*)indData;

		c_printf("[y]%s[g]%s\n",  "[LOC_XTRA_REPORT_SERVER] server1:", ind->server1);
		c_printf("[y]%s[g]%s\n",  "[LOC_XTRA_REPORT_SERVER] server2:", ind->server2);
		c_printf("[y]%s[g]%s\n",  "[LOC_XTRA_REPORT_SERVER] server3:", ind->server3);

		break;
	}
	case LOC_UTC_TIME_REQ_IND://5
	case LOC_XTRA_DATA_REQ_IND://6
	default:
		c_printf("[y]%s[g]%d\n",  "GNSS receive message id: ", msgId);
		break;
	}

	return;
}

static int getInputIntVal(void)
{
	int n = 0;
	int ret = 0;
	char cmdStr[10] = {0};
	char *p = NULL;
	char ch;

	memset(cmdStr, 0, sizeof(cmdStr));
	if (fgets(cmdStr, sizeof(cmdStr), stdin) == NULL)
		return -1;

	if (cmdStr[0] == '\n')
		return -2;

	p = cmdStr;
	while (*p) {
		if (sscanf(p, "%d%n", &ret, &n) == 1) {
			p += n;
		} else
			++p;
	}

	return ret;
}

static int getInputHexVal(void)
{
	int n = 0;
	int ret = 0;
	char cmdStr[10] = {0};
	char *p = NULL;
	char ch;

	memset(cmdStr, 0, sizeof(cmdStr));
	if (fgets(cmdStr, sizeof(cmdStr), stdin) == NULL)
		return -1;

	if (cmdStr[0] == '\n')
		return -2;

	p = cmdStr;
	while (*p) {
		if (sscanf(p, "%x%n", &ret, &n) == 1) {
			p += n;
		} else
			++p;
	}

	return ret;
}

static int getInputString(char *str, size_t size)
{
	int i = 0;
	int ret = 0;

	if (str == NULL || size <= 0) {
		return -1;
	}

	memset(str, 0, size);
	if (fgets(str, size, stdin) == NULL)
		return 0;

	for (i = 0; i < size; i++) {
		if (str[i] == '\n') {
			str[i] = '\0';
			break;
		}
	}

	ret = strlen(str);

	return ret;
}

static int test_QLRIL_api(void)
{
	int ret = 0;
	int on = 0;
	int handle = 0;
	int slotId = 0;
	int fool = 0;
	int type = 0;
	int cmdIdx = 0;
	int start, end, i, num;

	char ch = 0;
	char areaCode[10] = {0};
	char addr[30] = {0};
	char msg[200] = {0};
	char buf[512] = {0};

	if (handle == 0) {
		for (i = 0; i < 5; i++) {
			ret = QLRIL_Init(&handle);
			if (ret == 0) {
				c_printf("[g]%s\n", "QLRIL_Init success");

				/*
				ret = QLRIL_AddUnsolEventsListener(&handle, processUnsolInd_cb, NULL);
				if (ret != 0)
					c_printf ("[r]%s%d\n", "QLRIL_AddUnsolEventsListener failed with return:", ret);

				ret = QLRIL_RegisterUnsolEvents(&handle, 1000, 1070);
				if (ret != 0)
					c_printf ("[r]%s%d\n", "QLRIL_RegisterUnsolEvents failed with return:", ret);

				ret = QLRIL_UnregisterUnsolEvents(&handle, 1009, 1009);
				if (ret != 0)
					c_printf ("[r]%s%d\n", "QLRIL_UnregisterUnsolEvents failed with return:", ret);
				*/
				break;
			}

			sleep(1);
		}

		if (handle == 0 || i >= 5) {
			c_printf("[r]%s%d\n", "QLRIL_Init failed with return:", ret);
		}
	}

	memset(buf, 0, sizeof(buf));
	ret = QLRIL_GetVersion(buf, sizeof(buf));
	if (ret != 0)
		c_printf("[r]%s%d\n", "QLRIL_GetVersion failed with return:", ret);
	else
		c_printf("[g]%s%s\n", "QLRIL library current version:", buf);

	usage(&QLRIL_api_test);
	while(1) {
		printf("\n");
		c_printf("[y]%s", "Please input cmd index (-1: exit, 0: help):");
		cmdIdx = getInputIntVal();

		if (cmdIdx == -1) {
			if (handle != 0) {
				if (QLRIL_Exit(&handle) == 0)
					c_printf("[g]%s\n", "QLRIL_Exit success");
				else
					c_printf("[r]%s\n", "QLRIL_Exit failure");
			}
			break;
		}

		if (cmdIdx == -2)
			continue;

		switch (cmdIdx) {
		case 0:
			usage(&QLRIL_api_test);
			break;
		case 1:
			if (handle == 0) {
				ret = QLRIL_Init(&handle);
				if (ret == 0)
					c_printf("[g]%s\n", "QLRIL_Init success");
				else
					c_printf("[r]%s%d\n", "QLRIL_Init failed with return:", ret);
			} else {
				c_printf("[y]%s[y]%08X\n", "QLRIL_Init has executed already with handle = 0x", handle);
			}
			break;
		case 2:
			if (QLRIL_Exit(&handle) == 0)
				c_printf("[g]%s\n", "QLRIL_Exit success");
			else
				c_printf("[r]%s\n", "QLRIL_Exit failure");
			break;
		case 3:
			memset(buf, 0, sizeof(buf));
			ret = QLRIL_GetVersion(buf, sizeof(buf));
			if (ret == 0) {
				c_printf("[g]%s%s\n", "QLRIL library version: ", buf);
			} else
				c_printf("[r]%s%d\n", "QLRIL_GetVersion failed with return:", ret);
			break;
		case 4:
			ret = QLRIL_GetSimCardSlotId(&handle, &slotId);
			if (ret == 0) {
				c_printf("[g]%s%d\n", "Current SIM card slot ID: ", slotId);
			} else
				c_printf("[r]%s%d\n", "QLRIL_GetSimCardSlotId failed with return:", ret);
			break;
		case 5:
			c_printf ("[y]%s", "Please input SIM card slot ID (1 or 2):");
			slotId = getInputIntVal();

			ret = QLRIL_SetSimCardSlotId(&handle, slotId);
			if (ret == 0)
				c_printf("[g]%s\n", "QLRIL_SetSimCardSlotId success");
			else
				c_printf("[r]%s%d\n", "QLRIL_SetSimCardSlotId failed with return: ", ret);
			break;
		case 6:
		{
			char cmd[520] = {0};
			char cmd_resp[1024] = {0};

			for (;;) {
				c_printf("[y]%s", "Please input AT command(-1: exit):");
				ret = getInputString(cmd, sizeof(cmd));
				if (ret < 2)
					continue;

				if (strstr(cmd, "-1") != NULL){
					break;
				}

				memset(cmd_resp, 0, sizeof(cmd_resp));
				ret = QLRIL_SendAT(cmd, cmd_resp, sizeof(cmd_resp));
				if (ret < 0)
					c_printf ("[r]%s%d\n", "QLRIL_SendAT failed with return:", ret);
				else {
					c_printf ("[y]%s\n", "AT command response:");
					c_printf ("[g]%s\n", cmd_resp);
				}
			}

			break;
		}
		case 7:
		{
			int which = 0;
			int timeout = 0;

			c_printf ("[y]%s", "Please input which qcrild to reset(1,2,3; -1:exit):");//3 kill all qcrild
			which = getInputIntVal();
			if (which == -1)
				break;

			c_printf ("[y]%s", "Please input value of timeout(>= 0, Suggest: 20):");
			timeout = getInputIntVal();//second

			ret = QLRIL_ResetQcrild(which, timeout);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_ResetQcrild failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "QLRIL_ResetQcrild success with return:", ret);
			}
			break;
		}
		case 8:
		{
			int timeout = 0;

			ret = QLRIL_GetDefaultTimeout(&handle, &timeout);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetDefaultTimeout failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "The QLRIL default timeout:", timeout);
			}
			break;
		}
		case 9:
		{
			int timeout = 0;

			c_printf ("[y]%s", "Please input the default timeout(0 ~ 60, Suggest: 5):");
			timeout = getInputIntVal();//second

			ret = QLRIL_SetDefaultTimeout(&handle, timeout);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_SetDefaultTimeout failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "QLRIL_SetDefaultTimeout success with return:", ret);
			}
			break;
		}
		case 10:
			fool = 12345;//for test private data
			ret = QLRIL_AddUnsolEventsListener(&handle, processUnsolInd_cb, (void *)&fool);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_AddUnsolEventsListener success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_AddUnsolEventsListener failed with return:", ret);
			break;
		case 11:
			ret = QLRIL_DelUnsolEventsListener(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_DelUnsolEventsListener success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_DelUnsolEventsListener failed with return:", ret);
			break;
		case 12:
			c_printf ("[y]%s", "Input receive unsolicited events region start(value >= 1000):");
			start = getInputIntVal();

			if (start < 1000 || start > 1070) {
				c_printf("[r]%s\n", "Unsolicited events region start shuld be in band of 1000 ~ 1070");
				break;
			}

			c_printf ("[y]%s", "Input receive unsolicited events region end(value <= 1070):");
			end = getInputIntVal();

			if (end > 1070 || end < 1000) {
				c_printf("[r]%s\n", "Unsolicited events region end shuld be in band of 1000 ~ 1070");
				break;
			}

			if (end < start) {
				c_printf("[r]%s\n", "Unsolicited events region end shuldn't be less then or equal start");
				break;
			}

			ret = QLRIL_RegisterUnsolEvents(&handle, start, end);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_RegisterUnsolEvents success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_RegisterUnsolEvents failed with return:", ret);
			break;
		case 13:
			c_printf ("[y]%s", "Input not receive unsolicited events region start(value >= 1000):");
			start = getInputIntVal();

			if (start < 1000 || start > 1070) {
				c_printf("[r]%s\n", "Unsolicited events region start shuld be in band of 1000 ~ 1070");
				break;
			}

			c_printf ("[y]%s", "Input not receive unsolicited events region end(value <= 1070):");
			end = getInputIntVal();

			if (end < 1000 || end > 1070) {
				c_printf("[r]%s\n", "Unsolicited events region end shuld be in band of 1000 ~ 1070");
				break;
			}

			if (end < start) {
				c_printf("[r]%s\n", "Unsolicited events region end shuldn't be less then or equal start");
				break;
			}

			ret = QLRIL_UnregisterUnsolEvents(&handle, start, end);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_UnregisterUnsolEvents success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_UnregisterUnsolEvents failed with return:", ret);
			break;
		case 19:
		{
			char imsi[20] = {0};

			ret = QLRIL_GetIMSIForUSIM(&handle, imsi, sizeof(imsi));
			if (ret >= 0)
				c_printf ("[g]%s%s\n", "QLRIL_GetIMSIForUSIM success, IMSI:", imsi);
			else {
				c_printf ("[r]%s[y]%d\n", "QLRIL_GetIMSIForUSIM failed with return:", ret);
			}
			break;
		}
		case 20:
		{
			char **opt = NULL;
			ret = QLRIL_GetOperator(&handle, &opt);
			if (ret > 0) {
				if (opt[0]) {
					c_printf("[g]%s%s\n", "ONS: ", opt[0]);
					free(opt[0]);
				} else
					c_printf("[g]%s\n", "ONS:NULL");

				if (opt[1]) {
					c_printf("[g]%s%s\n", "EONS: ", opt[1]);
					free(opt[1]);
				} else
					c_printf("[g]%s\n", "EONS:NULL");

				if (opt[2]) {
					c_printf("[g]%s%s\n", "MCCMNC: ", opt[2]);
					free(opt[2]);
				} else
					c_printf("[g]%s\n", "MCCMNC:NULL");

				if (opt) {
					free(opt);
					opt = NULL;
				}
			} else
				c_printf ("[r]%s%d\n", "QLRIL_GetOperator failed with return:", ret);
			break;
		}
		case 21:
		{
			RIL_CardStatus_v6 *cardStatus = NULL;
			ret = QLRIL_GetIccCardStatus(&handle, (void **)&cardStatus);
			if (ret < 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GetIccCardStatus failed with return:", ret);
			} else {
				c_printf("[g]%s%d\n", "QLRIL_GetIccCardStatus ret = ", ret);
				if (cardStatus == NULL)
					break;
				/**
				 * Card state:
				 * 0: absent;
				 * 1: present
				 * 2: error;
				 * 3: restricted
				 */
				c_printf ("[b]%s[g]%d\n", "card_state(e.g. 0:absent; 1:present;): ", cardStatus->card_state);
				/**
				 * Pin state:
				 * 0: Unknown;
				 * 1: Enable not verified
				 * 2: Enable verified
				 * 3: Disabled
				 * 4: Enable blocked
				 * 5: Enable perm blocked
				 */
				c_printf ("[b]%s[g]%d\n", "universal_pin_state(e.g. 0:unknown;): ",
						cardStatus->universal_pin_state);
				c_printf ("[b]%s[g]%d\n", "gsm_umts_subscription_app_index: ",
						cardStatus->gsm_umts_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "cdma_subscription_app_index: ",
						cardStatus->cdma_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "ims_subscription_app_index: ",
						cardStatus->ims_subscription_app_index);
				c_printf ("[b]%s[g]%d\n", "num_applications: ", cardStatus->num_applications);
				for (i = 0; i < cardStatus->num_applications; i++) {
					/**
					 * APP Type:
					 * 0: Unknown;
					 * 1: SIM;
					 * 2: USIM;
					 * 3: RUIM;
					 * 4: CSIM;
					 * 5: ISIM;
					 */
					if (cardStatus->applications[i].app_type > RIL_APPTYPE_ISIM) {
						break;//something wrong
					}

					c_printf ("[b]%s[g]%d\n", "app_type(e.g 1:SIM; 2:USIM 3:RUIM 4:CSIM 5:ISIM): ",
							cardStatus->applications[i].app_type);
					/**
					 * APP state:
					 * 0: Unknown;
					 * 1: Detected;
					 * 2: If PIN1 or UPin is required
					 * 3: If PUK1 or Puk for UPin is required
					 * 4: perso_substate should be look at when app_state is assigned to this value
					 * 5: Ready
					 */
					c_printf ("[b]%s[g]%d\n", "app_state(e.g. 2: Pin req; 3: PUK req; 5:Ready): ",
							cardStatus->applications[i].app_state);
					/**
					 * applicable only if app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO
					 * Perso substate:
					 * 0: Unknown;
					 * 1: In progress;
					 * 2: Ready;
					 * ...
					 * For more information refer to ril.h(RIL_PersoSubstate)
					 */
					if (cardStatus->applications[i].app_state == RIL_APPSTATE_SUBSCRIPTION_PERSO)
						c_printf ("[b]%s[g]%d\n", "perso_substate(e.g. 2: Ready): ",
								cardStatus->applications[i].perso_substate);

					if (cardStatus->applications[i].aid_ptr) {
						c_printf ("[b]%s[g]%s\n", "aid_ptr: ", cardStatus->applications[i].aid_ptr);
						free(cardStatus->applications[i].aid_ptr);
					}

					if (cardStatus->applications[i].app_label_ptr) {
						c_printf ("[b]%s[g]%s\n", "app_label_ptr: ",
								cardStatus->applications[i].app_label_ptr);
						free(cardStatus->applications[i].app_label_ptr);
					}

					if (cardStatus->applications[i].app_type == RIL_APPTYPE_USIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_CSIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_ISIM)
						c_printf ("[b]%s[g]%d\n", "pin1_replaced: ",
								cardStatus->applications[i].pin1_replaced);
					/**
					 * Pin state:
					 * 0: Unknown;
					 * 1: Enable not verified
					 * 2: Enable verified
					 * 3: Disabled
					 * 4: Enabled blocked
					 * 5: Enabled perm blocked
					 */
					c_printf ("[b]%s[g]%d\n", "pin1(e.g. 1:Enable not verified, 2:Enable verified): ",
							cardStatus->applications[i].pin1);
					c_printf ("[b]%s[g]%d\n", "pin2(e.g. 3:Disabled, 4:Enable blocked): ",
							cardStatus->applications[i].pin2);
				}
			}

			if (cardStatus)
				free(cardStatus);
			cardStatus = NULL;
			break;
		}
		case 22:
		{
			int timeout = 0;

			c_printf ("[y]%s", "Please input toggle radio on and off(1 or 0):");
			on = getInputIntVal();

			c_printf ("[y]%s", "Please input timeout(second, suggest:10):");
			timeout = getInputIntVal();

			/* for "airplane" mode */
			ret = QLRIL_SetRadioPower(&handle, on, timeout);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_SetRadioPower failed with return:", ret);
			else
				c_printf ("[g]%s\n", "QLRIL_SetRadioPower success");
			break;
		}
		case 23:
		{
			/* Call this function before sleep and after wakeup */
			c_printf ("[y]%s", "Indicates the current state of the screen.  When the screen is off, the\n");
			c_printf ("[y]%s", "RIL should notify the baseband to suppress certain notifications\n");
			/* For more information refer to RIL_REQUEST_SCREEN_STATE in ril.h */
			c_printf ("[y]%s", "Please input toggle screen on and off(1 or 0) to :");
			on = getInputIntVal();

			ret = QLRIL_SetScreenState(&handle, on);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_SetScreenState failed with return:", ret);
			else
				c_printf ("[g]%s\n", "QLRIL_SetScreenState success");
			break;
		}
		case 24:
		{
			ret = QLRIL_RequestShutdown(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_RequestShutdown success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_RequestShutdown failed with return:", ret);
			break;
		}
		case 25:
		{
			char **resp = NULL;
			ret = QLRIL_GetDeviceIdentity(&handle, &resp);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetDeviceIdentity failed with return:", ret);
			else {
				c_printf("[y]%s%d\n","QLRIL_GetDeviceIdentity ret = ", ret);
				if (ret == 0 || resp == NULL)
					break;

				if (ret >= 4) {
					if (resp[0]) {
						c_printf ("[y]%s[g]%s\n", "IMEI: ", resp[0]);
						free(resp[0]);
					}

					if (resp[1]) {
						c_printf ("[y]%s[g]%s\n", "IMEISV: ", resp[1]);
						free(resp[1]);
					}

					if (resp[2]) {
						c_printf ("[y]%s[g]%s\n", "ESN: ", resp[2]);
						free(resp[2]);
					}

					if (resp[3]) {
						c_printf ("[y]%s[g]%s\n", "MEID: ", resp[3]);
						free(resp[3]);
					}
				}

				if (resp) {
					free(resp);
					resp = NULL;
				}
			}
			break;
		}
		case 26:
		{
			char imsi[20] = {0};

			ret = QLRIL_GetIMSI(&handle, imsi, sizeof(imsi));
			if (ret >= 0)
				c_printf ("[g]%s%s\n", "QLRIL_GetIMSI get IMSI:", imsi);
			else {
				c_printf ("[r]%s%d\n", "QLRIL_GetIMSI failed with return:", ret);
			}
			break;
		}
		case 27:
		{
			char aid[40] = {0};
			char imsi[20] = {0};

			c_printf("[y]%s", "Please input aid string(-1: exit):");
			ret = getInputString(aid, sizeof(aid));
			if (ret <= 0 || strncmp(aid, "-1", 2) == 0)
				break;

			ret = QLRIL_GetIMSIForApp(&handle, aid, imsi, sizeof(imsi));
			if (ret >= 0)
				c_printf ("[g]%s%s\n", "QLRIL_GetIMSIForApp success, IMSI:", imsi);
			else {
				c_printf ("[r]%s[y]%d\n", "QLRIL_GetIMSIForApp failed with return:", ret);
			}
			break;
		}
		case 28:
		{
			char IMEI[40] = {0};
			ret = QLRIL_GetIMEI(&handle, IMEI, sizeof(IMEI));
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetIMEI failed with return:", ret);
			else {
				c_printf ("[y]%s[g]%s[y]%s[g]%d\n", "IMEI code:[", IMEI, "] IMEI strength:", ret);
			}
			break;
		}
		case 29:
		{
			RIL_SignalStrength_v10 *ss = NULL;
			ret = QLRIL_GetSignalStrength(&handle, (void **)&ss);
			if (ret < 0 || ss == NULL) {
				c_printf ("[r]%s%d\n", "QLRIL_GetSignalStrength failed with return:", ret);
				break;
			}

			ret = 0;
			memset(buf, 0, sizeof(buf));
			if (ss->GW_SignalStrength.signalStrength != 99) {
				/* signalStrength: Valid values are (0-31, 99) as defined in TS 27.007 8.5
				 *
				 * bitErrorRate: bit error rate (0-7, 99) as defined in TS 27.007 8.5
				 */
				ret += snprintf(buf + ret, sizeof(buf) - ret, "GW: signalStrength[%d] bitErrorRate[%d]\n",
						ss->GW_SignalStrength.signalStrength, ss->GW_SignalStrength.bitErrorRate);
			}

			if (ss->CDMA_SignalStrength.dbm > 0 || ss->CDMA_SignalStrength.ecio > 0) {
				/* dbm: Valid values are positive integers.  This value is the actual RSSI value
				 *		multiplied by -1.  Example: If the actual RSSI is -75, then this response
				 *		value will be 75.
				 *
				 * ecio: Valid values are positive integers.  This value is the actual Ec/Io multiplied
				 *		by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
				 *		will be 125.
				 */
				ret += snprintf(buf + ret, sizeof(buf) - ret, "CDMA: dbm[%d] ecio[%d]\n",
						ss->CDMA_SignalStrength.dbm, ss->CDMA_SignalStrength.ecio);
			}

			if (ss->EVDO_SignalStrength.dbm > 0 || ss->EVDO_SignalStrength.ecio > 0 ||
					ss->EVDO_SignalStrength.signalNoiseRatio > 0) {
				/* dbm: Valid values are positive integers.  This value is the actual RSSI value
				 *		multiplied by -1.  Example: If the actual RSSI is -75, then this response
				 *		value will be 75.
				 *
				 * ecio: Valid values are positive integers.  This value is the actual Ec/Io multiplied
				 *		by -10.  Example: If the actual Ec/Io is -12.5 dB, then this response value
				 *		will be 125.
				 *
				 * signalNoiseRatio: Valid values are 0-8.  8 is the highest signal to noise ratio.
				 */
				ret += snprintf(buf + ret, sizeof(buf) - ret, "EVDO: dbm[%d] ecio[%d] signalNoiseRatio[%d]\n",
						ss->EVDO_SignalStrength.dbm, ss->EVDO_SignalStrength.ecio,
						ss->EVDO_SignalStrength.signalNoiseRatio);
			}

			if (ss->LTE_SignalStrength.signalStrength != 99 || ss->LTE_SignalStrength.rsrp != INT_MAX) {
				/* signalStrength: Valid values are (0-31, 99) as defined in TS 27.007 8.5
				 *
				 * rsrp: The current Reference Signal Receive Power in dBm multipled by -1.
				 *		 Range: 44 to 140 dbm
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 *
				 * rsrq: The current Reference Signal Receive Quality in dB multiplied by -1.
				 *		 Range: 20 to 3 dB.
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 * 
				 * rssnr: The current reference signal signal-to-noise ratio in 0.1 dB units.
				 *		 Range: -200 to +300 (-200 = -20.0 dB, +300 = 30dB).
				 *		 INT_MAX : 0x7FFFFFFF denotes invalid value.
				 * 
				 * cqi: The current Channel Quality Indicator.
				 *		 Range: 0 to 15.
				 *		 INT_MAX : 0x7FFFFFFF denotes invalid value.
				 * Reference: 3GPP TS 36.101.
				 */
				ret += snprintf(buf + ret, sizeof(buf) - ret, "LTE: signalStrength[%d] ",
						ss->LTE_SignalStrength.signalStrength);
				ret += snprintf(buf + ret, sizeof(buf) - ret, "rsrp[%d] rsrq[%d] rssnr[%d]\n",
						ss->LTE_SignalStrength.rsrp, ss->LTE_SignalStrength.rsrq,
						ss->LTE_SignalStrength.rssnr);

				if (ss->LTE_SignalStrength.cqi != INT_MAX) {
					ret += snprintf(buf + ret, sizeof(buf) - ret, "LTE: cqi[%d]\n",
							ss->LTE_SignalStrength.cqi);
				}
			}

			if (ss->TD_SCDMA_SignalStrength.rscp > 0 && ss->TD_SCDMA_SignalStrength.rscp != INT_MAX) {
				/* rscp: The Received Signal Code Power in dBm multipled by -1.
				 *		 Range : 25 to 120
				 *		 INT_MAX: 0x7FFFFFFF denotes invalid value.
				 */
				ret += snprintf(buf + ret, sizeof(buf) - ret, "TDSCDMA: rscp[%d]\n",
						ss->TD_SCDMA_SignalStrength.rscp);
			}

			buf[sizeof(buf) - 1] = 0;//prevent stack overflows
			if (strlen(buf) > 0) {
				c_printf("[g]%s\n", buf);
			}

			free(ss);
			ss = NULL;

			break;
		}
		case 30:
		{
			int appType;
			char aid[40] = {0};
			char phone_number[90];

			c_printf ("[y]%s", "Please input app type (1 ~ 5, e.g. 1: SIM, 2:USIM ...):");
			appType = getInputIntVal();

			c_printf("[y]%s", "Please input aid string(Enter is null, -1: exit):");
			ret = getInputString(aid, sizeof(aid));
			if (strncmp(aid, "-1", 2) == 0)
				break;

			ret = QLRIL_GetPhoneNumber(&handle, appType, (strlen(aid) == 0?NULL:aid),
					phone_number, sizeof(phone_number));
			if (ret <= 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetPhoneNumber failed with return:", ret);
			else {
				c_printf ("[y]%s[g]%d\n", "Phone Number lenght:", ret);
				c_printf ("[y]%s[g]%s\n", "Phone Number:", phone_number);
			}

			break;
		}
		case 31:
		{
			int retries = 0;
			char pin[10] = {0};
			c_printf("[y]%s", "Please input PIN code(-1: exit):");
			ret = getInputString(pin, sizeof(pin));
			if (ret < 4 || strncmp(pin, "-1", 2) == 0)
				break;

			ret = QLRIL_SupplyIccPin(&handle, pin, &retries);
			if (ret == 0)
				c_printf ("[g]%s\n", "PIN code is correct, SIM card is ready!");
			else {
				if (ret == -1038) {
					c_printf ("[g]%s\n", "The PIN code is cancelled, You should enable it first");
				} else if (ret == -1003) {
					if (retries > 0) {
						c_printf ("[y]%s%s[r]%d\n", "Warning!!! ",
								"PIN code is incrrect, the number of retries remaining:", retries);
					} else
						c_printf ("[r]%s%d\n", "PIN code is incrrect, SIM card is locked, "
								"PUK code required, retries:", retries);
				} else
					c_printf ("[r]%s[y]%d\n", "QLRIL_SupplyIccPin failed with return:", ret);
			}
			break;
		}
		case 32:
		{
			int retries = 0;
			char puk[20] = {0};
			char newPin[10] = {0};
			c_printf("[y]%s", "Please input PUK code(-1:exit):");
			ret = getInputString(puk, sizeof(puk));
			if (ret < 8 || strncmp(puk, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input new PIN code(-1:exit):");
			ret = getInputString(newPin, sizeof(newPin));
			if (ret < 4 || strncmp(newPin, "-1", 2) == 0)
				break;

			ret = QLRIL_SupplyIccPuk(&handle, puk, newPin, &retries);
			if (ret == 0)
				c_printf ("[g]%s\n", "PUK code is correct, SIM card is ready!");
			else {
				if (ret == -1003) {
					if (retries > 0) {
						c_printf ("[y]%s%s[r]%d\n", "Warning!!! ",
								"PUK code is incrrect, the number of retries remaining:", retries);
					} else
						c_printf ("[r]%s%s\n", "Error!!! ",
								"SIM card is scrapped. 0.0");
				} else
					c_printf ("[r]%s[y]%d\n", "QLRIL_SupplyIccPuk failed with return:", ret);
			}
			break;
		}
		case 33:
		{
			int retries = 0;
			char oldPin[10] = {0};
			char newPin[10] = {0};
			c_printf("[y]%s", "Please input old PIN code(-1:exit):");
			ret = getInputString(oldPin, sizeof(oldPin));
			if (ret < 4 || strncmp(oldPin, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input new PIN code(-1:exit):");
			ret = getInputString(newPin, sizeof(newPin));
			if (ret < 4 || strncmp(newPin, "-1", 2) == 0)
				break;

			ret = QLRIL_ChangeIccPin(&handle, oldPin, newPin, &retries);
			if (ret == 0)
				c_printf ("[g]%s\n", "PIN code is updated, SIM card is ready!");
			else {
				if (ret == -1006) {
					c_printf ("[g]%s\n", "The PIN code is cancelled, You can't change it now");
				} else if (ret == -1003) {
					if (retries > 0) {
						c_printf ("[y]%s%s[r]%d\n", "Warning!!! ",
								"PIN code is incrrect, the number of retries remaining:", retries);
					} else
						c_printf ("[y]%s%s\n", "Warning!!! ",
								"PIN code is incrrect, SIM card is locked, PUK code required");
				} else
					c_printf ("[r]%s[y]%d\n", "QLRIL_ChangeIccPin failed with return:", ret);
			}
			break;
		}
		case 34:
		{
			/* service class bit vector of
			 * services for which the specified barring facility
			 * is active. 0 means "disabled for all"
			 */
			int scvb = 0;
			int serviceClass;
			char facility[10] = {0};
			char password[10] = {0};
			char aid[40] = {0};

			c_printf("[y]%s", "Please input facility string(-1: exit; eg: AO for BAOC, SC for SIM lock):");
			ret = getInputString(facility, sizeof(facility));
			if (ret <= 0 || strncmp(aid, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input 4 to 8 numerical digits PIN password(Enter is null; -1:exit):");
			ret = getInputString(password, sizeof(password));
			if ((ret > 0 && (ret < 4 || ret > 8)) || strncmp(password, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input service class bit vector of services to query(e.g: 7):");
			serviceClass = getInputIntVal();
			if (serviceClass == -1)
				break;

			c_printf("[y]%s", "Please input aid string(Enter is null, -1: exit):");
			ret = getInputString(aid, sizeof(aid));
			if (strncmp(aid, "-1", 2) == 0)
				break;

			/*
			 * eg "AO" for BAOC, "SC" for SIM lock
			 */
			ret = QLRIL_QueryFacilityLockForApp(&handle, facility, (strlen(password) == 0)?NULL:password,
					serviceClass, (strlen(aid) == 0)?NULL:aid, &scvb);
			if (ret < 0) {
				c_printf ("[r]%s%d\n", "QLRIL_QueryFacilityLockForApp failed with return:", ret);
				if (scvb != 0)
					c_printf ("[y]%s%d\n", "QLRIL_QueryFacilityLockForApp scvb:", scvb);
			} else {
				c_printf ("[g]%s%d\n", "Service class vector:", scvb);
			}
			break;
		}
		case 35:
		{
			int retries = 0;
			int lockState = 0;
			char password[10] = {0};
			char aid[40] = {0};

			c_printf("[y]%s", "Please input lock state(0: unlock; 1: lock; -1:exit):");
			lockState = getInputIntVal();
			if (lockState == -1)
				break;

			c_printf("[y]%s", "Please input 4 to 8 numerical digits PIN password(-1:exit):");
			ret = getInputString(password, sizeof(password));
			if (ret < 4 || ret > 8 || strncmp(password, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input aid string(Enter is null, -1: exit):");
			ret = getInputString(aid, sizeof(aid));
			if (strncmp(aid, "-1", 2) == 0)
				break;

			/*
			 * eg "AO" for BAOC, "SC" for SIM lock
			 */
			ret = QLRIL_SetFacilityLockForApp(&handle, "SC", lockState, password,
					7, (strlen(aid) == 0)?NULL:aid, &retries);
			if (ret < 0) {
				if (ret == -1038) {
					if (lockState == 0)
						c_printf ("[g]%s\n", "The PIN code is cancelled, You should enable it first");
					else
						c_printf ("[g]%s\n", "The PIN code is enabled, You should cancel it first");
					break;
				} else if (ret == -1003) {
					if (retries > 0) {
						c_printf ("[y]%s%s[r]%d\n", "Warning!!! ",
								"PIN code is incrrect, the number of retries remaining:", retries);
					} else
						c_printf ("[r]%s%d\n", "PIN code is incrrect, SIM card is locked, "
								"PUK code required, retries:", retries);
				} else
					c_printf ("[r]%s%d\n", "QLRIL_SetFacilityLockForApp failed with return:", ret);
			} else {
				if (lockState == 0)
					c_printf ("[g]%s\n", "The PIN code is cancelled");
				else
					c_printf ("[g]%s\n", "The PIN code is enabled");
			}
			break;
		}
		case 36:
		{
			int cmd = 0;
			int fileid = 0;
			int p1, p2, p3;
			char path[20] = {0};
			char data[20] = {0};
			char pin2[20] = {0};
			char aid[40] = {0};
			RIL_SIM_IO_Response *resp;

			c_printf("[y]%s", "Please input value of cmd(e.g:178, -1:exit):");
			cmd = getInputIntVal();
			if (cmd == -1)
				break;

			c_printf("[y]%s", "Please input hex value of fileid(e.g:0x6f40):");
			fileid = getInputHexVal();

			c_printf("[y]%s", "Please input path string(e.g:3F007FFF, -1:exit):");
			ret = getInputString(path, sizeof(path));
			if (ret <= 0 || strncmp(path, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input value of p1(e.g:1, -1:exit):");
			p1 = getInputIntVal();
			if (p1 == -1)
				break;

			c_printf("[y]%s", "Please input value of p2(e.g:4, -1:exit):");
			p2 = getInputIntVal();
			if (p2 == -1)
				break;

			c_printf("[y]%s", "Please input value of p3(e.g:0, -1:exit):");
			p3 = getInputIntVal();
			if (p3 == -1)
				break;

			c_printf("[y]%s", "Please input data string(Enter is null, -1:exit):");
			ret = getInputString(data, sizeof(data));
			if (strncmp(data, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input pin2 string(Enter is null, -1:exit):");
			ret = getInputString(pin2, sizeof(pin2));
			if (strncmp(pin2, "-1", 2) == 0)
				break;

			c_printf("[y]%s", "Please input aid string(Enter is null, -1:exit):");
			ret = getInputString(aid, sizeof(aid));
			if (strncmp(aid, "-1", 2) == 0)
				break;

			ret = QLRIL_IccIOForApp(&handle, cmd, fileid, path, p1, p2, p3,
					(strlen(data) == 0?NULL:data), (strlen(pin2) == 0?NULL:pin2),
					(strlen(aid) == 0?NULL:aid), (void **)&resp);
			if (ret < 0) {
				c_printf ("[r]%s[y]%d\n", "QLRIL_IccIOForApp failed with return:", ret);
			} else {
				c_printf ("[b]%s[g]%d\n", "RIL_SIM_IO_Response.sw1 = ", resp->sw1);
				c_printf ("[b]%s[g]%d\n", "RIL_SIM_IO_Response.sw2 = ", resp->sw2);
				if (resp->simResponse) {
					c_printf ("[b]%s[g]%s\n", "RIL_SIM_IO_Response.simResponse:", resp->simResponse);
					free(resp->simResponse);
				}
			}
			break;
		}
		case 37:
		{
			int mode = 0;

			/* Query current network selectin mode */
			ret = QLRIL_GetNetworkSelectionMode(&handle, &mode);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetNetworkSelectionMode failed with return:", ret);
			else {
				c_printf ("[b]%s%d\n", "QLRIL_GetNetworkSelectionMode with return:", ret);
				c_printf ("[y]%s[g]%d\n", "Network selection mode(0: automatic selection;1: manual selection):", mode);
			}
			break;
		}
		case 38:
		{
			char version[50] = {0};

			/* Query current network selectin mode */
			ret = QLRIL_GetBasebandVersion(&handle, version, sizeof(version));
			if (ret <= 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetBasebandVersion failed with return:", ret);
			else {
				c_printf ("[g]%s%s\n", "baseband version:", version);
			}
			break;
		}
		case 39:
		{
			char aid[40] = {0};
			char iccid[40] = {0};

			c_printf("[y]%s", "Please input aid string(-1: exit):");
			ret = getInputString(aid, sizeof(aid));
			if (ret <= 0 || strncmp(aid, "-1", 2) == 0)
				break;

			/* Query SIM card ICCID */
			ret = QLRIL_GetSimIccId(&handle, aid, iccid, sizeof(iccid));
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetSimIccId failed with return:", ret);
			else {
				c_printf ("[g]%s%d\n", "SIM card ICCID lenght:", ret);
				c_printf ("[g]%s%s\n", "SIM card ICCID:", iccid);
			}
			break;
		}
		case 140:
		{
			char nitz[40] = {0};
			ret = QLRIL_GetNITZTime(&handle, nitz, sizeof(nitz));
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetNITZTime failed with return:", ret);
			else {
				c_printf ("[y]%s[g]%s[y]%s[g]%d\n", "NITZ time:[", nitz, "] string strength:", ret);
			}
			break;
		}
		case 101:
		{
			int timeout;
			char **resp = NULL;

			c_printf ("[y]%s", "Please input timeout (second, suggest:10):");
			timeout = getInputIntVal();

			/* Scans for available networks */
			ret = QLRIL_GetAvailableNetworks(&handle, &resp, timeout);
			if (ret < 0)
				c_printf("[r]%s%d\n", "QLRIL_GetAvailableNetworks failed with return:", ret);
			else {
				c_printf("[b]%s%d\n", "String items:", ret);
				c_printf("[b]%s\n",
					"response[n+0] is long alpha ONS or EONS"
					"response[n+1] is short alpha ONS or EONS"
					"response[n+2] is 5 or 6 digit numeric code (MCC + MNC)"
					"response[n+3] is a string value of the status:"
					"unknown, available, current, forbidden");
				for (i = 0; i < ret; i++) {
					if (resp[i] == NULL)
						continue;
					c_printf("[g]%s%d%s%s\n", "response[", i, "]:", resp[i]);
					free(resp[i]);
				}

				if (resp) {
					free(resp);
					resp = NULL;
				}
			}
			break;
		}
		case 40:
		{
			char **voiceRegistrationState = NULL;
			c_printf ("[y]%s", "Please input sim card slot ID (1 or 2):");
			slotId = getInputIntVal();

			ret = QLRIL_GetVoiceRegistrationState(&handle, slotId, &voiceRegistrationState);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetVoiceRegistrationState failed with return:", ret);
			} else {
				c_printf("[b]%s\n", "response[0]: e.g. 1 - Registered, home network\n"
						"response[1]: LAC\n"
						"response[2]: Cell ID\n"
						"response[3]: refer to RIL_RadioTechnology in ril.h e.g.:\n"
						"3  - UMTS; 14 - LTE; 16 - GSM;");
				for (i = 0; i < ret && i < 14; i++) {
					if (voiceRegistrationState[i] == NULL)
						continue;

					/* For more information refer to RIL_REQUEST_VOICE_REGISTRATION_STATE in
					 * include/telephony/ril.h
					 * response[0]: e.g. 1 - Registered, home network
					 * response[1]: LAC
					 * response[2]: Cell ID
					 * response[3]: refer to RIL_RadioTechnology in ril.h e.g.:
					 *	3  - UMTS
					 *	14 - LTE
					 *	16 - GSM
					 */
					c_printf("[g]%s%d%s%s\n", "response[", i, "]:", voiceRegistrationState[i]);
					free(voiceRegistrationState[i]);
				}

				if (voiceRegistrationState) {
					free(voiceRegistrationState);
					voiceRegistrationState = NULL;
				}
			}
			break;
		}
		case 41:
		{
			int radioTechnology = 0;
			ret = QLRIL_GetVoiceRadioTechnology(&handle, &radioTechnology);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetVoiceRadioTechnology failed with return:", ret);
			else {
				c_printf ("[g]%s%d\n", "QLRIL_GetVoiceRadioTechnology radioTechnology(3: UMTS; 16:GSM):", radioTechnology);
			}
			break;
		}
		case 42:
		{
			int state[3] = {0};
			ret = QLRIL_GetImsRegistrationState(&handle, state, sizeof(state));
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetImsRegistrationState failed with return:", ret);
			else {
				c_printf ("[y]%s%d\n", "current IMS registration state:", ret);
				for (i = 0; i < ret; i++) {
					if (i == 0)
						c_printf ("[g]%s%d\n", "Registration state(0 - Not registered; 1 - Registered):", state[i]);
					else if (i == 1 && state[0] == 1) {
						/**
						 * RIL_RadioTechnologyFamily:
						 * 1: 3GPP Technologies - GSM, WCDMA
						 * 2: 3GPP2 Technologies - CDMA
						 */
						c_printf ("[g]%s%d\n", "type in RIL_RadioTechnologyFamily:", state[i]);
					}
				}
			}
			break;
		}
		case 43:
			memset(addr, 0, sizeof(addr));
			c_printf("[y]%s", "Please input dest phone number(-1: exit):");
			ret = getInputString(addr, sizeof(addr));
			if (ret <= 0 || strncmp(addr, "-1", 2) == 0)
				break;

			if (strlen(addr) > sizeof(addr)) {
				c_printf("[r]%s", "Your input number string lenght is bigger then buffer size!");
				break;
			}
			int state[3] = {0};
			ret = QLRIL_GetImsRegistrationState(&handle, state, sizeof(state));
			ret = QLRIL_IMSServerState(&on);
			if(on && state[0] == 1) {
				c_printf("[y]%s\n", "use IMS");
				ret = QLRIL_IMSDial(&handle, addr, 0, 0, 2, 0, 0);//domain:2
				if (ret == 0)
					c_printf ("[g]%s\n", "QLRIL_IMSDial success");
				else {
					c_printf ("[r]%s%d\n", "Ensure IMS server has started return:", ret);
				}
			}
			else {				
				ret = QLRIL_Dial(&handle, addr, 0);
				if (ret == 0)
					c_printf("[g]%s\n", "QLRIL_Dial success");
				else
					c_printf("[r]%s%d\n", "QLRIL_Dial failed with return:", ret);
			}

			break;
		case 44:
		{
			ret = QLRIL_GetSimCardSlotId(&handle, &slotId);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetSimCardSlotId failed with return:", ret);
				break;
			}

			if (callFromSimCard != slotId) {
				c_printf ("[y]%s%d\n", "Reference current call from SIM card:", callFromSimCard);

				c_printf ("[y]%s", "Please input SIM card slot ID (1 or 2):");
				slotId = getInputIntVal();
				ret = QLRIL_SetSimCardSlotId(&handle, slotId);
				if (ret < 0) {
					c_printf("[r]%s%d\n", "QLRIL_SetSimCardSlotId failed with return:", ret);
					break;
				}
			}

			c_printf ("[y]%s%d\n", "Now accept call come from SIM card:", slotId);
			int state[3] = {0};
			ret = QLRIL_GetImsRegistrationState(&handle, state, sizeof(state));
			ret = QLRIL_IMSServerState(&on);
			if(on && state[0] == 1) {
				ret = QLRIL_IMSAcceptCall(&handle, 0);
				if (ret == 0) {
					c_printf ("[g]%s\n", "QLRIL_IMSAcceptCall success");
				}
				else {
					c_printf ("[r]%s%d\n", "QLRIL_IMSAcceptCall failed with return:", ret);
				}
			}
			else {
				ret = QLRIL_AcceptCall(&handle);
				if (ret == 0)
					c_printf ("[g]%s\n", "QLRIL_AcceptCall success");
				else {
					c_printf ("[r]%s%d\n", "QLRIL_AcceptCall failed with return:", ret);
				}
			}
			break;
		}
		case 45:
		{
			ret = QLRIL_GetSimCardSlotId(&handle, &slotId);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetSimCardSlotId failed with return:", ret);
				break;
			}

			if (callFromSimCard != slotId) {
				c_printf ("[y]%s%d\n", "Reference current call from SIM card:", callFromSimCard);

				c_printf ("[y]%s", "Please input SIM card slot ID (1 or 2):");
				slotId = getInputIntVal();
				ret = QLRIL_SetSimCardSlotId(&handle, slotId);
				if (ret < 0) {
					c_printf("[r]%s%d\n", "QLRIL_SetSimCardSlotId failed with return:", ret);
					break;
				}
			}

			c_printf ("[y]%s%d\n", "Now reject voice call come from SIM card:", slotId);
			ret = QLRIL_RejectCall(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_RejectCall success");
			else {
				c_printf ("[r]%s%d\n", "QLRIL_RejectCall failed with return:", ret);
			}
			break;
		}
		case 46:
		{
			int id = 0;
			ims_CallList *list = NULL;
			RIL_Call *call = NULL;
			c_printf ("[y]%s", "Please input sim card slot ID (1 or 2):");
			slotId = getInputIntVal();

			ret = QLRIL_GetCurrentCalls(&handle, slotId, (void **)&call);
			if (ret >= 0) {
				c_printf("[b]%s%d\n", "QLRIL_GetCurrentCalls items = ", ret);

				if (call == NULL) {
					c_printf("[y]%s\n", "RIL_Call memory pointer is NULL");
					break;
				}

				if (ret >= 20) {
					c_printf("[r]%s\n", "Warning!!! Calls items is too many");
				}

				for (i = 0; i < ret; i++) {
					c_printf("[g]%s%d\n", "state: ", call[i].state);
					c_printf("[g]%s%d\n", "index: ", call[i].index);
					c_printf("[g]%s%d\n", "isVoice: ", call[i].isVoice);
					/* and so on */
					if (call[i].name) {
						c_printf("[g]%s%s\n", "name: ", call[i].name);
						free(call[i].name);
						call[i].name = NULL;
					}

					if (call[i].number) {
						c_printf("[g]%s%s\n", "Call Id number: ", call[i].number);
						free(call[i].number);
						call[i].number = NULL;
					}
				}
				
				if (call) {
					free(call);
					call = NULL;
				}
			} else {
				c_printf("[r]%s%d\n", "QLRIL_GetCurrentCalls return:", ret);

				ret = QLRIL_IMSServerState(&on);
				if(on == 0){
					//c_printf("[r]%s\n", "Unregister IMS service\n");
					break;	
				}

				ret = QLRIL_IMSGetCurrentCalls(&handle, slotId, &id, (void **)&list);
				if (ret == 0) {
					c_printf("[r]%s\n", "No voice calls of IMS\n");
					break;
				}

				if (ret <= 0 || list == NULL) {
					c_printf ("[r]%s%d\n", "QLRIL_IMSGetCurrentCalls failed:", ret);
					break;
				}

				ims_CallList_Call **call = (ims_CallList_Call **)list->callAttributes.arg;
				if (call != NULL) {
					c_printf("[y]%s\n", "IMS Call state hint: 0-ACTIVE, 1-HOLDING, 2-DIALING,");
					c_printf("[y]%s\n", "3-ALERTING, 4-INCOMING, 5-WAITING, 6-END");
					for (i = 0; call[i] != NULL; i++) {
						ret = 0;
						memset(msg, 0, sizeof(msg));
						if (call[i]->has_index)
							ret += snprintf(msg + ret, sizeof(msg) - ret, "IMS Call index[%d]", call[i]->index);
						if (call[i]->number.arg)
							ret += snprintf(msg + ret, sizeof(msg) - ret, " Number[%s]", (char *)call[i]->number.arg);
						if (call[0]->has_state)
							ret += snprintf(msg + ret, sizeof(msg) - ret," state[%d]", call[i]->state);
						c_printf("[g]%s\n", msg);
					}
				}

				/* you must use this to release resp */
				ret = QLRIL_IMSFreeResponse(id, (void **)&list);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLRIL_IMSFreeResponse failed:", ret);
			}
			break;
		}
		case 47:
		{
			int index = 0;

			if (CallAcceptIndex > 0)
				c_printf ("[y]%s%d\n", "Reference current call index:", CallAcceptIndex);

			c_printf ("[y]%s", "Please input call index(1 or 2):");
			index = getInputIntVal();
			if (index < 1) {
				if (CallAcceptIndex > 0) {
					index = CallAcceptIndex;
				} else {
					c_printf ("[r]%s%d\n", "Wrong index:", index);
					break;
				}
			}

			c_printf ("[y]%s%d\n", "Now hangup connection index:", index);
			ret = QLRIL_HangupConnection(&handle, index);
			if (ret != 0) {
				/**
				 * return:
				 * 44: Received invalid arguments in request.
				 * For more information refer to RIL_Errno in ril.h
				 */
				c_printf ("[r]%s%d\n", "QLRIL_HangupConnection failed with return:", ret);
			} else {
				c_printf ("[g]%s\n", "QLRIL_HangupConnection success");
			}
			break;
		}
		case 48:
		{
			ret = QLRIL_HangupWaitingOrBackground(&handle);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_HangupWaitingOrBackground failed with return:", ret);
			else {
				c_printf ("[g]%s\n", "QLRIL_HangupWaitingOrBackground success");
			}
			break;
		}
		case 49:
		{
			ret = QLRIL_HangupForegroundResumeBackground(&handle);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_HangupForegroundResumeBackground failed with return:", ret);
			else {
				c_printf ("[g]%s\n", "QLRIL_HangupForegroundResumeBackground success");
			}
			break;
		}
		case 50:
		{
			int muteState;
			ret = QLRIL_GetMute(&handle, &muteState);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_GetMute failed with return:", ret);
			else {
				c_printf ("[g]%s[y]%d\n", "Current Mute state: ", muteState);
			}
			break;
		}
		case 51:
		{
			int enableMute;

			c_printf ("[y]%s", "Please input enable Mute(1:On; 0:Off):");
			enableMute = getInputIntVal();

			ret = QLRIL_SetMute(&handle, enableMute);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_SetMute failed with return:", ret);
			else {
				c_printf ("[g]%s\n", "QLRIL_SetMute success");
			}
			break;
		}
		case 52:
		{
			ret = QLRIL_SwitchWaitingOrHoldingAndActive(&handle);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_SwitchWaitingOrHoldingAndActive failed with return:", ret);
			else {
				c_printf ("[g]%s\n", "QLRIL_SwitchWaitingOrHoldingAndActive success");
			}
			break;
		}
		case 53:
		{
			ret = QLRIL_Conference(&handle);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_Conference failed with return:", ret);
			else {
				c_printf ("[g]%s\n", "QLRIL_Conference success");
			}
			break;
		}
		case 54:
		{
			//char *dev = "/dev/smd11";
			//char *dev = "/dev/smd8";
			char dev[] = "/dev/smd7";//for example
			RIL_Call *call = NULL;
			c_printf ("[y]%s", "Please input sim card slot ID (1 or 2):");
			slotId = getInputIntVal();

			ret = QLRIL_GetCurrentCallsByAtCmd(dev, slotId, (void **)&call);
			if (ret >= 0) {
				c_printf("[b]%s%d\n", "QLRIL_GetCurrentCallsByAtCmd items = ", ret);
				if (ret == 0)
					break;

				if (call == NULL) {
					c_printf("[y]%s\n", "RIL_Call memory pointer is NULL");
					break;
				}

				if (ret >= 20) {
					c_printf("[r]%s\n", "Warning!!! Calls items is too many");
				}

				for (i = 0; i < ret; i++) {
					c_printf("[g]%s%d\n", "state: ", call[i].state);
					c_printf("[g]%s%d\n", "index: ", call[i].index);
					c_printf("[g]%s%d\n", "isVoice: ", call[i].isVoice);
					/* and so on */
					if (call[i].name) {
						c_printf("[g]%s%s\n", "name: ", call[i].name);
						free(call[i].name);
						call[i].name = NULL;
					}

					if (call[i].number) {
						c_printf("[g]%s%s\n", "Call Id number: ", call[i].number);
						free(call[i].number);
						call[i].number = NULL;
					}
				}
			} else {
				c_printf ("[r]%s%d\n", "QLRIL_GetCurrentCallsByAtCmd failed with return:", ret);
			}

			if (call) {
				free(call);
				call = NULL;
			}
			break;
		}
		//IMS API
		case 56:
		{
			ret = QLRIL_IMSServerState(&on);
			c_printf ("[g]%s%s\n", "IMS server:", (on == 0)?"OFF":"ON");
			c_printf ("[g]%s%d\n", "QLRIL_IMSServerState return:", ret);
			break;
		}
		case 57:
		{
			c_printf ("[y]%s", "start or stop IMS server(1 - on; 0 - off):");
			on = getInputIntVal();

			ret = QLRIL_IMSServerEnable(&handle, on);
			if (ret == 0)
				c_printf ("[g]%s%s\n", (on == 0)?"Stop":"Start", " IMS server success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_IMSServerEnable failed:", ret);
			break;
		}
		case 58:
		{
			c_printf ("[y]%s", "set IMS registration state(1:REGISTERED; 2:NOT REGISTERED; 3:REGISTERING):");
			on = getInputIntVal();
			ret = QLRIL_IMSSendImsRegistrationState(&handle, on);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_IMSSendImsRegistrationState success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_IMSSendImsRegistrationState failed:", ret);
			break;
		}
case 59:
		{
			c_printf ("[y]%s", "set Roaming registration state(1:REGISTERED; 2:NOT REGISTERED):");
			on = getInputIntVal();
			ret = QLRIL_IMSSendRoamingRegistrationState(&handle, on);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_IMSSendRoamingRegistrationState success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_IMSSendRoamingRegistrationState failed:", ret);
			break;
		}
		case 60://SMS
		{
			RIL_SMS_Response *sms_resp = NULL;
			memset(areaCode, 0, sizeof(areaCode));
			memset(addr, 0, sizeof(addr));
			c_printf("[y]%s", "Please input area code (e.g. +86 for china, -1:exit):");
			ret = getInputString(areaCode, sizeof(areaCode));
			if (ret <= 0 || strncmp(areaCode, "-1", 2) == 0)
				break;

			if (strlen(areaCode) > 4) {
				c_printf("[r]%s", "Your input area code string lenght is bigger then 4");
				break;
			}

			ret = snprintf(addr, sizeof(areaCode), "%s", areaCode);

			c_printf("[y]%s", "Please input target phone number(-1:exit):");
			ret = getInputString(addr + ret, sizeof(addr) - strlen(areaCode));
			if (ret <= 0 || strncmp(addr + ret, "-1", 2) == 0)
				break;

			c_printf("[y]%s[g]%s\n", "Full target phone number: ", addr);

			memset(msg, 0, sizeof(msg));
			c_printf("[y]%s", "Please input sms message(-1:exit):");
			ret = getInputString(msg, sizeof(msg));
			if (ret <= 0 || strncmp(msg, "-1", 2) == 0)
				break;

			ret = QLRIL_SendSms(&handle, addr, msg, (void **)&sms_resp);
			if (ret == 0) {
				c_printf ("[g]%s\n", "QLRIL_SendSms success");
				if (sms_resp) {
					c_printf ("[b]%s%d\n", "messageRef:", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						c_printf ("[b]%s%s\n", "ackPDU:", sms_resp->ackPDU);
					}
					c_printf ("[b]%s%d\n", "errorCode(-1 if unknown or not applicable):",
							sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				c_printf ("[r]%s%d\n", "QLRIL_SendSms failed with return:", ret);
			break;
		}
		case 61:
		{
			RIL_SMS_Response *sms_resp = NULL;
			char pdu[200];
			//for example: "01000D9168815XXXXXXXFX000006C8329BFD0E01";
			c_printf("[y]%s", "Please input SMS PDU(-1:exit):");
			ret = getInputString(pdu, sizeof(pdu));
			if (ret <= 0 || strncmp(pdu, "-1", 2) == 0)
				break;

			ret = QLRIL_SendSmsByPDU(&handle, NULL, pdu, (void **)&sms_resp);
			if (ret == 0) {
				c_printf("[g]%s\n", "QLRIL_SendSmsByPDU success");
				if (sms_resp) {
					c_printf ("[b]%s%d\n", "messageRef:", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						c_printf ("[b]%s%s\n", "ackPDU:", sms_resp->ackPDU);
					}
					c_printf ("[b]%s%d\n", "errorCode(-1 if unknown or not applicable):",
							sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				c_printf("[r]%s%d\n", "QLRIL_SendSmsByPDU failed with return:", ret);
			break;
		}
		case 62:
		{
			RIL_SMS_Response *sms_resp = NULL;
			memset(areaCode, 0, sizeof(areaCode));
			memset(addr, 0, sizeof(addr));
			c_printf("[y]%s", "Please input area code (e.g. +86 for china, -1:exit):");
			ret = getInputString(areaCode, sizeof(areaCode));
			if (ret <= 0 || strncmp(areaCode, "-1", 2) == 0)
				break;

			if (strlen(areaCode) > 4) {
				c_printf("[r]%s", "Your input area code string lenght is bigger then 4");
				break;
			}

			ret = snprintf(addr, sizeof(areaCode), "%s", areaCode);

			c_printf("[y]%s", "Please input target phone number(-1:exit):");
			ret = getInputString(addr + ret, sizeof(addr) - strlen(areaCode));
			if (ret <= 0 || strncmp(addr + ret, "-1", 2) == 0)
				break;

			c_printf("[y]%s[g]%s\n", "Full target phone number: ", addr);

			memset(msg, 0, sizeof(msg));
			c_printf("[y]%s", "Please input sms message(-1:exit):");
			ret = getInputString(msg, sizeof(msg));
			if (ret <= 0 || strncmp(msg, "-1", 2) == 0)
				break;

			ret = QLRIL_SendImsGsmSms(&handle, addr, msg, (void **)&sms_resp);
			if (ret == 0) {
				c_printf("[g]%s\n", "QLRIL_SendImsGsmSms success");
				if (sms_resp) {
					c_printf ("[b]%s%d\n", "messageRef:", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						c_printf ("[b]%s%s\n", "ackPDU:", sms_resp->ackPDU);
					}
					c_printf ("[b]%s%d\n", "errorCode(-1 if unknown or not applicable):",
							sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				c_printf("[r]%s%d\n", "QLRIL_SendImsGsmSms failed with return:", ret);
			break;
		}
		case 63:
		{
			RIL_SMS_Response *sms_resp = NULL;
			char pdu[200];
			//for example: "01000D9168815XXXXXXXFX000006C8329BFD0E01";
			c_printf("[y]%s", "Please input SMS PDU(-1:exit):");
			ret = getInputString(pdu, sizeof(pdu));
			if (ret <= 0 || strncmp(pdu, "-1", 2) == 0)
				break;

			ret = QLRIL_SendImsGsmSmsByPDU(&handle, NULL, pdu, 1, 0, (void **)&sms_resp);
			if (ret == 0) {
				c_printf("[g]%s\n", "QLRIL_SendImsGsmSmsByPDU success");
				if (sms_resp) {
					c_printf ("[b]%s%d\n", "messageRef:", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						c_printf ("[b]%s%s\n", "ackPDU:", sms_resp->ackPDU);
					}
					c_printf ("[b]%s%d\n", "errorCode(-1 if unknown or not applicable):",
							sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				c_printf("[r]%s%d\n", "QLRIL_SendImsGsmSmsByPDU failed with return:", ret);
			break;
		}
		case 64:
		{
			int index = 0;
			c_printf ("[y]%s", "Please input index of SMS in the RUIM memory :");
			index = getInputIntVal();

			ret = QLRIL_DeleteSmsOnSim(&handle, index);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_DeleteSmsOnSim success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_DeleteSmsOnSim failed with return:", ret);
			break;
		}
		case 65:
		{
			int index = 0;
			c_printf ("[y]%s", "Please input index of CDMA SMS in the RUIM memory :");
			index = getInputIntVal();

			ret = QLRIL_DeleteSmsOnRuim(&handle, index);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_DeleteSmsOnRuim success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_DeleteSmsOnRuim failed with return:", ret);
			break;
		}
		case 70://Data network
		{
			char **resp = NULL;
			ret = QLRIL_GetDataRegistrationState(&handle, &resp);
			if (ret < 0) {
				c_printf("[r]%s%d\n", "QLRIL_GetDataRegistrationState failed with return:", ret);
			} else {
				c_printf("[b]%s\n", "response[0] is registration state 0-5,"
						"e.g 1:Registered, 2: Not registered\n"
						"response[1] is LAC if registered or NULL if not\n"
						"response[2] is CID if registered or NULL if not\n"
						"response[3] indicates the available data radio technology, e.g 3:UMTS, 14:LTE\n"
						"valid values as defined by RIL_RadioTechnology.\n"
						"response[5] The maximum number of simultaneous Data Calls that can be established\n"
						"For more information refer to RIL_REQUEST_DATA_REGISTRATION_STATE in ril.h");
				for (i = 0; i < ret && i < 14; i++) {
					if (resp[i] == NULL)
						continue;
					c_printf("[g]%s%d%s%s\n", "response[", i, "]:", resp[i]);
					free(resp[i]);
				}

				if (resp) {
					free(resp);
					resp = NULL;
				}
			}
			break;
		}
		case 71:
		{
			int radioTech = 0;
			int profileId = 0;
			char apn[40] = {0};
			char protocol[10] = {0};
			char cmdStr[20] = {0};
			char *str = NULL;
			char dnses[200] = {0};
			char buffer[100] = {0};
			RIL_Data_Call_Response_v11 *dataCall = NULL;

			c_printf ("[y]%s", "Input an character(d: default; o: others; -1: exit):");
			memset(cmdStr, 0, sizeof(cmdStr));
			ret = getInputString(cmdStr, sizeof(cmdStr));
			if (ret <= 0 || strncmp(cmdStr, "-1", 2) == 0)
				break;

			if (strncmp(cmdStr, "d", 1) == 0) {
				/*
				 * Radio technology:
				 * 3 - UMTS
				 * 14 - LTE
				 * For more information refer to RIL_RadioTechnology in ril.h
				 */
				ret = QLRIL_SetupDataCall(&handle, 14, 0, "", "", "", 0, "IP", (void **)&dataCall);
			} else {
				for (;;) {
					/*
					 * Radio technology:
					 * 3 - UMTS
					 * 14 - LTE
					 * For more information refer to RIL_RadioTechnology in ril.h
					 */
					c_printf ("[y]%s", "Please input value of radio technology (1~19, e.g:3 or 14; -1:exit):");
					radioTech = getInputIntVal();
					if (radioTech > 1 || radioTech < 19 || radioTech == -1) {
						break;
					} else
						c_printf("[r]%s\n", "Wrong value of radio technology (1~19)");
				}

				if (radioTech == -1)
					break;

				for (;;) {
					/*
					 * Profile Id:
					 * 0 - RIL_DATA_PROFILE_DEFAULT
					 * 1 - RIL_DATA_PROFILE_TETHERED
					 * 2 - RIL_DATA_PROFILE_IMS
					 * 3 - RIL_DATA_PROFILE_FOTA
					 * 4 - RIL_DATA_PROFILE_CBS
					 * 1000 - RIL_DATA_PROFILE_OEM_BASE
					 * 0xFFFFFFFF - RIL_DATA_PROFILE_INVALID
					 * For more information refer to RIL_DataProfile in ril.h
					 */
					c_printf ("[y]%s", "Please input value of profile id (0~10, e.g: 0; -1:exit):");
					profileId = getInputIntVal();
					if (profileId > 1 || profileId < 10 || profileId == -1) {
						break;
					} else
						c_printf("[r]%s\n", "Wrong value of profile id (1 ~ 10)");
				}

				if (profileId == -1)
					break;

				//ctnet for CHN-CT
				c_printf ("[y]%s", "Please input apn string (e.g: cmnet for CMCC; Enter is NULL; -1:exit):");
				ret = getInputString(apn, sizeof(apn));
				if (strncmp(apn, "-1", 2) == 0)
					break;

				c_printf ("[y]%s", "Please input protocol string (e.g: IP, IPV6, IPV4V6, or PPP; -1:exit):");
				ret = getInputString(protocol, sizeof(protocol));
				if (ret <= 0 || strncmp(protocol, "-1", 2) == 0)
					break;

				ret = QLRIL_SetupDataCall(&handle, radioTech, profileId, (strlen(apn) == 0?NULL:apn),
					"", "", 0, protocol, (void **)&dataCall);
			}

			if (ret > 0 && dataCall != NULL) {
				c_printf ("[b]%s%d%s\n", "setupDataCall response[", ret, "]:");
				snprintf (buf, sizeof(buf), "status:%d, retry:%d, callID:%d, active:%s\n"
						"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
						"gateways:%s,pcscf:%s,mtu:%d\n",
						dataCall->status, dataCall->suggestedRetryTime,
						dataCall->cid, (dataCall->active == 0)?"down":"up",
						dataCall->type, dataCall->ifname, dataCall->addresses,
						dataCall->dnses, dataCall->gateways, dataCall->pcscf, dataCall->mtu);
				buf[sizeof(buf) - 1] = 0;//prevent stack overflows
				c_printf("[g]%s\n", buf);

				if (dataCall->active == 0) {
					c_printf("[r]%s\n", "Network is not active, may be you can try 'others' mode");
				} else {
					if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {//FIXME maybe
						if(strlen(dataCall->dnses) > 31) {
							ret = snprintf(dnses, sizeof(dnses), "%s", "2400:3200::1");
							ret += snprintf(dnses + ret, sizeof(dnses) - ret, " %s", "2400:3200:baba::1");
							strcpy(buffer, dataCall->dnses);
							ret += snprintf(dnses + ret, sizeof(dnses) - ret, " %s", buffer);
							str = dnses;
						} else
							str = dataCall->dnses;
					} else {
						ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
						ret += snprintf(dnses + ret, sizeof(dnses) - ret, " %s", "4.2.2.2");
						ret += snprintf(dnses + ret, sizeof(dnses) - ret, " %s", "2400:3200::1");
						ret += snprintf(dnses + ret, sizeof(dnses) - ret, " %s", "2400:3200:baba::1");
						str = dnses;
					}

					ret = QLRIL_AppendDNSForIPV4V6(str);
					if (ret <= 0)
						c_printf("[r]%s%d\n", "QLRIL_SetDNSForIPV4V6 failure return:", ret);
					else
						c_printf ("[g]%s\n", "QLRIL_SetDNSForIPV4V6 success");

					if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
						ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
						if (ret < 0)
							c_printf("[r]%s%d\n", "QLRIL_SetRouteForIPV4V6 failed with return:", ret);
						else
							c_printf ("[g]%s\n", "QLRIL_SetRouteForIPV4V6 success");
					} else
						c_printf ("[g]%s\n", "QLRIL_SetupDataCall called failed without interface name");
				}

				if (dataCall->type)
					free(dataCall->type);
				if (dataCall->ifname)
					free(dataCall->ifname);
				if (dataCall->addresses)
					free(dataCall->addresses);
				if (dataCall->dnses)
					free(dataCall->dnses);
				if (dataCall->gateways)
					free(dataCall->gateways);
				if (dataCall->pcscf)
					free(dataCall->pcscf);

				if (dataCall) {
					free(dataCall);
					dataCall = NULL;
				}
			} else {
				c_printf ("[r]%s%d\n", "QLRIL_SetupDataCall failed with return:", ret);
			}

			break;
		}
		case 72:
			c_printf ("[y]%s", "Please input data connection allowed or not (0 or 1):");
			num = getInputIntVal();

			ret = QLRIL_SetDataAllowed(&handle, num);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_SetDataAllowed success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_SetDataAllowed failed with return:", ret);
			break;
		case 73:
		{
			RIL_Data_Call_Response_v11 *dataCallList = NULL;
			ret = QLRIL_GetDataCallList(&handle, (void **)&dataCallList);

			if (ret > 0) {
				if (ret >= 20) {
					c_printf("[r]%s\n%s\n%s\n", "Data Call list items is too many",
							"you shuld not run QLRIL_SetupDataCall() too many times",
							" May be you shuld run QLRIL_DeactivateDataCall() to release some items");
				}

				c_printf ("[g]%s[y]%d\n", "QLRIL_GetDataCallList success return items: ", ret);
				if (dataCallList == NULL) {
					c_printf("[r]%s\n", "QLRIL_GetDataCallList pointer is NULL");
					break;
				}

				for (i = 0; i < ret; i++) {
					c_printf ("[b]%s\n", "GetDataCallList response:");
					snprintf (buf, sizeof(buf), "status:%d, retry:%d, call_id:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s, pcscf:%s, mtu:%d\n",
							dataCallList[i].status, dataCallList[i].suggestedRetryTime,
							dataCallList[i].cid, (dataCallList[i].active == 0)?"down":"up",
							dataCallList[i].type, dataCallList[i].ifname, dataCallList[i].addresses,
							dataCallList[i].dnses, dataCallList[i].gateways,
							dataCallList[i].pcscf, dataCallList[i].mtu);
					buf[sizeof(buf) - 1] = 0;//prevent stack overflows
					c_printf("[g]%s\n", buf);

					if (dataCallList[i].type)
						free(dataCallList[i].type);
					if (dataCallList[i].ifname)
						free(dataCallList[i].ifname);
					if (dataCallList[i].addresses)
						free(dataCallList[i].addresses);
					if (dataCallList[i].dnses)
						free(dataCallList[i].dnses);
					if (dataCallList[i].gateways)
						free(dataCallList[i].gateways);
					if (dataCallList[i].pcscf)
						free(dataCallList[i].pcscf);
				}
			} else if (ret == 0) {
				c_printf ("[y]%s\n", "No Data Call list items, May be you shuld run QLRIL_SetupDataCall() firstly");
			} else {
				/*
				 * -1047: 47 - RIL_E_INVALID_CALL_ID (Received invalid call id in request)
				 */
				c_printf ("[r]%s%d\n", "QLRIL_GetDataCallList failed with return:", ret);
			}

			if (dataCallList) {
				free(dataCallList);
				dataCallList = NULL;
			}
			break;
		}
		case 74:
		{
			int callid;
			int reason;

			c_printf ("[y]%s", "Please input call id:");
			callid = getInputIntVal();

			//c_printf ("[y]%s\n", "reason: 0: No specific reason specified, 1: Radio shutdown requested");
			//c_printf ("[y]%s", "Please input reason:");
			//reason = getInputIntVal();
			reason = 0;

			ret = QLRIL_DeactivateDataCall(&handle, callid, reason);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_DeactivateDataCall success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_DeactivateDataCall failed with return:", ret);
			break;
		}
		case 75:
			ret = QLRIL_GetPreferredNetworkType(&handle, &type);
			if (ret == 0) {
				/**
				 * Return value:
				 * e.g.:
				 *  1  - GSM only
				 *  2  - WCDMA
				 *  ...
				 *	22 - TD-SCDMA, LTE, CDMA, EvDo GSM/WCDMA
				 *	For more information refer to RIL_PreferredNetworkType in ril.h
				 */
				c_printf ("[g]%s%d\n", "Preferred network type = ", type);
			} else
				c_printf ("[r]%s%d\n", "QLRIL_GetPreferredNetworkType failed with return:", ret);
			break;
		case 76:
			/**
			 * Return value:
			 * e.g.:
			 *  1  - GSM only
			 *  2  - WCDMA
			 *  ...
			 *	22 - TD-SCDMA, LTE, CDMA, EvDo GSM/WCDMA
			 *	For more information refer to RIL_PreferredNetworkType in ril.h
			 */
			c_printf ("[y]%s", "Please input preferred network type (0 ~ 22):");
			type = getInputIntVal();

			ret = QLRIL_SetPreferredNetworkType(&handle, type);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_SetPreferredNetworkType success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_SetPreferredNetworkType failed with return:", ret);
			break;
		case 77:
		{
			char dns[40] = {0};
			c_printf("[y]%s", "Please input DNS string(e.g: 8.8.8.8; -1: exit):");
			ret = getInputString(dns, sizeof(dns));
			if (ret < 7 || strncmp(dns, "-1", 2) == 0)
				break;

			ret = QLRIL_AppendDNSForIPV4V6(dns);
			if (ret > 0)
				c_printf ("[g]%s\n", "QLRIL_AppendDNSForIPV4V6 success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_AppendDNSForIPV4V6 failure return:", ret);
			break;
		}
		case 78:
		{
			char sure[4] = {0};
			c_printf("[y]%s", "Are you sure to clear the /etc/resolv.conf(y: yes; n: no -1: exit):");
			ret = getInputString(sure, sizeof(sure));
			if (ret <= 0 || strncmp(sure, "y", 1) != 0 || strncmp(sure, "-1", 2) == 0)
				break;

			ret = QLRIL_ClearResolvConfFile();
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_ClearResolvConfFile failure return:", ret);
			else
				c_printf ("[g]%s\n", "QLRIL_ClearResolvConfFile success");
			break;
		}
        case 79:
        {
            char ifname[20] = {0};
			c_printf("[y]%s", "Please input network interface name string(e.g:rmnet_data0, -1: exit):");
			ret = getInputString(ifname, sizeof(ifname));
			if (ret <= 0 || strncmp(ifname, "-1", 2) == 0)
				break;

            ret = QLRIL_EnableRNDIS(ifname);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_EnableRNDIS success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_EnableRNDIS failed with return:", ret);
			break;
        }
        case 80:
        {
            ret = QLRIL_DisableRNDIS();
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_DisableRNDIS success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_DisableRNDIS failed with return:", ret);
			break;
        }
		case 500://no use
		{
			int success;
			int cause;

			c_printf ("[y]%s", "Please input SIM card slot id(1 or 2):");
			slotId = getInputIntVal();

			c_printf ("[y]%s", "Please input success value(1 or 0):");
			success = getInputIntVal();

			c_printf ("[y]%s", "Please input failure cause value(211:memory capacity exceeded; 255: unspecified error):");
			cause = getInputIntVal();

			if (success != 0)
				success = 1;

			if (cause != 0xff)
				success = 211;

			ret = QLRIL_AcknowledgeLastIncomingGsmSms(&handle, slotId, success, cause);
			if (ret < 0)
				c_printf ("[r]%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms failed with return:", ret);
			else {
				c_printf ("[y]%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
			}
			break;
		}
		case 90:
			ret = QLRIL_GNSS_AddListener(&handle, gnss_ind_callback, (void *)&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_AddListener success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_AddListener failed with return:", ret);
			break;
		case 91:
			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_DelListener success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_DelListener failed with return:", ret);
			break;
		case 92:
		{
			c_printf ("[y]%s", "Input receive GNSS indication events region start(value >= 0):");
			start = getInputIntVal();

			if (start < 0 || start > 9) {
				c_printf("[r]%s\n", "GNSS indication events region start shuld be in band of 0 ~ 9");
				break;
			}

			c_printf ("[y]%s", "Input receive GNSS indication events region end(value <= 9):");
			end = getInputIntVal();

			if (end < 0 || end > 9) {
				c_printf("[r]%s\n", "GNSS indication events region end shuld be in band of 0 ~ 9");
				break;
			}

			if (end < start) {
				c_printf("[r]%s\n", "GNSS indication events region end shuldn't be less then or equal start");
				break;
			}
			ret = QLRIL_GNSS_RegisterEvents(&handle, start, end);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_RegisterEvents success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_RegisterEvents failed with return:", ret);
			break;
		}
		case 93:
		{
			c_printf ("[y]%s", "Input not receive GNSS indication events region start(value >= 0):");
			start = getInputIntVal();

			if (start < 0 || start > 9) {
				c_printf("[r]%s\n", "GNSS indication events region start shuld be in band of 0 ~ 9");
				break;
			}

			c_printf ("[y]%s", "Input not receive GNSS indication events region end(value <= 9):");
			end = getInputIntVal();

			if (end < 0 || end > 9) {
				c_printf("[r]%s\n", "GNSS indication events region end shuld be in band of 0 ~ 9");
				break;
			}

			if (end < start) {
				c_printf("[r]%s\n", "GNSS indication events region end shuldn't be less then or equal start");
				break;
			}
			ret = QLRIL_GNSS_UnregisterEvents(&handle, start, end);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_UnregisterEvents success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_UnregisterEvents failed with return:", ret);
			break;
		}
		case 94:
		{
			int mode;
			int recurrence;
			int minInterval;
			int prefAccuracy;
			int prefTime;

			c_printf ("[y]%s", "Input GNSS mode(0:GPS standalone, 1:AGPS MS-Based, 2:AGPS MS-Assisted, Suggest: 0):");
			mode = getInputIntVal();

			c_printf ("[y]%s", "Input GNSS recurrence(0: Receive recurring, 1: Request a single-shot):");
			recurrence = getInputIntVal();

			c_printf ("[y]%s", "Input GNSS minimum interval(milliscond of report nmea period, suggest:1000):");
			minInterval = getInputIntVal();

			c_printf ("[y]%s", "Input GNSS preferred Accuracy(suggest: 0):");
			prefAccuracy = getInputIntVal();

			c_printf ("[y]%s", "Input GNSS preferred time (second, suggest: 5):");
			prefTime = getInputIntVal();

			ret = QLRIL_GNSS_SetAttribute(&handle, mode, recurrence, minInterval, prefAccuracy, prefTime);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_SetAttribute success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_SetAttribute failed with return:", ret);
			break;
		}
		case 95:
		{
			ret = QLRIL_GNSS_StartNavigation(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_StartNavigation success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StartNavigation failed with return:", ret);
			break;
		}
		case 96:
		{
			ret = QLRIL_GNSS_StopNavigation(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_StopNavigation success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StopNavigation failed with return:", ret);
			break;
		}
		case 97:
		{
			int timeout = 60;
			double longitude = 0, latitude = 0, altitude = 0;
			float accuracy = 0;

			c_printf ("[y]%s", "Please input value of timeout( > 0, Suggest: 60):");
			timeout = getInputIntVal();

			ret = QLRIL_GNSS_GetLocation(&handle, &longitude, &latitude, &altitude, &accuracy, timeout);
			if (ret == 0) {
				c_printf ("[y]%s\n", "Current location:");
				printf ("Longitude:%lf\n", longitude);
				printf ("Latitude:%lf\n", latitude);
				printf ("Altitude:%lf\n", altitude);
				printf ("Accuracy:%lf\n", accuracy);
			} else {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_GetLocation failed with return:", ret);
				//FIXME: repair
				/* There is a problem when the first using, for unknown reasons,
				 * delete the listener and add listener again
				 */
				if (ret == -6)
					c_printf ("[y]%s%s\n", "Suggest you to change a big value of timeout",
							" or rerun QLRIL_GNSS_DelListener and QLRIL_GNSS_AddListener");
			}
			break;
		}
		case 98:
		{
			double longitude = 0, latitude = 0, altitude = 0;
			float accuracy = 0;

			c_printf ("[y]%s\n", "Test the GNSS API start:");
			/* Test step 1 */
			ret = QLRIL_GNSS_AddListener(&handle, gnss_ind_callback, (void *)&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_AddListener success");
			else {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_AddListener failed with return:", ret);
				break;
			}

			ret = QLRIL_GNSS_SetAttribute(&handle, 0, 0, 2000, 0, 10);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_SetAttribute failed with return:", ret);
			}

			ret = QLRIL_GNSS_RegisterEvents(&handle, 0, 9);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_RegisterEvents failed with return:", ret);
			}

			ret = QLRIL_GNSS_StartNavigation(&handle);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StartNavigation failed with return:", ret);
			}

			c_printf ("[g]%s\n", "Sleep 100s ...");
			sleep(50);

			ret = QLRIL_GNSS_UnregisterEvents(&handle, 2, 3);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_UnregisterEvents failed with return:", ret);
			}

			sleep(50);
			c_printf ("[g]%s\n", "Sleep end");

			ret = QLRIL_GNSS_StopNavigation(&handle);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StopNavigation failed with return:", ret);
			}

			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_DelListener failed with return:", ret);
				break;
			}

			/* Test step 2 */
			ret = QLRIL_GNSS_AddListener(&handle, NULL, NULL);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_AddListener success");
			else {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_AddListener failed with return:", ret);
				break;
			}

			ret = QLRIL_GNSS_SetAttribute(&handle, 0, 1, 500, 0, 5);
			if (ret != 0) {
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_SetAttribute failed with return:", ret);
			}

			c_printf ("[y]%s\n", "Test the QLRIL_GNSS_GetLocation(), May be wait for 1 minutes");
			ret = QLRIL_GNSS_GetLocation(&handle, &longitude, &latitude, &altitude, &accuracy, 60);//60s
			if (ret == 0) {
				c_printf ("[y]%s\n", "QLRIL_GNSS_GetLocation() Current location:");
				printf ("Longitude:%lf\n", longitude);
				printf ("Latitude:%lf\n", latitude);
				printf ("Altitude:%lf\n", altitude);
				printf ("Accuracy:%lf\n", accuracy);
			} else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_GetLocation failed with return:", ret);

			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_DelListener success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_DelListener failed with return:", ret);

			c_printf ("[y]%s\n", "Test the GNSS API end!");

			break;
		}
		case 99:
		{
			ret = QLRIL_GNSS_StartServer();
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_StartServer success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StartServer failed with return:", ret);
			break;
		}
		case 100:
		{
			ret = QLRIL_GNSS_StopServer();
			if (ret == 0)
				c_printf ("[g]%s\n", "QLRIL_GNSS_StopServer success");
			else
				c_printf ("[r]%s%d\n", "QLRIL_GNSS_StopServer failed with return:", ret);
			break;
		}
		default:
			c_printf("[r]%s\n", "Input command index error");
			while((ch=getchar())!='\n' && ch != EOF);
			break;
		}

		usleep(10000);
	}
}

int main(int argc,char *argv[])
{
	test_QLRIL_api();
}
