#ifdef LWM2M_SUPPORT_QLRIL_API
#include "fcntl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/statfs.h>
#include<sys/time.h>
#include "internals.h"
#include <ql_power/ql_power.h>
#include <RilPublic.h>
//#include <telephony/RilPublic.h>

#include "quectel_info_util.h"
#include <ql_misc.h>

//#define ATC_REQ_CMD_MAX_LEN     512
//#define ATC_RESP_CMD_MAX_LEN    512
#define POWER_STATUS 			"POWER_SUPPLY_STATUS"
#define POWER_CAPACITY			"POWER_SUPPLY_CAPACITY"
#define QL_PRV_TIME_ZONE         " " //Europe/Berlin


const char g_gcc_version[] = "GCC version: 6.4.1";

time_zone_offset_t zone_offset[] ={
  {"UTC-11:00", "Pacific/Midwayk"},
  {"UTC-10:00", "America/Adak"},
  {"UTC-09:30", "Pacific/Marquesas"},
  {"UTC-09:00", "America/Anchorage"},
  {"UTC-08:00", "America/Los_Angeles"},
  {"UTC-07:00", "America/Mazatlan"},
  {"UTC-06:00", "America/Mexico_city"},
  {"UTC-05:00", "America/New_York"},
  {"UTC-04:00", "America/Montserrat"},
  {"UTC-03:30", "America/St_Johns"},
  {"UTC-03:00", "America/Montevideo"},
  {"UTC-02:00", "America/Noronha"},
  {"UTC-01:00", "America/Scoresbysund"},
  {"UTC+00:00", "Africa/Abidjan"},
  {"UTC+01:00", "Africa/Algiers"},
  {"UTC+02:00", "Africa/cairo"},
  {"UTC+03:00", "Africa/Addis_Ababa"},
  {"UTC+03:30", "Asia/Tehran"},
  {"UTC+04:00", "Asia/Dubai"},
  {"UTC+04:30", "Asia/Kathmandu"},
  {"UTC+05:00", "Asia/Karachi"},
  {"UTC+05:30" ,"Asia/kolkata"},
  {"UTC+05:45", "Asia/Kathmandu"},
  {"UTC+06:00", "Asia/Kashgar"},
  {"UTC+06:30" ,"Asia/Rangoon"},
  {"UTC+07:00", "Asia/Bangkok"},
  {"UTC+08:00", "Asia/Macao"},
  {"UTC+08:30", "Asia/Pyongyang"},
  {"UTC+08:45", "Australia/Eucla"},
  {"UTC+09:00", "Asia/chaita"},
  {"UTC+09:30", "Australia/Adelaide"},
  {"UTC+10:00", "Australia/Hobart"},
  {"UTC+10:30", "Australia/Lord_Howe"},
  {"UTC+11:00", "Antartica/Casey"},
  {"UTC+12:00", "Antarctica/McMurdo"},
  {"UTC+12:45", "Pacific/Chatham"},
  {"UTC+13:00", "Pacific/Apia"},
  {"UTC+14:00", "Pacific/Kiritimati"}
};


/*========================================================================
 *@function:ql_str_chart
 *@brief:chart some chars.
 *@param:raw data
 *@param:new data
 *@param:char flag
 =======================================================================*/
void ql_str_chart(char *src, char *dest, char c)
{
	int m_start_index = 0;
	int m_end_index = 0;
	int flag = 0;
	size_t length = 0;
    int i = 0;
	length = strlen(src);
	//printf("Conway:size : %d , c = %c \n", length, c);
	for( i = 0; i < length; i++)
	{
		//printf("Conway:src[%d]:%c \n", i, *(src + i));
		if(*(src + i) == c)
		{
			if(0 == flag)
			{				
				m_start_index = i;
				flag = flag + 1;
				//printf("Conway:start_i = %d \n", i);
			} 
			else if (1 == flag)
			{
				m_end_index = i;
				//printf("Conway:m_end_index = %d \n", i);
				break;
			}
		}
	}
	//printf("Conway:start_index = %d, end_index = %d \n", m_start_index, m_end_index);
	//printf("Conway:target str = %s \n", (src + m_start_index));
	strncpy(dest, (src + m_start_index + 1), (m_end_index - m_start_index -1));

}


/*========================================================================
 *@function:get_wifi_mac_address
 *@brief:get mac address of wifi by at command.
 *@param:char array to save address.
 =======================================================================*/
void get_wifi_mac_address(char *address)
{
    int ret = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

	LOG("Conway:get_wifi_mac_address begin...\n");
    
    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
    strcpy(atc_cmd_req, "AT+QNVR=4678,0");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
    if (ret <= 0){
    	LOG("get:wifi mac address fail\n");
    }else{

    	//Parse atc_cmd_resp and get the mac address of wifi.
    	ql_str_chart(atc_cmd_resp, address, '"');
    	LOG_ARG("get :wifi mac address = %s \n", address);
    }
}


