/*
 * Copyright (c) 2014, Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "mmi_module.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <cutils/log.h>
#include <cutils/properties.h>
bool is_module=true;
#define KEY_INPUT_NAME "gpio"
#define TP_KEY_INPUT_NAME "goodix-ts"
#define TP_KEY_INPUT_NAME1 "fts_ts"
#define PON_KEY_INPUT_NAME "qpnp_pon"

static int get_input_screen_num(void)// 7:White screen 6:Blcak Screen
{
    int num = -1;
    char input_file[50];
    char input_name[50];
    int err, ret = 0;
    char ev_name[64];
    FILE *fd = NULL;

    for (int i = 0; i < 100; i++) {
        memset(input_file, 0, sizeof(input_file));
		memset(input_name, 0, sizeof(input_name));
        sprintf(input_file, "/sys/class/input/event%d/device/name", i);
        if (access(input_file, F_OK))
		    continue;

        fd = fopen(input_file, "r");
        if (fd == NULL) {
			ALOGE("fopen input file");
			continue;
		}
        if (fgets(input_name, sizeof(input_name) - 1, fd) != NULL) {
			fclose(fd);
			fd = NULL;
            if (strstr(input_name, KEY_INPUT_NAME) != NULL) {
                ret = 1;
                num = i;//white screen
                break;
            }
            if (strstr(input_name, TP_KEY_INPUT_NAME1) != NULL) {
                ret = 1;
                num = i;//black screen
                break;
            }
	    }
    }

  return num;
}

static int get_input_key_num(void)
{
    int num = -1;
    char input_file[50];
    char input_name[50];
    int err, ret = 0;
    char ev_name[64];
    FILE *fd = NULL;

    for (int i = 0; i < 100; i++) {
        memset(input_file, 0, sizeof(input_file));
		memset(input_name, 0, sizeof(input_name));
        sprintf(input_file, "/sys/class/input/event%d/device/name", i);
        if (access(input_file, F_OK))
		    continue;

        fd = fopen(input_file, "r");
        if (fd == NULL) {
			ALOGE("fopen input file");
			continue;
		}
        if (fgets(input_name, sizeof(input_name) - 1, fd) != NULL) {
			fclose(fd);
			fd = NULL;
            if (strstr(input_name, KEY_INPUT_NAME) != NULL) {
                ret = 1;
                num = i;//white screen
                break;
            }
	    }
    }

  return num;
}



char ql_get_key_data(int buf[3])
{
	int fd, ret;
    int num;
	//char buf_one[64], buf_x[64], buf_y[64], buf_z[64];
	struct input_event  event;
	struct pollfd fds[1];
    char input_name[64];

    num=get_input_key_num();
    if(num < 0 )
    {
        printf("can not find key or tp key input event\n");
        return NULL;
    }
    sprintf(input_name,"/dev/input/event%d",num);
    printf("input_name %s\n", input_name);
    fd = open(input_name, O_RDONLY);
    //fd = open("/dev/input/event6", O_RDONLY);
    if (fd < 0)
    {
        printf("Quectel ql_get_key_data failed\n");
        return -1;
    }
    fds[0].fd      = fd;
    fds[0].events  = POLLIN;
    while(1)
    {
        ret = poll(fds, 1, 5000);
        if(ret  == 0)
        {
            printf("Quectel ql_get_key_data time out\n");
        }
        else
        {
            printf("Quectel ql_get_key_data success\n");
            read(fd, &event, sizeof(event));
        }
        buf[0] = event.type;
        buf[1] = event.code;
        buf[2] = event.value;
        return 0;
    }
		
}



/**
* Defined case run in mmi mode,this mode support UI.
*
*/
static int32_t module_init(const mmi_module_t * module, unordered_map < string, string > &params) {
    if(module == NULL) {
        ALOGE("NULL point received");
        return FAILED;
    }
    ALOGI("module init start for module:[%s]", module->name);

    ALOGI("module init finished for module:[%s]", module->name);
    return SUCCESS;
}


static int32_t module_deinit(const mmi_module_t * module) {
    if(module == NULL) {
        ALOGE("NULL point received");
        return FAILED;
    }
    ALOGI("module deinit start for module:[%s]", module->name);

    ALOGI("module deinit finished for module:[%s]", module->name);
    return SUCCESS;
}


static int32_t module_stop(const mmi_module_t * module) {
    if(module == NULL) {
        ALOGE("NULL point received");
        return FAILED;
    }
    ALOGI("module stop start for module:[%s]", module->name);
	is_module=false;
    ALOGI("module stop finished for module:[%s]", module->name);
    return SUCCESS;
}

/**
* Before call Run function, caller should call module_init first to initialize the module.
* the "cmd" passd in MUST be defined in cmd_list ,mmi_agent will validate the cmd before run.
*
*/
static int32_t module_run(const mmi_module_t * module, const char *cmd, unordered_map < string, string > &params) 
{
    int ret = FAILED;
	signal(SIGUSR1, signal_handler);
    int buf[3];
    int value[5]={0};
    is_module=true;
    while(is_module)
    {
    ql_get_key_data(buf);
    ALOGI("THE KEY TYPE=%d",buf[0]);
    ALOGI("THE KEY CODE=%d",buf[1]);
    ALOGI("THE KEY VALUE=%d",buf[2]);

    if(buf[1]==139 && buf[2]==1)
    {
        value[0]=1;
        module->cb_print(NULL, SUBCMD_MMI, "menu", strlen("menu"), PRINT_DATA);
        ALOGI("the menu key press down");
    }
    if(buf[1]==158 && buf[2]==1)
    {
        value[1]=1;
        module->cb_print(NULL, SUBCMD_MMI, "back", strlen("back"), PRINT_DATA);
        ALOGI("the back key press down");
    }
    if(buf[1]==102 && buf[2]==1)
    {
        value[2]=1;
        module->cb_print(NULL, SUBCMD_MMI, "home", strlen("home"), PRINT_DATA);
        ALOGI("the home key press down");
    }
    if(buf[1]==115 && buf[2]==1)
    {
        value[3]=1;
        module->cb_print(NULL, SUBCMD_MMI, "vol_up", strlen("vol_up"), PRINT_DATA);
        ALOGI("the vol_up key press down");
    }
    if(buf[1]==114 && buf[2]==1)
    {
        value[4]=1;
        module->cb_print(NULL, SUBCMD_MMI, "vol_down", strlen("vol_down"), PRINT_DATA);
        ALOGI("the vol_down key press down");
    }
    if(value[0]==1 && value[1]==1 && value[2]==1 &&  value[3]==1 &&  value[4]==1)
    {
        ret=SUCCESS;
        break;
    } 
    }
	return ret;
}

static struct mmi_module_methods_t module_methods = {
    .module_init = module_init,
    .module_deinit = module_deinit,
    .module_run = module_run,
    .module_stop = module_stop,
};
/**
* Every mmi module must have a data structure named MMI_MODULE_INFO_SYM
* and the fields of this data structure must be initialize in strictly sequence as definition,
* please don't change the sequence as g++ not supported in CPP file.
*/
mmi_module_t MMI_MODULE_INFO_SYM = {
    .version_major = 1,
    .version_minor = 0,
    .name = "Key",
    .author = "Qualcomm Technologies, Inc.",
    .methods = &module_methods,
    .module_handle = NULL,
    .supported_cmd_list = NULL,
    .supported_cmd_list_size = 0,
    .cb_print = NULL, /**it is initialized by mmi agent*/
    .run_pid = -1,
};
