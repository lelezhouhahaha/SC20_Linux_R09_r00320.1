#include <stdlib.h>
#include <string.h>
#include <ql-mcm-api/ql_in.h>

#define MOBAP_SWITCH_CFG_NAME                    "/etc/mobap_switch.conf"

static int    h_sim   = 0;

enum MOBAP_STATUS
{
	DISABLE,
	ENABLE,
	KEEP
};
static char* errorcode[] = 
{
    "MCM_SUCCESS",                              //  0, /**<  Success. */
    "MCM_SUCCESS_CONDITIONAL_SUCCESS",          //  1, /**<  Conditional success. */
    "MCM_ERROR_MCM_SERVICES_NOT_AVAILABLE",     //  2, /**<  "MCM services not available. */
    "MCM_ERROR_GENERIC",                        //  3, /**<  Generic error. */
    "MCM_ERROR_BADPARM",                        //  4, /**<  Bad parameter. */
    "MCM_ERROR_MEMORY",                         //  5, /**<  Memory error. */
    "MCM_ERROR_INVALID_STATE",                  //  6, /**<  Invalid state. */
    "MCM_ERROR_MALFORMED_MSG",                  //  7, /**<  Malformed message. */
    "MCM_ERROR_NO_MEMORY",                      //  8, /**<  No memory. */
    "MCM_ERROR_INTERNAL",                       //  9, /**<  Internal error. */
    "MCM_ERROR_ABORTED",                        //  10, /**<  Action was aborted. */
    "MCM_ERROR_CLIENT_IDS_EXHAUSTED",           //  11, /**<  Client IDs have been exhausted. */
    "MCM_ERROR_UNABORTABLE_TRANSACTION",        //  12, /**<  Unabortable transaction. */
    "MCM_ERROR_INVALID_CLIENT_ID",              //  13, /**<  Invalid client ID. */
    "MCM_ERROR_NO_THRESHOLDS",                  //  14, /**<  No thresholds. */
    "MCM_ERROR_INVALID_HANDLE",                 //  15, /**<  Invalid handle. */
    "MCM_ERROR_INVALID_PROFILE",                //  16, /**<  Invalid profile. */
    "MCM_ERROR_INVALID_PINID",                  //  17, /**<  Invalid PIN ID. */
    "MCM_ERROR_INCORRECT_PIN",                  //  18, /**<  Incorrect PIN. */
    "MCM_ERROR_NO_NETWORK_FOUND",               //  19, /**<  No network found. */
    "MCM_ERROR_CALL_FAILED",                    //  20, /**<  Call failed. */
    "MCM_ERROR_OUT_OF_CALL",                    //  21, /**<  Out of call. */
    "MCM_ERROR_NOT_PROVISIONED",                //  22, /**<  Not provisioned. */
    "MCM_ERROR_MISSING_ARG",                    //  23, /**<  Missing argument. */
    "MCM_ERROR_ARG_TOO_LONG",                   //  24, /**<  Argument is too long. */
    "MCM_ERROR_INVALID_TX_ID",                  //  25, /**<  Invalid Tx ID. */
    "MCM_ERROR_DEVICE_IN_USE",                  //  26, /**<  Device is in use. */
    "MCM_ERROR_OP_NETWORK_UNSUPPORTED",         //  27, /**<  OP network is not supported. */
    "MCM_ERROR_OP_DEVICE_UNSUPPORTED",          //  28, /**<  OP device is not supported. */
    "MCM_ERROR_NO_EFFECT",                      //  29, /**<  No effect. */
    "MCM_ERROR_NO_FREE_PROFILE",                //  30, /**<  No free profile. */
    "MCM_ERROR_INVALID_PDP_TYPE",               //  31, /**<  Invalid PDP type. */
    "MCM_ERROR_INVALID_TECH_PREF",              //  32, /**<  Invalid technical preference. */
    "MCM_ERROR_INVALID_PROFILE_TYPE",           //  33, /**<  Invalid profile type. */
    "MCM_ERROR_INVALID_SERVICE_TYPE",           //  34, /**<  Invalid service type. */
    "MCM_ERROR_INVALID_REGISTER_ACTION",        //  35, /**<  Invalid register action. */
    "MCM_ERROR_INVALID_PS_ATTACH_ACTION",       //  36, /**<  Invalid PS attach action. */
    "MCM_ERROR_AUTHENTICATION_FAILED",          //  37, /**<  Authentication failed. */
    "MCM_ERROR_PIN_BLOCKED",                    //  38, /**<  PIN is blocked. */
    "MCM_ERROR_PIN_PERM_BLOCKED",               //  39, /**<  PIN is permanently blocked. */
    "MCM_ERROR_SIM_NOT_INITIALIZED",            //  40, /**<  SIM is not initialized. */
    "MCM_ERROR_MAX_QOS_REQUESTS_IN_USE",        //  41, /**<  Maximum QoS requests are in use. */
    "MCM_ERROR_INCORRECT_FLOW_FILTER",          //  42, /**<  Incorrect flow filter. */
    "MCM_ERROR_NETWORK_QOS_UNAWARE",            //  43, /**<  Network QoS is unaware. */
    "MCM_ERROR_INVALID_ID",                     //  44, /**<  Invalid ID. */
    "MCM_ERROR_INVALID_QOS_ID",                 //  45, /**<  Invalid QoS ID. */
    "MCM_ERROR_REQUESTED_NUM_UNSUPPORTED",      //  46, /**<  Requested number is not supported. */
    "MCM_ERROR_INTERFACE_NOT_FOUND",            //  47, /**<  Interface was not found. */
    "MCM_ERROR_FLOW_SUSPENDED",                 //  48, /**<  Flow is suspended. */
    "MCM_ERROR_INVALID_DATA_FORMAT",            //  49, /**<  Invalid data format. */
    "MCM_ERROR_GENERAL",                        //  50, /**<  General error. */
    "MCM_ERROR_UNKNOWN",                        //  51, /**<  Unknown error. */
    "MCM_ERROR_INVALID_ARG",                    //  52, /**<  Invalid argument. */
    "MCM_ERROR_INVALID_INDEX",                  //  53, /**<  Invalid index. */
    "MCM_ERROR_NO_ENTRY",                       //  54, /**<  No entry. */
    "MCM_ERROR_DEVICE_STORAGE_FULL",            //  55, /**<  Device storage is full. */
    "MCM_ERROR_DEVICE_NOT_READY",               //  56, /**<  Device is not ready. */
    "MCM_ERROR_NETWORK_NOT_READY",              //  57, /**<  Network is not ready. */
    "MCM_ERROR_CAUSE_CODE",                     //  58, /**<  Cause code error. */
    "MCM_ERROR_MESSAGE_NOT_SENT",               //  59, /**<  Message was not sent. */
    "MCM_ERROR_MESSAGE_DELIVERY_FAILURE",       //  60, /**<  Message delivery failure. */
    "MCM_ERROR_INVALID_MESSAGE_ID",             //  61, /**<  Invalid message ID. */
    "MCM_ERROR_ENCODING",                       //  62, /**<  Encoding error. */
    "MCM_ERROR_AUTHENTICATION_LOCK",            //  63, /**<  Authentication lock error. */
    "MCM_ERROR_INVALID_TRANSITION",             //  64, /**<  Invalid transition. */
    "MCM_ERROR_NOT_A_MCAST_IFACE",              //  65, /**<  Not an MCast interface. */
    "MCM_ERROR_MAX_MCAST_REQUESTS_IN_USE",      //  66, /**<  Maximum MCast requests are in use. */
    "MCM_ERROR_INVALID_MCAST_HANDLE",           //  67, /**<  Invalid MCast handle. */
    "MCM_ERROR_INVALID_IP_FAMILY_PREF",         //  68, /**<  Invalid IP family preference. */
    "MCM_ERROR_SESSION_INACTIVE",               //  69, /**<  Session is inactive. */
    "MCM_ERROR_SESSION_INVALID",                //  70, /**<  Session is invalid. */
    "MCM_ERROR_SESSION_OWNERSHIP",              //  71, /**<  Session ownership error. */
    "MCM_ERROR_INSUFFICIENT_RESOURCES",         //  72, /**<  Insufficient resources. */
    "MCM_ERROR_DISABLED",                       //  73, /**<  Disabled. */
    "MCM_ERROR_INVALID_OPERATION",              //  74, /**<  Invalid operation. */
    "MCM_ERROR_INVALID_CMD",                    //  75, /**<  Invalid command. */
    "MCM_ERROR_TPDU_TYPE",                      //  76, /**<  Transfer Protocol data unit type error. */
    "MCM_ERROR_SMSC_ADDR",                      //  77, /**<  Short message service center address error. */
    "MCM_ERROR_INFO_UNAVAILABLE",               //  78, /**<  Information is not available. */
    "MCM_ERROR_SEGMENT_TOO_LONG",               //  79, /**<  Segment is too long. */
    "MCM_ERROR_SEGMENT_ORDER",                  //  80, /**<  Segment order error. */
    "MCM_ERROR_BUNDLING_NOT_SUPPORTED",         //  81, /**<  Bundling is not supported. */
    "MCM_ERROR_OP_PARTIAL_FAILURE",             //  82, /**<  OP partial failure. */
    "MCM_ERROR_POLICY_MISMATCH",                //  83, /**<  Policy mismatch. */
    "MCM_ERROR_SIM_FILE_NOT_FOUND",             //  84, /**<  SIM file was not found. */
    "MCM_ERROR_EXTENDED_INTERNAL",              //  85, /**<  Extended internal error. */
    "MCM_ERROR_ACCESS_DENIED",                  //  86, /**<  Access is denied. */
    "MCM_ERROR_HARDWARE_RESTRICTED",            //  87, /**<  Hardware is restricted. */
    "MCM_ERROR_ACK_NOT_SENT",                   //  88, /**<  Acknowledgement was not sent. */
    "MCM_ERROR_INJECT_TIMEOUT",                 //  89, /**<  Inject timeout error. */
    "MCM_ERROR_INCOMPATIBLE_STATE",             //  90, /**<  Incompatible state. */
    "MCM_ERROR_FDN_RESTRICT",                   //  91, /**<  Fixed dialing number restrict error. */
    "MCM_ERROR_SUPS_FAILURE_CAUSE",             //  92, /**<  SUPS failure cause. */
    "MCM_ERROR_NO_RADIO",                       //  93, /**<  No radio. */
    "MCM_ERROR_NOT_SUPPORTED",                  //  94, /**<  Not supported. */
    "MCM_ERROR_NO_SUBSCRIPTION",                //  95, /**<  No subscription. */
    "MCM_ERROR_CARD_CALL_CONTROL_FAILED",       //  96, /**<  Card call control failed. */
    "MCM_ERROR_NETWORK_ABORTED",                //  97, /**<  Network was aborted. */
    "MCM_ERROR_MSG_BLOCKED",                    //  98, /**<  Message was blocked. */
    "MCM_ERROR_INVALID_SESSION_TYPE",           //  99, /**<  Invalid session type. */
    "MCM_ERROR_INVALID_PB_TYPE",                //  100, /**<  Invalid phonebook type. */
    "MCM_ERROR_NO_SIM",                         //  101, /**<  No SIM was found. */
    "MCM_ERROR_PB_NOT_READY",                   //  102, /**<  Phonebook not ready. */
    "MCM_ERROR_PIN_RESTRICTION",                //  103, /**<  PIN restriction. */
    "MCM_ERROR_PIN2_RESTRICTION",               //  104, /**<  PIN2 restriction. */
    "MCM_ERROR_PUK_RESTRICTION",                //  105, /**<  PIN unlocking key restriction. */
    "MCM_ERROR_PUK2_RESTRICTION",               //  106, /**<  PIN unlocking key2 restriction. */
    "MCM_ERROR_PB_ACCESS_RESTRICTED",           //  107, /**<  Phonebook access is restricted. */
    "MCM_ERROR_PB_DELETE_IN_PROG",              //  108, /**<  Phonebook delete is in progress. */
    "MCM_ERROR_PB_TEXT_TOO_LONG",               //  109, /**<  Phonebook text is too long. */
    "MCM_ERROR_PB_NUMBER_TOO_LONG",             //  110, /**<  Phonebook number is too long. */
    "MCM_ERROR_PB_HIDDEN_KEY_RESTRICTION",      //  111, /**<  Phonebook hidden key restriction. */
    "MCM_ERROR_PB_NOT_AVAILABLE",               //  112, /**<  Phonebook is not available. */
    "MCM_ERROR_DEVICE_MEMORY_ERROR",            //  113, /**<  Device memory error. */
    "MCM_ERROR_SIM_PIN_BLOCKED",                //  114, /**<  SIM PIN is blocked. */
    "MCM_ERROR_SIM_PIN_NOT_INITIALIZED",        //  115, /**<  SIM PIN is not initialized. */
    "MCM_ERROR_SIM_INVALID_PIN",                //  116, /**<  SIM PIN is invalid. */
    "MCM_ERROR_SIM_INVALID_PERSO_CK",           //  117, /**<  SIM invalid personalization CK. */
    "MCM_ERROR_SIM_PERSO_BLOCKED",              //  118, /**<  SIM personalization blocked. */
    "MCM_ERROR_SIM_PERSO_INVALID_DATA",         //  119, /**<  SIM personalization contains invalid data. */
    "MCM_ERROR_SIM_ACCESS_DENIED",              //  120, /**<  SIM access is denied. */
    "MCM_ERROR_SIM_INVALID_FILE_PATH",          //  121, /**<  SIM file path is invalid. */
    "MCM_ERROR_SIM_SERVICE_NOT_SUPPORTED",      //  122, /**<  SIM service is not supported. */
    "MCM_ERROR_SIM_AUTH_FAIL",                  //  123, /**<  SIM authorization failure. */
    "MCM_ERROR_SIM_PIN_PERM_BLOCKED"            //  124, /**<  SIM PIN is permanently blocked. */
};