/*========================================================================
 *@function:get_bt_mac_address
 *@brief:get mac address of bt by at command.
 *@param:char array to save address.
 =======================================================================*/
void get_bt_mac_address(char *address)
{
	int ret = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

    memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
    strcpy(atc_cmd_req,"AT+QNVR=447,0");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

    
    if (ret <= 0){
    	LOG("get bt mac address fail\n");
    }else{
        //Parse atc_cmd_resp and get the mac address of bt.
        ql_str_chart(atc_cmd_resp, address, '"');
        LOG_ARG("get bt_mac_address = %s \n", address);
    }
}

/*========================================================================
 *@function:get_imei2
 *@brief:get imei2  by at command.
 *@param:char array to save imei2.
 =======================================================================*/
void get_imei2(char *imei)
{
	int ret = 0;
	char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
	char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

	memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
	strcpy(atc_cmd_req,"AT+EGMR=0,10");
    ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
	LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

    if (ret <= 0){
            LOG("get imei2 fail\n");
    }else{
    	//parse atc_cmd_resp
    	ql_str_chart(atc_cmd_resp, imei, '"');
    	LOG_ARG("get:imei2 = %s \n", imei);
    }
	
}

/*========================================================================
 *@function: ql_parse_meminfo_str
 *@brief: parse the string which read from the file:"/proc/meminfo"
 *@param: meminfo:char array to save meminfo
 *@param: type:0:memTotal; 1:memfree; 2:memAvailable
 =======================================================================*/
void ql_parse_meminfo_str(char *meminfo, char *buff , int type)
{	
	bool isFirstKIndex = false;
	int first_k_index = 0;
	int number_first_index = 0;
	bool isFirstNumberIndex = false;
	size_t lenght;
    int i = 0;
	if(NULL == meminfo || NULL == buff)
	{
		LOG("meminfo or buff need init.");
		return;
	}
	
	lenght = strlen(meminfo);

	for( i = 0; i < lenght; i++ )
	{	
		//printf("Conway:meminfo[%d] = %c\n", i,  *(meminfo + i));	
		
		if(*(meminfo + i) >= '0' && *(meminfo + i) <= '9' && !isFirstNumberIndex)
		{
			//printf("Conway:number_index = %d\n", i);
			number_first_index = i;
			isFirstNumberIndex = true;
		}
		else if ('k' == *(meminfo + i) && !isFirstKIndex)
		{
			//printf("Conway:k_index = %d\n", i);
			first_k_index = i;
			isFirstKIndex = true;
			break;
		} 
	}
	
	switch (type)
	{
		case 0:   /*memTotal*/
			strncpy(buff, 
					(meminfo + number_first_index),
					(first_k_index - number_first_index - 1)
				);
			LOG_ARG("Conway: memTotal = %s \n", buff);
		
			break;
		
		case 1:  /*memfree*/
			//first_k_index:k index; +1:B_index; +1:space_index
			strncpy(buff, 
					(meminfo + (first_k_index + 1 + 1 + number_first_index + 1)), 
					(first_k_index - number_first_index - 1)
				);
			LOG_ARG("Conway: memfree = %s \n", buff);	
		
			break;

		case 2: /*memAvailable*/
			strncpy(buff, 
					(meminfo + (first_k_index + 2) * 2 + number_first_index + 2), 
					(first_k_index - number_first_index -1)
				);
			LOG_ARG("Conway: memAvailable = %s \n", buff);
		
			break;
			
	}

}

/*========================================================================
 *@function:get_memInfo
 *@brief:get meminfo by open the file:"/proc/meminfo"
 *@param:char array to save meminfo
 *@param:0:memTotal; 1:memfree; 2:memAvailable
 =======================================================================*/
long get_memInfo(int type)
{	
	int fd,size;
	char meminfo[128];
	char *info[32] = {0};
	size_t lenght;
	char *end;
	long mem_info;
	
	fd = open("/proc/meminfo", O_RDONLY);

	if(fd < 0)
	{
		LOG("failed to open file:/proc/meminfo.");	
		return;
	}
	
	size = read(fd, meminfo, sizeof(meminfo));
	LOG_ARG("meminfo:\n%s\n", meminfo);
	ql_parse_meminfo_str(meminfo, info, type);

	mem_info = strtol(info, &end, 10);
	LOG_ARG("Conway:mem_info = %1u \n", mem_info);
	close(fd);
	return mem_info;
			
}


/*========================================================================
 *@function:ql_parse_sub_powerinfo_str
 *@brief:parse sub powerinfo.
 *@param:sub string.
 =======================================================================*/
