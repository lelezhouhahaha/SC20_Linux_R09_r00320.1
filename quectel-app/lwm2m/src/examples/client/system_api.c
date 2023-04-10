/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH, Germany.
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
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/
/*
 *  event_handler.c
 *
 *  Callback for value changes.
 *
 *  Created on: 21.01.2015
 *  Author: Achim Kraus
 *  Copyright (c) 2014 Bosch Software Innovations GmbH, Germany. All rights reserved.
 */
#include "liblwm2m.h"
#include "lwm2mclient.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/time.h>
#include "platform.h"
#include "internals.h"
#include <errno.h>
#include <ql_ota.h>
#include <curl/curl.h>
#include "fcntl.h"
#include "lwm2m_configure.h"
#include <ql_factory_reset.h>
#include <stdio.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

/// HTTP response status codes:
#define HTTP_SUCCESS             200
#define HTTP_NOT_FORBIDDEN       403
#define HTTP_NOT_FOUND           404
#define HTTP_SERVICE_UNAVAILABLE 503 ///< The server is currently unavailable

#define OMADM_CURL_LOW_SPEED_LIMIT 1000 ///< 1K

#define OMADM_CURL_LOW_SPEED_TIME 20 ///< sec

#define OMADM_CURL_MAXREDIRS 50L

#define MAX_RETRY_COUNT 1 
#define RETRY_TIMEOUT   3
#define RETRY_RATIO     2
#define RMNET_IFACENAME_LENGTH 13

static int current_progress = 0;
#define IP_MAX_LENGTH 16

enum mo_errors {
    MO_ERROR_NONE = 0,
    MO_ERROR_IN_PROGRESS = 101,
    MO_ERROR_SUCCESS = 200,
    MO_ERROR_AUTHENTICATION_ACCEPTED = 212,
    MO_ERROR_OPERATION_CANCELED = 214,
    MO_ERROR_NOT_EXECUTED = 215,
    MO_ERROR_NOT_MODIFIED = 304,
    MO_ERROR_INVALID_CREDENTIALS = 401,
    MO_ERROR_FORBIDDEN = 403,
    MO_ERROR_NOT_FOUND = 404,
    MO_ERROR_NOT_ALLOWED = 405,
    MO_ERROR_OPTIONAL_FEATURE_NOT_SUPPORTED = 406,
    MO_ERROR_MISSING_CREDENTIALS = 407,
    MO_ERROR_INCOMPLETE_COMMAND = 412,
    MO_ERROR_URI_TOO_LONG = 414,
    MO_ERROR_ALREADY_EXISTS = 418,
    MO_ERROR_DEVICE_FULL = 420,
    MO_ERROR_PERMISSION_DENIED = 425,
    MO_ERROR_COMMAND_FAILED = 500,
    MO_ERROR_COMMAND_NOT_IMPLEMENTED = 501,
    MO_ERROR_SESSION_INTERNAL = 506,
    MO_ERROR_ATOMIC_FAILED = 507
};

enum network_state {
    NW_STATE_DEVICE_NONE = 0,
    NW_STATE_DEVICE_DOWN = 1,
    NW_STATE_DEVICE_LINKED = 2,
    NW_STATE_DEVICE_UNPLUGGED = 3,
    NW_STATE_DEVICE_UNKNOW = 4,
    NW_STATE_DEVICE_ERROR = 99
};

#ifdef LWM2M_EMBEDDED_MODE

static void prv_value_change(void* context,
                             const char* uriPath,
                             const char * value,
                             size_t valueLength)
{
    lwm2m_uri_t uri;
    if (lwm2m_stringToUri(uriPath, strlen(uriPath), &uri))
    {
        handle_value_changed(context, &uri, value, valueLength);
    }
}

void init_value_change(lwm2m_context_t * lwm2m)
{
    system_setValueChangedHandler(lwm2m, prv_value_change);
}

#else

void init_value_change(lwm2m_context_t * lwm2m)
{
}

