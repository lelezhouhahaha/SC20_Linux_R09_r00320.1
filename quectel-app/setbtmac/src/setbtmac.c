/*
 *      Copyright:  (C) 2019 quectel
 *                  All rights reserved.
 *
 *       Filename:  btdamo.c
 *    Description:  从NV获取BT地址，在设置BT的MAC. 
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
#include <cutils/properties.h>
#include <qmi-framework/qmi_client.h>
#include <qmi-framework/qmi_client_instance_defs.h>
#include <qmi/device_management_service_v01.h>

#define ATC_REQ_CMD_MAX_LEN     100
#define ATC_RESP_CMD_MAX_LEN    100
#define HCISMD_SET_FILE         "/sys/module/hci_smd/parameters/hcismd_set"

static qmi_client_type dms_qmi_client;

int bt_qmi_dms_init(void)
{
    qmi_client_error_type qmi_client_err = QMI_NO_ERR;
    qmi_idl_service_object_type dms_service;
    qmi_client_qmux_instance_type qmi_modem_port;
    int timeout = 4;

    dms_service = dms_get_service_object_v01();
    if (dms_service == NULL) {
        printf("%s: Not able to get dms service handle\n", __func__);
        return -1;
    }

    qmi_modem_port = QMI_CLIENT_INSTANCE_ANY;

    qmi_client_err = qmi_client_init_instance(dms_service, qmi_modem_port,
            NULL, NULL, NULL, timeout, &dms_qmi_client);

    if (qmi_client_err != QMI_NO_ERR) {
        printf("%s: Error while initializing qmi_client_init_instance: %d\n",
                __func__, qmi_client_err);
        return -2;
    }

    return 0;
}

int bt_qmi_dms_exit(void)
{
    qmi_client_error_type qmi_client_err = QMI_NO_ERR;

    qmi_client_err = qmi_client_release(dms_qmi_client);
    if(qmi_client_err != QMI_NO_ERR){
        printf("Error: while releasing qmi_client : %d\n", qmi_client_err);
        return -1;
    }

    return 0;
}

int qmi_dms_get_bt_address(unsigned char *pBtAddr)
{
    qmi_client_error_type qmi_client_err;
    dms_get_mac_address_req_msg_v01 addr_req;
    dms_get_mac_address_resp_msg_v01 addr_resp;

    if (pBtAddr == NULL) {
        printf("%s: Error pBtAddr is NULL", __func__);
        return -1;
    }

    /* clear the request content */
    memset(&addr_req, 0, sizeof(addr_req));
    memset(&addr_resp, 0, sizeof(addr_resp));

    /* Request to get the Bluetooth address */
    addr_req.device = DMS_DEVICE_MAC_BT_V01;
    qmi_client_err = qmi_client_send_msg_sync(dms_qmi_client,
            QMI_DMS_GET_MAC_ADDRESS_REQ_V01, &addr_req, sizeof(addr_req), &addr_resp,
            sizeof(addr_resp), 2000);

    if (qmi_client_err != QMI_NO_ERR) {
        printf("%s: Error %d\n", __func__, qmi_client_err);
        return -2;
    }

    printf("%s: addr_resp.mac_address_valid %d addr_resp.mac_address_len %d",
            __func__, addr_resp.mac_address_valid, addr_resp.mac_address_len);

    if (addr_resp.mac_address_valid && (addr_resp.mac_address_len == 6)) {
        memcpy(pBtAddr, addr_resp.mac_address, addr_resp.mac_address_len);
        printf("%s: Successfully Read BT address\n", __func__);
    }

    return 0;
}

