/*******************************************************************************
 *
 * Copyright (c) 2013, 2014, 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v2.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v20.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    David Navarro, Intel Corporation - initial API and implementation
 *******************************************************************************/
#define LOG_TAG "Mango-platform.c"
#include "lwm2m_android_log.h"
#include "call_stack.h"
#include <cutils/properties.h>
#include <unistd.h>
#include <liblwm2m.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include "platform.h"
#include "internals.h"
#include <errno.h>
#include "fcntl.h"
#define MAX_LW_INFO_NAME_LEN    64 
FILE * g_fp = NULL;
int g_fd = -1;
#define LWM2M_LOG_CFG_NAME                     "/etc/lwm2m_log.conf"
#define filepath                     "/data/lwm2m/lwm2m_log.log"
int isLogenable = -1;
#ifndef LWM2M_MEMORY_TRACE
#define data_path  "/data/lwm2m/lw_data.dat"
int write_data(void* data, int length){
    FILE *fp;
    int i;
	int fd;
    LOG("write_data enter");

	fd = open(data_path, O_RDWR|O_CREAT, 0777);
	if(fd < 0){
		LOG("fd < 0!");
	}
	close(fd);
    if (access(data_path, F_OK) == 0)
    {
        //LOG("if /sdcard/lw_data.dat exist, remove it");
		remove(data_path);
    }

    fp = fopen(data_path,"wb");
    if(fp == NULL)
    {
        LOG("file lw_data.dat error: \n");
        perror("fopen: ");
        return -1;
    }
    fwrite(data, length, 1, fp);
    fclose(fp);
    //LOG("write_data exit");
    return 0;
}


int read_data(void* data, int length){
    FILE *fp;
    int i;
    LOG("read_data enter");

    fp = fopen(data_path,"rb");
    if(fp == NULL)
    {
        LOG("file lw_data.dat error\n");
        return -1;
    }
    fread(data, length, 1, fp);
    fclose(fp);

    //LOG("read_data exit");
    return 0;
}

int delete_data(){
    LOG("delete_data enter");

    if (access(data_path, F_OK) == 0)
    {
        //LOG("read_data done, remove /sdcard/lw_data.dat exist");
		remove(data_path);
    }
    //LOG("delete_data exit");
    return 0;
}

int dump_connect_info(){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    read_data(connect_info, sizeof(dm_server_connect_info));
    LOG_ARG("endpointname = %s,\nserverAddr = %s,\nserverPort = %s,\npsk = %s,\npskLen = %d\npskId = %s,\nlifetime = %d,\nlastRegistrationTime = %d,\nconnect_info->saved_session.id = %s", 
		           connect_info->endpointname, connect_info->serverAddr, connect_info->serverPort, 
		           connect_info->psk, connect_info->pskLen, connect_info->pskId, connect_info->lifetime, 
		           connect_info->lastRegistrationTime, connect_info->saved_session.id);

	return 0;
}

int parse_connect_info(bool* isColdStart, char *server, char* serverPort, char* name, char* pskId, char* psk, uint16_t* pskLen){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    int ret = read_data(connect_info, sizeof(dm_server_connect_info));
    if(ret == -1){
        return;
    }
    
    LOG_ARG("endpointname = %s,\nserverAddr = %s,\nserverPort = %s,\npsk = %s,\npskLen = %d\npskId = %s,\nlifetime = %d,\nlastRegistrationTime = %d,\nconnect_info->saved_session.id = %s", connect_info->endpointname, connect_info->serverAddr, connect_info->serverPort, 
		           connect_info->psk, connect_info->pskLen, connect_info->pskId, connect_info->lifetime, connect_info->lastRegistrationTime, connect_info->saved_session.id);

	time_t tv_sec = lwm2m_gettime();
	if (tv_sec >= 0)
	{   
	    LOG_ARG("tv_sec = %d, connect_info->lastRegistrationTime = %d, connect_info->lifetime = %d", tv_sec, connect_info->lastRegistrationTime, connect_info->lifetime);
	    if (connect_info->lastRegistrationTime + connect_info->lifetime + 30 > tv_sec){
	        LOG("still within lifetime, try to recovery last registration");
			*isColdStart = false;
	    } else {
	        LOG("data is out-of-date, delete it");
			delete_data();
		}
	}

    if (!(*isColdStart) && !lwm2m_check_is_vzw_netwrok()){
        memset(server, 0, MAX_SIZE);
        memset(serverPort, 0, MAX_SIZE);
        memset(name, 0, MAX_SIZE);
        memset(pskId, 0, MAX_SIZE);
        memset(psk, 0, MAX_SIZE);
        strcpy(server, connect_info->serverAddr);
        strcpy(serverPort, connect_info->serverPort);
        strcpy(name, connect_info->endpointname);
        memcpy(psk, connect_info->psk, connect_info->pskLen);
        *pskLen = connect_info->pskLen;
        strcpy(pskId, connect_info->pskId);
	}
	return 0;
}