void system_reboot()
{
    quec_save_full_regist_flag(true);//set full register need flag

    char* dev_reboot = "Device will reboot, please wait!";
    quec_lwm2m_client_info_notify(dev_reboot);
    #if 0
    time_t tv_sec = lwm2m_gettime();
    int timegap = tv_sec - g_atc_apn_changed_time;
    if(timegap < 90){
        LOG_ARG("AT CMD modify APN called ,will wait %d seconds\n", timegap);
        sleep(timegap);
    }
    #endif
    system("reboot"); //add by joe reboot system,02 Registration Update sent after Power Cycle

    exit(1);
}

void system_factoryreset()
{
    int ret = 0;

    char* dev_reboot = "Device will factory reset and reboot!";
    quec_lwm2m_client_info_notify(dev_reboot);

    ret = QL_Factory_Reset();

    return;
}

static size_t download_write_callback_func(char *data, size_t size, size_t nmemb, void *userdata)
{
    //LOG("download_write_callback_func enter !\n");

    FILE* out_file = (FILE*) userdata;
    size_t written_size;
    written_size = fwrite(data, size, nmemb, out_file);
    return written_size;
}

int download_on_progess(void *context, unsigned int progress, int err_code)
{
    LOG_ARG("FUMO: Download progress: %d\n", progress);
    LOG_ARG("FUMO: Download err_code: %d\n", err_code);
    if(context==NULL)
        LOG("download_on_progess context is NULL\n");

    if (MO_ERROR_SUCCESS == err_code) {
        if (progress < 100) {
            return MO_ERROR_IN_PROGRESS;
        } else { /* completed */
            return MO_ERROR_SUCCESS;
        }
    } else {
        return MO_ERROR_COMMAND_FAILED;
    }
    return MO_ERROR_SUCCESS;
}

int download_firmware_progress_callback(void* userdata, double dltotal, double dlnow, double ultotal, double ulnow)
{
    int percent = (int)((dlnow/dltotal)*100);
    //LOG_ARG("download_firmware_progress_callback percent=%d\n", percent);
    if (current_progress != percent) {
        current_progress = percent;
        LOG_ARG("download_firmware_progress_callback current_progress=%d\n", current_progress);
        char oubanddownload[64]={0};
        snprintf(oubanddownload, 64, "Outband package download percentage: %d\% !\n", current_progress);
        quec_lwm2m_client_info_notify(oubanddownload);
        if(current_progress == 100){
            char* outband_finish = "Outband package download finished!";
            quec_lwm2m_client_info_notify(outband_finish);
            lwm2m_indicate_firmware_upgrade_status("fotadownloaded");
            if(lwm2m_check_is_vzw_netwrok()){
                //quec_queue_update_register_for_vzw_dm_server();
            }
            prv_set_firmware_object_download_state_changed(GetLwm2mH_ptr(),"2");
            set_fw_update_download_state(2);
        }
    }
    return 0;
}

int quec_mcm_system_call
(
    char    *cmd,
    char    *args[]
)
{
    int childExitStatus = 0;
    pid_t pid = fork();
    if ( pid == 0 )
    {
        LOG("quec_mcm_system_call enter\n");
        execvp(cmd, args);
        LOG("quec_mcm_system_call exit\n");
    }
    waitpid(pid, &childExitStatus, 0);
    return childExitStatus;
}

int configure_ip_route_for_fota_download(char* old_ifname, char* new_ifname){
    LOG_ARG("configure_ip_route_for_fota_download oldifname=%s newifname=%s\n",old_ifname,new_ifname);

    char *args[10] = {0};
    args[0] = "ip";
    args[1] = "ro";
    args[2] = "del";
    args[3] = "default";
    args[4] = "dev";
    args[5] = old_ifname;
    quec_mcm_system_call("ip", args);
    
    sleep(2);
    char *args1[10] = {0};
    args[0] = "ip";
    args[1] = "ro";
    args[2] = "add";
    args[3] = "default";
    args[4] = "dev";
    args[5] = new_ifname;

    quec_mcm_system_call("ip", args1);
    
    sleep(2);
}

int remove_host_ip_route(char* hostIp){
    LOG_ARG("remove_host_ip_route hostIp=%s\n",hostIp);

    char *args[10] = {0};
    args[0] = "ip";
    args[1] = "ro";
    args[2] = "del";
    args[3] = "to";
    args[4] = hostIp;

    quec_mcm_system_call("ip", args);

}