int get_simcard_state(void)
{
	int ret = 0;
	
	QL_MCM_SIM_CARD_STATUS_INFO_T   t_info = {0};
	

	char *card_state[] = {  "UNKNOWN", 
							"ABSENT", 
							"PRESENT", 
							"ERROR_UNKNOWN", 
							"ERROR_POWER_DOWN", 
							"ERROR_POLL_ERROR", 
							"ERROR_NO_ATR_RECEIVED", 
							"ERROR_VOLT_MISMATCH", 
							"ERROR_PARITY_ERROR", 
							"ERROR_SIM_TECHNICAL_PROBLEMS"};            
	char *card_type[]  = {  "UNKNOWN", "ICC", "UICC"};
	char *app_state[]  = {  "UNKNOWN", 
							"DETECTED",
							"PIN1_REQ",
							"PUK1_REQ",
							"INITALIZATING",
							"PERSO_CK_REQ",
							"PERSO_PUK_REQ",
							"PERSO_PERMANENTLY_BLOCKED",
							"PIN1_PERM_BLOCKED",
							"ILLEGAL",
							"READY"};            
	ret = QL_MCM_SIM_GetCardStatus(h_sim, E_QL_MCM_SIM_SLOT_ID_1, &t_info);
	printf("MOBAP_SWITCH, QL_MCM_SIM_GetCardStatus ret = %s, card_state=%s, card_type=%s, app_state=%s, pin1_retries=%d\n", 
			errorcode[ret], 
			card_state[t_info.e_card_state - 0xB01],
			card_type[t_info.e_card_type - 0xB00],
			app_state[t_info.card_app_info.app_3gpp.app_state - 0xB00],
			t_info.card_app_info.app_3gpp.pin1_num_retries);
	
	return ret;	
}
static void ql_mcm_mobileap_nfy
(
    mobap_client_handle_type        h_mobap,
    E_QL_MOBILEAP_IND_EVENT_TYPE_T  e_msg_id,
    void                            *pv_data,
    void                            *contextPtr    
)
{
    switch(e_msg_id)
    {
    case E_QL_MOBILEAP_ENABLED_EV:
        printf("MOBAP_SWITCH, \nEnabled Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_CONNECTING_EV:
        printf("MOBAP_SWITCH, \nLan Connecting Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_CONNECTING_FAIL_EV:
        printf("MOBAP_SWITCH, \nLan Connecting Fail Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_IPv6_CONNECTING_FAIL_EV:
        printf("MOBAP_SWITCH, \nLan IPv6 Connecting Fail Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_CONNECTED_EV:
        printf("MOBAP_SWITCH, \nLan Connected Event Received\n");
        break;

    case E_QL_MOBILEAP_STA_CONNECTED_EV:
        printf("MOBAP_SWITCH, \nStation Connected Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_IPv6_CONNECTED_EV:
        printf("MOBAP_SWITCH, \nLan IPv6 Connected Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_CONNECTING_EV:
        printf("MOBAP_SWITCH, \nWan Connecting Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_CONNECTING_FAIL_EV:
        printf("MOBAP_SWITCH, \nWan Connecting Fail Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_IPv6_CONNECTING_FAIL_EV:
        printf("MOBAP_SWITCH, \nWan IPv6 Connecting Fail Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_CONNECTED_EV:
        printf("MOBAP_SWITCH, \nWan Connected Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_IPv6_CONNECTED_EV:
        printf("MOBAP_SWITCH, \nWan IPv6 Connected Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_DISCONNECTED_EV:
        printf("MOBAP_SWITCH, \nWan Disconnected Event Received\n");
        break;

    case E_QL_MOBILEAP_WAN_IPv6_DISCONNECTED_EV:
        printf("MOBAP_SWITCH, \nWan IPv6 Disconnected Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_DISCONNECTED_EV:
        printf("MOBAP_SWITCH, \nLan Disconnected Event Received\n");
        break;

    case E_QL_MOBILEAP_LAN_IPv6_DISCONNECTED_EV:
        printf("MOBAP_SWITCH, \nLan IPv6 Disconnected Event Received\n");
        break;

    case E_QL_MOBILEAP_DISABLED_EV:
        printf("MOBAP_SWITCH, \nDisabled Event Received\n");
        break;

    default:
        printf("MOBAP_SWITCH, \nUnknown Event: 0x%x\n", e_msg_id);
        break;
    }
}

int  set_mobap_status(enum MOBAP_STATUS status)
{
	int ret = 0;
	int h_mobap = 0;

	switch(status){
	
	case ENABLE:
			ret = QL_MCM_MobileAP_Init(&h_mobap);
			printf("MOBAP_SWITCH, QL_MCM_MobileAP_Init ret = %d, h_mobap=0x%X\n", ret, h_mobap);
			if(!!ret)
			goto ERR_QL_MCM_MobileAP_Init;
			
			ret = QL_MCM_MobileAP_AddRxIndMsgHandler(ql_mcm_mobileap_nfy, (void*)0x12345678);
			printf("MOBAP_SWITCH, QL_MCM_MobileAP_AddRxIndMsgHandler ret = %d\n", ret);
			if(!!ret)
			goto ERR_QL_MCM_MobileAP_AddRxIndMsgHandler;
			
			ret = QL_MCM_MobileAP_Enable(h_mobap);
			printf("MOBAP_SWITCH, QL_MCM_MobileAP_Enable ret = %d\n", ret);	
			if(!!ret)
			goto ERR_QL_MCM_MobileAP_Enable;
			
			ret = QL_MCM_MobileAP_ConnectBackhaul(h_mobap, E_QL_MOBILEAP_IP_TYPE_V4);
			printf("MOBAP_SWITCH, QL_MCM_MobileAP_ConnectBackhaul ret = %d\n", ret);	
			if(!!ret)
			goto ERR_QL_MCM_MobileAP_ConnectBackhaul;

			QL_MOBAP_IPV4_WWAN_CONFIG_INFO_T    t_wwan_cfg = {0};
			struct in_addr addr;
			char command[128];
			ret = QL_MCM_MobileAP_GetIPv4WWANCfg(h_mobap, &t_wwan_cfg);
			printf("MOBAP_SWITCH, QL_MCM_MobileAP_GetIPv4WWANCfg ret = %d, IPV4 Config Info:...\n", ret);            
			if(!ret){
				addr.s_addr = ntohl(t_wwan_cfg.v4_addr);
				printf("MOBAP_SWITCH, v4_addr_valid:%d v4_addr:%s \n",t_wwan_cfg.v4_addr_valid,inet_ntoa(addr));
				addr.s_addr = ntohl(t_wwan_cfg.v4_prim_dns_addr);
				printf("MOBAP_SWITCH, v4_prim_dns_addr_valid:%d v4_prim_dns_addr:%s \n",t_wwan_cfg.v4_prim_dns_addr_valid,inet_ntoa(addr));
				snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",inet_ntoa(addr));
				system(command);                                                                                                                                             
				addr.s_addr = ntohl(t_wwan_cfg.v4_sec_dns_addr);
				printf("MOBAP_SWITCH, v4_sec_dns_addr_valid:%d v4_sec_dns_addr:%s \n",t_wwan_cfg.v4_sec_dns_addr_valid,inet_ntoa(addr));
				snprintf(command, sizeof(command), "echo 'nameserver %s' >> /etc/resolv.conf",inet_ntoa(addr));
				system(command);                                                                                                                                             
			}
			else 
			goto ERR_QL_MCM_MobileAP_GetIPv4WWANCfg;
	break;
	
	case DISABLE:
	
	break;
	
	default:
	
	break;

	}

	return ret;
	
ERR_QL_MCM_MobileAP_Init:
	printf("MOBAP_SWITCH,  ERR_QL_MCM_MobileAP_Init \n");
ERR_QL_MCM_MobileAP_AddRxIndMsgHandler:
	printf("MOBAP_SWITCH,  ERR_QL_MCM_MobileAP_AddRxIndMsgHandler \n");
ERR_QL_MCM_MobileAP_Enable:
	printf("MOBAP_SWITCH,  ERR_QL_MCM_MobileAP_Enable \n");
ERR_QL_MCM_MobileAP_ConnectBackhaul:
	printf("MOBAP_SWITCH,  ERR_QL_MCM_MobileAP_ConnectBackhaul \n");
ERR_QL_MCM_MobileAP_GetIPv4WWANCfg:
	printf("MOBAP_SWITCH,  ERR_QL_MCM_MobileAP_GetIPv4WWANCfg \n");

	return -1;
		
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
                QL_API_LOGE("Malloc memory error.");
                fclose(fp);
                return NULL;
            }
            strcpy(p, buf + key_len + 1);
        }
    }

    fclose(fp);

    return p;
}



enum MOBAP_STATUS get_status_form_conf(void)
{
	char *status;
	status = conf_get_by_key(MOBAP_SWITCH_CFG_NAME, "DEFAULT_STATUS");
	if(status == NULL)
	return KEEP;	
	else if(!strncmp(status, "ON", 2))
	return ENABLE;
	else 
	return KEEP;
}



int main(int argc, char **agrv)
{
	int ret = 0;
	enum MOBAP_STATUS status = DISABLE;
	
	
	switch(argc)
	{
		case 1:
		status = get_status_form_conf();
		break;
		case 2:
		if(!strncmp(agrv[1], "on", 2))
			status = ENABLE;
		else if(!strncmp(agrv[1], "off", 3))
			status = DISABLE;
		else
		{
			printf("MOBAP_SWITCH, invalid parameter\n");
			status = KEEP;
			return -1;
//			status = get_status_form_conf();
		}
		break;
		
		default:
			printf("MOBAP_SWITCH, invalid parameter\n");
			status = KEEP;
			return -1;
//			status = get_status_form_conf();
			break;
	}
	
	if(!get_simcard_state())
	{
		printf("MOBAP_SWITCH, simcard status err\n");	
		return -1;
	}
		printf("MOBAP_SWITCH, simcard status ok\n");	
		
	ret = set_mobap_status(status);
	
	return ret;
	
}