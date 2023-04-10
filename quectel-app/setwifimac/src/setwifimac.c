/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  setwifimac.c
 *    Description:  从NV获取WIFI地址，再设置WIFI的MAC. 
 *                 
 *        Version:  1.0.0(2019年11月27日)
 *         Author:  Peeta <peeta.chen@quectel.com>
 *      ChangeLog:  1, Release initial version on "2019年11月27日 18时43分59秒"
 *
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ql_misc.h>
#include <cutils/properties.h>

#define ATC_REQ_CMD_MAX_LEN     100
#define ATC_RESP_CMD_MAX_LEN    100
#define WCNSS_CTRL				"/dev/wcnss_ctrl"

int main (int argc, char **argv)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    char *temp = NULL;
    char mac[12] = {0};
    char macstr[18] = {0};
    char cmdStr[50] = {0};
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

    strcpy((char *)atc_cmd_req, "AT+QNVR=4678,0");
    ret = QLMISC_SendAT(atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
    printf("QMISC_SendAT \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret < 0) {
        goto error1;
    }

    temp = strtok(atc_cmd_resp, "\r\n");
    while(temp)
    {
        //printf("%s\n",temp);
        if (strstr(temp, "+QNVR:") != NULL) {
            //printf ("temp:(%s)\n", temp);
            sscanf(temp, "+QNVR:%*[^0-9,A-Z]%[0-9,a-z,A-Z]", mac);
        }
        temp = strtok(NULL,"\n");
    }

    printf("mac:(%s)\n", mac);

#if 0
    char machex[6] = {0};
    char strhex[5] = {0};
    for (i = 0; i < 5; i++) {
        sprintf(strhex, "0x%c%c", mac[i*2], mac[i*2 + 1]);
        printf("strhex = %s\n", strhex);

        machex[i] = (unsigned char)strtol(strhex, NULL, 16);

        printf("machex[%d] = 0x%x\n", i, machex[i]);
    }
#endif

    for (i = 0; i < 5; i++) {
        offset += sprintf(macstr + offset, "%c%c:", mac[i*2],mac[i*2 + 1]);
    }
    offset += sprintf(macstr + offset, "%c%c", mac[i*2],mac[i*2 + 1]);

    printf("macstr:(%s)\n", macstr);
    if (strlen(macstr) != 17)
        goto error1;

    memset(cmdStr, 0, sizeof(cmdStr));

    sprintf(cmdStr, "WIFI_NV_MAC");
	property_set(cmdStr, macstr);

#if 0
    sprintf(cmdStr, "setprop WIFI_NV_MAC %s\n", macstr);

    pid_t status = system(cmdStr);
    if (status == -1) {
        perror("system setprop");
        goto error1;
    } else {
        if(WIFEXITED(status)){
            if(0 == WEXITSTATUS(status)){
                printf("system setprop WIFI_NV_MAC successfully\n");
            } else {
                printf("system setprop WIFI_NV_MAC failed: %d \n",WEXITSTATUS(status));
            }
        } else
            printf("system setprop WIFI_NV_MAC failed: %d \n",WEXITSTATUS(status));
    }

    memset(cmdStr, 0, sizeof(cmdStr));
    sprintf(cmdStr, "ifconfig wlan0 up\n");

    status = system(cmdStr);
    if (status == -1) {
        perror("system ifconfig wlan0 up");
        goto error1;
    } else {
        if(WIFEXITED(status)){
            if(0 == WEXITSTATUS(status)){
                printf("system ifconfig wlan0 up: successfully\n");
            } else {
                printf("system ifconfig wlan0 up: failed: %d \n",WEXITSTATUS(status));
            }
        } else
            printf("system ifconfig wlan0 up: %d \n",WEXITSTATUS(status));
    }
#endif

error1:

    return 0;
} /* ----- End of main() ----- */