int remove_default_route(){
    LOG("remove_default_route\n");
    char *args[10] = {0};
    args[0] = "ip";
    args[1] = "ro";
    args[2] = "del";
    args[3] = "default";
    quec_mcm_system_call("ip", args);
}

int configure_default_route(char* ifname){
    LOG_ARG("configure_default_route ifname=%s\n",ifname);
   
    char *args[10] = {0};
    args[0] = "ip";
    args[1] = "ro";
    args[2] = "add";
    args[3] = "default";
    args[4] = "dev";
    args[5] = ifname;

    quec_mcm_system_call("ip", args);
}

int setup_dns_ip_route(char *ifname,char *dns){
	char *p = NULL;
    LOG_ARG("setup_dns_ip_route dnses=%s\n",dns);
	p = strtok(dns, " ");
	while (p != NULL) {
        LOG_ARG("add ip route for dns=%s\n",p);
		setIprouteforApnVZWADMIN(ifname,p,NULL);
		p = strtok( NULL, " ");
        LOG_ARG("p dns2=%s\n",p);
	}

}


int quec_fota_download_url_host_resolver(const char *package_url, char* ipaddr)
{
    int ret = 0;
    char* hostname;
    char* buff;
    char URL[256]={0};
    struct addrinfo hints;
    struct addrinfo *res, *res_p;

    if(!package_url)
    {
        LOG("invalid package_url\n");
        return -1;
    }
    LOG_ARG("fota package_url = %s\n", package_url);
    if (0 == strncmp(package_url, "https://", strlen("https://")))
    {
        buff = package_url+strlen("https://");
        memcpy(URL,buff,strlen(buff));
        LOG_ARG("fota package_url URL= %s\n", URL);
    }else{
        return -1;
    }

    hostname=strtok(URL,"/");
    if(hostname == NULL || strlen(hostname)==0){
        return -1;
    }
    LOG_ARG("fota package hostname = %s\n", hostname);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;

    ret = getaddrinfo(hostname, NULL, &hints, &res);
    if(ret != 0)
    {
        LOG_ARG("getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
    {
        char host[16] = {0};
        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        if(ret != 0)
        {
            LOG_ARG("getnameinfo: %s!!!\n", gai_strerror(ret));
        }
        else
        {
            LOG_ARG("got outband fota package url host ip: %s!!!\n", host);
            strcpy(ipaddr,host);
            break;
        }
    }

    freeaddrinfo(res);
    return ret;

}

void quec_configure_route_for_fota_download(char *package_url){

    char ipaddr[16]={0};
    // vzw need  class 2 apn download
    char *ifname = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(ifname, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn2_ifname(ifname);

    LOG_ARG("ifname=%s\n", ifname);

    if(quec_fota_download_url_host_resolver(package_url, ipaddr) == 0){
        LOG_ARG("ipaddr=%s\n", ipaddr);
        //add ip route here
        if(strlen(ifname) != 0 && strstr(ifname,"rmnet") != NULL){
            setIprouteforApnVZWADMIN(ifname,ipaddr,NULL);
        }
    }
    lwm2m_free(ifname);
}

int quec_fota_download_url_host_resolver_att(const char *package_url, char* ipaddr)
{
    int ret = 0;
    char* hostname;
    char* buff;
    char URL[256]={0};
    struct addrinfo hints;
    struct addrinfo *res, *res_p;

    if(!package_url)
    {
        LOG("invalid package_url\n");
        return -1;
    }
    LOG_ARG("fota package_url = %s\n", package_url);
    if (0 == strncmp(package_url, "https://", strlen("https://")))
    {
        buff = package_url+strlen("https://");
        memcpy(URL,buff,strlen(buff));
        LOG_ARG("fota package_url URL= %s\n", URL);
    }else if (0 == strncmp(package_url, "http://", strlen("http://"))){
        buff = package_url+strlen("http://");
        memcpy(URL,buff,strlen(buff));
        LOG_ARG("fota package_url URL= %s\n", URL);
    }else{
        return -1;
    }

    hostname=strtok(URL,":");
    if(hostname == NULL || strlen(hostname)==0){
        return -1;
    }
    LOG_ARG("fota package hostname = %s\n", hostname);
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;

    ret = getaddrinfo(hostname, NULL, &hints, &res);
    if(ret != 0)
    {
        LOG_ARG("getaddrinfo: %s\n", gai_strerror(ret));
        return -1;
    }

    for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
    {
        char host[16] = {0};
        ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        if(ret != 0)
        {
            LOG_ARG("getnameinfo: %s!!!\n", gai_strerror(ret));
        }
        else
        {
            LOG_ARG("got outband fota package url host ip: %s!!!\n", host);
            strcpy(ipaddr,host);
            break;
        }
    }

    freeaddrinfo(res);
    return ret;

}

void quec_configure_route_for_fota_download_att(char *package_url){

    char ipaddr[16]={0};
    // vzw need  class 2 apn download
    
    att_apn_id *att_apn_id = lwm2m_get_apnid();
    char *ifname = att_apn_id->global_ifname;

    LOG_ARG("att ifname=%s\n", ifname);

    if(quec_fota_download_url_host_resolver_att(package_url, ipaddr) == 0){
        LOG_ARG("ipaddr=%s\n", ipaddr);
        //add ip route here
        if(strlen(ifname) != 0 && strstr(ifname,"rmnet") != NULL){
            setIprouteforApnVZWADMIN(ifname,ipaddr,NULL);
        }
    }
}

void* fota_download_work_thread(void* args){
    
    char *fota_download_path_url = (char*)args;
    CURL *curl;
    bool ret = true;
    CURLcode status;
    long http = 0;
    FILE *out_file;
    current_progress = 0;
    LOG("download_work_thread enter\n");
    if (fota_download_path_url == NULL) {
        LOG("fota_download_path invalid\n");
        return (void*)"false";
    }

    LOG_ARG("download_work_thread fota_download_path_url=%s\n", fota_download_path_url);
    
    curl = curl_easy_init();
    if (curl == NULL) {
        LOG("download_work curl init error\n");
        return (void*)"false";
    }

    remove(QUEC_LWM2M_FOTA_UPDATE_FILE);
    out_file = fopen(QUEC_LWM2M_FOTA_UPDATE_FILE, "ab");
    chmod(QUEC_LWM2M_FOTA_UPDATE_FILE,0777);
    if (out_file == NULL) {
        LOG_ARG("download_work_thread error to open file=%s\n", QUEC_LWM2M_FOTA_UPDATE_FILE);
        return (void*)"false";
    }

    curl_easy_setopt(curl, CURLOPT_URL, fota_download_path_url);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);
    
    // Setup SSL configuration
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    
    // Setup Data Callback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, download_write_callback_func);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);
    
    // Setup Progress Callback
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, download_firmware_progress_callback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, download_on_progess);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    
    // Setup minimal download speed threshold & time of low speed endured before aborting
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, OMADM_CURL_LOW_SPEED_LIMIT);
    curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, OMADM_CURL_LOW_SPEED_TIME);

    // Setup redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, OMADM_CURL_MAXREDIRS);

    // To prevent sending not thread safe signals in case when we work outside of main thread
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    
    int sleepTimer = RETRY_TIMEOUT;
    
    for (int retry=0; retry < MAX_RETRY_COUNT; retry++) {
        status = curl_easy_perform(curl);
        if (CURLE_OK == status) {
            // PRIMARY_IP & PRIMARY_PORT
            char *str_primary_ip;
            long port;
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &str_primary_ip);
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_PORT, &port);
            LOG_ARG("DOWNLOAD: curl - primary ip: [%s], port: [%ld]\n", str_primary_ip, port);
            // LOCAL_IP & LOCAL_PORT
            char *str_local_ip;
            curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &str_local_ip);
            curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT , &port);
            LOG_ARG("DOWNLOAD: curl - local ip: [%s], port: [%ld]\n", str_local_ip, port);
            // SSL_VERIFYRESULT
            long crt_res = 0;
            curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &crt_res);
            if(crt_res!=0) {
                LOG_ARG("DOWNLOAD: certificate verify error [%ld]\n",crt_res);
            }
            retry = MAX_RETRY_COUNT;
        } else if ( /* Couldn't resolve host. The given remote host was not resolved */
                status == CURLE_COULDNT_RESOLVE_HOST ||
                /* Failed to connect() to host or proxy */
                status == CURLE_COULDNT_CONNECT      ||
                /* A problem occurred somewhere in the SSL/TLS handshake */
                status == CURLE_SSL_CONNECT_ERROR ) {
            LOG_ARG("DOWNLOAD: CURL can't perform comand, res = %d, retry counter = %d\n", status, retry);
            sleep(sleepTimer);
            sleepTimer = sleepTimer * RETRY_RATIO;
            ret = false;
        } else {
            LOG_ARG("DOWNLOAD: CURL can't perform comand, res = %d\n", status);
            retry = MAX_RETRY_COUNT;
            ret = false;
        }
    }
    if (status != CURLE_OK) {
        LOG_ARG("download_work_thread curl_easy_perform() failed: %s\n", (char*)curl_easy_strerror(status));
        ret = false;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
    LOG_ARG("download_work_thread curl: Server response code %ld\n", http);
    if (status == CURLE_OK &&
            (http == HTTP_NOT_FORBIDDEN ||
             http == HTTP_NOT_FOUND ||
             http == HTTP_SERVICE_UNAVAILABLE)) {
        status = CURLE_COULDNT_CONNECT;
        ret = false;
    }    
     LOG_ARG("[Quectel]download_work_thread curl: Server response http %ld,status = %d\n", http,status);

     fclose(out_file);
     curl_easy_cleanup(curl);
     ///pthread_exit((void*)status);
     if(args != NULL)
     lwm2m_free(args);

    if(ret){
        return (void*)"true";
    }else{
        remove(QUEC_LWM2M_FOTA_UPDATE_FILE);
        prv_set_firmware_object_download_state_changed(GetLwm2mH_ptr(),"0");
        lwm2m_indicate_firmware_upgrade_status("idle");
        return (void*)"false";
    }
}