void ql_parse_sub_powerinfo_str(char *info, char *target)
{
	size_t length = 0;
	char *status;
	int m_equal_index = 0;
    int i = 0;
	if (NULL == info || NULL == target) 
	{
		LOG("info or target is null when parse sub powerinfo.\n");
		return;
	}

	length = strlen(info);
	
	for (i = 0; i < length; i++)
	{	
		LOG_ARG("Conway:info[%d] = %c\n", i, *(info + i));
		if ('=' == *(info + i)) 
		{
			m_equal_index = i;
		}
	}

	strncpy(target, (info + m_equal_index + 1), (length - m_equal_index - 1));
	LOG_ARG("Conway:sub powerinfo: %s; m_equal_index = %d\n", target, m_equal_index);
	
}


/*========================================================================
 *@function:get_powerInfo
 *@brief:get power info.
 *@param:char array to save target str.
 *@param:0:power status; 1:power capacity.
 =======================================================================*/
void get_powerInfo(char *target, int type)
{
    char buf[512];
	int flag = 0;
	size_t lenght = 0;
	int last_space_index = 0;
	char *powerinfo;
	char *status;
	char *capacity;
	//char target[128] = {0};
    int i = 0;
    int ret = QL_Power_Info("battery", buf);
 	if(ret < 0 || NULL == target)
 	{
 		LOG("failed to get powerinfo!\n");
		return;
 	}
	
    //printf("Conway:powerinfo =\n%s\n", buf);
	powerinfo = (char *)malloc(128);
	
	for( i = 0; i < sizeof(buf); i++)
	{	
		//printf("Conway:buf[%d] = %c\n", i, *(buf + i));
		//the target flag string is space, but its ascii is 10 from logs.
		if(10 == (int)*(buf + i))
		{
			//printf("Conway:space_index = %d, last_space_index = %d, flag = %d\n", i, last_space_index, flag);
			memset(powerinfo, 0, 128);
			if(flag > 0)
			{
				strncpy(powerinfo, (buf + last_space_index + 1), (i - last_space_index));	
			}
			else
			{
				strncpy(powerinfo, buf, i);
			}
			//printf("Conway:powerinfo = %s\n",powerinfo);
			last_space_index = i;
			flag++;
		}
		
		status = strstr(powerinfo, POWER_STATUS);
		if (NULL != status && 0 == type)
		{
			//printf("Conway:status = %s\n",status);
			ql_parse_sub_powerinfo_str(status, target);
			break;
		}

		capacity = strstr(powerinfo, POWER_CAPACITY);
		if (NULL != capacity && 1 == type)
		{
			//printf("Conway:capacity = %s\n",capacity);
			ql_parse_sub_powerinfo_str(capacity, target);
			break;
		}
		
	}

	status = NULL;
	capacity = NULL;
	free(powerinfo);
}


/*========================================================================
 *@function:get_linux_version
 *@brief:get linux version.
 *@param:char array to save linux version.
 =======================================================================*/
void get_linux_version(char *version)
{
	int fd,size;
	size_t lenght;

	if(NULL == version)
	{	
		LOG("failed to get linux version.");	
		return;		
	}
	
	fd = open("/proc/version", O_RDONLY);
	if(fd < 0)
	{	
		LOG("failed to open file:/proc/version");	
		return;
	}
	
	size = read(fd, version, sizeof(version));
	LOG_ARG("linux version:\n%s\n", version);
	//parse version string.
	close(fd);

}


/*========================================================================
 *@function:get_gcc_version
 *@brief:get gcc version.
 *@param:char array to save gcc version.
 =======================================================================*/
void get_gcc_version(char *version)
{
	strcpy(version, g_gcc_version);	
}

/*========================================================================
 *@function:ql_parse_bp_version_str
 *@brief:parse bp version.
 *@param:char array to save bp version.
 *@param:char array to save target version.
 =======================================================================*/
void ql_parse_sw_version_str(char *buff, char *version)
{
	size_t len = 0;
	int flag = 0;
	int colon_index = 0;
	char *sw_version;
    int i = 0;
	if (NULL == buff || NULL == version)
	{
		LOG("failed to get bt version!");
		return;
	}
	
	len = strlen(buff);
	for ( i = 0; i < len; i++)
	{
		if(':' == *(buff + i))
		{
			colon_index = i;
			break;
		}
	}
	
	sw_version = (char *) malloc(4 * (len - colon_index)); 
	strncpy(sw_version, (buff + colon_index + 1),(len - colon_index));	
	//printf("Conway: sw_version = %s\n", sw_version);
    int j = 0;
	for ( j = 0; j < strlen(sw_version); j++)
	{
		if ('A' == *(sw_version + j))
		{
			//printf("Conway:flag = %d\n", flag);
			flag++;
			if (3 == flag)
			{
				strncpy(version, (sw_version + j + 1), 2);
				break;
			}
		}
	}
	
	//printf("Conway: version = %s\n", version);
	free(sw_version);
}

/*========================================================================
 *@function:get_bp_version
 *@brief:get bp version.
 *@param:char array to save bp version.
 =======================================================================*/