bool within_life_time(){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    int ret = read_data(connect_info, sizeof(dm_server_connect_info));
    if(ret == -1){
        return false;
    }
	time_t tv_sec = lwm2m_gettime();
	if (connect_info->lastRegistrationTime + connect_info->lifetime + 30 > tv_sec){
		LOG("still within lifetime");
		return true;
	} else {
		LOG("without lifetime");
		return false;
	}

}

char *conf_get_by_key(const char *fname, const char *key)
{
    int len = 0;
    int key_len = 0;
    char buf[256];
    FILE *fp = NULL;
    char *p = NULL;

    if (fname == NULL || key == NULL) {
        return NULL;
    }
    key_len = strlen(key);
    if (key_len < 1) {
        return NULL;
    }
    fp = fopen(fname, "rb");
    if (fp == NULL) {
        return NULL;
    }
    while (fgets(buf, sizeof(buf), fp)) {
        if (buf[0] == '#') {
            continue;
        }
        len = strlen(buf);
        if (len < 2) {
            continue;
        }
        /* Remove the EOL */
        /* EOL == CR */
        if (buf[len - 1] == '\r') {
            buf[len - 1] = 0;
            len -= 1;
        } else if (buf[len - 1] == '\n') {
            /* EOL = CR+LF */
            if (buf[len - 2] == '\r') {
                buf[len - 2] = 0;
                len -= 2;
            } else {
                /* EOL = LF */
                buf[len - 1] = 0;
                len -= 1;
            }
        }
        /* Skip the empty line */
        if (len == 0) {
            continue;
        }
        if (strncasecmp(buf, key, key_len) == 0 && buf[key_len] == '=') {
            p = malloc(len - key_len);
            if (p == NULL) {
                fclose(fp);
                return NULL;
            }
            strcpy(p, buf + key_len + 1);
        }
    }
    fclose(fp);
    return p;
}

bool is_enable_form_conf(void)
{
	char *status;
	if(isLogenable == -1){
		status = conf_get_by_key(LWM2M_LOG_CFG_NAME, "DEFAULT_STATUS");
		if(status == NULL)
		isLogenable = 0;
		else if(!strncmp(status, "ON", 2))
		isLogenable = 1;
		else
		isLogenable = 0;
	}
		return (isLogenable == 1);
}


//add by joe start
int update_session_info_for_vzw(int instanceId,lwm2m_context_t *lwm2mH, mbedtls_ssl_session* saved_session){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    LOG_ARG("update_session_info_for_vzw enter instanceId=%d",instanceId);
    int read_conn_info = 0;
    read_conn_info = read_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    
    if(read_conn_info != 0) return;
    
    if(saved_session != NULL){
        memcpy(&(connect_info->saved_session), saved_session, sizeof(mbedtls_ssl_session));
    }
	lwm2mH->connect_info = connect_info;
    write_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    LOG_ARG("update_session_info_for_vzw connect_info->location=%s",connect_info->location);
    LOG_ARG("update_session_info_for_vzw exit instanceId=%d",instanceId);
    //free(connect_info);
    //connect_info = NULL;
}

int update_time_location_for_vzw(int instanceId,time_t regist_time, char *location,time_t lifetime){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    LOG_ARG("update_time_location_for_vzw enter instanceId=%d",instanceId);
    int read_conn_info = 0;
    read_conn_info = read_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    if(read_conn_info != 0) return;
    //if(regist_time != NULL)//fix bug by joe 20201023
    {
        connect_info->lastRegistrationTime = regist_time;
        LOG_ARG("update_time_location_for_vzw connect_info.lastRegistrationTime=%d",connect_info->lastRegistrationTime);
    }
    
    if(location != NULL){
        //connect_info.location = location;
        strcpy(connect_info->location,location);
        LOG_ARG("update_time_location_for_vzw connect_info.location=%s",connect_info->location);
    }

    if(lifetime != NULL){
        connect_info->lifetime = lifetime;
        LOG_ARG("update_time_location_for_vzw connect_info.lifetime=%d",connect_info->lifetime);
    }

    write_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    LOG_ARG("update_session_info_for_vzw connect_info->location=%s",connect_info->location);
    LOG_ARG("update_time_location_for_vzw exit instanceId=%d",instanceId);
    //free(connect_info);
    //connect_info = NULL;
}