bool fota_outband_download_with_url(char* url){
    bool ret = true;
    void *retval;
    char *packageURL=NULL;

    packageURL = lwm2m_malloc(256*sizeof(char));
    memset(packageURL, 0, 256*sizeof(char));
    strcpy(packageURL,url);
    LOG("fota_outband_download_with_url enter !\n");
#ifdef ENABLE_LWM2M_NETWORK_MANAGERMENT
    if(lwm2m_check_is_vzw_netwrok()){
        quec_configure_route_for_fota_download(url);
    }else if(lwm2m_check_is_att_netwrok()){
        quec_configure_route_for_fota_download_att(url);
    }
#endif
    pthread_t tidp;
    if (pthread_create(&tidp, NULL, fota_download_work_thread, packageURL) == -1)
    {
        LOG("fota_download_work_thread create error!\n");
        pthread_exit((void*)0);
        return false;
    }
    //prv_set_firmware_object_download_state_changed(GetLwm2mH_ptr(),"1");
    #if 0
    pthread_join(tidp,&retval);//wait for download end
    LOG_ARG("download_work_thread return=%s !\n",(char*)retval);
    if(strcmp((char*)retval,"true") == 0){
        ret = true;
    }else{
        ret = false;
    }
    //ret = download_work_thread(url);
    #endif
    LOG("fota_outband_download_with_url exit !\n");
    return ret;
}