void get_sw_version(char *version)
{	
	int ret = 0;
    static int flag = 10;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};

	if (flag > 0) 
	{
		flag--;
		memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

	    strcpy((char *)atc_cmd_req, "AT+QGMR?");
        ret = QLRIL_SendAtCmd(&g_qlril_handle, atc_cmd_req, atc_cmd_resp, sizeof(atc_cmd_resp));
	    LOG_ARG("QLRIL_SendAtCmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
	    if (ret < 0) {
			LOG("failed to get bp verion.");
	    }
		
		ql_parse_sw_version_str(atc_cmd_resp, version);
		LOG_ARG("Conway:version = %s \n", version);
	}
}


/*========================================================================
 *@function:get_current_time
 *@brief:get current time
 =======================================================================*/
time_t get_current_time()
{	
	//struct tm now_time;
	struct timeval tv;
	if (0 != gettimeofday(&tv, NULL)) 
	{
		LOG("failed get time of system!\n");
		return 0;
	}

	//localtime_r(&tv.tv_sec, &now_time);
	//printf("tv_sec:%d\n",tv.tv_sec);
	return tv.tv_sec;
}

/*========================================================================
 *@function:get_utc_time_offset_zone
 *@brief:get time zone and time offset
 *@param:buff to save target char
 *@param:0:time_offset; 1:time_zone
 =======================================================================*/

void get_utc_time_offset_zone(char *buff, int type)
{	
	int16_t time_offset = 0;
	time_t t1, t2;
	struct tm *tm_local, *tm_utc;
	char offset[QL_PRV_OFFSET_MAXLEN] = {0};
    int i = 0;
	if (NULL == buff)
	{
		LOG("buff is not init when get time offset of zone!");
		return;
	}
	
	time(&t1);
	t2 = t1;
	//printf("t1=%ul,t2=%ul\n", t1, t2);

	tm_local = localtime(&t1);
	//printf("localtime=%d:%d:%d\n", tm_local->tm_hour, tm_local->tm_min, tm_local->tm_sec);

	t1 = mktime(tm_local);
	tm_utc = gmtime(&t2);
	//printf("utcutctime=%d:%d:%d\n", tm_utc->tm_hour, tm_utc->tm_min, tm_utc->tm_sec);
	t2 = mktime(tm_utc);
	
	//printf("t1=%ul\nt2=%ul\n", t1, t2);

	time_offset = (t1 - t2);
	//printf("Conway:time_offset = %d\n", time_offset / 3600);
		
	if (time < 0) 
	{
		snprintf(offset, 
			sizeof(offset), 
			"UTC-%02d:%02d", 
			(-1 * (time_offset / 3600)), 
			(-1 * (time_offset / 60 % 60))
			);
	}
	else
	{
		snprintf(offset, 
			sizeof(offset), 
			"UTC+%02d:%02d", 
			(time_offset / 3600), 
			(time_offset / 60 % 60)
			);
	}
	
	//printf("time_offset = %s\n", offset);

	if (0 == type) 
	{
		strlcpy(buff, offset, QL_PRV_OFFSET_MAXLEN);
		//printf("Conway:offset = %s\n", buff);
		return;
	}

	
	if (1 == type)
	{	
		strlcpy(buff, QL_PRV_TIME_ZONE, QL_PRV_TIMEZONE_MAXLEN);
		for ( i = 0; i < sizeof(zone_offset)/sizeof(zone_offset[0]); i++) 
		{
			if (!strcmp(offset, zone_offset[i].time_offset))
			{
				strlcpy(buff, zone_offset[i].time_zone, QL_PRV_TIMEZONE_MAXLEN);
				break;
			}
		}
	}
	
	//printf("Conway:time_zone = %s\n", buff);
	
	return;
}



#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/statfs.h>
#include<sys/time.h>
#include "internals.h"
#include <ql_power/ql_power.h>

#include "quectel_info_util.h"

#define ATC_REQ_CMD_MAX_LEN     512
#define ATC_RESP_CMD_MAX_LEN    512
#define POWER_STATUS 			"POWER_SUPPLY_STATUS"
#define POWER_CAPACITY			"POWER_SUPPLY_CAPACITY"
#define QL_PRV_TIME_ZONE         " " //Europe/Berlin


const char g_gcc_version[] = "GCC version: 6.4.1";

