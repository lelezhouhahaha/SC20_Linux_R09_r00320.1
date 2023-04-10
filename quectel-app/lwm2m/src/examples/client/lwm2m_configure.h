#ifndef LWM2M_CONFIGURE_H
#define LWM2M_CONFIGURE_H

#define LWM2M_MAX_SERVER_CNT  4

#define Bootstrap_Server_ID  	0
#define DM_Server_ID         	1
#define Diagnostics_Server_ID   2
#define Repository_Server_ID    3

# define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)

#define MAX_VAL_NUM_ITEM 0xFFFFFFFF

#define QUEC_EFS_LWM2M_SERVER_DEF_INI			"/data/lwm2m/lwm2m_server.ini"
#define QUEC_EFS_LWM2M_SERVER_VZW_INI			"/data/lwm2m/lwm2m_server_vzw.ini"
#define QUEC_EFS_LWM2M_SERVER_ATT_INI			"/data/lwm2m/lwm2m_server_att.ini"

#define QUEC_LWM2M_BS_SERVER_VZW_INI			"/persist/lwm2m/lwm2m_bs_server.ini" //add by joe for vzw
#define QUEC_LWM2M_DIAG_SERVER_VZW_INI			"/persist/lwm2m/lwm2m_diag_server.ini" //add by joe for vzw

#include "liblwm2m.h"
#include <stdbool.h>

#define MAX_ID_LENGTH (128)
#define M2M_COM_ATTZ			"m2m.com.attz"
#define ATT_M2M_GLOBAL          "attm2mglobal"

typedef unsigned int dsat_num_item_type;

typedef enum
{
	LWM2M_CLIENT_NET_DEF = 0,
	LWM2M_CLIENT_NET_VZW,
	LWM2M_CLIENT_NET_ATT,
}lwm2m_client_net_type;

typedef struct
{
	bool     valid;
	int      server_id;
	char     server_url[256];
	bool     bootstrap;
	int      security_mode;
    char     name[64];
	char     psk_id[64];
	char     psk_key[128];
	int      life_time;
	int      period_min;
	int      period_max;
	int      disable_timeout;
	bool     storing;
	char     binding[4];
}lwm2m_server_info_t;

// typedef struct{
	// int     mask;//LSB0:apn_name;LSB1:estab_time; LSB2: estab_result;LSB3:estab_result;LSB4:estab_rej_cause;LSB5:release_time
	// char    apn_name[256];
	// int     estab_time;
	// int     estab_result;
	// int     estab_rej_cause;
	// int     release_time;
// }lwm2m_apn_conn_info_t;

typedef struct{
	char       unique_id[128];
	char       manufacture[128];
	char       model[128];
	char       sw_version[128];
	char       fw_version[128];
	char       hw_version[128];
	int64_t    upgrade_time;
}lwm2m_hostdevice_info_t;

typedef struct {
    char *global_id;
    char *attz_id;
    char global_ifname[13];
    int callid;
}att_apn_id;

//add by joe start
#define CLASS2_APN "VZWADMIN"
#define CLASS3_APN "VZWINTERNET"
#define CLASS6_APN "VZWCLASS6"
#define CLASS7_APN "VZWIOTTS"

typedef struct {
    int profileid;
    int callid;
    char iface_name[13];
    char ip_addr[128];
    char gw_addr[128];
    char apn_name[128];
}vzw_apn_info;

typedef struct {
    vzw_apn_info vzwadmin_apn2;
    vzw_apn_info vzwinternet_apn3;
    vzw_apn_info vzwclass6_apn6;
    
}vzw_apn_id;
//add by joe end
void lwm2m_load_server_configuration();

lwm2m_server_info_t *lwm2m_get_server_info(int server_idx);

bool lwm2m_update_server_info(int server_idx,lwm2m_server_info_t  *server_info_ptr);

void lwm2m_delete_server_info(int server_idx);

void lwm2m_reload_server_config(lwm2m_client_net_type net_type);

void lwm2m_save_apn_conn_info(int id, lwm2m_apn_conn_info_t *apn_conn_info);

bool lwm2m_load_apn_conn_info(int id, lwm2m_apn_conn_info_t *apn_conn_info);

void lwm2m_save_hostdevice_info(int id, lwm2m_hostdevice_info_t *dev_info);

bool lwm2m_load_hostdevice_info(int id, lwm2m_hostdevice_info_t *dev_info);

void  lwm2m_update_fota_pkg_dload_info(int dload_pkg_size, uint8_t delete);

bool lwm2m_get_fota_pkg_dload_info(int *dload_size);

void lwm2m_init_apn_info();

int lwm2m_getApnInfo (att_apn_id *apnid);

att_apn_id * lwm2m_get_apnid();

int lwm2m_setApnEnable (int enable, char *id);

int lwm2m_enableApn (bool enable, char *id, char *apn);

int send_AT_Command (char * command, char * resp);

char *ho_trim(char *s);

int lwm2m_enableDefaultApn ();
//boolean FUNC_GetDeviceImei(char *pNum);
void init_vzw_apn_info();
vzw_apn_id * get_vzw_apnid();
int enableApnNetworkByprofile(bool enable, int id, char *apn);
#endif