int main (int argc, char **argv)
{
    int i = 0;
    int ret = 0;
    int offset = 0;
    int qmi_dms_init_flag = 0;
    FILE *fp = NULL;
    char *temp = NULL;
    char mac[6] = {0};
    char macstr[18] = {0};
    char bdaddrStr[50] = {0};
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    pid_t status = 0;
#if 0
    pid_t status = system("setprop bluez.setmac false");
    if(0 != WEXITSTATUS(status)){
        perror("setprop bluez.setmac false");
    }
#endif
    property_set("bluez.setmac", "false");

    do {
        if (fp == NULL) {
            fp = fopen(HCISMD_SET_FILE, "w");
            if (fp == NULL) {
                perror("fopen hcismd_set file.");
            }
        }

        if (fp != NULL) {
            fseek(fp, 0, SEEK_SET);
            if (fputs("1", fp) < 0) {
                perror("fputs 1");
                fclose(fp);
                fp = NULL;
                continue;
            } else {
                fflush(fp);
            }

            //hciconfig hci0 up
            status = system("hciconfig hci0 up");
            if (status == -1) {
                perror("hciconfig hci0 up");
                //goto error2;
            } else {
                if(WIFEXITED(status)){
                    if(0 == WEXITSTATUS(status)){
                        printf("hciconfig hci0 up successfully.\n");
                        if (bt_qmi_dms_init() == 0) {
                            qmi_dms_init_flag = 1;
                            ret = qmi_dms_get_bt_address(mac);
                            if (ret == 0)
                                break;
                            else printf("qmi_dms_get_bt_address failed: %d\n", ret);
                        } else
                            qmi_dms_init_flag = 0;
                    } else {
                        printf("hciconfig hci0 up failed: %d \n",WEXITSTATUS(status));
                    }
                } else
                    printf("hciconfig hci0 up exit code %d \n",WEXITSTATUS(status));
            }
        }

        sleep(1);
    } while(i++ < 30);

    if (i >= 30)
        goto error1;

    //printf("mac:(%s)\n", mac);
    ret = 0;
    printf("mac:");
    for (i = 0; i < 6; i++) {
        printf("%x ", mac[i]);
        if (mac[i] != 0)
            ret = 1;
    }
    printf("\n");

    if (ret == 0) {
        printf ("Nv BT Mac is 00:00:00:00, error exit!\n");
        goto error1;
    }

    for (i = 5; i > 0; i--) {
        offset += sprintf(macstr + offset, "%02X:", mac[i]);
        //offset += sprintf(macstr + offset, "%c%c:", mac[i*2],mac[i*2 + 1]);
        //printf("macstr:(%s) offset = %d\n", macstr, offset);
    }
    offset += sprintf(macstr + offset, "%02X", mac[i]);
    //offset += sprintf(macstr + offset, "%c%c", mac[i*2],mac[i*2 + 1]);

    printf("macstr:(%s)\n", macstr);
    if (strlen(macstr) != 17)
        goto error1;

    //bdaddr -i hci0 5D:C7:68:3D:87:34
    sprintf(bdaddrStr, "bdaddr -i hci0 %s\n", macstr);

    status = system(bdaddrStr);
    if (status == -1) {
        perror("bdaddr");
        goto error1;
    } else {
        if(WIFEXITED(status)){
            if(0 == WEXITSTATUS(status)){
                printf("bdaddr -i hci0 successfully.\n");
            } else {
                printf("bdaddr -i hci0 failed %d \n",WEXITSTATUS(status));
            }
        } else
            printf("bdaddr -i hci0 exit code %d \n",WEXITSTATUS(status));
    }

    sleep(1);

    property_set("bluez.setmac", "true");
    memset(bdaddrStr,'\0',50);
    sprintf(bdaddrStr,"btnvtool -b %s\n",macstr);
    status = system(bdaddrStr);
    if (status == -1) {
        perror("btnvtool error");
        goto error1;
    } else {
        if(WIFEXITED(status)){
            if(0 == WEXITSTATUS(status)){
                printf("btnvtool successfully.\n");
            } else {
                printf("btnvtool failed %d \n",WEXITSTATUS(status));
            }   
        } else
            printf("btnvtool exit code %d \n",WEXITSTATUS(status));
    }   

    sleep(1);

    property_set("bluedroid.setmac", "true");

#if 0
    sprintf(atc_cmd_req, "setprop bluez.setmac true\n");
    status = system("setprop bluez.setmac true");
    if(0 != WEXITSTATUS(status)){
        perror("setprop bluez.setmac true");
    }

    sleep(1);

    fseek(fp, 0, SEEK_SET);
    if (fputs("1", fp) < 0) {
        perror("fputs 1");
        goto error1;
    }
    fflush(fp);
#endif

error1:
    if (qmi_dms_init_flag) {
        ret = bt_qmi_dms_exit();
        //if (ret < 0) {
        //return -2;
        //}
        qmi_dms_init_flag = 0;
    }

error2:
    if (fp) {
        fseek(fp, 0, SEEK_SET);
        if (fputs("0", fp) < 0) {
            perror("fputs 0");
            goto error1;
        }

        fflush(fp);

        fclose(fp);
        fp = NULL;
    }

    return 0;
} /* ----- End of main() ----- */

