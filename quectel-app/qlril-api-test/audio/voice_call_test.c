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
#include "msm_audio.h"

static int callFromSimCard = -1;
static int CallAcceptIndex = -1;
static int sndHandle = 0;
static pthread_t play_music_tid = 0;
static pthread_t voice_call_tid = 0;

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
	{1, "QLRIL_Init"},
	{2, "QLRIL_Exit"},
	{3, "QLRIL_GetVersion"},
	{4, "QLRIL_GetOperator"},
	{5, "QLRIL_GetCurrentCalls"},
	{6, "QLRIL_GetSimCardSlotId"},
	{7, "QLRIL_SetSimCardSlotId"},
	{8, "QLRIL_Dial"},
	{9, "QLRIL_SendSms"},
	{10, "QLRIL_SendSmsByPDU"},
	{11, "QLRIL_AcceptCall"},
	{12, "QLRIL_RejectCall"},
	{13, "QLRIL_SetupDataCall"},
	{14, "QLRIL_GetVoiceRegistrationState"},
	{15, "QLRIL_SetDataAllowed"},
	{16, "QLRIL_GetPreferredNetworkType"},
	{17, "QLRIL_SetPreferredNetworkType"},
	{18, "QLRIL_GetIccCardStatus"},
	{19, "QLRIL_SupplyIccPin"},
	{20, "QLRIL_SupplyIccPuk"},
	{21, "QLRIL_ChangeIccPin"},
	{22, "QLRIL_GetIMSI"},
	{23, "QLRIL_GetIMSIForApp"},
	{24, "QLRIL_GetDataCallList"},
	{25, "QLRIL_DeactivateDataCall"},
	{26, "QLRIL_SetRadioPower"},
	{27, "QLRIL_RequestShutdown"},
	{28, "QLRIL_GetVoiceRadioTechnology"},
	{29, "QLRIL_GetImsRegistrationState"},
	{30, "QLRIL_GetIMEI"},
	{31, "QLRIL_HangupConnection"},
	{32, "QLRIL_HangupWaitingOrBackground"},
	{33, "QLRIL_HangupForegroundResumeBackground"},
	{34, "QLRIL_GetMute"},
	{35, "QLRIL_SetMute"},
	{36, "QLRIL_GetSignalStrength"},
	{37, "QLRIL_SendAtCmd"},
	{38, "QLRIL_GetSIMPhoneNumber"},
	{39, "QLRIL_GetDeviceIdentity"},
	{40, "QLRIL_SetScreenState"},

	{50, "QLRIL_AddUnsolEventsListener"},
	{51, "QLRIL_DelUnsolEventsListener"},
	{52, "QLRIL_RegisterUnsolEvents"},
	{53, "QLRIL_UnregisterUnsolEvents"},
	{54, "QLRIL_GNSS_AddListener"},
	{55, "QLRIL_GNSS_DelListener"},
	{56, "QLRIL_GNSS_SetAttribute"},
	{57, "QLRIL_GNSS_RegisterEvents"},
	{58, "QLRIL_GNSS_UnregisterEvents"},
	{59, "QLRIL_GNSS_StartNavigation"},
	{60, "QLRIL_GNSS_StopNavigation"},
	{61, "QLRIL_GNSS_GetLocation"},
	{62, "Run the GNSS API test"},

	{-1, "Exit!"}
};

func_api_test_t QLRIL_api_test = {"QL RIL API", api_testlist};

static void usage(func_api_test_t *pt_test)
{
	int i;

	printf("Copyright (C) 2020 Quectel, Smart Linux\n");
	printf("Group Name:%s, Supported test cases:\n", pt_test->group_name);

	for (i = 0;; i++) {
		printf("%d:\t%s\n", pt_test->test_cases[i].cmdIdx, pt_test->test_cases[i].funcName);
		if (pt_test->test_cases[i].cmdIdx == -1)
			break;
	}
}

static void *play_music_thread(void *args)
{
	int ret = 0;
	int i = 0;
	char *wavfile = (char *)args;

	if (wavfile == NULL || strlen(wavfile) <= 0) {
		printf("wavfile is invalid\n");
		return NULL;
	}

	audio_stream_enable(&sndHandle);
	for (i = 0; i < 20; i++) {
		ret = audio_play_wav(&sndHandle, wavfile);
		if (ret != 0)
			break;
		sleep(1);
	}
	audio_stream_disable(&sndHandle);
	play_music_tid = 0;
}

static void *voice_call_thread(void *args)
{
	int ret = 0;
	int i = 0;

	voice_stream_disable(&sndHandle);
	if (play_music_tid != 0)
		audio_play_stop(&sndHandle);

	while(play_music_tid != 0 && (i++ < 20))
		usleep(100000);

	if (i >= 20)
		return NULL;

	voice_stream_enable(&sndHandle);
}

void processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
	int i = 0;
	int num = 0;
	int ret = 0;
	void *privData = NULL;
	static int times = 0;

	char radioState[100] = {0};

	RIL_SignalStrength_v10 ss;

	printf("\n");
	if (times < 3) {
		times++;

		ret = QLRIL_GetPrivateData(handle, &privData);
		if (ret == 0) {
			printf("QLRIL_GetPrivateData fool = %d\n", *(int *)privData);
		}
	}

	printf("The unsolicited event[%d] come from slot ID[%d]", slotId, event);

	switch (event) {
		case RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED://1000
			ret = QLRIL_ParseEvent_RadioState(radioState, sizeof(radioState), data, size);
			if (ret < 0) {
				printf("QLRIL_ParseEvent_RadioStateChanged error with ret = %d\n", ret);
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
				printf("RadioState: [%s] ret = %d\n", radioState, ret);
			}
			break;
		case RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED://1001
		{
			RIL_Call *call = NULL;
			/* RIL_UNSOL_RESPONSE_CALL_STATE_CHANGED: Data is NULL */
			ret = QLRIL_GetCurrentCalls(handle, slotId, (void **)&call);
			if (ret >= 0) {
				printf("QLRIL_GetCurrentCalls items = %d\n", ret);

				if (ret == 0) {
					callFromSimCard = -1;
					CallAcceptIndex = -1;
					if (play_music_tid != 0)
						audio_play_stop(&sndHandle);

					voice_stream_disable(&sndHandle);
					break;
				}

				if (call == NULL) {
					printf("RIL_Call memory pointer is NULL\n");
					break;
				}

				for (i = 0; i < ret; i++) {
					printf("state: %d\n", call[i].state);
					if (call[i].state == 4) {
						callFromSimCard = slotId;
						if (play_music_tid == 0) {
							ret = pthread_create(&play_music_tid, NULL,
									play_music_thread,
									(void *)"/etc/mmi/qualsound.wav");
							if (ret != 0) {
								perror("pthread_create");
							}
						}
					}

					if ((call[i].state == 0 || call[i].state == 2 || call[i].state == 3) &&
							call[i].isVoice == 1) {//FIXME
						CallAcceptIndex = call[i].index;
					}

					if (call[i].state == 0 && call[i].isVoice == 1) {//FIXME
						ret = pthread_create(&voice_call_tid, NULL, voice_call_thread, NULL);
					}

					printf("%s%d\n", "index: ", call[i].index);
					printf("%s%d\n", "isVoice: ", call[i].isVoice);
					/* and so on TODO fix me */
					if (call[i].name) {
						printf("%s%s\n", "name: ", call[i].name);
						free(call[i].name);
						call[i].name = NULL;
					}

					if (call[i].number) {
						printf("%s%s\n", "Call Id number: ", call[i].number);
						free(call[i].number);
						call[i].number = NULL;
					}
				}
			} else {
				printf("%s%d\n", "QLRIL_GetCurrentCalls with return:", ret);
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
			printf("%s\n", "Receive event: RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED");
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
				printf("QLRIL_GetVoiceRegistrationState error with ret = %d\n", ret);
			} else {
				/*printf("%s\n", "response[0]: e.g. 1 - Registered, home network\n"
				  "response[1]: LAC\n"
				  "response[2]: Cell ID\n"
				  "response[3]: refer to RIL_RadioTechnology in ril.h e.g.:\n"
				  "3  - UMTS; 14 - LTE; 16 - GSM;");*/
				printf("QLRIL_GetVoiceRegistrationState ret = %d\n", ret);
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
						printf("%s%d%s%s\n", "response[", i, "](1: Registered, home network; 2:Not registered...):",
								voiceRegistrationState[i]);
					else if (i == 2)
						printf("%s%d%s%s\n", "response[", i, "](Cell ID):",
								voiceRegistrationState[i]);
					else if (i == 3)
						printf("%s%d%s%s\n", "response[", i, "](RadioTechnology: 3: UMTS; 14: LTE; 16:GSM):",
								voiceRegistrationState[i]);
					else
						printf("%s%d%s%s\n", "response[", i, "]:", voiceRegistrationState[i]);
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
			char *sms[3];
			ret = QLRIL_ParseEvent_NewSMS(&sms, sizeof(sms), data, size);
			if (ret < 0) {
				printf("%s%d\n", "QLRIL_ParseEvent_NewSMS error with ret = ", ret);
			} else {
				if (sms[0]) {
					printf("%s%s", "Sms details:", sms[0]);
					free(sms[0]);
				}

				if (sms[1]) {
					printf("%s%s%s\n", "Sms message: [", sms[1], "]");
					free(sms[1]);
				}

				if (sms[2]) {
					printf("%s%s%s\n", "Sms raw data of pdu: [", sms[2], "]");
					free(sms[2]);
				}
			}

			ret = QLRIL_AcknowledgeLastIncomingGsmSms(handle, slotId, 1, 0xD3);
			if (ret == 0)
				printf("%s\n", "QLRIL_AcknowledgeLastIncomingGsmSms success");
			else
				printf("%s%d\n", "QLRIL_AcknowledgeLastIncomingGsmSms ret = ", ret);
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT://1004
		{
			char *sms_status = NULL;
			ret = QLRIL_ParseEvent_String(&sms_status, data, size);
			if (ret < 0) {
				printf("%s%d\n", "QLRIL_ParseEvent_String error with ret = ", ret);
			} else {
				if (sms_status) {
					printf("%s%s\n", "the PDU of an SMS-STATUS-REPORT: ", sms_status);
					free(sms_status);
				}
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_NEW_SMS_ON_SIM://1005
		{
			int *index = NULL;
			ret = QLRIL_ParseEvent_Ints(&index, data, size);
			if (ret < 0)
				printf("QLRIL_ParseEvent_Ints failed with ret = %d\n", ret);
			else {
				printf("The slot index on the SIM that contains the %d new message:\n", ret);
				if (ret == 0 || index == NULL)
					break;

				for(i = 0; i < ret; i++) {
					printf("index[%d] = %d", i, index[i]);
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
			printf("%s\n", "Receive event: RIL_UNSOL_NITZ_TIME_RECEIVED");
			ret = QLRIL_ParseEvent_NITZTime(&NITZTimeData, data, size);
			if (ret == 0) {
				if (NITZTimeData != NULL) {
					printf("%s%s\n", "NITZ Time: ", NITZTimeData);
					free(NITZTimeData);
				}
			} else
				printf("%s%d\n", "parse NITZ Time error with ret = ", ret);
			break;
		}
		case RIL_UNSOL_SIGNAL_STRENGTH://1009
			ret = QLRIL_ParseEvent_SignalStrength(&ss, sizeof(ss), data, size);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_ParseEvent_SignalStrength error with ret = ", ret);
			} else {
				printf("SIM card[%d] signal strength Info:", slotId);
				/**
				 * UINT_MAX = 2147483647
				 */
				printf("[signalStrength=%d, bitErrorRate=%d, "
						"CDMA_SS.dbm=%d, CDMA_SSecio=%d, "
						"EVDO_SS.dbm=%d, EVDO_SS.ecio=%d, "
						"EVDO_SS.signalNoiseRatio=%d, \n"
						"LTE_SS.signalStrength=%d, LTE_SS.rsrp=%d, LTE_SS.rsrq=%d, "
						"LTE_SS.rssnr=%d, LTE_SS.cqi=%d, TDSCDMA_SS.rscp=%d]\n",
						ss.GW_SignalStrength.signalStrength,
						ss.GW_SignalStrength.bitErrorRate,
						ss.CDMA_SignalStrength.dbm,
						ss.CDMA_SignalStrength.ecio,
						ss.EVDO_SignalStrength.dbm,
						ss.EVDO_SignalStrength.ecio,
						ss.EVDO_SignalStrength.signalNoiseRatio,
						ss.LTE_SignalStrength.signalStrength,
						ss.LTE_SignalStrength.rsrp,
						ss.LTE_SignalStrength.rsrq,
						ss.LTE_SignalStrength.rssnr,
						ss.LTE_SignalStrength.cqi,
						ss.TD_SCDMA_SignalStrength.rscp);
			}
			break;
		case RIL_UNSOL_DATA_CALL_LIST_CHANGED://1010
		{
			char buf[200] = {0};

			RIL_Data_Call_Response_v11 *dc = NULL;
			ret = QLRIL_ParseEvent_DataCallList((void **)&dc, data, size);
			if (ret < 0) {
				printf("QLRIL_ParseEvent_DataCallList failed with return: %d\n", ret);
			} else {
				printf("QLRIL_ParseEvent_DataCallList items = %d\n", ret);

				if (ret == 0)
					break;

				if (dc == NULL) {
					printf("Error RIL_Data_Call_Response_v11 memory pointer is NULL\n");
					break;
				}

				for (i = 0; i < ret; i++) {
					printf("%s\n", "DataCallList response:");
					snprintf(buf, sizeof(buf), "status:%d, retry:%d, callID:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s, pcscf:%s, mtu:%d\n",
							dc[i].status, dc[i].suggestedRetryTime,
							dc[i].cid, (dc[i].active == 0)?"down":"up",
							dc[i].type, dc[i].ifname, dc[i].addresses,
							dc[i].dnses, dc[i].gateways, dc[i].pcscf, dc[i].mtu);
					buf[sizeof(buf) - 1] = 0;//prevent stack overflows
					printf("%s\n", buf);

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

			if (dc)
				free(dc);

			break;
		}
		case RIL_UNSOL_STK_SESSION_END://1012
			printf("%s\n", "Receive event: RIL_UNSOL_STK_SESSION_END");
			break;
		case RIL_UNSOL_STK_PROACTIVE_COMMAND://1013
		{
			char *sat_usat = NULL;

			ret = QLRIL_ParseEvent_String(&sat_usat, data, size);
			if (ret < 0) {
				printf("%s%d\n", "QLRIL_ParseEvent_String error with ret = ", ret);
			} else {
				if (sat_usat) {
					printf("%s%s\n", "SAT/USAT String: ", sat_usat);
					free(sat_usat);
				}
			}
			break;
		}
		case RIL_UNSOL_CALL_RING://1018
		{
			char resp[4];
			printf("%s\n", "Receive event: RIL_UNSOL_CALL_RING");
			ret = QLRIL_ParseEvent_CallRing(resp, sizeof(resp), data, size);
			if (ret < 0) {
				printf("%s%d\n", "QLRIL_ParseEvent_CallRing error with ret = ", ret);
			} else {
				printf("%s\n", "QLRIL_ParseEvent_CallRing success");
				for (i = 0; i < sizeof(resp); i++) {
					printf("%s%d%s%d\n", "response[", i, "] = ", resp[i]);
				}
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_SIM_STATUS_CHANGED://1019
		{
			RIL_CardStatus_v6 *cardStatus = NULL;
			ret = QLRIL_GetIccCardStatus(handle, (void **)&cardStatus);
			if (ret < 0) {
				printf("QLRIL_GetIccCardStatus with return: %d\n", ret);
			} else {
				printf("QLRIL_GetIccCardStatus ret = %d\n", ret);
				if (cardStatus == NULL)
					break;

				/**
				 * Card state:
				 * 0: absent;
				 * 1: present
				 * 2: error;
				 * 3: restricted
				 */
				printf("%s%d\n", "card_state(e.g. 0:absent; 1:present;): ", cardStatus->card_state);
				/**
				 * Pin state:
				 * 0: Unknown;
				 * 1: Enable not verified
				 * 2: Enable verified
				 * 3: Disabled
				 * 4: Enable blocked
				 * 5: Enable perm blocked
				 */
				printf("%s%d\n", "universal_pin_state(e.g. 0:unknown;): ",
						cardStatus->universal_pin_state);
				printf("%s%d\n", "gsm_umts_subscription_app_index: ",
						cardStatus->gsm_umts_subscription_app_index);
				printf("%s%d\n", "cdma_subscription_app_index: ",
						cardStatus->cdma_subscription_app_index);
				printf("%s%d\n", "ims_subscription_app_index: ",
						cardStatus->ims_subscription_app_index);
				printf("%s%d\n", "num_applications: ", cardStatus->num_applications);
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

					printf("%s%d\n", "app_type(e.g 1:SIM; 2:USIM 3:RUIM 4:CSIM 5:ISIM): ",
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
					printf("%s%d\n", "app_state(e.g. 2: Pin req; 3: PUK req; 5:Ready): ",
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
						printf("%s%d\n", "perso_substate(e.g. 2: Ready): ",
								cardStatus->applications[i].perso_substate);

					if (cardStatus->applications[i].aid_ptr) {
						printf("%s%s\n", "aid_ptr: ", cardStatus->applications[i].aid_ptr);
						free(cardStatus->applications[i].aid_ptr);
					}

					if (cardStatus->applications[i].app_label_ptr) {
						printf("%s%s\n", "app_label_ptr: ",
								cardStatus->applications[i].app_label_ptr);
						free(cardStatus->applications[i].app_label_ptr);
					}

					if (cardStatus->applications[i].app_type == RIL_APPTYPE_USIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_CSIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_ISIM)
						printf("%s%d\n", "pin1_replaced: ",
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
					printf("%s%d\n", "pin1(e.g. 1:Enable not verified, 2:Enable verified): ",
							cardStatus->applications[i].pin1);
					printf("%s%d\n", "pin2(e.g. 3:Disabled, 4:Enable blocked): ",
							cardStatus->applications[i].pin2);
				}
			}

			if (cardStatus)
				free(cardStatus);
			cardStatus = NULL;
			break;
		}
		case RIL_UNSOL_VOICE_RADIO_TECH_CHANGED://1035
		{
			int radioTech = 0;
			ret = QLRIL_ParseEvent_Int(&radioTech, data, size);
			if (ret < 0) {
				printf("QLRIL_ParseEvent_Int error with return:%d\n", ret);
			} else {
				/* RIL_RadioTechnology */
				printf("Radio technology:[%d](3: UMTS, 14:LTE ...), return:%d", radioTech, ret);
			}
			break;
		}
		case RIL_UNSOL_RESPONSE_ADN_INIT_DONE://1047
			printf("%s\n", "Receive event: RIL_UNSOL_RESPONSE_ADN_INIT_DONE");
			break;
		case RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED:
		{
			int *uicc_subscription_status = NULL;
			ret = QLRIL_ParseEvent_Ints((int **)&uicc_subscription_status, data, size);
			if (ret < 0)
				printf("QLRIL_ParseEvent_Ints error with ret = %d\n", ret);
			else {
				printf("uicc_subscription_status items = %d\n", ret);
				if (ret == 0 || index == NULL)
					break;

				printf("%s\n", "0 - Subscription Deactivated; 1 - Subscription Activated");
				for(i = 0; i < num; i++) {
					printf("%s%d%s%d\n", "status[", i, "] = ", uicc_subscription_status[i]);
				}
			}
			break;
		}
		default:
			break;
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

		printf("%s\n", "Current location:");
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

		printf("%s%d\n",  "[LOC_STATUS_INFO] status:", status_info->status.status);
		break;
	}
	case LOC_SV_INFO_IND://2
	{
		ql_loc_sv_info_ind_msg_v01 *ind = (ql_loc_sv_info_ind_msg_v01*)indData;

		printf("%s0x%X, 0x%X, 0x%X\n", "[LOC_SV_INFO] mask of ephemeris, almanac, used_in_fix",
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
		printf("%s%d\n",  "[LOC_NMEA_INFO] length:", ind->length);
		printf("%s%s\n",  "[LOC_NMEA_INFO] nmea:", ind->nmea);
		break;
	}
	case LOC_CAPABILITIES_INFO_IND://4
	{
		ql_loc_capabilities_info_ind_msg_v01 *ind = (ql_loc_capabilities_info_ind_msg_v01*)indData;
		printf("%s%d\n",  "[LOC_CAPABILITIES_INFO] capabilities:", ind->capabilities);
		break;
	}
	case LOC_AGPS_STATUS_IND://7
	{
		ql_loc_agps_status_ind_msg_v01 *ind = (ql_loc_agps_status_ind_msg_v01*)indData;
		ql_agps_status_t_v01 *status = (ql_agps_status_t_v01 *)&ind->status;

		printf("[LOC_AGPS_STATUS_INFO] type: %d", status->type);
		printf("[LOC_AGPS_STATUS_INFO] status: %d", status->status);
		printf("[LOC_AGPS_STATUS_INFO] ipv4 address: %d", status->ipv4_addr);
		//FIXME
		//status->ipv6_addr
		//status->ssid
		//status->password
		break;
	}
	case LOC_NI_NOTIFICATION_IND://8
	{
		ql_loc_ni_notification_ind_msg_v01 *ind = (ql_loc_ni_notification_ind_msg_v01*)indData;

		printf("%s%d\n",  "[LOC_NI_NOTIFICATION_IND] notification_id:", ind->notification.notification_id);
		printf("%s%d\n",  "[LOC_NI_NOTIFICATION_IND] ni_type:", ind->notification.ni_type);
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

		printf("%s%s\n",  "[LOC_XTRA_REPORT_SERVER] server1:", ind->server1);
		printf("%s%s\n",  "[LOC_XTRA_REPORT_SERVER] server2:", ind->server2);
		printf("%s%s\n",  "[LOC_XTRA_REPORT_SERVER] server3:", ind->server3);

		break;
	}
	case LOC_UTC_TIME_REQ_IND://5
	case LOC_XTRA_DATA_REQ_IND://6
	default:
		printf("GNSS receive message id: %d", msgId);
		break;
	}

	return;
}

static int test_QLRIL_api(void)
{
	int ret = 0;
	int handle = 0;
	int slotId = 0;
	int cmdIdx = 0;
	int fool = 0;
	int type = 0;
	int start, end, i, num;

	char cmdStr[20] = {0};
	char areaCode[10] = {0};
	char addr[30] = {0};
	char msg[200] = {0};
	char buf[200] = {0};

	if (handle == 0) {
		ret = QLRIL_Init(&handle);
		if (ret == 0) {
			printf("%s\n", "QLRIL_Init success");

			ret = QLRIL_AddUnsolEventsListener(&handle, processUnsolInd_cb, NULL);
			if (ret != 0)
				printf("%s%d\n", "QLRIL_AddUnsolEventsListener with return:", ret);

			ret = QLRIL_RegisterUnsolEvents(&handle, 1000, 1048);
			if (ret != 0)
				printf("%s%d\n", "QLRIL_RegisterUnsolEvents with return:", ret);

			ret = QLRIL_UnregisterUnsolEvents(&handle, 1009, 1009);
			if (ret != 0)
				printf("%s%d\n", "QLRIL_UnregisterUnsolEvents with return:", ret);
		} else
			printf("%s\n", "QLRIL_Init failure");
	}

	snd_card_init(&sndHandle);
	usage(&QLRIL_api_test);
	while(1) {
		printf("\n");
		printf("%s", "Please input cmd index (-1: exit, 0: help):");
		if (scanf("%d", &cmdIdx) == EOF) {
			printf("%s\n", "Input error");
			continue;
		}

		if (cmdIdx == -1) {
			if (handle != 0) {
				if (QLRIL_Exit(&handle) == 0)
					printf("%s\n", "QLRIL_Exit success");
				else
					printf("%s\n", "QLRIL_Exit failure");
			}
			break;
		}

		switch (cmdIdx) {
		case 0:
			usage(&QLRIL_api_test);
			break;
		case 1:
			if (handle == 0) {
				ret = QLRIL_Init(&handle);
				if (ret == 0)
					printf("%s\n", "QLRIL_Init success");
				else
					printf("%s\n", "QLRIL_Init failure");
			} else {
				printf("%s%08X\n", "QLRIL_Init has executed already with handle = 0x", handle);
			}
			break;
		case 2:
			if (QLRIL_Exit(&handle) == 0)
				printf("%s\n", "QLRIL_Exit success");
			else
				printf("%s\n", "QLRIL_Exit failure");
			break;
		case 3:
			memset(buf, 0, sizeof(buf));
			ret = QLRIL_GetVersion(buf, sizeof(buf));
			if (ret == 0) {
				printf("%s%s\n", "ql ril library version: ", buf);
			} else
				printf("%s%d\n", "QLRIL_GetVersion with return:", ret);
			break;
		case 4:
		{
			char **opt = NULL;
			ret = QLRIL_GetOperator(&handle, &opt);
			if (ret > 0) {
				if (opt[0]) {
					printf("ONS:%s\n", opt[0]);
					free(opt[0]);
				}

				if (opt[1]) {
					printf("EONS:%s\n", opt[1]);
					free(opt[1]);
				}

				if (opt[2]) {
					printf("MCCMNC:%s\n", opt[2]);
					free(opt[2]);
				}

				if (opt) {
					free(opt);
					opt = NULL;
				}
			} else
				printf("%s%d\n", "QLRIL_GetOperator with return:", ret);
			break;
		}
		case 5:
		{
			RIL_Call *call = NULL;

			printf("%s", "Please input sim card slot ID (1 or 2):");
			if (scanf("%d", &slotId) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_GetCurrentCalls(&handle, slotId, (void **)&call);
			if (ret >= 0) {
				printf("QLRIL_GetCurrentCalls items = %d\n", ret);

				if (ret == 0)
					break;

				if (call == NULL) {
					printf("Error RIL_Call memory pointer is NULL\n");
					break;
				}

				for (i = 0; i < ret; i++) {
					printf("%s%d\n", "state: ", call[i].state);
					printf("%s%d\n", "index: ", call[i].index);
					printf("%s%d\n", "isVoice: ", call[i].isVoice);
					/* and so on */
					if (call[i].name) {
						printf("%s%s\n", "name: ", call[i].name);
						free(call[i].name);
						call[i].name = NULL;
					}

					if (call[i].number) {
						printf("%s%s\n", "Call Id number: ", call[i].number);
						free(call[i].number);
						call[i].number = NULL;
					}
				}
			} else {
				printf("%s%d\n", "QLRIL_GetCurrentCalls with return:", ret);
			}

			if (call) {
				free(call);
				call = NULL;
			}
			break;
		}
		case 6:
			ret = QLRIL_GetSimCardSlotId(&handle, &slotId);
			if (ret == 0) {
				printf("%s%d\n", "Current slot ID: ", slotId);
			} else
				printf("%s%d\n", "QLRIL_GetSimCardSlotId with return:", ret);
			break;
		case 7:
			printf("%s", "Please input sim card slot ID (1 or 2):");
			if (scanf("%d", &slotId) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SetSimCardSlotId(&handle, slotId);
			if (ret == 0)
				printf("%s\n", "QLRIL_SetSimCardSlotId success");
			else
				printf("%s%d\n", "QLRIL_SetSimCardSlotId with return: ", ret);
			break;
		case 8:
			memset(addr, 0, sizeof(addr));
			printf("%s", "Please input dest phone number:");
			if (scanf("%s", addr) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (strlen(addr) > sizeof(addr)) {
				printf("%s", "Your input number string lenght is bigger then buffer size!");
				break;
			}

			ret = QLRIL_Dial(&handle, addr, 0);
			if (ret == 0)
				printf("%s\n", "QLRIL_Dial success");
			else
				printf("%s%d\n", "QLRIL_Dial with return:", ret);
			break;
		case 9:
		{
			RIL_SMS_Response *sms_resp = NULL;
			memset(areaCode, 0, sizeof(areaCode));
			memset(addr, 0, sizeof(addr));
			printf("Please input area code (e.g. +86 for china):");
			if (scanf("%s", areaCode) == EOF) {
				printf("Input error");
				break;
			}

			if (strlen(areaCode) > 4) {
				printf("Your input area code string lenght is bigger then 4");
				break;
			}

			ret = snprintf(addr, sizeof(areaCode), "%s", areaCode);

			printf("Please input target phone number:");
			if (scanf("%s", addr + ret) == EOF) {
				printf("Input error");
				break;
			}

			if (strlen(addr + ret) > (sizeof(addr) - ret)) {
				printf("Your input number string lenght is bigger then buffer size");
				break;
			}

			printf("Full target phone number:%s\n", addr);

			memset(msg, 0, sizeof(msg));
			printf("Please input sms message:");
			if (scanf("%s", msg) == EOF) {
				printf("Input error");
				break;
			}

			if (strlen(msg) > sizeof(msg)) {
				printf("Your input message string lenght is bigger then buffer size!");
				break;
			}

			ret = QLRIL_SendSms(&handle, addr, msg, (void **)&sms_resp);
			if (ret == 0) {
				printf("QLRIL_SendSms success");
				if (sms_resp) {
					printf("messageRef:%d\n", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						printf("ackPDU:[%s]\n", sms_resp->ackPDU);
					}
					printf("errorCode(-1 if unknown or not applicable):%d\n", sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				printf("QLRIL_SendSms with return:%d\n", ret);
			break;
		}
		case 10:
		{
			RIL_SMS_Response *sms_resp = NULL;
			char pdu[50] = "01000D91688159153068F3000006C8329BFD0E01";
			ret = QLRIL_SendSmsByPDU(&handle, NULL, pdu, (void **)&sms_resp);
			if (ret == 0) {
				printf("QLRIL_SendSmsByPDU success\n");
				if (sms_resp) {
					printf("messageRef:%d\n", sms_resp->messageRef);
					if (sms_resp->ackPDU) {
						free(sms_resp->ackPDU);
						printf("ackPDU:[%s]\n", sms_resp->ackPDU);
					}
					printf("errorCode(-1 if unknown or not applicable):%d\n", sms_resp->errorCode);
					free(sms_resp);
					sms_resp = NULL;
				}
			} else
				printf("QLRIL_SendSmsByPDU failed with return:%d\n", ret);
			break;
		}
		case 11:
			if (callFromSimCard < 1 || callFromSimCard > 2) {
				printf("%s%d\n", "There is no incoming telegram! slot id: ", callFromSimCard);
				/* TODO Fix me */
				break;
			} else
				printf("%s%d%s\n", "There is an incoming telegram from SIMCard[", callFromSimCard, "]");
			ret = QLRIL_AcceptCall(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_AcceptCall success");
			else {
				printf("%s%d\n", "QLRIL_AcceptCall with return:", ret);
			}
			break;
		case 12:
			if (callFromSimCard < 1 || callFromSimCard > 2) {
				printf("%s%d\n", "There is no incoming telegram! slot id: ", callFromSimCard);
				/* TODO Fix me */
				break;
			} else
				printf("%s%d%s\n", "There is an incoming telegram from SIMCard[", callFromSimCard, "]");

			ret = QLRIL_RejectCall(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_RejectCall success");
			else {
				printf("%s%d\n", "QLRIL_RejectCall with return:", ret);
			}
			break;
		case 13:
		{
			char dns1[20] = {0};
			char dns2[20] = {0};
			RIL_Data_Call_Response_v11 *dataCall = NULL;

			printf("%s\n", "Input an character: y - default; n - others;");
			memset(cmdStr, 0, sizeof(cmdStr));
			if (scanf("%s", cmdStr) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (strncmp(cmdStr, "y", 1) == 0) {
				ret = QLRIL_SetupDataCall(&handle, 14, 0, "", "", "", 0, "IP", (void **)&dataCall);
				if (ret > 0) {
					printf("setupDataCall response[%d]:\n", ret);
					snprintf(buf, sizeof(buf), "status:%d, retry:%d, callID:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s,pcscf:%s,mtu:%d\n",
							dataCall->status, dataCall->suggestedRetryTime,
							dataCall->cid, (dataCall->active == 0)?"down":"up",
							dataCall->type, dataCall->ifname, dataCall->addresses,
							dataCall->dnses, dataCall->gateways, dataCall->pcscf, dataCall->mtu);
					buf[sizeof(buf) - 1] = 0;//prevent stack overflows
					printf("%s\n", buf);

					if (dataCall->ifname && strlen(dataCall->ifname) > 0) {
						memset(dns1, 0, sizeof(dns1));
						memset(dns2, 0, sizeof(dns2));
						if (strlen(dataCall->dnses) >= 15) {
							sscanf(dataCall->dnses, "%s%*[ ]%s", dns1, dns2);
							//printf("dns1: %s, dns2: %s\n", dns1, dns2);
						} else if (strlen(dataCall->dnses) >= 7) {
							sscanf(dataCall->dnses, "%s", dns1);
						} else {
							snprintf(dns1, sizeof(dns1), "%s", "8.8.8.8");
							snprintf(dns2, sizeof(dns1), "%s", "4.2.2.2");
						}

						ret = QLRIL_SetRouteAndDNS(dataCall->ifname, dns1, dns2);
						if (ret != 0)
							printf("%s%d\n", "QLRIL_SetRouteAndDNS error with ret = ", ret);
						else
							printf("%s\n", "QLRIL_SetupDataCall success");
					} else
						printf("%s\n", "QLRIL_SetupDataCall called failed without interface name");

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
					printf("QLRIL_SetupDataCall with return:%d\n", ret);
				}
			} else {
				printf("QLRIL_SetupDataCall:Please look forward to!\n");
			}

			break;
		}
		case 14:
		{
			char **voiceRegistrationState = NULL;

			printf("%s", "Please input sim card slot ID (1 or 2):");
			if (scanf("%d", &slotId) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_GetVoiceRegistrationState(&handle, slotId, &voiceRegistrationState);
			if (ret != 0) {
				printf("QLRIL_GetVoiceRegistrationState failed with ret = %d\n", ret);
			} else {
				printf("QLRIL_GetVoiceRegistrationState ret = %d\n", ret);
				printf("%s\n", "response[0]: e.g. 1 - Registered, home network\n"
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
					printf("response[%d]:%s\n", i, voiceRegistrationState[i]);
					free(voiceRegistrationState[i]);
				}

				if (voiceRegistrationState) {
					free(voiceRegistrationState);
					voiceRegistrationState = NULL;
				}
			}
			break;
		}
		case 15:
			printf("%s", "Please input data connection allowed or not (0 or 1):");
			if (scanf("%d", &num) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SetDataAllowed(&handle, num);
			if (ret == 0)
				printf("%s\n", "QLRIL_SetDataAllowed success");
			else
				printf("%s%d\n", "QLRIL_SetDataAllowed with return:", ret);
			break;
		case 16:
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
				printf("%s%d\n", "Preferred network type = ", type);
			} else
				printf("%s%d\n", "QLRIL_GetPreferredNetworkType with return:", ret);
			break;
		case 17:
			/**
			 * Return value:
			 * e.g.:
			 *  1  - GSM only
			 *  2  - WCDMA
			 *  ...
			 *	22 - TD-SCDMA, LTE, CDMA, EvDo GSM/WCDMA
			 *	For more information refer to RIL_PreferredNetworkType in ril.h
			 */
			printf("%s", "Please input preferred network type (0 ~ 22):");
			if (scanf("%d", &type) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SetPreferredNetworkType(&handle, type);
			if (ret == 0)
				printf("%s\n", "QLRIL_SetPreferredNetworkType success");
			else
				printf("%s%d\n", "QLRIL_SetPreferredNetworkType with return:", ret);
			break;
		case 18:
		{
			RIL_CardStatus_v6 *cardStatus = NULL;
			ret = QLRIL_GetIccCardStatus(&handle, (void **)&cardStatus);
			if (ret < 0) {
				printf("QLRIL_GetIccCardStatus with return: %d\n", ret);
			} else {
				printf("QLRIL_GetIccCardStatus ret = %d\n", ret);
				if (cardStatus == NULL)
					break;

				/**
				 * Card state:
				 * 0: absent;
				 * 1: present
				 * 2: error;
				 * 3: restricted
				 */
				printf("%s%d\n", "card_state(e.g. 0:absent; 1:present;): ", cardStatus->card_state);
				/**
				 * Pin state:
				 * 0: Unknown;
				 * 1: Enable not verified
				 * 2: Enable verified
				 * 3: Disabled
				 * 4: Enable blocked
				 * 5: Enable perm blocked
				 */
				printf("%s%d\n", "universal_pin_state(e.g. 0:unknown;): ",
						cardStatus->universal_pin_state);
				printf("%s%d\n", "gsm_umts_subscription_app_index: ",
						cardStatus->gsm_umts_subscription_app_index);
				printf("%s%d\n", "cdma_subscription_app_index: ",
						cardStatus->cdma_subscription_app_index);
				printf("%s%d\n", "ims_subscription_app_index: ",
						cardStatus->ims_subscription_app_index);
				printf("%s%d\n", "num_applications: ", cardStatus->num_applications);
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

					printf("%s%d\n", "app_type(e.g 1:SIM; 2:USIM 3:RUIM 4:CSIM 5:ISIM): ",
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
					printf("%s%d\n", "app_state(e.g. 2: Pin req; 3: PUK req; 5:Ready): ",
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
						printf("%s%d\n", "perso_substate(e.g. 2: Ready): ",
								cardStatus->applications[i].perso_substate);

					if (cardStatus->applications[i].aid_ptr) {
						printf("%s%s\n", "aid_ptr: ", cardStatus->applications[i].aid_ptr);
						free(cardStatus->applications[i].aid_ptr);
					}

					if (cardStatus->applications[i].app_label_ptr) {
						printf("%s%s\n", "app_label_ptr: ",
								cardStatus->applications[i].app_label_ptr);
						free(cardStatus->applications[i].app_label_ptr);
					}

					if (cardStatus->applications[i].app_type == RIL_APPTYPE_USIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_CSIM ||
							cardStatus->applications[i].app_type == RIL_APPTYPE_ISIM)
						printf("%s%d\n", "pin1_replaced: ",
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
					printf("%s%d\n", "pin1(e.g. 1:Enable not verified, 2:Enable verified): ",
							cardStatus->applications[i].pin1);
					printf("%s%d\n", "pin2(e.g. 3:Disabled, 4:Enable blocked): ",
							cardStatus->applications[i].pin2);
				}
			}

			if (cardStatus)
				free(cardStatus);
			cardStatus = NULL;
			break;
		}
		case 19:
		{
			int retries = 0;
			char pin[10] = {0};
			printf("%s", "Please input PIN code:");
			if (scanf("%s", pin) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SupplyIccPin(&handle, pin, &retries);
			if (ret == 0)
				printf("%s\n", "PIN code is correct, SIM card is ready!");
			else {
				if (ret > 0) {
					if (retries > 0) {
						printf("%s%s%d\n", "Warning!!! ",
								"PIN code is incrrect, the number of retries remaining:", retries);
					} else
						printf("%s%s\n", "Warning!!! ",
								"PIN code is incrrect, SIM card is locked, PUK code required");
				} else
					printf("%s%d\n", "QLRIL_SupplyIccPin with return:", ret);
			}
			break;
		}
		case 20:
		{
			int retries = 0;
			char puk[20] = {0};
			char newPin[10] = {0};
			printf("%s", "Please input PUK code:");
			if (scanf("%s", puk) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Please input new PIN code:");
			if (scanf("%s", newPin) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SupplyIccPuk(&handle, puk, newPin, &retries);
			if (ret == 0)
				printf("%s\n", "PUK code is correct, SIM card is ready!");
			else {
				if (ret > 0) {
					if (retries > 0) {
						printf("%s%s%d\n", "Warning!!! ",
								"PUK code is incrrect, the number of retries remaining:", retries);
					} else
						printf("%s%s\n", "Error!!! ",
								"SIM card is scrapped. 0.0");
				} else
					printf("%s%d\n", "QLRIL_SupplyIccPuk with return:", ret);
			}
			break;
		}
		case 21:
		{
			int retries = 0;
			char oldPin[10] = {0};
			char newPin[10] = {0};
			printf("%s", "Please input old PIN code:");
			if (scanf("%s", oldPin) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Please input new PIN code:");
			if (scanf("%s", newPin) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_ChangeIccPin(&handle, oldPin, newPin, &retries);
			if (ret == 0)
				printf("%s\n", "PIN code is updated, SIM card is ready!");
			else {
				if (ret > 0) {
					if (retries > 0) {
						printf("%s%s%d\n", "Warning!!! ",
								"PIN code is incrrect, the number of retries remaining:", retries);
					} else
						printf("%s%s\n", "Warning!!! ",
								"PIN code is incrrect, SIM card is locked, PUK code required");
				} else
					printf("%s%d\n", "QLRIL_ChangeIccPin with return:", ret);
			}
			break;
		}
		case 22:
		{
			char imsi[20] = {0};

			ret = QLRIL_GetIMSI(&handle, imsi, sizeof(imsi));
			if (ret >= 0)
				printf("%s%s\n", "QLRIL_GetIMSI success, IMSI:", imsi);
			else {
				printf("%s%d\n", "QLRIL_GetIMSI with return:", ret);
			}
			break;
		}
		case 23:
		{
			char aid[40] = {0};
			char imsi[20] = {0};

			printf("%s", "Please input aid string:");
			if (scanf("%s", aid) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_GetIMSIForApp(&handle, aid, imsi, sizeof(imsi));
			if (ret >= 0)
				printf("%s%s\n", "QLRIL_GetIMSIForApp success, IMSI:", imsi);
			else {
				printf("%s%d\n", "QLRIL_GetIMSIForApp with return:", ret);
			}
			break;
		}
		case 24:
		{
			RIL_Data_Call_Response_v11 *dataCallList = NULL;
			ret = QLRIL_GetDataCallList(&handle, (void **)&dataCallList);
			if (ret > 0) {
				printf("QLRIL_GetDataCallList success return items: %d", ret);
				if (dataCallList == NULL) {
					printf("QLRIL_GetDataCallList pointer is NULL\n");
					break;
				}

				for (i = 0; i < ret; i++) {
					printf("GetDataCallList response:\n");
					snprintf(buf, sizeof(buf), "status:%d, retry:%d, callID:%d, active:%s\n"
							"type:%s, ifname:%s, addresses:%s, dnses:%s\n"
							"gateways:%s, pcscf:%s, mtu:%d\n",
							dataCallList[i].status, dataCallList[i].suggestedRetryTime,
							dataCallList[i].cid, (dataCallList[i].active == 0)?"down":"up",
							dataCallList[i].type, dataCallList[i].ifname, dataCallList[i].addresses,
							dataCallList[i].dnses, dataCallList[i].gateways, dataCallList[i].pcscf, dataCallList[i].mtu);
					buf[sizeof(buf) - 1] = 0;//prevent stack overflows
					printf("%s\n", buf);

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
				printf("There is no Data Call list, May be you shuld run QLRIL_SetupDataCall() firstly\n");
			} else {
				printf("QLRIL_GetDataCallList with return:%d\n", ret);
			}

			if (dataCallList) {
				free(dataCallList);
				dataCallList = NULL;
			}
			break;
		}
		case 25:
		{
			int callid;
			int reason;

			printf("%s", "Please input call id:");
			if (scanf("%d", &callid) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Please input reason:");
			if (scanf("%d", &reason) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_DeactivateDataCall(&handle, callid, reason);
			if (ret == 0)
				printf("%s\n", "QLRIL_DeactivateDataCall success");
			else
				printf("%s%d\n", "QLRIL_DeactivateDataCall with return:", ret);
			break;
		}
		case 26:
		{
			int on;

			printf("%s", "Please input toggle radio on and off(1 or 0):");
			if (scanf("%d", &on) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			/* for "airplane" mode */
			ret = QLRIL_SetRadioPower(&handle, on, 5);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_SetRadioPower with return:", ret);
			else
				printf("%s\n", "QLRIL_SetRadioPower success");
			break;
		}
		case 27:
		{
			ret = QLRIL_RequestShutdown(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_RequestShutdown success");
			else
				printf("%s%d\n", "QLRIL_RequestShutdown with return:", ret);
			break;
		}
		case 28:
		{
			int radioTechnology = 0;
			ret = QLRIL_GetVoiceRadioTechnology(&handle, &radioTechnology);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_GetVoiceRadioTechnology with return:", ret);
			else {
				printf("%s%d\n", "QLRIL_GetVoiceRadioTechnology radioTechnology(3: UMTS; 16:GSM): ", radioTechnology);
			}
			break;
		}
		case 29:
		{
			int state[3] = {0};
			ret = QLRIL_GetImsRegistrationState(&handle, state, sizeof(state));
			if (ret < 0)
				printf("%s%d\n", "QLRIL_GetImsRegistrationState with return:", ret);
			else {
				printf("%s\n", "current IMS registration state:");
				for (i = 0; i < ret; i++) {
					if (i == 0)
						printf("Registration state(0 - Not registered; 1 - Registered): %d", state[i]);
					else if (i == 1 && state[i] == 1) {
						/**
						 * RIL_RadioTechnologyFamily:
						 * 1: 3GPP Technologies - GSM, WCDMA
						 * 2: 3GPP2 Technologies - CDMA
						 */
						printf("type in RIL_RadioTechnologyFamily: %d", state[i]);
					}
				}
			}
			break;
		}
		case 30:
		{
			char IMEI[40] = {0};
			ret = QLRIL_GetIMEI(&handle, IMEI, sizeof(IMEI));
			if (ret < 0)
				printf("QLRIL_GetIMEI with return:%d\n", ret);
			else {
				printf("IMEI code:[%s] IMEI strength: %d", IMEI, ret);
			}
			break;
		}
		case 31:
		{
			if (CallAcceptIndex < 0) {
				printf("Can not hang up connection with wrong index[%d]", CallAcceptIndex);
				printf ("Please input call index:");
				if (scanf("%d", &CallAcceptIndex) == EOF) {
					printf("Input error");
					break;
				}

				if (CallAcceptIndex < 0) {
					printf ("Wrong index[%d]", CallAcceptIndex);
					break;
				}
			}

			ret = QLRIL_HangupConnection(&handle, CallAcceptIndex);
			if (ret != 0) {
				/**
				 * return:
				 * 44: Received invalid arguments in request.
				 * For more information refer to RIL_Errno in ril.h
				 */
				printf("%s%d\n", "QLRIL_HangupConnection with return:", ret);
			} else {
				printf("%s\n", "QLRIL_HangupConnection success");
			}
			break;
		}
		case 32:
		{
			ret = QLRIL_HangupWaitingOrBackground(&handle);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_HangupWaitingOrBackground with return:", ret);
			else {
				printf("%s\n", "QLRIL_HangupWaitingOrBackground success");
			}
			break;
		}
		case 33:
		{
			ret = QLRIL_HangupForegroundResumeBackground(&handle);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_HangupForegroundResumeBackground with return:", ret);
			else {
				printf("%s\n", "QLRIL_HangupForegroundResumeBackground success");
			}
			break;
		}
		case 34:
		{
			int muteState;
			ret = QLRIL_GetMute(&handle, &muteState);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_GetMute with return:", ret);
			else {
				printf("%s%d\n", "Current Mute state: ", muteState);
			}
			break;
		}
		case 35:
		{
			int enableMute;

			printf("%s", "Please input enable Mute(1:On; 0:Off):");
			if (scanf("%d", &enableMute) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SetMute(&handle, enableMute);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_SetMute with return:", ret);
			else {
				printf("%s\n", "QLRIL_SetMute success");
			}
			break;
		}
		case 36:
		{
			RIL_SignalStrength_v10 *ss = NULL;
			ret = QLRIL_GetSignalStrength(&handle, (void **)&ss);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_GetSignalStrength with return:", ret);
			else {
				/**
				 * UINT_MAX = 2147483647
				 */
				if (ss != NULL) {
					printf("[signalStrength=%d, bitErrorRate=%d, "
							"CDMA_SS.dbm=%d, CDMA_SSecio=%d, "
							"EVDO_SS.dbm=%d, EVDO_SS.ecio=%d,"
							"EVDO_SS.signalNoiseRatio=%d,\n"
							"LTE_SS.signalStrength=%d, LTE_SS.rsrp=%d, LTE_SS.rsrq=%d, "
							"LTE_SS.rssnr=%d, LTE_SS.cqi=%d, TDSCDMA_SS.rscp=%d]\n",
							ss->GW_SignalStrength.signalStrength,
							ss->GW_SignalStrength.bitErrorRate,
							ss->CDMA_SignalStrength.dbm,
							ss->CDMA_SignalStrength.ecio,
							ss->EVDO_SignalStrength.dbm,
							ss->EVDO_SignalStrength.ecio,
							ss->EVDO_SignalStrength.signalNoiseRatio,
							ss->LTE_SignalStrength.signalStrength,
							ss->LTE_SignalStrength.rsrp,
							ss->LTE_SignalStrength.rsrq,
							ss->LTE_SignalStrength.rssnr,
							ss->LTE_SignalStrength.cqi,
							ss->TD_SCDMA_SignalStrength.rscp);
					free(ss);
					ss = NULL;
				}
			}
			break;
		}
		case 37:
		{
			char cmd[520] = {0};
			char cmd_resp[1024] = {0};

			for (;;) {
				memset(cmd, 0, sizeof(cmd));
				memset(cmd_resp, 0, sizeof(cmd_resp));

				printf("%s", "Please input AT command(-1: exit):");
				if (scanf("%s", cmd) == EOF) {
					printf("%s\n", "Input error");
					break;
				}

				if (strstr(cmd, "-1") != NULL){
					break;
				}

				QLRIL_SendAtCmd(&handle, cmd, cmd_resp, sizeof(cmd_resp));
				if (ret < 0)
					printf("%s%d\n", "QLRIL_SendAtCmd with return:", ret);
				else {
					printf("%s\n", "AT command response:");
					printf("%s\n", cmd_resp);
				}
			}

			break;
		}
		case 38:
		{
			int appType;
			char aid[40] = {0};
			char phone_number[90];

			printf("%s", "Please input app type (1 ~ 4, e.g. 1: SIM, 2:USIM ...):");
			if (scanf("%d", &appType) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Please input aid string:");
			if (scanf("%s", aid) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_GetPhoneNumber(&handle, appType, aid, phone_number, sizeof(phone_number));
			if (ret < 0)
				printf("%s%d\n", "QLRIL_GetPhoneNumber with return:", ret);
			else {
				printf("%s%d\n", "Phone Number lenght:", ret);
				printf("%s%s\n", "Phone Number:", phone_number);
			}

			break;
		}
		case 39:
		{
			char **resp = NULL;
			ret = QLRIL_GetDeviceIdentity(&handle, &resp);
			if (ret < 0)
				printf("QLRIL_GetDeviceIdentity failed with ret = %d\n", ret);
			else {
				printf("QLRIL_GetDeviceIdentity ret = %d\n", ret);
				if (ret == 0 || resp == NULL)
					break;

				if (ret >= 4) {
					if (resp[0]) {
						printf("IMEI: %s\n", resp[0]);
						free(resp[0]);
					}

					if (resp[1]) {
						printf("IMEISV: %s\n", resp[1]);
						free(resp[1]);
					}

					if (resp[2]) {
						printf("ESN: %s\n", resp[2]);
						free(resp[2]);
					}

					if (resp[3]) {
						printf("MEID: %s\n", resp[3]);
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
		case 40:
		{
			int on;

			/* Call this function before sleep and after wakeup */
			printf("%s", "Indicates the current state of the screen.  When the screen is off, the\n");
			printf("%s", "RIL should notify the baseband to suppress certain notifications\n");
			/* For more information refer to RIL_REQUEST_SCREEN_STATE in ril.h */
			printf("%s", "Please input toggle screen on and off(1 or 0) to :");
			if (scanf("%d", &on) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_SetScreenState(&handle, on);
			if (ret < 0)
				printf("%s%d\n", "QLRIL_SetScreenState with return:", ret);
			else
				printf("%s\n", "QLRIL_SetScreenState success");
			break;
		}
		case 50:
			fool = 12345;//for test private data
			ret = QLRIL_AddUnsolEventsListener(&handle, processUnsolInd_cb, (void *)&fool);
			if (ret == 0)
				printf("QLRIL_AddUnsolEventsListener success\n");
			else
				printf("QLRIL_AddUnsolEventsListener with return:%d\n", ret);
			break;
		case 51:
			ret = QLRIL_DelUnsolEventsListener(&handle);
			if (ret == 0)
				printf("QLRIL_DelUnsolEventsListener success\n");
			else
				printf("QLRIL_DelUnsolEventsListener with return:%d\n", ret);
			break;
		case 52:
			printf("%s", "Input receive unsolicited events region start(value >= 1000):");
			if (scanf("%d", &start) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (start < 1000 || start > 1048) {
				printf("%s\n", "Unsolicited events region start shuld be in band of 1000 ~ 1048");
				break;
			}

			printf("%s", "Input receive unsolicited events region end(value <= 1048):");
			if (scanf("%d", &end) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (end > 1048 || end < 1000) {
				printf("%s\n", "Unsolicited events region end shuld be in band of 1000 ~ 1048");
				break;
			}

			if (end < start) {
				printf("%s\n", "Unsolicited events region end shuldn't be less then or equal start");
				break;
			}

			ret = QLRIL_RegisterUnsolEvents(&handle, start, end);
			if (ret == 0)
				printf("%s\n", "QLRIL_RegisterUnsolEvents success");
			else
				printf("%s%d\n", "QLRIL_RegisterUnsolEvents with return:", ret);
			break;
		case 53:
			printf("%s", "Input not receive unsolicited events region start(value >= 1000):");
			if (scanf("%d", &start) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (start < 1000 || start > 1048) {
				printf("%s\n", "Unsolicited events region start shuld be in band of 1000 ~ 1048");
				break;
			}

			printf("%s", "Input not receive unsolicited events region end(value <= 1048):");
			if (scanf("%d", &end) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (end < 1000 || end > 1048) {
				printf("%s\n", "Unsolicited events region end shuld be in band of 1000 ~ 1048");
				break;
			}

			if (end < start) {
				printf("%s\n", "Unsolicited events region end shuldn't be less then or equal start");
				break;
			}

			ret = QLRIL_UnregisterUnsolEvents(&handle, start, end);
			if (ret == 0)
				printf("%s\n", "QLRIL_UnregisterUnsolEvents success");
			else
				printf("%s%d\n", "QLRIL_UnregisterUnsolEvents with return:", ret);
			break;
		case 54:
			ret = QLRIL_GNSS_AddListener(&handle, gnss_ind_callback, (void *)&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_AddListener success");
			else
				printf("%s%d\n", "QLRIL_GNSS_AddListener with return:", ret);
			break;
		case 55:
			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_DelListener success");
			else
				printf("%s%d\n", "QLRIL_GNSS_DelListener with return:", ret);
			break;
		case 56:
		{
			int mode;
			int recurrence;
			int minInterval;
			int prefAccuracy;
			int prefTime;

			printf("%s", "Input GNSS mode(0:GPS standalone, 1:AGPS MS-Based, 2:AGPS MS-Assisted, Suggest: 0):");
			if (scanf("%d", &mode) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Input GNSS recurrence(0: Receive recurring, 1: Request a single-shot):");
			if (scanf("%d", &recurrence) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Input GNSS minimum interval(milliscond of report nmea period, suggest: 1000):");
			if (scanf("%d", &minInterval) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Input GNSS preferred Accuracy(suggest: 0):");
			if (scanf("%d", &prefAccuracy) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			printf("%s", "Input GNSS preferred time (second, suggest: 5):");
			if (scanf("%d", &prefTime) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			ret = QLRIL_GNSS_SetAttribute(&handle, mode, recurrence, minInterval, prefAccuracy, prefTime);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_SetAttribute success");
			else
				printf("%s%d\n", "QLRIL_GNSS_SetAttribute with return:", ret);
			break;
		}
		case 57:
		{
			printf("%s", "Input receive GNSS indication events region start(value >= 0):");
			if (scanf("%d", &start) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (start < 0 || start > 9) {
				printf("%s\n", "GNSS indication events region start shuld be in band of 0 ~ 9");
				break;
			}

			printf("%s", "Input receive GNSS indication events region end(value <= 9):");
			if (scanf("%d", &end) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (end < 0 || end > 9) {
				printf("%s\n", "GNSS indication events region end shuld be in band of 0 ~ 9");
				break;
			}

			if (end < start) {
				printf("%s\n", "GNSS indication events region end shuldn't be less then or equal start");
				break;
			}
			ret = QLRIL_GNSS_RegisterEvents(&handle, start, end);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_RegisterEvents success");
			else
				printf("%s%d\n", "QLRIL_GNSS_RegisterEvents with return:", ret);
			break;
		}
		case 58:
		{
			printf("%s", "Input not receive GNSS indication events region start(value >= 0):");
			if (scanf("%d", &start) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (start < 0 || start > 9) {
				printf("%s\n", "GNSS indication events region start shuld be in band of 0 ~ 9");
				break;
			}

			printf("%s", "Input not receive GNSS indication events region end(value <= 9):");
			if (scanf("%d", &end) == EOF) {
				printf("%s\n", "Input error");
				break;
			}

			if (end < 0 || end > 9) {
				printf("%s\n", "GNSS indication events region end shuld be in band of 0 ~ 9");
				break;
			}

			if (end < start) {
				printf("%s\n", "GNSS indication events region end shuldn't be less then or equal start");
				break;
			}
			ret = QLRIL_GNSS_UnregisterEvents(&handle, start, end);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_UnregisterEvents success");
			else
				printf("%s%d\n", "QLRIL_GNSS_UnregisterEvents with return:", ret);
			break;
		}
		case 59:
		{
			ret = QLRIL_GNSS_StartNavigation(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_StartNavigation success");
			else
				printf("%s%d\n", "QLRIL_GNSS_StartNavigation with return:", ret);
			break;
		}
		case 60:
		{
			ret = QLRIL_GNSS_StopNavigation(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_StopNavigation success");
			else
				printf("%s%d\n", "QLRIL_GNSS_StopNavigation with return:", ret);
			break;
		}
		case 61:
		{
			double longitude = 0, latitude = 0, altitude = 0;
			float accuracy = 0;

			//timeout e.g: 60s
			ret = QLRIL_GNSS_GetLocation(&handle, &longitude, &latitude, &altitude, &accuracy, 60);//60s
			if (ret == 0) {
				printf("%s\n", "Current location:");
				printf("Longitude:%lf\n", longitude);
				printf("Latitude:%lf\n", latitude);
				printf("Altitude:%lf\n", altitude);
				printf("Accuracy:%lf\n", accuracy);
			} else
				printf("%s%d\n", "QLRIL_GNSS_GetLocation with return:", ret);
			break;
		}
		case 62:
		{
			double longitude = 0, latitude = 0, altitude = 0;
			float accuracy = 0;

			cmdIdx = 0;

			printf("%s\n", "Test the GNSS API start:");
			/* Test step 1 */
			ret = QLRIL_GNSS_AddListener(&handle, gnss_ind_callback, (void *)&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_AddListener success");
			else {
				printf("%s%d\n", "QLRIL_GNSS_AddListener with return:", ret);
				break;
			}

			ret = QLRIL_GNSS_SetAttribute(&handle, 0, 0, 2000, 0, 10);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_SetAttribute with return:", ret);
			}

			ret = QLRIL_GNSS_RegisterEvents(&handle, 0, 9);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_RegisterEvents with return:", ret);
			}

			ret = QLRIL_GNSS_StartNavigation(&handle);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_StartNavigation with return:", ret);
			}

			printf("%s\n", "Sleep 100s ...");
			sleep(50);

			ret = QLRIL_GNSS_UnregisterEvents(&handle, 2, 3);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_UnregisterEvents with return:", ret);
			}

			sleep(50);
			printf("%s\n", "Sleep end");

			ret = QLRIL_GNSS_StopNavigation(&handle);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_StopNavigation with return:", ret);
			}

			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_DelListener with return:", ret);
				break;
			}

			/* Test step 2 */
			ret = QLRIL_GNSS_AddListener(&handle, NULL, NULL);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_AddListener success");
			else {
				printf("%s%d\n", "QLRIL_GNSS_AddListener with return:", ret);
				break;
			}

			ret = QLRIL_GNSS_SetAttribute(&handle, 0, 1, 500, 0, 5);
			if (ret != 0) {
				printf("%s%d\n", "QLRIL_GNSS_SetAttribute with return:", ret);
			}

			printf("Test the QLRIL_GNSS_GetLocation(), May be wait for 1 minutes\n");
			ret = QLRIL_GNSS_GetLocation(&handle, &longitude, &latitude, &altitude, &accuracy, 60);//60s
			if (ret == 0) {
				printf("QLRIL_GNSS_GetLocation() Current location:\n");
				printf("Longitude:%lf\n", longitude);
				printf("Latitude:%lf\n", latitude);
				printf("Altitude:%lf\n", altitude);
				printf("Accuracy:%lf\n", accuracy);
			} else
				printf("QLRIL_GNSS_GetLocation with return:%d\n", ret);

			ret = QLRIL_GNSS_DelListener(&handle);
			if (ret == 0)
				printf("%s\n", "QLRIL_GNSS_DelListener success");
			else
				printf("%s%d\n", "QLRIL_GNSS_DelListener with return:", ret);

			printf("%s\n", "Test the GNSS API end!");

			break;
		}
		default:
			printf("%s\n", "Input command index error");
			break;
		}
	}

	snd_card_exit(&sndHandle);
}

int main(int argc,char *argv[])
{
	test_QLRIL_api();
}