int start_fota_firmware_update(){
    int ret = 0 ;
    LOG("start_fota_firmware_update enter !\n");
    ret = QL_OTA_Set_Package_Path(QUEC_LWM2M_FOTA_UPDATE_FILE);
    if(ret != 0){
        LOG("QL_OTA_Set_Package_Path FAILED!\n");
        return ret;
    }
    
    quec_save_full_regist_flag(true);//set full register need flag
    quec_save_fw_update_execute_flag(true);
    char* fota_update = "Firmware upgrading! Please wait for device reboot";
    quec_lwm2m_client_info_notify(fota_update);
    prv_set_firmware_object_download_state_changed(GetLwm2mH_ptr(),"3");
    //start fota update
    sleep(5);//wait for reqest notify to server
    ret = QL_OTA_Start_Update();
    if(ret != 0){
        LOG("QL_OTA_Start_Update FAILED!\n");
        return ret;
    }
    
    LOG("start_fota_firmware_update exit !\n");
    return ret;
}

int quec_fota_update_work_thread(){
    pthread_t tidp;
    if (pthread_create(&tidp, NULL, start_fota_firmware_update, NULL) == -1)
    {
        LOG("fota update thread create error!\n");
        return 1;
    }
    return 0;
}

int check_network_link_status(char *if_name)
{
    
    int net_fd;
    char statue[20];
	char cmd[100];
    int ret = NW_STATE_DEVICE_UNKNOW;
	memset( cmd, 0, 100 ); 
	sprintf(cmd, "/sys/devices/virtual/net/%s/operstate",if_name);
    LOG_ARG("link status cmd:%s\n",cmd);
    
    net_fd=open(cmd,O_RDONLY);
    if(net_fd<0)
    {
    
        LOG("open err\n");
        return NW_STATE_DEVICE_ERROR;
    }
    
    LOG("open success\n");
    memset(statue,0,sizeof(statue));
    read(net_fd,statue,10);
    LOG_ARG("%s statue is %s",if_name, statue);
    if(NULL!=strstr(statue,"up"))
    {
        LOG_ARG("%s on line\n",if_name);
        ret = NW_STATE_DEVICE_LINKED;
    }
    else if(NULL!=strstr(statue,"down"))
    {
       LOG_ARG("%s off line\n",if_name);
       ret = NW_STATE_DEVICE_DOWN;
    }
    else
    {
        LOG("link status unknown\n");
        ret= NW_STATE_DEVICE_UNKNOW;
    }
    close(net_fd);
    return ret;
}