int write_data_for_vzw(int instanceId,void* data, int length){
    FILE *fp;
    int i;
    LOG_ARG("write_data enter instanceId=%d",instanceId);
    char conn_info_fname[MAX_LW_INFO_NAME_LEN];
    memset(conn_info_fname, 0, MAX_LW_INFO_NAME_LEN);
    snprintf((char *)conn_info_fname, MAX_LW_INFO_NAME_LEN, "/data/lwm2m/lw_data_instance_%d.dat", instanceId);

    if (access(conn_info_fname, F_OK) == 0)
    {
        LOG_ARG("if %s exist, remove it\n",conn_info_fname);
		remove(conn_info_fname);
    }

    fp = fopen(conn_info_fname,"wb");
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",conn_info_fname);
        perror("fopen: ");
        return -1;
    }
    fwrite(data, length, 1, fp);
    fclose(fp);
    LOG("write_data exit");
    return 0;
}


int read_data_for_vzw(int instanceId,void* data, int length){
    FILE *fp;
    int i;
    LOG_ARG("read_data enter instanceId=%d",instanceId);
    char conn_info_fname[MAX_LW_INFO_NAME_LEN];
    memset(conn_info_fname, 0, MAX_LW_INFO_NAME_LEN);
    snprintf((char *)conn_info_fname, MAX_LW_INFO_NAME_LEN, "/data/lwm2m/lw_data_instance_%d.dat", instanceId);

    fp = fopen(conn_info_fname,"rb");
    if(fp == NULL)
    {
        LOG_ARG("open file %s error: \n",conn_info_fname);
        return -1;
    }
    fread(data, length, 1, fp);
    fclose(fp);

    LOG("read_data exit");
    return 0;
}

int delete_data_for_vzw(int instanceId){
    LOG_ARG("delete_data enter instanceId=%d",instanceId);
    char conn_info_fname[MAX_LW_INFO_NAME_LEN];
    memset(conn_info_fname, 0, MAX_LW_INFO_NAME_LEN);
    snprintf((char *)conn_info_fname, MAX_LW_INFO_NAME_LEN, "/data/lwm2m/lw_data_instance_%d.dat", instanceId);

    if (access(conn_info_fname, F_OK) == 0)
    {
        LOG_ARG("read_data done, remove %s",conn_info_fname);
		remove(conn_info_fname);
    }
    LOG("delete_data exit");
    return 0;
}

int dump_connect_info_for_vzw(int instanceId){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    read_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    LOG_ARG("instanceId=%d, endpointname = %s,\nserverAddr = %s,\nserverPort = %s,\npsk = %s,\npskLen = %d\npskId = %s,\nlifetime = %d,\nlastRegistrationTime = %d,\nlocation = %s,\nconnect_info->saved_session.id = %s", 
                   instanceId,
		           connect_info->endpointname, connect_info->serverAddr, connect_info->serverPort, 
		           connect_info->psk, connect_info->pskLen, connect_info->pskId, connect_info->lifetime, 
		           connect_info->lastRegistrationTime, connect_info->location, connect_info->saved_session.id);
    //free(connect_info);
    //connect_info = NULL;
	return 0;
}

int parse_connect_info_for_vzw(int instanceId,bool* isColdStart, char *server, char* serverPort, char* name, char* pskId, char* psk, uint16_t* pskLen){
	dm_server_connect_info * connect_info = (dm_server_connect_info *)malloc(sizeof(dm_server_connect_info));
    memset(connect_info, 0, sizeof(dm_server_connect_info));
    int ret = read_data_for_vzw(instanceId,connect_info, sizeof(dm_server_connect_info));
    if(ret == -1){
        return;
    }

    LOG_ARG("instanceId=%d,endpointname = %s,\nserverAddr = %s,\nserverPort = %s,\npsk = %s,\npskLen = %d\npskId = %s,\nlifetime = %d,\nlastRegistrationTime = %d,\nconnect_info->saved_session.id = %s", instanceId,connect_info->endpointname, connect_info->serverAddr, connect_info->serverPort, 
		           connect_info->psk, connect_info->pskLen, connect_info->pskId, connect_info->lifetime, connect_info->lastRegistrationTime, connect_info->saved_session.id);

	time_t tv_sec = lwm2m_gettime();
	if (tv_sec >= 0)
	{   
	    LOG_ARG("tv_sec = %d, connect_info->lastRegistrationTime = %d, connect_info->lifetime = %d", tv_sec, connect_info->lastRegistrationTime, connect_info->lifetime);
	    if (connect_info->lastRegistrationTime + connect_info->lifetime > tv_sec){
	        LOG("still within lifetime, try to recovery last registration");
			*isColdStart = false;
	    } else {
	        LOG("data is out-of-date, delete it");
			delete_data_for_vzw(instanceId);
		}
	}
    //free(connect_info);
    //connect_info = NULL;
	return 0;
}

