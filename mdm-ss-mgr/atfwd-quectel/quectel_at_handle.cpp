/*!
  @file
  sendcmd.cpp

  @brief
  Places a Remote Procedure Call (RPC) to Android's AtCmdFwd Service

*/

/*===========================================================================

  Copyright (c) 2011 Qualcomm Technologies, Inc. All Rights Reserved

  Qualcomm Technologies Proprietary

  Export of this technology or software is regulated by the U.S. Government.
  Diversion contrary to U.S. law prohibited.

  All ideas, data and information contained in or disclosed by
  this document are confidential and proprietary information of
  Qualcomm Technologies, Inc. and all rights therein are expressly reserved.
  By accepting this material the recipient agrees that this material
  and the information contained therein are held in confidence and in
  trust and will not be used, copied, reproduced in whole or in part,
  nor its contents revealed in any manner to others without the express
  written permission of Qualcomm Technologies, Inc.

  ===========================================================================*/

/*===========================================================================

  EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


  when       who     what, where, why
  --------   ---     ----------------------------------------------------------
  2018/11/17   geoff   First cut.


  ===========================================================================*/

/*===========================================================================
 *							NOTE
 *
 * return the devinfo  results for AT Command(AT+QDEVINFO?)
 * need add driver file:\kernel\drivers\qdevinfo\quectel_devinfo.c
 * chmod 444 for /sys/kernel/debug/mmc0/mmc0:0001/ext_csd
 * modify code:\kernel\drivers\platform\msm\qpnp-revid.c for get pmic information
 * maybe it also need add semiux permision
 ===========================================================================*/

