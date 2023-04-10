
#ifndef LWM2M_GETINFO_H_
#define LWM2M_GETINFO_H_
#include <ql-mcm-api/ql_in.h>
#include "object_location.h"
typedef uint32_t lwm2m_handle_type;
/** Maxmum cells in cell info response. */
#define QL_LWM2M_CELL_INFO_GSM_MAX     20
#define QL_LWM2M_CELL_INFO_UMTS_MAX    20
#define QL_LWM2M_CELL_INFO_LTE_MAX     20

#define LWM2M_GET_CELL_INFO_REQ_V01 0x0510
#define LWM2M_GET_CELL_INFO_RESP_V01 0x0510

#define LW_NETWORK_MODE_UNKNOWN -1
#define LW_NETWORK_MODE_GSM 0
#define LW_NETWORK_MODE_WCDMA 2
#define LW_NETWORK_MODE_CDMA 3
#define LW_NETWORK_MODE_LTE 6

typedef enum 
{
    E_QL_LWM2M_RADIO_TECH_TD_SCDMA = 1,
    E_QL_LWM2M_RADIO_TECH_GSM      = 2,    /**<  GSM; only supports voice. */
    E_QL_LWM2M_RADIO_TECH_HSPAP    = 3,    /**<  HSPA+. */
    E_QL_LWM2M_RADIO_TECH_LTE      = 4,    /**<  LTE. */
    E_QL_LWM2M_RADIO_TECH_EHRPD    = 5,    /**<  EHRPD. */
    E_QL_LWM2M_RADIO_TECH_EVDO_B   = 6,    /**<  EVDO B. */
    E_QL_LWM2M_RADIO_TECH_HSPA     = 7,    /**<  HSPA. */
    E_QL_LWM2M_RADIO_TECH_HSUPA    = 8,    /**<  HSUPA. */
    E_QL_LWM2M_RADIO_TECH_HSDPA    = 9,    /**<  HSDPA. */
    E_QL_LWM2M_RADIO_TECH_EVDO_A   = 10,   /**<  EVDO A. */
    E_QL_LWM2M_RADIO_TECH_EVDO_0   = 11,   /**<  EVDO 0. */
    E_QL_LWM2M_RADIO_TECH_1xRTT    = 12,   /**<  1xRTT. */
    E_QL_LWM2M_RADIO_TECH_IS95B    = 13,   /**<  IS95B. */
    E_QL_LWM2M_RADIO_TECH_IS95A    = 14,   /**<  IS95A. */
    E_QL_LWM2M_RADIO_TECH_UMTS     = 15,   /**<  UMTS. */
    E_QL_LWM2M_RADIO_TECH_EDGE     = 16,   /**<  EDGE. */
    E_QL_LWM2M_RADIO_TECH_GPRS     = 17,   /**<  GPRS. */
    E_QL_LWM2M_RADIO_TECH_NONE     = 18    /**<  No technology selected. */
}E_QL_LWM2M_RADIO_TECH_TYPE_T;

typedef struct
{
    uint32_t    cid;        /**<   Cell ID, (0 indicates information is not represent).*/
    char        plmn[3];    /**<   MCC/MNC inforamtion coded as octet 3, 4, and 5 in 3GPP TS 24.008 Section 10.5.1.3.(This field should be ignored when cid is not present). */
    uint16_t    lac;        /**<   Location area code.(This field should be ignord when cid is not present). */
    uint16_t    arfcn;      /**<   Absolute RF channel number. */
    uint8_t     bsic;       /**<   Base station identity code. (0 indicates information is not present). */
}QL_LWM2M_GSM_INFO_T;

typedef struct
{
    uint32_t cid;           /**<   Cell ID (0 indicates information is not present). */
    uint32_t lcid;          /**<   UTRAN Cell ID (0 indicates information is not present). */
    char plmn[3];           /**<   MCC/MNC information coded as octet 3, 4, and 5 in 3GPP TS 24.008 Section 10.5.1.3.(This field should be ignored when cid is not present). */
    uint16_t lac;           /**<   Location area code. (This field should be ignored when cid is not present). */
    uint16_t uarfcn;        /**<   UTRA absolute RF channel number. */
    uint16_t psc;           /**<   Primary scrambling code. */
}QL_LWM2M_UMTS_INFO_T;

typedef struct
{
    uint32_t cid;           /**<   Global cell ID in the system information block (0 indicates information is not present). */
    char plmn[3];           /**<   MCC/MNC information coded as octet 3, 4, and 5 in 3GPP TS 24.008 Sextion 10.5.1.3.(This filed should be ignored when cid is not present). */
    uint16_t tac;           /**<   Tracing area code (This field should be ignored when cid is not present). */
    uint16_t pci;           /**<   Physical cell ID. Range: 0 to 503. */
    uint16_t earfcn;        /**<   E-UTRA absolute radio frequency channel number of the cell. RANGE: 0 TO 65535. */
}QL_LWM2M_LTE_INFO_T;
/** Gets cell information. */
typedef struct 
{
    E_QL_LWM2M_RADIO_TECH_TYPE_T serving_rat;
    uint8_t                 gsm_info_valid;                         /**< Must be set to TRUE if gsm_info is being passed. */
    uint8_t                 gsm_info_len;                           /**< Must be set to the number of elements in entry*/
    QL_LWM2M_GSM_INFO_T    gsm_info[QL_LWM2M_CELL_INFO_GSM_MAX];  /**<   GSM cell information (Serving and neighbor. */  
    uint8_t                 umts_info_valid;                        /**< Must be set to TRUE if umts_info is being passed. */
    uint8_t                 umts_info_len;                          /**< Must be set to the number of elements in entry*/
    QL_LWM2M_UMTS_INFO_T   umts_info[QL_LWM2M_CELL_INFO_UMTS_MAX];/**<   UMTS cell information (Serving and neighbor). */
    uint8_t                 lte_info_valid;                         /**< Must be set to TRUE if lte_info is being passed. */
    uint8_t                 lte_info_len;                           /**< Must be set to the number of elements in entry*/
    QL_LWM2M_LTE_INFO_T    lte_info[QL_LWM2M_CELL_INFO_LTE_MAX];  /**<   LTE cell information (Serving and neighbor). */
}QL_LWM2M_CELL_INFO_T;

typedef struct
{
    int signalStrength;
    int linkQuality;
    int linkUtilization;
} QL_LWM2M_CONN_INFO_T;

int QL_LWM2M_GetMcc();
int QL_LWM2M_GetMnc();
uint32_t QL_LWM2M_GetCellInfo();
int QL_LWM2M_GetSignalStrength();
int QL_LWM2M_GetRsRq();
int QL_LWM2M_GetIccid(char * iccid);
int QL_LWM2M_GetImsi(char * imsi);
int QL_LWM2M_GetNumber(char * num);
int QL_LWM2M_GetImei(char * imei);
int QL_LWM2M_GetGps(location_data_t * data);
//int QL_LWM2M_GetIpaddr(char * address);
int QL_LWM2M_GetAbout(ql_module_about_info_s * about);
int QL_LWM2M_GetConnInfo(QL_LWM2M_CONN_INFO_T * conn_info);
char * QL_LWM2M_GetHardversion();
char * QL_LWM2M_GetProductname();

#ifdef LWM2M_SUPPORT_QLRIL_API
void QL_LWM2M_SwithRadioPower();
int QL_LWM2M_GetDataRegistrationState();

#endif
#endif
