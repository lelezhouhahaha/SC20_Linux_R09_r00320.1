/*
 *  Copyright:  (C) 2020 quectel
 *				All rights reserved.
 *   Filename:  ql_misc_test.c
 *Description:  This file is used to test for ql-misc daemon.
 *    Version:  1.0.0
 *     Author:  Peeta <peeta.chen@quectel.com>
 *  ChangeLog:  1, Release initial version on 2020.09.22
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <cutils/properties.h>

#include <ql_misc.h>
#include <RilPublic.h>
#include <ql-mcm-api/ql_in.h>

#include "c_printf.h"

#define MCM_SERVICE_READY_FILE      "/tmp/mcm_service_ready.flag"

typedef struct {
	int cmdIdx;
	char *funcName;
} st_api_test_case;

typedef struct {
	char *group_name;
	st_api_test_case *test_cases;
} func_api_test_t;

st_api_test_case api_testlist[] = {
	{0, "Help, Show all the QLMISC API"},

	{1, "QLMISC_SendAT"},
	{2, "QLMISC_USBCableState"},
	{3, "QLMISC_IsffbmMode"},
	{4, "QLSCREEN_Init"},
	{5, "QLSCREEN_Exit"},
	{6, "QLSCREEN_PowerOn"},
	{7, "QLSCREEN_PowerOff"},
	{8, "QLSCREEN_GetBrightness"},
	{9, "QLSCREEN_SetBrightness"},
	{10, "Suppress certain notifications"},
	{11, "Not suppress certain notifications"},
	{12, "QLMISC_EnterHibernate"},
	{13, "QLMISC_IsRunning"},
	{14, "QLMISC_RebootModem"},
	{15, "Send some AT commands to query informations"},
	{16, "turn on the screen by ql-power-manager process"},
	{17, "turn off the screen by ql-power-manager process"},

	{-1, "Exit!"}
};

func_api_test_t QLMISC_api_test = {"QLMISC API", api_testlist};

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

void processUnsolInd_cb(int *handle, int slotId, int event, void *data, size_t size)
{
	//TODO something
	c_printf ("[b]%s[y]%d[b]%s[y]%d\n", "The unsolicited event[", event, "] come from slot ID:", slotId);

	return;
}

static int qlril_init_and_add_listener(int *handle)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < 5; i++) {
		ret = QLRIL_Init(handle);
		if (ret != 0) {
			sleep(1);
			continue;
		}

		c_printf("[g]%s\n", "QLRIL_Init success");
		break;
	}

	if (i >= 5 || *handle == 0) {
		c_printf("[r]%s%d\n", "QLRIL_Init failure:", ret);
		return -1;
	} else {
		ret = QLRIL_AddUnsolEventsListener(handle, processUnsolInd_cb, NULL);
		if (ret != 0) {
			c_printf("[g]%s%d\n", "QLRIL_AddUnsolEventsListener failure:", ret);
		} else {
			ret = QLRIL_RegisterUnsolEvents(handle, 1000, 1048);
			if (ret != 0) {
				c_printf("[r]%s%d\n", "QLRIL_RegisterUnsolEvents failure:", ret);
				return -2;
			}

			ret = QLRIL_UnregisterUnsolEvents(handle, 1009, 1009);
			if (ret != 0) {
				c_printf("[r]%s%d\n", "QLRIL_UnregisterUnsolEvents failure:", ret);
				return -3;
			}
		}
	}

	return 0;
}

static int mcmnw_init(int *handle)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < 5; i++) {
		if(!access(MCM_SERVICE_READY_FILE, F_OK)) {
			ret = QL_MCM_NW_Client_Init(handle);
			if (ret < 0) {
				sleep(1);
				continue;
			}

			c_printf("[g]%s\n", "QL_MCM_NW_Client_Init success");
			break;
		}
	}

	if (i >= 5 || *handle == 0) {
		c_printf("[r]%s%d\n", "QL_MCM_NW_Client_Init failure:", ret);
		return -1;
	}

	return 0;
}

static void voice_Call_Ind(unsigned int ind_id, void* ind_data, uint32_t ind_data_len)
{
	c_printf ("[b]%s[y]%d[b]%s\n", "The unsolicited event[", ind_id, "]");

	return;
}

static int mcmvoice_init(int *handle)
{
	int ret = 0;
	int i = 0;

	for (i = 0; i < 5; i++) {
		if(!access(MCM_SERVICE_READY_FILE, F_OK)) {
			ret = QL_Voice_Call_Client_Init(handle);
			if (ret < 0) {
				sleep(1);
				continue;
			}

			c_printf("[g]%s\n", "QL_Voice_Call_Client_Init success");

			if (*handle > 0) {
				ret = QL_Voice_Call_AddCommonStateHandler(*handle,
						(QL_VoiceCall_CommonStateHandlerFunc_t)voice_Call_Ind);
				if (ret < 0) {
					c_printf("[r]%s%d\n", "QL_Voice_Call_AddCommonStateHandler failure:", ret);
					return -1;
				}
			}

			break;
		}
	}

	if (i >= 5 || *handle == 0) {
		c_printf("[r]%s%d\n", "QL_MCM_NW_Client_Init failure:", ret);
		return -2;
	}

	return 0;
}

static int test_QLMISC_api(void)
{
	int ret = 0;
	int cmdIdx = 0;
	int screen_handle = 0;
	int qlril_handle = 0;
	int mcmnw_handle = 0;
	int mcmvoice_handle = 0;

	usage(&QLMISC_api_test);
	while(1) {
		printf("\n");
		c_printf("[y]%s", "Please input cmd index (-1: exit, 0: help):");
		cmdIdx = getInputIntVal();

		if (cmdIdx == -1) {
			if (screen_handle != 0) {
				if (QLSCREEN_Exit(&screen_handle) == 0)
					c_printf("[g]%s\n", "QLSCREEN_Exit success");
				else
					c_printf("[r]%s\n", "QLSCREEN_Exit failure");
			}

			if (qlril_handle != 0) {
				ret = QLRIL_Exit(&qlril_handle);
				if (ret < 0)
					c_printf("[r]%s\n", "QLRIL_Exit failure");
				else
					c_printf("[g]%s\n", "QLRIL_Exit success");
			}

			if (mcmnw_handle != 0) {
				ret = QL_MCM_NW_Client_Deinit(mcmnw_handle);
				if (ret < 0)
					c_printf("[r]%s\n", "QL_MCM_NW_Client_Deinit failure");
				else
					c_printf("[g]%s\n", "QL_MCM_NW_Client_Deinit success\n");
			}

			if (mcmvoice_handle != 0) {
				ret = QL_Voice_Call_Client_Deinit(mcmnw_handle);
				if (ret < 0)
					c_printf("[r]%s\n", "QL_Voice_Call_Client_Deinit failure");
				else
					c_printf("[g]%s\n", "QL_Voice_Call_Client_Deinit success\n");
			}

			break;
		}

		if (cmdIdx == -2)
			continue;

		switch (cmdIdx) {
			case 0:
				usage(&QLMISC_api_test);
				break;
			case 1:
			{
				char cmd[500] = {0};
				char cmd_resp[1024] = {0};

				for (;;) {
					c_printf("[w]%s", "Please input AT command(-1: exit):");
					ret = getInputString(cmd, sizeof(cmd));
					if (ret < 2)
						continue;

					if (strstr(cmd, "-1") != NULL){
						break;
					}

					memset(cmd_resp, 0, sizeof(cmd_resp));
					ret = QLMISC_SendAT(cmd, cmd_resp, sizeof(cmd_resp));
					if (ret < 0)
						c_printf("[r]%s%d\n", "QLMISC_SendAT failed with return:", ret);
					else {
						c_printf("AT command response[%d]:\n", ret);
						c_printf("[g]%s\n", cmd_resp);
					}
				}

				break;
			}
			case 2:
			{
				ret = QLMISC_USBCableState();
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLMISC_USBCableState failure:", ret);
				else
					c_printf("[g]%s%s\n", "USB Cable:", (ret == 1)?"Connect":"Disconnect");

				break;
			}
			case 3:
			{
				ret = QLMISC_IsffbmMode();
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLMISC_IsffbmMode failure:", ret);
				else
					c_printf("[g]%s%s\n", "ffbm mode:", (ret == 1)?"yes":"no");

				break;
			}
			case 4:
			{
                if (screen_handle != 0) {
                    c_printf("[y]%s\n", "QLSCREEN_Init has been executed?");
                    break;
                }

				ret = QLSCREEN_Init(&screen_handle);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_Init failure:", ret);
				else
					c_printf("[g]%s\n", "QLSCREEN_Init success");

				break;
			}
			case 5:
			{
                if (screen_handle == 0) {
					c_printf("[y]%s\n", "Please run QLSCREEN_Init firstly");
                    break;
                }

				ret = QLSCREEN_Exit(&screen_handle);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_Exit failure:", ret);
				else
					c_printf("[g]%s\n", "QLSCREEN_Exit success");

				break;
			}
			case 6:
			{
				ret = QLSCREEN_PowerOn(screen_handle);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_PowerOn failure:", ret);
				else
					c_printf("[g]%s\n", "QLSCREEN_PowerOn success");

				break;
			}
			case 7:
			{
				ret = QLSCREEN_PowerOff(screen_handle);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_PowerOff failure:", ret);
				else
					c_printf("[g]%s\n", "QLSCREEN_PowerOff success");

				break;
			}
			case 8:
			{
				unsigned char value = 0;

				ret = QLSCREEN_GetBrightness(&value);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_GetBrightness failure:", ret);
				else
					c_printf("[g]%s%d\n", "Screen brightness value:", value);

				break;
			}
			case 9:
			{
				unsigned char value = 255;

				c_printf("[w]%s", "Please input screen brightness(0 ~ 255):");
				value = getInputIntVal();

				ret = QLSCREEN_SetBrightness(value);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLSCREEN_SetBrightness failure:", ret);
				else
					c_printf("[g]%s\n", "QLSCREEN_SetBrightness success");

				break;
			}
			case 10:
			{
				ret = QLMISC_IsRunning("qlrild");
				if (ret > 0) {
					c_printf("[g]%s%d\n", "qlrild is running, pid:", ret);
					if (qlril_handle == 0) {
						ret = qlril_init_and_add_listener(&qlril_handle);
						if (ret != 0) {
							c_printf("[r]%s%d", "qlrild init failure:", ret);
							qlril_handle = 0;
						}
					}

					if (qlril_handle != 0) {
						ret = QLRIL_SetScreenState(&qlril_handle, 0);//0 - off, 1 - on
						if (ret < 0) {
							c_printf("[r]%s%d", "QLRIL_SetScreenState failure:", ret);
							QLRIL_Exit(&qlril_handle);
							qlril_handle = 0;
						} else
							c_printf("[g]%s\n", "QLRIL_SetScreenState success");
					}
				}

				ret = QLMISC_IsRunning("mcm_ril_service");
				if (ret > 0) {
					c_printf("[g]%s%d\n", "mcm_ril_service is running, pid:", ret);
					if (mcmnw_handle == 0) {
						ret = mcmnw_init(&mcmnw_handle);
						if (ret != 0) {
							c_printf("[r]%s%d", "mcm nw init failure:", ret);
							mcmnw_handle = 0;
						}
					}

					if (mcmnw_handle > 0) {
						ret = QL_MCM_NW_SetLowPowerMode(mcmnw_handle, 1);//1 - low power on
						if (ret < 0) {
							c_printf("[r]%s%d", "QL_MCM_NW_SetLowPowerMode failure:", ret);
							QL_MCM_NW_Client_Deinit(mcmnw_handle);
							mcmnw_handle = 0;
						} else
							c_printf("[g]%s%d\n", "QL_MCM_NW_SetLowPowerMode success:", ret);

						if (mcmvoice_handle == 0) {
							ret = mcmvoice_init(&mcmvoice_handle);
							if (ret != 0) {
								c_printf("[r]%s%d", "mcm voice init failure:", ret);
								mcmvoice_handle = 0;
							}
						}
					}
				}

				break;
			}
			case 11:
			{
				ret = QLMISC_IsRunning("qlrild");
				if (ret > 0) {
					if (qlril_handle != 0) {
						ret = QLRIL_SetScreenState(&qlril_handle, 1);//0 - off, 1 - on
						if (ret < 0)
							c_printf("[r]%s%d", "QLRIL_SetScreenState failure:", ret);
						else
							c_printf("[g]%s\n", "QLRIL_SetScreenState success");
					}
				}

				ret = QLMISC_IsRunning("mcm_ril_service");
				if (ret > 0) {
					if (mcmnw_handle > 0) {
						ret = QL_MCM_NW_SetLowPowerMode(mcmnw_handle, 0);//0 - low power off
						if (ret < 0)
							c_printf("[r]%s%d", "QL_MCM_NW_SetLowPowerMode failure:", ret);
						else
							c_printf("[g]%s%d\n", "QL_MCM_NW_SetLowPowerMode success:", ret);
					}
				}

				break;
			}
			case 12:
			{
				ret = QLMISC_USBCableState();
				if (ret == 1) {
					c_printf("[r]%s\n", "***Can not enter hibernate!***");
					c_printf("[w]%s\n", "Please unplug the USB cable firstly");
					break;
				}

				c_printf("[g]%s\n", "***System Sleep***");
				usleep(10);
				ret = QLMISC_EnterHibernate();
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLMISC_EnterHibernate failure:", ret);
				else
					c_printf("[g]%s\n", "***System wakeup***");

				break;
			}
			case 13:
			{
				char process[40] = {0};

				c_printf("[w]%s", "Please input process name(e.g: qlrild; -1: exit):");
				ret = getInputString(process, sizeof(process));
				if (ret < 0)
					break;

				if (strstr(process, "-1") != NULL){
					break;
				}

				ret = QLMISC_IsRunning(process);
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLMISC_IsRunning failure:", ret);
				else {
					if (ret > 0)
						c_printf("[g]%s%s%d\n", process, " is running, pid:", ret);
					else
						c_printf("[g]%s%s\n", process, " is not running.");
				}

				break;
			}
			case 14:
			{
				ret = QLMISC_RebootModem();
				if (ret < 0)
					c_printf("[r]%s%d\n", "QLMISC_RebootModem failure:", ret);
				else
					c_printf("[g]%s\n", "reboot modem success");

				break;
			}
			case 15:
			{
				int i;
				char cmd_resp[1024] = {0};
				char *cmd[] = {
					"ATI",
					"AT+CPIN?",
					"AT+COPS?",
					"AT$QCSIMAPP=?",
					"AT+QFSGVERSION?",
					"AT+QGMR",
					"AT+CSUB",
					"AT+QMBNCFG=\"list\"",
					"at+cimi",
					"at+iccid",
					"at+egmr=0,7",
					"at+egmr=0,10",
					"at+egmr=0,5",
					"AT+CSQ",
					"AT+CREG?",
					"AT^SYSCONFIG?",
					"AT+CGSN",
					"AT+CCLK?",
					"AT+CGDCONT?",
					"AT+CGPADDR",
					"AT+ICCID",
					NULL,
							  };

				for (i = 0; cmd[i] != NULL; i++) {
					memset(cmd_resp, 0, sizeof(cmd_resp));
					ret = QLMISC_SendAT(cmd[i], cmd_resp, sizeof(cmd_resp));
					if (ret < 0)
						c_printf("[r]%s%d\n", "QLMISC_SendAT failed with return:", ret);
					else {
						c_printf("AT command response[%d]:\n", ret);
						c_printf("[g]%s\n", cmd_resp);
					}
				}
				break;
			}
            case 16:
            {
                property_set("ctrl.screen.power", "on");
                break;
            }
            case 17:
            {
                property_set("ctrl.screen.power", "off");
                break;
            }
			default:
			{
				char ch = 0;
				c_printf("[r]%s\n", "Input command index error");
				while((ch=getchar())!='\n' && ch != EOF);
				break;
			}
		}

		usleep(10000);
	}
}

int main(int argc,char *argv[])
{
	test_QLMISC_api();
}