bool parse_is_bootstrapd_for_vzw(){
    FILE *fp;
    int i;
    bool ret = false;
    LOG("parse_is_bootstrapd_for_vzw enter");
    char conn_info_fname[MAX_LW_INFO_NAME_LEN];
    memset(conn_info_fname, 0, MAX_LW_INFO_NAME_LEN);
    for(i = 1 ; i < 4; i++){ //joe use as a workround
        snprintf((char *)conn_info_fname, MAX_LW_INFO_NAME_LEN, "/data/lwm2m/lw_data_instance_%d.dat", i);
        fp = fopen(conn_info_fname,"rb");
        if(fp == NULL)
        {
            LOG_ARG("try open file %s error: \n",conn_info_fname);
            continue;
        }
        LOG_ARG("open file %s success: So device has been bootstrapd before! \n",conn_info_fname);
        fclose(fp);
        ret = true;
    }

    return ret;
    LOG_ARG("parse_is_bootstrapd_for_vzw exit ret=%d",ret);

}

//add by joe end
void * lwm2m_malloc(size_t s)
{
    return malloc(s);
}

void lwm2m_free(void * p)
{
    free(p);
}

char * lwm2m_strdup(const char * str)
{
    return strdup(str);
}

#endif

int lwm2m_strncmp(const char * s1,
                     const char * s2,
                     size_t n)
{
    return strncmp(s1, s2, n);
}

time_t lwm2m_gettime(void)
{
    struct timeval tv;

    if (0 != gettimeofday(&tv, NULL))
    {
        return -1;
    }

    return tv.tv_sec;
}

static char * settime(char * time_s){
    time_t timer=time(NULL);
    strftime(time_s, 20, "%Y-%m-%d %H:%M:%S",localtime(&timer));
    return time_s;
}
void lwm2m_printf(const char * format, ...)
{
    FILE * fp = NULL;
	int fd;
	char tmp[256];
    struct stat buf;
	char *file_name = NULL;
	char *file_name_prev = NULL;
	int loop;

	fd = open(filepath, O_RDWR|O_CREAT, 0777);
    if(fd > 0){
		if(!fstat(fd, &buf)){
			if(buf.st_size >= LWM2M_MAX_LOG_SIZE * 1024){
				file_name = (char *)malloc(strlen(filepath)+4);
				file_name_prev = (char *)malloc(strlen(filepath)+4);
				if(file_name && file_name_prev) {
					for(loop = LWM2M_MAX_LOG_COUNT; loop > 0; loop--) {
						sprintf(file_name, "%s.%d", filepath, loop);
						if(LWM2M_MAX_LOG_COUNT == loop
						   && !access(file_name, 0)) {
							unlink(file_name);
							continue;
						}
						if(!access(file_name, 0)) {
							sprintf(file_name_prev, "%s.%d", filepath, loop+1);
							rename(file_name, file_name_prev);
						}
					}
					sprintf(file_name, "%s.1", filepath);
					rename(filepath, file_name);
				}
				if(file_name) {
					free(file_name);
				}
				if(file_name_prev) {
					free(file_name_prev);
				}

			}
		}
		close(fd);
	}
	fp = fopen(filepath,"a+");
    va_list ap;

    va_start(ap, format);

    vfprintf(stderr, format, ap);
    if(fp){
		memset(tmp, 0,sizeof(tmp));
		settime(tmp);
		fprintf(fp,"%s",tmp);
        vfprintf(fp, format, ap);
		fclose(fp);
    }else{
        printf("platform::lwm2m_printf fd = null.\n");
	}

    va_end(ap);
}

void lwm2m_release_log_filefd(){

    if(g_fd > 0){
        fsync(g_fd);
    }

    if(g_fp){
        fclose(g_fp);
        g_fp = NULL;
    }

}