void quec_check_apn_network_available(){

//get apn2
    char *ifname_class2 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(ifname_class2, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn2_ifname(ifname_class2);

    char *apnname_class2 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(apnname_class2, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn2_name(apnname_class2);

    if(strlen(ifname_class2) != 0 && strstr(ifname_class2,"rmnet") != NULL){
        if(check_network_link_status(ifname_class2) == NW_STATE_DEVICE_DOWN){
            //try reactive apn2 network
            LOG_ARG("try reactive network ifname=%s\n", ifname_class2);
            enableApnNetworkByprofile(true,2,apnname_class2);//vzwadmin profile ID is always 2
        }
    }else if(strlen(ifname_class2) == 0){
        LOG("try start class2 apn network \n");
        enableApnNetworkByprofile(true,2,apnname_class2);//vzwadmin profile ID is always 2
    }

//get apn3
    char *ifname_class3 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    int NETLINK_STATUE = NW_STATE_DEVICE_NONE;
    memset(ifname_class3, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn3_ifname(ifname_class3);

    char *apnname_class3 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(apnname_class3, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn3_name(apnname_class3);
    
    if(strlen(ifname_class3) != 0 && strstr(ifname_class3,"rmnet") != NULL){
        NETLINK_STATUE = check_network_link_status(ifname_class3);
        if(NETLINK_STATUE == NW_STATE_DEVICE_DOWN){
            //try reactive apn3 network
            LOG_ARG("try reactive network ifname=%s\n", ifname_class3);
            enableApnNetworkByprofile(true,3,apnname_class3);//vzwinternet profile ID is always 3
        }else if(NETLINK_STATUE == NW_STATE_DEVICE_UNKNOW || NETLINK_STATUE == NW_STATE_DEVICE_LINKED){
           //try set class3 apn to default network
           remove_default_route();
           configure_default_route(ifname_class3);
        }
    }else if(strlen(ifname_class3) == 0){
        LOG("try start class3 apn network \n");
        enableApnNetworkByprofile(true,3,apnname_class3);//VZWINTERNET profile ID is always 3
    }

    
    lwm2m_free(ifname_class2);
    lwm2m_free(ifname_class3);

    lwm2m_free(apnname_class2);
    lwm2m_free(apnname_class3);    
}
void quec_check_class2_apn_network_available(){
    //get apn2
    char *ifname_class2 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(ifname_class2, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn2_ifname(ifname_class2);

    char *apnname_class2 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
    memset(apnname_class2, 0, sizeof(RMNET_IFACENAME_LENGTH));
    get_vzw_apn2_name(apnname_class2);
    if(strlen(ifname_class2) != 0 && strstr(ifname_class2,"rmnet") != NULL){
        if(check_network_link_status(ifname_class2) == NW_STATE_DEVICE_DOWN){
            //try reactive apn2 network
            LOG_ARG("try reactive network ifname=%s\n", ifname_class2);
            enableApnNetworkByprofile(true,2,apnname_class2);//vzwadmin profile ID is always 2
        }
    }else if(strlen(ifname_class2) == 0){
        LOG("try start class2 apn network \n");
        enableApnNetworkByprofile(true,2,apnname_class2);//vzwadmin profile ID is always 2
    }
    lwm2m_free(ifname_class2);
    lwm2m_free(apnname_class2);
}

void quec_lwm2m_get_ip_by_ifname(char* addrouput)
{
     char ipAddr[IP_MAX_LENGTH];
 
     ipAddr[0] = '\0';
 
     struct ifaddrs * ifAddrStruct = NULL;
     void * tmpAddrPtr = NULL;
     //get apn3
     char *ifname_class3 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
     int NETLINK_STATUE = NW_STATE_DEVICE_NONE;
     memset(ifname_class3, 0, sizeof(RMNET_IFACENAME_LENGTH));
     get_vzw_apn3_ifname(ifname_class3);
     
     strcpy(addrouput,"127.0.0.1");//default value
     if (getifaddrs(&ifAddrStruct) != 0)
     {
         //if wrong, go out!
         LOG("Somting is Wrong!\n");
         return;
     }
 
     struct ifaddrs * iter = ifAddrStruct;
 
     while (iter != NULL) {
         if (iter->ifa_addr != NULL && iter->ifa_addr->sa_family == AF_INET) { //if ip4
             // is a valid IP4 Address
             tmpAddrPtr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
             char addressBuffer[INET_ADDRSTRLEN];
             inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
             if (strlen(ipAddr) + strlen(addressBuffer) < IP_MAX_LENGTH )
             {
                 if (strlen(ipAddr) > 0)
                 {
                      strcat(ipAddr, ";");
                 }
                 if (strcmp(iter->ifa_name, ifname_class3) ==0){
                     strcat(ipAddr, addressBuffer);
                     LOG_ARG("if name =%s, ip=%s\n",ifname_class3,ipAddr);
                     strcpy(addrouput,ipAddr);
                     break;
                 }
             }
             else
             {
                 LOG("Too many ips!\n");
                break;
             }
         }

         iter = iter->ifa_next;
     }
     //releas the struct
     freeifaddrs(ifAddrStruct);
     return ;
}

void quec_lwm2m_get_class2_ip_by_ifname(char* addrouput)
{
     char ipAddr[IP_MAX_LENGTH];
 
     ipAddr[0] = '\0';
 
     struct ifaddrs * ifAddrStruct = NULL;
     void * tmpAddrPtr = NULL;
     //get apn3
     char *ifname_class2 = lwm2m_malloc(RMNET_IFACENAME_LENGTH);
     int NETLINK_STATUE = NW_STATE_DEVICE_NONE;
     memset(ifname_class2, 0, sizeof(RMNET_IFACENAME_LENGTH));
     get_vzw_apn2_ifname(ifname_class2);
     
     strcpy(addrouput,"127.0.0.1");//default value
     if (getifaddrs(&ifAddrStruct) != 0)
     {
         //if wrong, go out!
         LOG("Somting is Wrong!\n");
         return;
     }
 
     struct ifaddrs * iter = ifAddrStruct;
 
     while (iter != NULL) {
         if (iter->ifa_addr != NULL && iter->ifa_addr->sa_family == AF_INET) { //if ip4
             // is a valid IP4 Address
             tmpAddrPtr = &((struct sockaddr_in *)iter->ifa_addr)->sin_addr;
             char addressBuffer[INET_ADDRSTRLEN];
             inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
             if (strlen(ipAddr) + strlen(addressBuffer) < IP_MAX_LENGTH )
             {
                 if (strlen(ipAddr) > 0)
                 {
                      strcat(ipAddr, ";");
                 }
                 if (strcmp(iter->ifa_name, ifname_class2) ==0){
                     strcat(ipAddr, addressBuffer);
                     LOG_ARG("if name =%s, ip=%s\n",ifname_class2,ipAddr);
                     strcpy(addrouput,ipAddr);
                     break;
                 }
             }
             else
             {
                 LOG("Too many ips!\n");
                break;
             }
         }

         iter = iter->ifa_next;
     }
     //releas the struct
     freeifaddrs(ifAddrStruct);
     return ;
}

#endif
