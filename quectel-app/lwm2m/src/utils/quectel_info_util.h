#ifndef __QUECTEL_ATC_UTIL__
#define __QUECTEL_ATC_UTIL__

#include<sys/time.h>
#include <ql-mcm-api/ql_in.h>

typedef uint32 atc_client_handle_type;

#define QL_PRV_OFFSET_MAXLEN 	10 //+HH:MM\0 at max
#define QL_PRV_TIMEZONE_MAXLEN 	25


typedef struct
{
	char time_offset[QL_PRV_OFFSET_MAXLEN];
	char time_zone[QL_PRV_TIMEZONE_MAXLEN];
}time_zone_offset_t;

void get_wifi_mac_address(char *address);
void get_bt_mac_address(char *address);
void get_imei2(char *imei);
void get_imei_and_meid(char *serial_number, int type);
long get_memInfo(int type);
void get_powerInfo(char *target, int type);
void get_linux_version(char *version);
void get_gcc_version(char *version);
void get_sw_version(char *version);
time_t get_current_time();
void get_utc_time_offset_zone(char *buff, int type);

#endif /*__QUECTEL_ATC_UTIL__.h*/
