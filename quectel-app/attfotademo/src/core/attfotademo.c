#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <curl/curl.h>
#include "fcntl.h"
#include <ql-mcm-api/ql_in.h>


#define QUEC_LWM2M_FOTA_UPDATE_FILE          "/data/update_ext4.zip"
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
bool download_progress_finish = false;

int server_sockfd = -1;
int client_sockfd = -1;
ql_module_about_info_s about;
char *packageURL_to_beta="http://116.247.104.27:6029/Lemon/beta-SC20ALSAR09A03.zip";
char *packageURL_to_normal="http://116.247.104.27:6029/Lemon/SC20ALSAR09A03-beta.zip";
int QL_LWM2M_GetAbout(ql_module_about_info_s * about){
    int    ret     = E_QL_OK;
    ret = QL_Module_About_Get(about);
    printf("QL_Module_About_Get ret = %d\n", ret);
}

int start_fota_firmware_update(){
    int ret = 0 ;
    printf("start_fota_firmware_update enter !\n");
    ret = QL_OTA_Set_Package_Path(QUEC_LWM2M_FOTA_UPDATE_FILE);
    if(ret != 0){
        printf("QL_OTA_Set_Package_Path FAILED!\n");
        return ret;
    }
    
    char* fota_update = "Firmware upgrading! Please wait for device reboot";
    quec_lwm2m_client_info_notify(fota_update);
    //start fota update
    ret = QL_OTA_Start_Update();
    if(ret != 0){
        printf("QL_OTA_Start_Update FAILED!\n");
        return ret;
    }
    
    printf("start_fota_firmware_update exit !\n");
    return ret;
}

int quec_fota_update_work_thread(){
    pthread_t tidp;
    if (pthread_create(&tidp, NULL, start_fota_firmware_update, NULL) == -1)
    {
        printf("fota update thread create error!\n");
        return 1;
    }
    pthread_join(tidp,NULL);//wait for network start
    return 0;
}

static size_t download_write_callback_func(char *data, size_t size, size_t nmemb, void *userdata)
{
    //LOG("download_write_callback_func enter !\n");

    FILE* out_file = (FILE*) userdata;
    size_t written_size;
    written_size = fwrite(data, size, nmemb, out_file);
	fflush(out_file);
    return written_size;
}

int download_on_progess(void *context, unsigned int progress, int err_code)
{
    printf("FUMO: Download progress: %d\n", progress);
    printf("FUMO: Download err_code: %d\n", err_code);
    if(context==NULL)
        printf("download_on_progess context is NULL\n");

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
        printf("download_firmware_progress_callback current_progress=%d\n", current_progress);
        char oubanddownload[64]={0};
        snprintf(oubanddownload, 64, "DFOTA package download percentage: %d\% !\n", current_progress);
        quec_lwm2m_client_info_notify(oubanddownload);
        if(current_progress == 100){
            char* outband_finish = "DFOTA package download finished!";
            quec_lwm2m_client_info_notify(outband_finish);
			download_progress_finish = true;
        }
    }
    return 0;
}

int fota_download_work_thread(void* args){
    
    char *fota_download_path_url = (char*)args;
    CURL *curl;
    int ret = 1;
    CURLcode status;
    long http = 0;
    FILE *out_file;
    current_progress = 0;
    printf("download_work_thread enter\n");
    if (fota_download_path_url == NULL) {
        printf("fota_download_path invalid\n");
        return (void*)"false";
    }

    printf("download_work_thread fota_download_path_url=%s\n", fota_download_path_url);
    
    curl = curl_easy_init();
    if (curl == NULL) {
        printf("download_work curl init error\n");
        return (void*)"false";
    }

   // remove(QUEC_LWM2M_FOTA_UPDATE_FILE);
    out_file = fopen(QUEC_LWM2M_FOTA_UPDATE_FILE, "ab");
    chmod(QUEC_LWM2M_FOTA_UPDATE_FILE,0777);
    if (out_file == NULL) {
        printf("download_work_thread error to open file=%s\n", QUEC_LWM2M_FOTA_UPDATE_FILE);
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
            printf("DOWNLOAD: curl - primary ip: [%s], port: [%ld]\n", str_primary_ip, port);
            // LOCAL_IP & LOCAL_PORT
            char *str_local_ip;
            curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &str_local_ip);
            curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT , &port);
            printf("DOWNLOAD: curl - local ip: [%s], port: [%ld]\n", str_local_ip, port);
            // SSL_VERIFYRESULT
            long crt_res = 0;
            curl_easy_getinfo(curl, CURLINFO_SSL_VERIFYRESULT, &crt_res);
            if(crt_res!=0) {
                printf("DOWNLOAD: certificate verify error [%ld]\n",crt_res);
            }
            retry = MAX_RETRY_COUNT;
        } else if ( /* Couldn't resolve host. The given remote host was not resolved */
                status == CURLE_COULDNT_RESOLVE_HOST ||
                /* Failed to connect() to host or proxy */
                status == CURLE_COULDNT_CONNECT      ||
                /* A problem occurred somewhere in the SSL/TLS handshake */
                status == CURLE_SSL_CONNECT_ERROR ) {
            printf("DOWNLOAD: CURL can't perform comand, res = %d, retry counter = %d\n", status, retry);
            sleep(sleepTimer);
            sleepTimer = sleepTimer * RETRY_RATIO;
            ret = 0;
        } else {
            printf("DOWNLOAD: CURL can't perform comand, res = %d\n", status);
            retry = MAX_RETRY_COUNT;
            ret = 0;
        }
    }
    if (status != CURLE_OK) {
        printf("download_work_thread curl_easy_perform() failed: %s\n", (char*)curl_easy_strerror(status));
        ret = 0;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http);
    printf("download_work_thread curl: Server response code %ld\n", http);
    if (status == CURLE_OK &&
            (http == HTTP_NOT_FORBIDDEN ||
             http == HTTP_NOT_FOUND ||
             http == HTTP_SERVICE_UNAVAILABLE)) {
        status = CURLE_COULDNT_CONNECT;
        ret = 0;
    }    
     printf("[Quectel]download_work_thread curl: Server response http %ld,status = %d\n", http,status);

     fclose(out_file);
     curl_easy_cleanup(curl);
     ///pthread_exit((void*)status);
     if(args != NULL)
     free(args);

    return ret;
}