time_zone_offset_t zone_offset[] ={
  {"UTC-11:00", "Pacific/Midwayk"},
  {"UTC-10:00", "America/Adak"},
  {"UTC-09:30", "Pacific/Marquesas"},
  {"UTC-09:00", "America/Anchorage"},
  {"UTC-08:00", "America/Los_Angeles"},
  {"UTC-07:00", "America/Mazatlan"},
  {"UTC-06:00", "America/Mexico_city"},
  {"UTC-05:00", "America/New_York"},
  {"UTC-04:00", "America/Montserrat"},
  {"UTC-03:30", "America/St_Johns"},
  {"UTC-03:00", "America/Montevideo"},
  {"UTC-02:00", "America/Noronha"},
  {"UTC-01:00", "America/Scoresbysund"},
  {"UTC+00:00", "Africa/Abidjan"},
  {"UTC+01:00", "Africa/Algiers"},
  {"UTC+02:00", "Africa/cairo"},
  {"UTC+03:00", "Africa/Addis_Ababa"},
  {"UTC+03:30", "Asia/Tehran"},
  {"UTC+04:00", "Asia/Dubai"},
  {"UTC+04:30", "Asia/Kathmandu"},
  {"UTC+05:00", "Asia/Karachi"},
  {"UTC+05:30" ,"Asia/kolkata"},
  {"UTC+05:45", "Asia/Kathmandu"},
  {"UTC+06:00", "Asia/Kashgar"},
  {"UTC+06:30" ,"Asia/Rangoon"},
  {"UTC+07:00", "Asia/Bangkok"},
  {"UTC+08:00", "Asia/Macao"},
  {"UTC+08:30", "Asia/Pyongyang"},
  {"UTC+08:45", "Australia/Eucla"},
  {"UTC+09:00", "Asia/chaita"},
  {"UTC+09:30", "Australia/Adelaide"},
  {"UTC+10:00", "Australia/Hobart"},
  {"UTC+10:30", "Australia/Lord_Howe"},
  {"UTC+11:00", "Antartica/Casey"},
  {"UTC+12:00", "Antarctica/McMurdo"},
  {"UTC+12:45", "Pacific/Chatham"},
  {"UTC+13:00", "Pacific/Apia"},
  {"UTC+14:00", "Pacific/Kiritimati"}
};


/*========================================================================
 *@function:ql_str_chart
 *@brief:chart some chars.
 *@param:raw data
 *@param:new data
 *@param:char flag
 =======================================================================*/
void ql_str_chart(char *src, char *dest, char c)
{
	int m_start_index = 0;
	int m_end_index = 0;
	int flag = 0;
	size_t length = 0;

	length = strlen(src);
	//printf("Conway:size : %d , c = %c \n", length, c);
	for(int i = 0; i < length; i++)
	{
		//printf("Conway:src[%d]:%c \n", i, *(src + i));
		if(*(src + i) == c)
		{
			if(0 == flag)
			{				
				m_start_index = i;
				flag = flag + 1;
				//printf("Conway:start_i = %d \n", i);
			} 
			else if (1 == flag)
			{
				m_end_index = i;
				//printf("Conway:m_end_index = %d \n", i);
				break;
			}
		}
	}
	//printf("Conway:start_index = %d, end_index = %d \n", m_start_index, m_end_index);
	//printf("Conway:target str = %s \n", (src + m_start_index));
	strncpy(dest, (src + m_start_index + 1), (m_end_index - m_start_index -1));

}


/*========================================================================
 *@function:get_wifi_mac_address
 *@brief:get mac address of wifi by at command.
 *@param:char array to save address.
 =======================================================================*/
void get_wifi_mac_address(char *address)
{
    int ret = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    atc_client_handle_type h_atc = 0;

	LOG("Conway:get_wifi_mac_address begin...\n");
    ret = QL_ATC_Client_Init(&h_atc);
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret == E_QL_OK){
        memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
        memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
        strcpy(atc_cmd_req, "AT+QNVR=4678,0");
        ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
        LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

         ret = QL_ATC_Client_Deinit(h_atc);
         LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    }else{
        LOG("init failed.\n");
    }

	//Parse atc_cmd_resp and get the mac address of wifi.
	ql_str_chart(atc_cmd_resp, address, '"');
	LOG_ARG("Conway:wifi mac address = %s \n", address);
	

}


/*========================================================================
 *@function:get_bt_mac_address
 *@brief:get mac address of bt by at command.
 *@param:char array to save address.
 =======================================================================*/
void get_bt_mac_address(char *address)
{
	int ret = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
    atc_client_handle_type h_atc = 0;

    ret = QL_ATC_Client_Init(&h_atc);
    LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
    if(ret == E_QL_OK){
        memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
        memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
        strcpy(atc_cmd_req,"AT+QNVR=447,0");
        ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
        LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

         ret = QL_ATC_Client_Deinit(h_atc);
         LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
    }else{
        LOG("init failed.\n");
    }

	//Parse atc_cmd_resp and get the mac address of bt.
	ql_str_chart(atc_cmd_resp, address, '"');
	LOG_ARG("Conway:bt_mac_address = %s \n", address);

	
}

/*========================================================================
 *@function:get_imei2
 *@brief:get imei2  by at command.
 *@param:char array to save imei2.
 =======================================================================*/
