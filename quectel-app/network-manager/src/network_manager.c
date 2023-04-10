/*
 *  Copyright:  (C) 2020 quectel
 *				All rights reserved.
 *   Filename:  network_manager.c
 *Description:  network manager
 *    Version:  1.0.0
 *     Author:  Peeta <peeta.chen@quectel.com>
 *  ChangeLog:  1, Release initial version on 2020.10.13
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/reboot.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <linux/input.h>
#include <cutils/properties.h>

#include <RilPublic.h>
#include <ql_misc.h>
#include <fileparser/iniparser.h>

#define MSMNETWORK_DEBUG 1
#ifdef MSMNETWORK_DEBUG

#ifndef LOG_TAG
#define LOG_TAG "networkmgr"
#endif
#include <cutils/log.h>

#define QLOGD_IF(fmt, ...)      ALOGD_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGW_IF(fmt, ...)      ALOGW_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGE_IF(fmt, ...)      ALOGE_IF(1, "[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGD(fmt, ...)         ALOGD("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGV(fmt, ...)         ALOGV("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGI(fmt, ...)         ALOGI("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGW(fmt, ...)         ALOGW("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)
#define QLOGE(fmt, ...)         ALOGE("[%s:%d] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#else

#define QLRILD_DEBUG 0
#define QLOGD_IF
#define QLOGW_IF
#define QLOGE_IF
#define QLOGD
#define QLOGV
#define QLOGI
#define QLOGW
#define QLOGE

#endif

#define NETWORK_MANAGER_CONFIG	"/etc/network_manager.ini"
#define SIM_SLOT_NUM 2
#define RETRY_COUNT 100
/**
 * @brief 
 * 
 */
struct simcardstate {
	int present;
	int app_state;
	int registration;
	int radio_tech;
	int network_on;
};
/**
 * @brief 
 * 
 */
struct simcardstate simcard[SIM_SLOT_NUM] = {0};

/**
 * @brief 
 * 
 */
dictionary *ini = NULL;

/**
 * @brief 
 * 
 * @param timeout
 * @return int:
 */
 
 /**
  * @brief Check qlrild service ready or not
  * 
  * @param handle : qlril handle
  * @param timeout : 0 block; !0 seconds
  * @return int : 0 success; !0 fail
  */

static int used_card = 0;

static int QL_RILD_Ready(int *handle, unsigned short timeout)
{
	int ret = 0;
	int block = !timeout;
	unsigned short count = timeout;

	QLOGD("Enter");

	do {
		ret = QLMISC_IsRunning("qlrild");
		if (ret > 0) {
			QLOGD("qlrild is running");
			ret = 0;
			break;
		}
		sleep(1);
	} while (block || (count--));

	count = timeout;
	do {
		ret = QLRIL_Init(handle);
		if (ret == 0){
			QLOGD("QLRIL_Init success");
			break;
		}
		sleep(1);
	} while (block || (count--));

	QLOGD("Exit");

	return ret;
}