int fota_download_with_url(){
    int ret = 1;
    char *packageURL;
    void *retval;
    char *url;
    if(strlen(about.firmware_version) != 0 && (strstr(about.firmware_version,"A04") != NULL || strstr(about.firmware_version,"beta") != NULL || strstr(about.firmware_version,"Beta") != NULL || strstr(about.firmware_version,"BETA") != NULL)){
        url = packageURL_to_normal;//normal version upgrade to beta
    }else{
        url = packageURL_to_beta;//beta version upgrade to  normal
    }
    packageURL = malloc(256*sizeof(char));
    memset(packageURL, 0, 256*sizeof(char));
    strcpy(packageURL,url);
    printf("fota_outband_download_with_url enter !\n");

    pthread_t tidp;
    if (pthread_create(&tidp, NULL, fota_download_work_thread, packageURL) == -1)
    {
        printf("fota_download_work_thread create error!\n");
        pthread_exit((void*)0);
        return 0;
    }
    pthread_join(tidp,NULL);
    printf("fota_outband_download_with_url exit !\n");
	if (access(QUEC_LWM2M_FOTA_UPDATE_FILE, F_OK) != 0){
		printf("update_ex.zip not exist !\n");
		printf("START firmware download again!\n");
		download_progress_finish = false;
		fota_download_with_url();
	}
	if(download_progress_finish == true){
		printf("call quec_fota_update_work_thread !\n");
		quec_fota_update_work_thread();
	}
    return ret;
}

void quec_lwm2m_client_info_notify(char *info){
    int btye;
    char char_send[256];
    if(client_sockfd == -1) {
        printf("quec_lwm2m_client_info_notify client_sockfd = -1\n");
        return;
    }
    if(info == NULL || strlen(info) == 0){
        printf("quec_lwm2m_client_info_notify info empty\n");
        return;
    } 

    strcpy(char_send,info);
    if(btye = send(client_sockfd,char_send,strlen(char_send),0) == -1)
    {
        printf("send socket failed\n");
    }else{
        printf("send notify info :%s size=%d\n",char_send,strlen(char_send));
    }

}

int quec_start_local_socket_server(){
    //int server_sockfd,client_sockfd;
    int server_len,client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int i,btye;
    char* char_send;
    char rebuf[100];
    int revlen;
    int opt = 1;
    int ret;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;//inet_addr("127.0.0.1");//htonl(INADDR_ANY);//
    server_address.sin_port = htons(56700);//5680;
    server_len = sizeof(server_address);
    
    int childExitStatus = 0;
    server_sockfd = socket(AF_INET,SOCK_STREAM,0);

    if(0 != setsockopt(server_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))){
        printf("socket setsockopt fail \n");
        return 0;
    }

    /*
    struct timeval timeout = {90,0};//wait for 90s
    if (0 != setsockopt(server_sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval)) )
    {
        printf("set accept timeout failed \n");
        return 0;
    }   */
    if(-1 == bind(server_sockfd,(struct sockaddr *)&server_address,server_len)){
        printf("bind socket fail \n");
        return 0;
    }
    if(-1 == listen(server_sockfd,5)){
        printf("listen socket fail \n");
        return 0;
    }
    
    //int client_pid = 0;
    while(1){
        
        client_len = sizeof(client_address);
        //if(client_sockfd = -1)
        {
            client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,(socklen_t *)&client_len);
            quec_lwm2m_client_info_notify("Current firmware version: ");
            quec_lwm2m_client_info_notify(about.firmware_version);
        }
        if(client_sockfd == -1){
            printf("accept socket failed\n");
        }else{
            printf("accept socket success, START firmware download and upgrade!\n");
			fota_download_with_url();
        }

        #if 0
        client_pid = fork();
        if (client_pid == 0){
            //close(server_sockfd);
            int serial = 0;
        }
        #endif
        while (1)
        {

            bzero(rebuf, sizeof(rebuf));

            revlen = read(client_sockfd, rebuf, sizeof(rebuf));
            //printf("attfotademo read %s...\n",rebuf);
            if ((memcmp("fotadownload", rebuf, strlen("fotadownload"))) == 0){
                printf("read command: start download fota package...\n");
                //fota_download_with_url();
                continue;
            }else if ((memcmp("fotaupgrade", rebuf, strlen("fotaupgrade"))) == 0){
                printf("read command: start firmware upgrading...\n");
                //quec_fota_update_work_thread();
                break;
            }
            break;
        }


    }
    return 0;

}


int main(int argc, char *argv[])
{

    remove(QUEC_LWM2M_FOTA_UPDATE_FILE);

    QL_LWM2M_GetAbout(&about);

    //for socket connection
    quec_start_local_socket_server();

}