void get_imei2(char *imei)
{
	int ret = 0;
	char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
	char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
	atc_client_handle_type h_atc = 0;

	ret = QL_ATC_Client_Init(&h_atc);
	LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
	if(ret == E_QL_OK){
		memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
		memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));
		strcpy(atc_cmd_req,"AT+EGMR=0,10");
		ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
		LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);

		 ret = QL_ATC_Client_Deinit(h_atc);
		 LOG_ARG("QL_ATC_Client_Deinit ret=%d\n", ret);
	}else{
		LOG("init failed.\n");
	}

	//parse atc_cmd_resp
	ql_str_chart(atc_cmd_resp, imei, '"');
	LOG_ARG("Conway:imei2 = %s \n", imei);

	
}

/*========================================================================
 *@function: ql_parse_meminfo_str
 *@brief: parse the string which read from the file:"/proc/meminfo"
 *@param: meminfo:char array to save meminfo
 *@param: type:0:memTotal; 1:memfree; 2:memAvailable
 =======================================================================*/
void ql_parse_meminfo_str(char *meminfo, char *buff , int type)
{	
	bool isFirstKIndex = false;
	int first_k_index = 0;
	int number_first_index = 0;
	bool isFirstNumberIndex = false;
	size_t lenght;

	if(NULL == meminfo || NULL == buff)
	{
		LOG("meminfo or buff need init.");
		return;
	}
	
	lenght = strlen(meminfo);

	for(int i = 0; i < lenght; i++ )
	{	
		//printf("Conway:meminfo[%d] = %c\n", i,  *(meminfo + i));	
		
		if(*(meminfo + i) >= '0' && *(meminfo + i) <= '9' && !isFirstNumberIndex)
		{
			//printf("Conway:number_index = %d\n", i);
			number_first_index = i;
			isFirstNumberIndex = true;
		}
		else if ('k' == *(meminfo + i) && !isFirstKIndex)
		{
			//printf("Conway:k_index = %d\n", i);
			first_k_index = i;
			isFirstKIndex = true;
			break;
		} 
	}
	
	switch (type)
	{
		case 0:   /*memTotal*/
			strncpy(buff, 
					(meminfo + number_first_index),
					(first_k_index - number_first_index - 1)
				);
			LOG_ARG("Conway: memTotal = %s \n", buff);
		
			break;
		
		case 1:  /*memfree*/
			//first_k_index:k index; +1:B_index; +1:space_index
			strncpy(buff, 
					(meminfo + (first_k_index + 1 + 1 + number_first_index + 1)), 
					(first_k_index - number_first_index - 1)
				);
			LOG_ARG("Conway: memfree = %s \n", buff);	
		
			break;

		case 2: /*memAvailable*/
			strncpy(buff, 
					(meminfo + (first_k_index + 2) * 2 + number_first_index + 2), 
					(first_k_index - number_first_index -1)
				);
			LOG_ARG("Conway: memAvailable = %s \n", buff);
		
			break;
			
	}

}

/*========================================================================
 *@function:get_memInfo
 *@brief:get meminfo by open the file:"/proc/meminfo"
 *@param:char array to save meminfo
 *@param:0:memTotal; 1:memfree; 2:memAvailable
 =======================================================================*/
long get_memInfo(int type)
{	
	int fd,size;
	char meminfo[128];
	char *info[32] = {0};
	size_t lenght;
	char *end;
	long mem_info;
	
	fd = open("/proc/meminfo", O_RDONLY);

	if(fd < 0)
	{
		LOG("failed to open file:/proc/meminfo.");	
		return;
	}
	
	size = read(fd, meminfo, sizeof(meminfo));
	LOG_ARG("meminfo:\n%s\n", meminfo);
	ql_parse_meminfo_str(meminfo, info, type);

	mem_info = strtol(info, &end, 10);
	LOG_ARG("Conway:mem_info = %1u \n", mem_info);
	close(fd);
	return mem_info;
			
}


/*========================================================================
 *@function:ql_parse_sub_powerinfo_str
 *@brief:parse sub powerinfo.
 *@param:sub string.
 =======================================================================*/
void ql_parse_sub_powerinfo_str(char *info, char *target)
{
	size_t length = 0;
	char *status;
	int m_equal_index = 0;
	
	if (NULL == info || NULL == target) 
	{
		LOG("info or target is null when parse sub powerinfo.\n");
		return;
	}

	length = strlen(info);
	
	for (int i = 0; i < length; i++)
	{	
		LOG_ARG("Conway:info[%d] = %c\n", i, *(info + i));
		if ('=' == *(info + i)) 
		{
			m_equal_index = i;
		}
	}

	strncpy(target, (info + m_equal_index + 1), (length - m_equal_index - 1));
	LOG_ARG("Conway:sub powerinfo: %s; m_equal_index = %d\n", target, m_equal_index);
	
}


/*========================================================================
 *@function:get_powerInfo
 *@brief:get power info.
 *@param:char array to save target str.
 *@param:0:power status; 1:power capacity.
 =======================================================================*/