/*===========================================================================

  INCLUDE FILES

  ===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_NIDEBUG 0
#define LOG_NDDEBUG 0
#define LOG_TAG "Atfwd_Sendcmd"

//#include <binder/BpBinder.h>
//#include <binder/IServiceManager.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include "common_log.h"
#include "quectel_at_handle.h"
#include <cutils/properties.h>
//#include <sys/msg.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "cJSON.h"
#define FIFO "/tmp/ififo"

#define MAX_KEYS 57

//#include <binder/IPCThreadState.h>
//#include <binder/ProcessState.h>
#ifdef QUECTEL_QGMR_CMD
#define ATFWD_DATA_PROP_QUEC_VER            "ro.build.quectelversion.release"
#define ATFWD_DATA_PROP_ANDROID_VER			 "ro.build.version.release"
#define QGMR_RESP_BUF_SIZE (380)
#define QUEC_LINUX_VERSION                  "/etc/build_version"

extern "C" void quec_qgmr_handle( const AtCmd *cmd ,AtCmdResponse *response)
{
    int ret=0;
    FILE *fp = NULL;
    int offset = 0;
    char *ptr;
    char str[50] = {0};
    char buffer[50] = {0};

    fp = fopen(QUEC_LINUX_VERSION, "r");
    if (fp != 0) {
        while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
            ptr = strchr(buffer, '=');
            if (!strncmp(buffer, "mainversion", ptr - buffer)) {
                sscanf(ptr + 1, "%s", str);
                printf("%s:%d  mainversion=(%s)\n", __func__, __LINE__, str);
                break;
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    printf("tokens[0]:%s\n", cmd->tokens[0]);
    printf("tokens[1]:%s\n", cmd->tokens[1]);

    ret = 20;
    ptr = strstr(cmd->tokens[0], "SC");
    if (ptr) {
        sscanf(ptr + 2, "%d%*s", &ret);
    }

    sprintf(buffer, "Quectel\nSC%d\nRevision: ", ret);

    if (strlen(str) == 0) {
        sprintf(str, "Linux3.18.071.01.001");
    }

    sprintf(response->response, "%s%s%s_%s", buffer, cmd->tokens[0], cmd->tokens[1], str);
    response->result = 1;
    return;
}
#endif


#ifdef QUECTEL_RF_CMD
extern "C" void quec_rf_handle( const AtCmd *cmd ,AtCmdResponse *response)
{
    sprintf(response->response, "%s", "");
    response->result = 1;
    printf("end:%s:%d\n", __func__, __LINE__);
    return;
}
#endif

#ifdef QUECTEL_QAPSUB_CMD
#define QUECTEL_ANDROID_VERSION            "ro.build.quectelversion.release"
#define Q_RESP_BUF_SIZE 390

extern "C" void quec_qapsub_handle(const AtCmd *cmd, AtCmdResponse *response)
{
    int ret = 0;
    FILE *fp = NULL;
    int offset = 0;
    char *ptr;
    char str[50] = {0};
    char buffer[50] = {0};

    fp = fopen(QUEC_LINUX_VERSION, "r");
    if (fp != 0) {
        while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
            ptr = strchr(buffer, '=');
            if (!strncmp(buffer, "subversion", ptr - buffer)) {
                sscanf(ptr + 1, "%s", str);
                printf("%s:%d  subversion=(%s)\n", __func__, __LINE__, str);
                break;
            }
            memset(buffer, 0, sizeof(buffer));
        }
    }

    if (strlen(str) == 0) {
        sprintf(response->response, "APSubEdition: V01");
    } else {
        sprintf(response->response, "APSubEdition: %s", str);
    }

    response->result = 1;
    return;
}

#endif // QUECTEL_AT_QAPSUB_FEATURE

#ifdef QUECTEL_LWM2M_CMD
cJSON *pHostDev_info_Obj = NULL;

int lwm2m_initialize_configuration()
{
	struct stat stat_buf;
	int 	fd;
	char   *json_buf = NULL;

	cJSON  *pobj_list;
	cJSON  *dev_list;
	if (!(0 == lstat( "/data/lwm2m/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
    {
		mkdir("/data/lwm2m/", ALLPERMS);
    }
	if(0 == lstat("/data/lwm2m/lwm2m_hostdevice.ini",&stat_buf))
	{
		fd = open("/data/lwm2m/lwm2m_hostdevice.ini", O_RDONLY);

		if(fd >= 0)
		{
			
			int  json_buf_sz = 0;

			json_buf = (char *)malloc(1024);
			json_buf_sz = read(fd,json_buf, 1024);

			close(fd);
			
			pHostDev_info_Obj = cJSON_Parse(json_buf);
			free(json_buf);
		}
	}
	if(pHostDev_info_Obj == NULL){	
		// pHostDev_info_Obj = cJSON_CreateObject();
		// dev_list = cJSON_CreateArray();
		
		// cJSON_AddItemToObject(pHostDev_info_Obj,"id", dev_list);	

		// lwm2m_init_hostdevice_info(dev_list, 0);
		// lwm2m_init_hostdevice_info(dev_list, 1);
		
		// json_buf = cJSON_Print(pHostDev_info_Obj);
		
		// fd = open("/data/lwm2m/lwm2m_hostdevice.ini", O_CREAT|O_TRUNC|O_RDWR,0777);

		// if(fd < 0)
			// return 0;
		
		// write(fd, json_buf, strlen(json_buf));	
		// free(json_buf);
		// close(fd);	
	}
	return 1;
}

extern "C" bool lwm2m_load_hostdevice_info(int id, lwm2m_hostdevice_info_t *dev_info)
{
	cJSON  *info_list_obj = NULL;
	cJSON  *info_obj = NULL;
	cJSON  *id_item = NULL;
	cJSON  *manu_item = NULL;
	cJSON  *model_item = NULL;
	cJSON  *sw_item = NULL;

	if(id > 1 || id < 0 || dev_info == NULL ||pHostDev_info_Obj == NULL){
		return false;
	}

	info_list_obj = cJSON_GetObjectItem(pHostDev_info_Obj,"id");

	info_obj = cJSON_GetArrayItem(info_list_obj, id);

	if(info_obj == NULL)
		return false;

	id_item = cJSON_GetObjectItem(info_obj,"unique_id");
	manu_item = cJSON_GetObjectItem(info_obj,"manufacture");
	model_item = cJSON_GetObjectItem(info_obj,"model");
	sw_item = cJSON_GetObjectItem(info_obj,"sw_version");

	if(id_item != NULL){
		memset(dev_info->unique_id, 0, 128);
		strcpy(dev_info->unique_id, id_item->valuestring);
	}
	if(manu_item != NULL){
		memset(dev_info->manufacture, 0, 128);
		strcpy(dev_info->manufacture, manu_item->valuestring);
	}
	if(model_item != NULL){
		memset(dev_info->model, 0, 128);
		strcpy(dev_info->model, model_item->valuestring);
	}
	if(sw_item != NULL){
		memset(dev_info->sw_version, 0, 128);
		strcpy(dev_info->sw_version, sw_item->valuestring);
	}
	return true;
}

extern "C" void quec_odis_handle(const AtCmd *cmd, AtCmdResponse *response)
{
    int i;
	int fd;
	char buf[512];
    char *resp_buf = NULL;
    int offset = 0;
    lwm2m_hostdevice_info_t  hostdev_info;
    response->result = 1;
    resp_buf = response->response;

    //sprintf(response->response, "ODIS: ,%d", cmd->ntokens);
    if(strcasecmp(cmd->tokens[0], "ALL") == 0){
        lwm2m_initialize_configuration();
        lwm2m_load_hostdevice_info(0, &hostdev_info);
        // offset += sprintf(resp_buf + offset, "+ODIS: Host Device ID: %s\n",hostdev_info.unique_id);//bb
        // offset += sprintf(resp_buf + offset, "+ODIS: Host Device Manufacturer: %s\n",hostdev_info.manufacture);
        // offset += sprintf(resp_buf + offset, "+ODIS: Host Device Model: %s\n",hostdev_info.model);//pmic
        // offset += sprintf(resp_buf + offset, "+ODIS: Host Device Software Version: %s",hostdev_info.sw_version);//PA
        sprintf(response->response, "+ODIS:%s, %s, %s, %s.", hostdev_info.unique_id, hostdev_info.manufacture, hostdev_info.model, hostdev_info.sw_version);
    }else{
        if (cmd->ntokens < 4) {
            printf("Incorrect parameter!");
            sprintf(response->response, "ODIS:ERROR! ,%d", cmd->ntokens);
            response->result = 0;
            return;
        }
        if(mkfifo(FIFO, 0777) != 0 && errno != EEXIST)
        {
            perror("Mkfifo error");
            sprintf(response->response, "ODIS:Fail! Mkfifo error! %s",strerror(errno));
            response->result = 0;
            return;
        }
        printf("open for writing ... \n");
        //fd=open(FIFO,O_WRONLY);
        fd=open(FIFO,O_WRONLY|O_NONBLOCK);
        printf("opened... \n");
        if(fd < 0)
        {
            perror("Failed to open fifo:");
            sprintf(response->response, "ODIS:Fail! fd < 0,Please run lwm2m first.");
            response->result = 0;
            return;
        }
        sprintf(buf, "%s,%s,%s,%s", cmd->tokens[0], cmd->tokens[1],cmd->tokens[2],cmd->tokens[3]);
        write(fd,buf,strlen(buf));
        close(fd);
        //unlink(FIFO);
        sprintf(response->response, "ODIS:write success. buf = %s .",buf);
    }
    response->result = 1;
    return;
}

#endif // QUECTEL_LWM2M_CMD

#ifdef QUECTEL_QDEVINFO_CMD
#define QUEC_EMCP_INFO 	"/proc/quec_emcp_info"
#define QUEC_PMU_INFO 	"/proc/quec_pmu_info"
#define QUEC_CPU_INFO 	"/proc/cpuinfo"
#define QUEC_EMCP_NAME  "/sys/class/mmc_host/mmc0/mmc0:0001/name"
#define QUEC_EMCP_MEMINFO  "/proc/meminfo"
#define QUEC_EMMC_SIZE   "/sys/block/mmcblk0/size"
#define QUEC_INFO_LEN 128
#define RESULT_BUF_SIZE 256
// Maybe you should change QMI_ATCOP_AT_RESP_MAX_LEN for your requirement
extern "C" int quectel_search_str( char *str1,char *str2)
{
    int i;//,j;
    char *buf;
    char str_Hardware[]="Qualcomm Technologies, Inc ";
    buf=strstr(str1,str_Hardware)+strlen(str_Hardware);
    for(i=0;buf[i] != '\0';i++) {
        if(buf[i] == '\n') {
            buf[i]='\0';
            break;
        }
    }
    if((buf != NULL) && (strlen(buf) < QUEC_INFO_LEN))
        snprintf(str2,strlen(buf)+1,"%s",buf);
    else
        sprintf(str2,"get cpuinfo error!!");
    return 0;
}

extern "C" void get_cpu_info( char *info)
{
    size_t size;
    char buf[4096]={'\0'};
    char cpu_str[QUEC_INFO_LEN]={'\0'};
    FILE *fp = NULL;
    if((fp=fopen(QUEC_CPU_INFO, "r")) != NULL)
    {
        size = fread(buf,4096,1,fp);
        if(fp)
            fclose(fp);
        quectel_search_str(buf,cpu_str);
        sprintf(info,"%s",cpu_str);
        return;
    }
}

extern "C" void get_file_info( const char *file,char *info)
{
    int i;
    size_t size;
    char devinfo_str[QUEC_INFO_LEN]={'\0'};
    FILE *fp = NULL;
    if((fp=fopen(file, "r")) != NULL)
    {
        size = fread(devinfo_str,QUEC_INFO_LEN,1,fp);
        if(fp)
            fclose(fp);
        for(i=0;devinfo_str[i]!='\0';i++)
        {
            if(devinfo_str[i]=='\n')
            {
                devinfo_str[i]='\0';
                break;
            }
        }
        sprintf(info,"%s",devinfo_str);
        return;
    }
}

extern "C" int get_flash_info(char *buf)
{
    FILE *fp = NULL;
    size_t filesize = 0;
    ssize_t readsize = 0;
    int totlesize = 0;
    int offset = 0;
    char *line = NULL;
    char buffer[50] = {0};
    char *ptr;

    //printf("%s:%d  Peeta I'm here", __func__, __LINE__);

    if (buf == NULL) {
        printf("%s:%d buf is NULL\n", __func__, __LINE__);
        return -1;
    }

    /* EMCP name */
    fp = fopen(QUEC_EMCP_NAME, "r");
    if (fp == 0) {
        printf("%s: unable to open %s\n", __func__, QUEC_EMCP_NAME);
        return -1;
    }

    readsize = getline(&line, &filesize, fp);
    if (readsize < 0) {
        printf("%s:%d failed to getline\n", __func__, __LINE__);
        return -1;
    }

    offset += snprintf(buf, readsize, "%s", line);

    if (line)
        free(line);
    line = NULL;

    fclose(fp);

    /* EMMC size */
    fp = fopen(QUEC_EMMC_SIZE, "r");
    if (fp == 0) {
        printf("%s: unable to open %s\n", __func__, QUEC_EMMC_SIZE);
        return -1;
    }

    readsize = getline(&line, &filesize, fp);
    if (readsize < 0) {
        printf("%s:%d failed to getline\n", __func__, __LINE__);
        return -1;
    }

    //printf("%s:%d  line:%s", __func__, __LINE__, line);

    sscanf(line, "%d", &totlesize); //MB

    if (line)
        free(line);
    line = NULL;

    fclose(fp);

    totlesize /= 2048;
    if (totlesize % 4096 == 0)
        totlesize = (totlesize/4096) * 4; //GB
    else
        totlesize = (totlesize/4096 + 1) * 4; //GB
    //printf("%s:%d totlesize = %d, buf is:%s. offset = %d", __func__, __LINE__, totlesize, buf, offset);
    offset += sprintf(buf + offset - 1, ",%dG", totlesize);
    //printf("%s:%d totlesize = %d, buf is:%s. offset = %d", __func__, __LINE__, totlesize, buf, offset);

    /* size */
    fp = fopen(QUEC_EMCP_MEMINFO, "r");
    if (fp == 0) {
        printf("%s: unable to open %s\n", __func__, QUEC_EMCP_MEMINFO);
        return -1;
    }

    while(fgets(buffer, sizeof(buffer) - 1, fp) != NULL) {
        ptr = strchr(buffer, ':');
        if (!strncmp(buffer, "MemTotal", ptr - buffer)) {
            sscanf(ptr + 1, "%d%*s", &totlesize);
            totlesize = totlesize/1024;

            //printf("%s:%d  memTotal size: %dMB", __func__, __LINE__, totlesize);

            if (totlesize <= 512)
                offset += sprintf(buf + offset - 1, ",512M");
            else {
                totlesize = totlesize/1024 + 1;
                offset += sprintf(buf + offset - 1, ",%dG", totlesize);
            }
            //printf("%s:%d totlesize = %d, buf is:%s\n", __func__, __LINE__, totlesize, buf);
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    fclose(fp);

    return 0;
}

extern "C" void quec_qdevinfo_handle(const AtCmd *cmd, AtCmdResponse *response)
{
    int i;
    char *resp_buf=NULL;
    char devinfo[QUEC_INFO_LEN]={'\0'};
    int offset=0;

    response->result = 1;
    resp_buf = response->response;

    if (cmd->ntokens <= 0) {
        printf("%s:%d ntokens is 0!\n", __func__, __LINE__);
        free(resp_buf);
        response->response = NULL;
        response->result = 0;
        return;
    }

    for (i = 0; i < cmd->ntokens; i++) {
        //printf("%s:%d Peeta:tokens[%d] = %s", __func__, __LINE__, i, cmd->tokens[i]);
        if (cmd->tokens[i] == NULL) {
            printf("%s:%d tokens is NULL\n", __func__, __LINE__);
            free(resp_buf);
            response->response = NULL;
            response->result = 0; // failure
            return;
        }
    }

    if(strcasecmp(cmd->tokens[0], "FLASH") == 0) {
        //printf("%s:%d  Peeta I'm here", __func__, __LINE__);
        //get_file_info(QUEC_EMCP_INFO,devinfo);
        if (get_flash_info(devinfo) == 0) {
            offset += sprintf(resp_buf + offset, "+QDEVINFO: \"Flash\",%s", devinfo);
            response->result = 1;
        } else {
            offset += sprintf(resp_buf + offset, "+QDEVINFO: \"Flash\",NULL");
            response->result = 1;
        }
    } else if(strcasecmp(cmd->tokens[0], "BB") == 0) {
        get_cpu_info(devinfo);
        offset += sprintf(resp_buf+offset, "+QDEVINFO: \"BB\",%s",devinfo);
    } else if(strcasecmp(cmd->tokens[0], "PA")==0) {

        if(cmd->tokens[1] && cmd->tokens[2])
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",%s,%s", cmd->tokens[1], cmd->tokens[2]);//PA
        else if (cmd->tokens[1])
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",%s", cmd->tokens[1]);//PA
        else
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",NULL");
    } else if (strcasecmp(cmd->tokens[0], "PMIC") == 0) {
        get_file_info(QUEC_PMU_INFO,devinfo);
        offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PMIC\",%s", devinfo);
    } else if (strcasecmp(cmd->tokens[0], "CHECK") == 0) {
        //printf("%s:%d Peeta:I'm here!", __func__, __LINE__);
        offset += sprintf(resp_buf+offset, "+QDEVINFO: \"BB\"\n");//bb
        offset += sprintf(resp_buf + offset, "+QDEVINFO: \"Flash\"\n");
        offset += sprintf(resp_buf + offset, "+QDEVINFO: \"PMIC\"\n");//pmic
        offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\"");//PA
    } else if(strcasecmp(cmd->tokens[0], "ALL") == 0) {
        memset(devinfo, 0, QUEC_INFO_LEN);
        get_cpu_info(devinfo);
        offset += sprintf(resp_buf+offset, "+QDEVINFO: \"BB\",%s\n", devinfo);//bb

        memset(devinfo, 0, QUEC_INFO_LEN);
        if (get_flash_info(devinfo) == 0) {
            offset += sprintf(resp_buf + offset, "+QDEVINFO: \"Flash\",%s\n", devinfo);
        } else
            offset += sprintf(resp_buf + offset, "+QDEVINFO: \"Flash\",NULL\n");

        memset(devinfo, 0, QUEC_INFO_LEN);
        get_file_info(QUEC_PMU_INFO,devinfo);
        offset += sprintf(resp_buf + offset, "+QDEVINFO: \"PMIC\",%s\n", devinfo);//pmic
        if(cmd->tokens[1] && cmd->tokens[2])
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",%s,%s", cmd->tokens[1], cmd->tokens[2]);//PA
        else if (cmd->tokens[1])
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",%s", cmd->tokens[1]);//PA
        else
            offset += sprintf(resp_buf+offset, "+QDEVINFO: \"PA\",NULL");
        //printf("%s:%d length = %ld, resp_buf: %s", __func__, __LINE__, strlen(resp_buf), resp_buf);
    } else {
        printf("%s:%d something is wrong!\n", __func__, __LINE__);
        response->result = 0;
        free(response->response);
        response->response = NULL;
    }

    return;
}
#endif