static int QL_SIM_Ready(int *handle, unsigned short timeout)
{
	int ret = 0;
	int i = 0;
	int j = 0;
	RIL_CardStatus_v6 *cardStatus = NULL;
	int block = !timeout;
	unsigned short count = timeout;

	int slotId = 0;
	count = timeout;

	QLOGD("Enter");
	/* wait SIM card ready */
	do {
		ret = QLRIL_GetSimCardSlotId(handle, &slotId);
		if(ret != 0){
			QLOGW("QLRIL_GetSimCardSlotId failed with return:%d", ret);
		}
		else {
			QLOGD("QLRIL_GetSimCardSlotId success slotId:%d", slotId);
			break;
		}
		sleep(1);
		
	} while (block || (count--));

	if(ret != 0){
		QLOGE("No SIM card slot is ready");
		return ret;
	}

	for (i = 0; i < (used_card != 0 ? used_card : SIM_SLOT_NUM); i++) {
		if (used_card != 0)
			i = used_card -1;

		ret = QLRIL_SetSimCardSlotId(handle, i + 1);
		if (ret != 0) {
			QLOGW("QLRIL_SetSimCardSlotId failed with return:%d", ret);
			continue;
		}
		count = timeout;
		do {
			ret = QLRIL_GetIccCardStatus(handle, (void **)&cardStatus);
			if (ret < 0)
				QLOGW("QLRIL_GetIccCardStatus slotId:%d failed with return:%d", i + 1, ret);
			else
				QLOGD("QLRIL_GetIccCardStatus slotId:%d ret = %d", i + 1, ret);

			if (ret == 0 && cardStatus != NULL) {
				if (cardStatus->card_state == 1) {
					simcard[i].present = 1;
					for (j = 0; j < cardStatus->num_applications; j++) {
						if (cardStatus->applications[j].app_type > RIL_APPTYPE_ISIM)
							continue;

						if (cardStatus->applications[j].app_state == 5) {
							simcard[i].app_state = 5;
							used_card = i + 1;
							return ret;
						}
					}
					break;
				}
			}
			sleep(1);
		} while (--count);
	}

	QLOGD("Exit");

	return ret;
}

/**
 * @brief
 *
 * @param handle
 * @return int  >= 0 success; < 0 failed
 */
static int QL_SIM_Info(int *handle)
{
	int i = used_card -1;
	int ret = 0;
	char **resp = NULL;

	QLOGD("Enter");

	if (simcard[i].present != 1 && simcard[i].app_state != 5) {
		QLOGD("SIM Card[%d] is not present or ready", i + 1);
	} else
		QLOGD("SIM Card[%d] is present and ready", i + 1);

	ret = QLRIL_SetSimCardSlotId(handle, i + 1);
	if (ret != 0) {
		QLOGW("QLRIL_SetSimCardSlotId failed with return:%d", ret);
	}

	ret = QLRIL_GetDataRegistrationState(handle, &resp);
	if (ret < 0)
		QLOGW("QLRIL_GetDataRegistrationState failed with return:%d", ret);
	else
		QLOGD("QLRIL_GetDataRegistrationState ret = %d", ret);

	if (ret >= 0 && resp != NULL) {
		int num = ret;
		int j = 0;
		for (j = 0; j < num && j < 14; j++) {
			if (resp[j] == NULL)
				continue;

			//printf ("resp[%d]:%s\n", j, resp[j]);
			if (j == 0) {
				ret = atoi(resp[j]);
				simcard[i].registration = ret;
			} else if (j == 3) {
				ret = atoi(resp[j]);
				simcard[i].radio_tech = ret;
			}

			free(resp[i]);
			resp[i] = NULL;
		}

		if(resp) {
			free(resp);
			resp = NULL;
		}
	}

	QLOGD("Exit");

	return ret;
}