void get_powerInfo(char *target, int type)
{
    char buf[512];
	int flag = 0;
	size_t lenght = 0;
	int last_space_index = 0;
	char *powerinfo;
	char *status;
	char *capacity;
	//char target[128] = {0};
	
    int ret = QL_Power_Info("battery", buf);
 	if(ret < 0 || NULL == target)
 	{
 		LOG("failed to get powerinfo!\n");
		return;
 	}
	
    //printf("Conway:powerinfo =\n%s\n", buf);
	powerinfo = (char *)malloc(128);
	
	for(int i = 0; i < sizeof(buf); i++)
	{	
		//printf("Conway:buf[%d] = %c\n", i, *(buf + i));
		//the target flag string is space, but its ascii is 10 from logs.
		if(10 == (int)*(buf + i))
		{
			//printf("Conway:space_index = %d, last_space_index = %d, flag = %d\n", i, last_space_index, flag);
			memset(powerinfo, 0, 128);
			if(flag > 0)
			{
				strncpy(powerinfo, (buf + last_space_index + 1), (i - last_space_index));	
			}
			else
			{
				strncpy(powerinfo, buf, i);
			}
			//printf("Conway:powerinfo = %s\n",powerinfo);
			last_space_index = i;
			flag++;
		}
		
		status = strstr(powerinfo, POWER_STATUS);
		if (NULL != status && 0 == type)
		{
			//printf("Conway:status = %s\n",status);
			ql_parse_sub_powerinfo_str(status, target);
			break;
		}

		capacity = strstr(powerinfo, POWER_CAPACITY);
		if (NULL != capacity && 1 == type)
		{
			//printf("Conway:capacity = %s\n",capacity);
			ql_parse_sub_powerinfo_str(capacity, target);
			break;
		}
		
	}

	status = NULL;
	capacity = NULL;
	free(powerinfo);
}


/*========================================================================
 *@function:get_linux_version
 *@brief:get linux version.
 *@param:char array to save linux version.
 =======================================================================*/
void get_linux_version(char *version)
{
	int fd,size;
	size_t lenght;

	if(NULL == version)
	{	
		LOG("failed to get linux version.");	
		return;		
	}
	
	fd = open("/proc/version", O_RDONLY);
	if(fd < 0)
	{	
		LOG("failed to open file:/proc/version");	
		return;
	}
	
	size = read(fd, version, sizeof(version));
	LOG_ARG("linux version:\n%s\n", version);
	//parse version string.
	close(fd);

}


/*========================================================================
 *@function:get_gcc_version
 *@brief:get gcc version.
 *@param:char array to save gcc version.
 =======================================================================*/
void get_gcc_version(char *version)
{
	strcpy(version, g_gcc_version);	
}


/*========================================================================
 *@function:get_imei_and_meid
 *@brief:get gcc version.
 *@param:serial number which you will get
 *@param:0:imei; 1:meid;
 =======================================================================*/
void get_imei_and_meid(char *serial_number, int type)
{
	int ret = 0;
	dm_client_handle_type h_dm = 0;

	ret = QL_MCM_DM_Client_Init(&h_dm);
	LOG_ARG("QL_MCM_DM_Client_Init ret = %d with h_dm= %d\n", ret, h_dm);
	ql_dm_device_serial_numbers_t dm_device_serial_numbers;
	memset(&dm_device_serial_numbers, 0, sizeof(dm_device_serial_numbers));
	ret = QL_MCM_DM_GetSerialNumbers(h_dm,&dm_device_serial_numbers);
	
	if (ret < 0) 
	{
		LOG_ARG("QL_MCM_DM_GetSerialNumbers fail ret = %d\n", ret);
		return;
	} 
	
	LOG_ARG("Conway:SerialNumbers imei:%s; meid:%s\n",
				 dm_device_serial_numbers.imei,dm_device_serial_numbers.meid);


	switch (type)
	{
		case 0:
			strlcpy(serial_number, dm_device_serial_numbers.imei, sizeof(dm_device_serial_numbers.imei));
			break;
		
		case 1:
			strlcpy(serial_number, dm_device_serial_numbers.imei, sizeof(dm_device_serial_numbers.imei));
			break;
	}

}

/*========================================================================
 *@function:ql_parse_bp_version_str
 *@brief:parse bp version.
 *@param:char array to save bp version.
 *@param:char array to save target version.
 =======================================================================*/
void ql_parse_sw_version_str(char *buff, char *version)
{
	size_t len = 0;
	int flag = 0;
	int colon_index = 0;
	char *sw_version;
	
	if (NULL == buff || NULL == version)
	{
		LOG("failed to get bt version!");
		return;
	}
	
	len = strlen(buff);
	for (int i = 0; i < len; i++)
	{
		if(':' == *(buff + i))
		{
			colon_index = i;
			break;
		}
	}
	
	sw_version = (char *) malloc(4 * (len - colon_index)); 
	strncpy(sw_version, (buff + colon_index + 1),(len - colon_index));	
	//printf("Conway: sw_version = %s\n", sw_version);

	for (int j = 0; j < strlen(sw_version); j++)
	{
		if ('A' == *(sw_version + j))
		{
			//printf("Conway:flag = %d\n", flag);
			flag++;
			if (3 == flag)
			{
				strncpy(version, (sw_version + j + 1), 2);
				break;
			}
		}
	}
	
	//printf("Conway: version = %s\n", version);
	free(sw_version);
}

