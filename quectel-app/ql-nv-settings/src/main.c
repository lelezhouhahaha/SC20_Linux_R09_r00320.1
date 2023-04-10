/*
 * Copyright:  (C) 2020 Quectel
 *             All rights reserved.
 *   Version:  1.0.0(20200916)
 *    Author:  Peeta [fulinux] <peeta.chen@quectel.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#include <ql_misc.h>

int main(int argc, char **argv){
	int i = 0;
	int ret = 0;
	int state = 0;
	int enable = 1;
	int syncnv = 1;
	char *str = NULL;
	char cmd[100] = {0};
	char resp[512] = {0};

	if (argc > 1) {
		if (strncmp(argv[1], "0", 1) == 0) {
			enable = 0;
		}
	}

	for (i = 0; i < 40; i++) {
        memset(cmd, 0, sizeof(cmd));
        ret = snprintf(cmd, sizeof(cmd), "AT+CFUN?");

        ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
        if (ret < 0) {
            printf ("Error: send AT failed[%d]\n", ret);
            sleep(1);
            continue;
        }

        str = strstr(resp, "OK");
        if (str != NULL) {
			str = strstr(resp, "1");
			if (str != NULL)
				state = 1;//airplane mode off
			else
				state = 0;//airplane mode on
        } else {
			sleep(1);
			continue;
		}

		if (state == enable) {
            printf ("The airplane mode is %s\n", (enable == 0)?"on":"off");
			//This is exactly the same result as we expected
			break;
		}

        memset(cmd, 0, sizeof(cmd));
		if (enable == 0) {
            ret = snprintf(cmd, sizeof(cmd), "AT+CFUN=0");
		} else {
            ret = snprintf(cmd, sizeof(cmd), "AT+CFUN=1");
		}

        ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
        if (ret < 0) {
            printf ("Error: send AT failed[%d]\n", ret);
            sleep(1);
            continue;
        }

        str = strstr(resp, "OK");
        if (str != NULL) {
            printf ("Set airplane mode %s\n", (enable == 0)?"on":"off");
            break;
        }

		sleep(1);
	}

	for (i = 0; i < 10; i++) {
		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "AT+QNVFR=\"/nv/item_files/ims/ims_hybrid_enable\"\r\n");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		str = strstr(resp, "00");
		if (str != NULL) {
			printf ("ims_hybrid_enable return 00\n");
			break;
		}

		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "AT+QNVFW=\"/nv/item_files/ims/ims_hybrid_enable\",00\r\n");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		sleep(1);
	}

	for (i = 0; i < 10; i++) {
		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "AT+QNVFR=\"/nv/item_files/modem/qmi/cat/qmi_cat_mode\"\r\n");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		str = strstr(resp, "00");
		if (str != NULL) {
			printf ("qmi_cat_mode return 00\n");
			break;
		}

		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "AT+QNVFW=\"/nv/item_files/modem/qmi/cat/qmi_cat_mode\",00\r\n");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

        str = strstr(resp, "OK");
        if (str != NULL) {
            printf ("Set airplane mode %s\n", (enable == 0)?"on":"off");
            break;
        }

		sleep(1);
	}

	for (i = 0; i < 10; i++) {
		printf ("[at+qnvw=7165,0,\"18000000\"] begin\n");
		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "at+qnvr=7165,0");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		str = strstr(resp, "18000000");
		if (str != NULL) {
			printf ("[at+qnvr=7165,0] return 18000000\n");
			break;
		}

		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "at+qnvw=7165,0,\"18000000\"");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		sleep(1);
	}

	for (i = 0; i < 10; i++) {
		printf ("[at+qnvw=4398,0,\"01\"] begin\n");
		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "at+qnvr=4398,0");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		str = strstr(resp, "01");
		if (str != NULL) {
			printf ("[at+qnvr=4398,0] return 01\n");
			break;
		}

		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd), "at+qnvw=4398,0,\"01\"");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		sleep(1);
	}

	for (i = 0; i < 10; i++) {
		printf ("set enable_thin_ui_cfg begin\n");
		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd),
				"at+qnvfr=\"/nv/item_files/Thin_UI/enable_thin_ui_cfg\"");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		str = strstr(resp, "01");
		if (str != NULL) {
			printf ("enable_thin_ui_cfg return 01\n");
			break;
		}

		memset(cmd, 0, sizeof(cmd));
		ret = snprintf(cmd, sizeof(cmd),
				"at+qnvfw=\"/nv/item_files/Thin_UI/enable_thin_ui_cfg\",01");
		ret = QLMISC_SendAT(cmd, resp, sizeof(resp));
		if (ret < 0) {
			printf ("Error: send AT failed[%d]\n", ret);
			continue;
		}

		sleep(1);
	}

	return 0;
}