#ifdef QUECTEL_QAPCMD_CMD

#define QUEC_CHGE_INFO  "/sys/class/power_supply/battery/charging_enabled"
#define QUEC_LWM2M_BS_SERVER_VZW_INI	"/persist/lwm2m/lwm2m_bs_server.ini" //add by joe for vzw
#define QUEC_LWM2M_DATA_FOLDER  "/data/lwm2m/"
#define QUEC_LWM2M_CONF	"/persist/lwm2m_conf/lwm2m.conf"

extern "C" void enable_lwm2m_client(bool enable){
    FILE *fp;
    fp=fopen("/etc/lwm2m.conf","w+");
    if(fp){
        if(enable)
            fprintf(fp,"%s","DEFAULT_STATUS=ON\n");
        else
            fprintf(fp,"%s","DEFAULT_STATUS=OFF\n");
    }
    fclose(fp);
}

extern "C" void quec_qapcmd_handle( const AtCmd *cmd ,AtCmdResponse *response)
{
    int i;
    int offset=0;
    FILE *fp = NULL;
    char *resp_buf=NULL;
    char buf[128] = {0};
	int fd = 0;
	struct stat stat_buf;

    resp_buf = (char *)malloc(RESP_BUF_SIZE);
    if(resp_buf == NULL) {
        printf("%s:%d  No Memory\n", __func__, __LINE__);
        return; // error
    }
    memset(resp_buf, 0, RESP_BUF_SIZE);

    response->result = 1;
    response->response = resp_buf;

    if (cmd->ntokens <= 0) {
        printf("%s:%d ntokens is 0!\n", __func__, __LINE__);
        free(response->response);
        response->response = NULL;
        response->result = 0;
        return;
    }

    for (i = 0; i < cmd->ntokens; i++) {
        //printf("%s:%d tokens[%d]:(%s)\n", __func__, __LINE__, i, cmd->tokens[i]);
        if (cmd->tokens[i] == NULL) {
            //printf("%s:%d failed to remove quotation\n", __func__, __LINE__);
            free(response->response);
            response->response = NULL;
            response->result = 0; // failure
            return;
        }
    }
    //printf("QAPCMD tokens[0]=%s\n", cmd->tokens[0]);
    //printf("QAPCMD tokens[1]=%s\n", cmd->tokens[1]);
    if(strcasecmp(cmd->tokens[0], "CHGENABLE")==0) {
        if(cmd->ntokens==1) {
            fp = fopen(QUEC_CHGE_INFO, "r");
            if (fp == NULL) {
                printf("%s: unable to open %s\n", __func__, QUEC_CHGE_INFO);
                return;
            }

            while (fgets(buf, sizeof(buf), fp) != NULL);

            fclose(fp);

            //sscanf(buf, "%d", &i);
            //offset += sprintf(resp_buf + offset, "+QAPCMD:CHGENABLE,%d", !i);
            offset += sprintf(resp_buf + offset, "+QAPCMD:CHGENABLE,%s", buf);

            printf("+QAPCMD:CHGENABLE get: %s\n", buf);
            response->result = 1;
        }else if((cmd->ntokens == 2)&&((strcasecmp(cmd->tokens[1], "0")==0)
                    ||(strcasecmp(cmd->tokens[1], "1")==0))) {
            fp = fopen(QUEC_CHGE_INFO, "w");
            if (fp == NULL) {
                printf("%s: unable to open %s\n", __func__, QUEC_CHGE_INFO);
                return;
            }

#if 0
            if (strcasecmp(cmd->tokens[1], "0") == 0) {
                if (fputs("1", fp) < 0) {
                    printf("%s: failed to write %s to %s", __func__, cmd->tokens[1], QUEC_CHGE_INFO);
                    return;
                }
            } else {
                if (fputs("0", fp) < 0) {
                    printf("%s: failed to write %s to %s", __func__, cmd->tokens[1], QUEC_CHGE_INFO);
                    return;
                }
            }
#endif
            if (fputs(cmd->tokens[1], fp) < 0) {
                printf("%s: failed to write %s to %s\n", __func__, cmd->tokens[1], QUEC_CHGE_INFO);
                return;
            }

            fclose(fp);

            //no response
            printf("+QAPCMD:CHGENABLE set: %s\n", cmd->tokens[1]);
            response->result = 1;
        }else{
            free(response->response);
            response->response = NULL;
            response->result = 0; // error
            printf("QAPCMD_AT CHGENABLE error\n");
            return;
        }
    } else if((strcasecmp(cmd->tokens[0], "CHECK")==0) && (cmd->ntokens==1)) {
        offset += snprintf(resp_buf+offset, (RESP_BUF_SIZE-offset), "+QAPCMD:version1.1\n");
        offset += snprintf(resp_buf+offset, (RESP_BUF_SIZE-offset), "+QAPCMD:CHGENABLE");
        //offset += snprintf(resp_buf+offset, (RESP_BUF_SIZE-offset), "+QAPCMD:CHANGESIM\n");
    } else if((strcasecmp(cmd->tokens[0], "ALL")==0) && (cmd->ntokens==1)) {
        fp = fopen(QUEC_CHGE_INFO, "r");
        if (fp == NULL) {
            printf("%s: unable to open %s\n", __func__, QUEC_CHGE_INFO);
            return;
        }

        while (fgets(buf, sizeof(buf), fp) != NULL);

        fclose(fp);

#if 0
        sscanf(buf, "%d", &i);
        offset += sprintf(resp_buf + offset, "+QAPCMD:CHGENABLE,%d", !i);

        printf("+QAPCMD:CHGENABLE get: %d", !i);
#else
        offset += sprintf(resp_buf + offset, "+QAPCMD:CHGENABLE,%s", buf);
        printf("+QAPCMD:CHGENABLE get: %s\n", buf);
#endif

        response->result = 1;
    }else if(strcasecmp(cmd->tokens[0], "qlwcfgbs")==0) {//add by joe for lwm2m config start
        if (cmd->ntokens < 2) {
            printf("Incorrect parameter!");
            sprintf(response->response, "QAPCMD:ERROR! ,%d", cmd->ntokens);
            response->result = 0;
            return;
        }
        #if 1
        char *filename = QUEC_LWM2M_BS_SERVER_VZW_INI;
        if (access(filename, F_OK) == 0)
        {
            printf("if QUEC_LWM2M_BS_SERVER_VZW_INI exist, remove it");
            remove(QUEC_LWM2M_BS_SERVER_VZW_INI);
        }
        if (!(0 == lstat( "/persist/lwm2m/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
        {
            mkdir("/persist/lwm2m/", ALLPERMS);
        }
        system("rm -rf /data/lwm2m");
        fp = fopen(QUEC_LWM2M_BS_SERVER_VZW_INI,"w+");
        if(fp == NULL)
        {
            printf("file QUEC_LWM2M_BS_SERVER_VZW_INI error: \n");
            perror("Failed to open file:");
            sprintf(response->response, "QAPCMD:Fail! fd < 0,Vzw lwm2m bs server set error.");
            response->result = 0;
        }
        fwrite(cmd->tokens[1], strlen(cmd->tokens[1]), 1, fp);
        fclose(fp);
        sprintf(buf, "%s,%s", cmd->tokens[0],cmd->tokens[1]);
        #else
        if(mkfifo(FIFO, 0777) != 0 && errno != EEXIST)
        {
            perror("Mkfifo error");
            sprintf(response->response, "QAPCMD:Fail! Mkfifo error! %s",strerror(errno));
            response->result = 0;
            return;
        }
        printf("open for writing ... \n");
        //fd=open(FIFO,O_WRONLY);
        fd=open(FIFO,O_WRONLY|O_NONBLOCK);
        printf("opened... \n");
        if(fd < 0)
        {
            perror("Failed to open fifo:");
            sprintf(response->response, "QAPCMD:Fail! fd < 0,Vzw lwm2m bs server already set.");
            response->result = 0;
            return;
        }
        sprintf(buf, "%s,%s", cmd->tokens[0],cmd->tokens[1]);
        write(fd,buf,strlen(buf));
        close(fd);
        //unlink(FIFO);
        #endif
        sprintf(response->response, "QPACD:write success. buf = %s",buf);
        response->result = 1;
        system("reboot");
    }else if(strcasecmp(cmd->tokens[0], "qlwreset")==0) {
        if (cmd->ntokens < 2) {
            printf("Incorrect parameter!");
            sprintf(response->response, "QAPCMD:ERROR! ,%d", cmd->ntokens);
            response->result = 0;
            return;
        }
        #if 1
        char *filename = QUEC_LWM2M_DATA_FOLDER;
        if (access(filename, F_OK) == 0)
        {
            printf("if QUEC_LWM2M_DATA_FOLDER exist, remove it");
            system("rm -rf /data/lwm2m");
            system("rm -rf /persist/lwm2m");
        }
        sprintf(buf, "%s,%s", cmd->tokens[0],cmd->tokens[1]);
        #else
        if(mkfifo(FIFO, 0777) != 0 && errno != EEXIST)
        {
            perror("Mkfifo error");
            sprintf(response->response, "QAPCMD:Fail! Mkfifo error! %s",strerror(errno));
            response->result = 0;
            return;
        }
        printf("open for writing ... \n");
        //fd=open(FIFO,O_WRONLY);
        fd=open(FIFO,O_WRONLY|O_NONBLOCK);
        printf("opened... \n");
        if(fd < 0)
        {
            perror("Failed to open fifo:");
            sprintf(response->response, "QAPCMD:Fail! fd < 0,Please run lwm2m first.");
            response->result = 0;
            return;
        }
        sprintf(buf, "%s,%s", cmd->tokens[0],cmd->tokens[1]);
        write(fd,buf,strlen(buf));
        close(fd);
        //unlink(FIFO);
        #endif
        sprintf(response->response, "QPACD:write success. buf = %s",buf);
        response->result = 1;
    }//add by joe for lwm2m config end
    else if(strcasecmp(cmd->tokens[0], "qlwon")==0) {

        char *filename = QUEC_LWM2M_CONF;
        if (access(filename, F_OK) == 0)
        {
            printf("if QUEC_LWM2M_CONF exist, remove it");
            remove(QUEC_LWM2M_CONF);
        }
        if (!(0 == lstat( "/persist/lwm2m_conf/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
        {
            mkdir("/persist/lwm2m_conf/", ALLPERMS);
        }

        fp = fopen(QUEC_LWM2M_CONF,"w+");
        if(fp == NULL)
        {
            printf("file QUEC_LWM2M_CONF error: \n");
            perror("Failed to open file:");
            sprintf(response->response, "QAPCMD:Fail! fd < 0,lwm2m.conf set error.");
            response->result = 0;
            return;
        }
        fwrite("DEFAULT_STATUS=ON\n", 20, 1, fp);
        fclose(fp);
        sprintf(buf, "%s", cmd->tokens[0]);
        sprintf(response->response, "QPACD:write success. buf = %s",buf);
        response->result = 1;
    }else if(strcasecmp(cmd->tokens[0], "qlwoff")==0) {
        char *filename2 = QUEC_LWM2M_CONF;
        if (access(filename2, F_OK) == 0)
        {
            printf("if QUEC_LWM2M_CONF exist, remove it");
            remove(QUEC_LWM2M_CONF);
        }
        if (!(0 == lstat( "/persist/lwm2m_conf/", &stat_buf) && (S_ISDIR (stat_buf.st_mode))))
        {
            mkdir("/persist/lwm2m_conf/", ALLPERMS);
        }

        fp = fopen(QUEC_LWM2M_CONF,"w+");
        if(fp == NULL)
        {
            printf("file QUEC_LWM2M_CONF error: \n");
            perror("Failed to open file:");
            sprintf(response->response, "QAPCMD:Fail! fd < 0,lwm2m.conf set error.");
            response->result = 0;
            return;
        }
        fwrite("DEFAULT_STATUS=OFF\n", 20, 1, fp);
        fclose(fp);
        sprintf(buf, "%s", cmd->tokens[0]);
        sprintf(response->response, "QPACD:write success. buf = %s",buf);
        response->result = 1;
    }else {
        printf("%s:%d something is wrong!\n", __func__, __LINE__);
        free(response->response);
        response->response = NULL;
        response->result = 0;
    }

    return;
}
#endif

#define QUEC_FSC_DATA "/dev/block/bootdevice/by-name/fsc"
extern "C" void quec_qcfg_handle (const AtCmd *cmd, AtCmdResponse *response)
{
	int is_enable,len=0;
	int fd;
	char *resp_buf = NULL;
	char fsc_buff[1024]={0};

	response->result = 0;

	if (cmd->ntokens <= 0) {
		LOGE("%s:%d ntokens is 0!", __func__, __LINE__);
		response->response = NULL;
		return ;
	}

	if (strcasecmp(cmd->tokens[0], "DUMPENABLE") != 0) {
		response->response = NULL;
		return ;
	}//at+qcfg=dumpenable,<>

	resp_buf = (char *)malloc(1024);
	if(!resp_buf)
		return;
	response->response = resp_buf;	

	fd = open(QUEC_FSC_DATA, O_RDWR ); // "/dev/block/by-name/fsc"
	if( fd < 0 ){
		LOGE("QUEC_FSC_DATA open failed\n");
		sprintf(resp_buf, "open file error");
		return;
	}

	if( read( fd , fsc_buff, 1024 ) <= 0 ){
		sprintf(resp_buf, "read fsc error");
		close(fd );
		return;
	}

	if(strstr((const char *)(fsc_buff+768),"enabledump")!=NULL){ // 根据sbl中的判断
		is_enable = 1;
	}else{
		is_enable = 0;
	}
	
	if (cmd->ntokens == 1) { 
		response->result = 1;
		sprintf(resp_buf, "DumpEnable: %d", is_enable);
		response->response = resp_buf;
		
	}else if(cmd->ntokens == 2){
			
		response->result = 1;
		
		if(!strcasecmp(cmd->tokens[1], "0")){
			
			if( 1 == is_enable ){
				lseek( fd, 0, SEEK_SET);
				lseek( fd, 768, SEEK_CUR );
				len = write( fd , "dis---dump" , strlen("dis---dump"));
				if(len!=strlen("dis---dump")){
					response->result = 0;
					sprintf(resp_buf, "write flag error: %d", len);
				}
				fsync(fd );
			}
			
		}else if(!strcasecmp(cmd->tokens[1], "1")){
		
			if( 0 == is_enable ){
				lseek( fd, 0, SEEK_SET);
				lseek( fd, 768, SEEK_CUR );
				len = write( fd , "enabledump" , strlen("enabledump"));
				if(len!=strlen("enabledump")){
					response->result = 0;
					sprintf(resp_buf, "write flag error: %d", len);
				}
				fsync(fd );
			}
		}
	}else{
		sprintf(resp_buf, "error : Too many parameters");
	}

	close(fd);
	
}