static int QL_SIM_Check_DataCall_Status(int *handle)
{
	int i = used_card -1;
	int ret = 0;
	RIL_Data_Call_Response_v11 *dataCallList = NULL;

	QLOGD("Enter");

	if (simcard[i].registration != 1) {
		QLOGW("SIM card[%d] can't setup data call", i + 1);
	}

	ret = QLRIL_SetSimCardSlotId(handle, i + 1);
	if (ret != 0) {
		QLOGW("QLRIL_SetSimCardSlotId failed with return:%d", ret);
	}

	ret = QLRIL_GetDataCallList(handle, (void **)&dataCallList);
	if (ret < 0)
		QLOGW("QLRIL_GetDataCallList failed with return:%d", ret);
	else
		QLOGD("QLRIL_GetDataCallList ret = %d", ret);

	if (ret > 0 && dataCallList != NULL) {
		int num = ret;
		int j = 0;
		for (j = 0; j < num; j++) {
			if (dataCallList[j].active != 0) {
				simcard[i].network_on = 1;
			}

			if (dataCallList[j].type)
				free(dataCallList[j].type);
			if (dataCallList[j].ifname)
				free(dataCallList[j].ifname);
			if (dataCallList[j].addresses)
				free(dataCallList[j].addresses);
			if (dataCallList[j].dnses)
				free(dataCallList[j].dnses);
			if (dataCallList[j].gateways)
				free(dataCallList[j].gateways);
			if (dataCallList[j].pcscf)
				free(dataCallList[j].pcscf);
		}

		free(dataCallList);
		dataCallList = NULL;
	}

	QLOGD("Exit");

	return ret;
}
static int QL_SIM_Start_DataCall(int *handle)
{
	int i = used_card -1;
	int ret = 0;
	int profileid = 0;
	char apn[100] = {0};
	char protocol[10] = {0};
	char buf[100] = {0};
	char dnses[200] = {0};
	char **resp = NULL;
	char *str = NULL;
	RIL_Data_Call_Response_v11 *dataCall = NULL;

	QLOGD("Enter");

	if (simcard[i].registration != 1) {
		QLOGW("SIM card[%d] can't setup data call", i + 1);
	}
	if (simcard[i].network_on == 1){
		/* shift to non-networked card */
		QLOGD("SIM card[%d] network is on", i + 1);
		return;
	}
	ret = QLRIL_SetSimCardSlotId(handle, i + 1);
	if (ret != 0) {
		QLOGD("QLRIL_SetSimCardSlotId failed with return:%d", ret);
	}

	ret = QLRIL_GetOperator(handle, &resp);
	if (ret < 0)
		QLOGW("QLRIL_GetOperator failed with return:%d", ret);
	else
		QLOGD("QLRIL_GetOperator success");

	if (ret > 0) {
		if (resp[1] != NULL && resp[2] != NULL) {
			snprintf(buf, sizeof(buf), "%s:mccmnc", resp[1]);
			str = iniparser_getstring(ini, buf, NULL);

			if (str != NULL) {
				if (strcmp(str, resp[2]) == 0) {
					memset(buf, 0, sizeof(buf));
					snprintf(buf, sizeof(buf), "%s:profileid", resp[1]);
					profileid = iniparser_getint(ini, buf, -1);

					memset(buf, 0, sizeof(buf));
					snprintf(buf, sizeof(buf), "%s:apn", resp[1]);
					str = iniparser_getstring(ini, buf, NULL);

					if (str != NULL) {
						snprintf(apn, sizeof(apn), "%s", str);
					}

					memset(buf, 0, sizeof(buf));
					snprintf(buf, sizeof(buf), "%s:protocol", resp[1]);
					str = iniparser_getstring(ini, buf, NULL);

					if (str != NULL) {
						snprintf(protocol, sizeof(protocol), "%s", str);
					}
				}
			}
		}
	}

	if (strlen(protocol) == 0) {
		snprintf(protocol, sizeof(protocol), "IPV4V6");
	}
	/* if ret equal to -4, try again */
	int try_count = 3;
retry:		
	QLRIL_SetDefaultTimeout(handle, 60);
	ret = QLRIL_SetupDataCall(handle, 14, profileid, apn, "", "", 0, protocol, (void **)&dataCall);
	if (ret < 0)
		QLOGW("QLRIL_SetupDataCall failed with return:%d", ret);
	else
		QLOGD("QLRIL_SetupDataCall success");

	if (ret > 0 && dataCall != NULL) {
		QLOGD("SIM card[%d] active is %s", i + 1, (dataCall->active == 0)?"down":"up");
		if (dataCall->active != 0) {
			simcard[i].network_on = 1;
			if (dataCall->dnses != NULL && strlen(dataCall->dnses) > 0) {
				str = dataCall->dnses;
			} else {
				ret = snprintf(dnses, sizeof(dnses), "%s", "8.8.8.8");//for example
				ret += snprintf(dnses + ret, sizeof(dnses), " %s", "4.2.2.2");
				ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200::1");
				ret += snprintf(dnses + ret, sizeof(dnses), " %s", "2400:3200:baba::1");
				str = dnses;
			}

			ret = QLRIL_SetDNSForIPV4V6(str);
			if (ret <= 0) {
				QLOGW("QLRIL_SetDNSForIPV4V6 failed with return:%d", ret);
			} else {
				QLOGD("QLRIL_SetDNSForIPV4V6 success");
			}

			if (dataCall->ifname != NULL && strlen(dataCall->ifname) > 0) {
				ret = QLRIL_SetRouteForIPV4V6(dataCall->ifname);
				if (ret < 0)
					QLOGW("QLRIL_SetRouteForIPV4V6 failed with return:%d", ret);
				else
					QLOGD("QLRIL_SetRouteForIPV4V6 success");
			} else
				QLOGW("QLRIL_SetupDataCall called failed without interface name");
		} else {
			simcard[i].network_on = 0;
			QLOGW("SIM card[%d] setup data call failed, Maybe change profile id or apn", i + 1);
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
		simcard[i].network_on = 0;
		QLOGW("SIM card[%d] QLRIL_SetupDataCall failed with return:%d", i + 1, ret);
		if((ret == -4) && (try_count-- > 0)){
			QLOGD("timeout retry data call");
			profileid += 1;
			goto retry;
		}
	}

	QLOGD("Exit");

	return ret;
}

int main (int argc, char **argv)
{
	int i, j, k;
	int ret = 0;
	int num = 0;
	int handle = 0;
	char buf[100] = {0};
	int datacallfail = 0;

	memset(simcard, 0, sizeof(simcard));

	int opt;
	int daemon = 0;
	char profile[256] = {NETWORK_MANAGER_CONFIG};
	int timeout = 3;

	QLOGD("NetWork Manager starting...");

	while ((opt = getopt(argc, argv, "dc:t:")) != -1) {
		switch (opt) {
			case 'd':{
				//TBD
				daemon = 1;
				break;
			}
			case 'c':{
				memset(profile, 0, sizeof(profile));
				strncpy(profile, optarg, sizeof(profile));
				break;
			}
			case 't':{
				timeout = atoi(optarg);
				break;
			}
		}
	}

	if (ini == NULL) {
		ini = iniparser_load(profile);
	} 
	//iniparser_dump(ini, stderr);

	ret = iniparser_getboolean(ini, "general:on", -1);
	if (ret != 1) {
		QLOGD("Mobile network don't need to setup data call");
		ret = 0;
		goto err;
	}

	ret = iniparser_getint(ini, "callsolt:id", 0);
	if (ret > 0 && ret <= SIM_SLOT_NUM ) {
		QLOGD("The configuration file sets the data call slotid:%d", ret);
		used_card = ret;
	}
wait_rild:
	ret = QL_RILD_Ready(&handle, timeout);
	if (ret != 0) {
		QLOGE("qlrild service is not start\n");
        sleep(3);
		goto wait_rild;
	}

	do {
		QL_SIM_Ready(&handle, timeout);
		if (used_card == 0) {
			datacallfail ++;
			continue;
		}

		QL_SIM_Info(&handle);

		/* */
		QL_SIM_Check_DataCall_Status(&handle);

		QL_SIM_Start_DataCall(&handle);

		if(simcard[used_card -1].network_on == 1) {
			QLOGD("SIM card[%d]: Network is working", used_card);
			datacallfail = 0;
		}
		else {
			datacallfail++;
			sleep(1);
		}

	} while (datacallfail != 0 && datacallfail < RETRY_COUNT);


err:
	iniparser_freedict(ini);//free ini

	if (handle != 0)
		QLRIL_Exit(&handle);

	QLOGD("NetWork Manager Finished");

	return ret;
} /* ----- End of main() ----- */