/*========================================================================
 *@function:get_bp_version
 *@brief:get bp version.
 *@param:char array to save bp version.
 =======================================================================*/
void get_sw_version(char *version)
{	
	int ret = 0;
    static int flag = 10;
    static atc_client_handle_type h_atc = 0;
    char atc_cmd_req[ATC_REQ_CMD_MAX_LEN] = {0};
    char atc_cmd_resp[ATC_RESP_CMD_MAX_LEN] = {0};
	

    if (flag == 10) {
        ret = QL_ATC_Client_Init(&h_atc);
        LOG_ARG("QL_ATC_Client_Init ret=%d with h_atc=0x%x\n", ret, h_atc);
        if(ret != E_QL_OK){
            return;
        }
    }

	if (flag > 0) 
	{
		flag--;
		memset(atc_cmd_req,  0, sizeof(atc_cmd_req));
	    memset(atc_cmd_resp, 0, sizeof(atc_cmd_resp));

	    strcpy((char *)atc_cmd_req, "AT+QGMR?");
	    ret = QL_ATC_Send_Cmd(h_atc, atc_cmd_req, atc_cmd_resp, ATC_RESP_CMD_MAX_LEN);
	    LOG_ARG("QL_ATC_Send_Cmd \"%s\" ret=%d with resp=\n%s\n", atc_cmd_req, ret, atc_cmd_resp);
	    if (ret < 0) {
			LOG("failed to get bp verion.");
	    }
		
		ql_parse_sw_version_str(atc_cmd_resp, version);
		LOG_ARG("Conway:version = %s \n", version);
	}
}


/*========================================================================
 *@function:get_current_time
 *@brief:get current time
 =======================================================================*/
time_t get_current_time()
{	
	//struct tm now_time;
	struct timeval tv;
	if (0 != gettimeofday(&tv, NULL)) 
	{
		LOG("failed get time of system!\n");
		return 0;
	}

	//localtime_r(&tv.tv_sec, &now_time);
	//printf("tv_sec:%d\n",tv.tv_sec);
	return tv.tv_sec;
}

/*========================================================================
 *@function:get_utc_time_offset_zone
 *@brief:get time zone and time offset
 *@param:buff to save target char
 *@param:0:time_offset; 1:time_zone
 =======================================================================*/

void get_utc_time_offset_zone(char *buff, int type)
{	
	int16_t time_offset = 0;
	time_t t1, t2;
	struct tm *tm_local, *tm_utc;
	char offset[QL_PRV_OFFSET_MAXLEN] = {0};

	if (NULL == buff)
	{
		LOG("buff is not init when get time offset of zone!");
		return;
	}
	
	time(&t1);
	t2 = t1;
	//printf("t1=%ul,t2=%ul\n", t1, t2);

	tm_local = localtime(&t1);
	//printf("localtime=%d:%d:%d\n", tm_local->tm_hour, tm_local->tm_min, tm_local->tm_sec);

	t1 = mktime(tm_local);
	tm_utc = gmtime(&t2);
	//printf("utcutctime=%d:%d:%d\n", tm_utc->tm_hour, tm_utc->tm_min, tm_utc->tm_sec);
	t2 = mktime(tm_utc);
	
	//printf("t1=%ul\nt2=%ul\n", t1, t2);

	time_offset = (t1 - t2);
	//printf("Conway:time_offset = %d\n", time_offset / 3600);
		
	if (time < 0) 
	{
		snprintf(offset, 
			sizeof(offset), 
			"UTC-%02d:%02d", 
			(-1 * (time_offset / 3600)), 
			(-1 * (time_offset / 60 % 60))
			);
	}
	else
	{
		snprintf(offset, 
			sizeof(offset), 
			"UTC+%02d:%02d", 
			(time_offset / 3600), 
			(time_offset / 60 % 60)
			);
	}
	
	//printf("time_offset = %s\n", offset);

	if (0 == type) 
	{
		strlcpy(buff, offset, QL_PRV_OFFSET_MAXLEN);
		//printf("Conway:offset = %s\n", buff);
		return;
	}

	
	if (1 == type)
	{	
		strlcpy(buff, QL_PRV_TIME_ZONE, QL_PRV_TIMEZONE_MAXLEN);
		for (int i = 0; i < sizeof(zone_offset)/sizeof(zone_offset[0]); i++) 
		{
			if (!strcmp(offset, zone_offset[i].time_offset))
			{
				strlcpy(buff, zone_offset[i].time_zone, QL_PRV_TIMEZONE_MAXLEN);
				break;
			}
		}
	}
	
	//printf("Conway:time_zone = %s\n", buff);
	
	return;
}

#endif
