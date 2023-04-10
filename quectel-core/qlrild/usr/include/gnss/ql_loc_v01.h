#ifndef QL_LOC_SERVICE_01_H
#define QL_LOC_SERVICE_01_H
/**
  @file ql_loc_v01.h

  @brief This is the public header file which defines the ql_loc service Data structures.

  This header file defines the types and structures that were defined in
  ql_loc. It contains the constant values defined, enums, structures,
  messages, and service message IDs (in that order) Structures that were
  defined in the IDL as messages contain mandatory elements, optional
  elements, a combination of mandatory and optional elements (mandatory
  always come before optionals in the structure), or nothing (null message)

  An optional element in a message is preceded by a uint8_t value that must be
  set to true if the element is going to be included. When decoding a received
  message, the uint8_t values will be set to true or false by the decode
  routine, and should be checked before accessing the values that they
  correspond to.

  Variable sized arrays are defined as static sized arrays with an unsigned
  integer (32 bit) preceding it that must be set to the number of elements
  in the array that are valid. For Example:

  uint32_t test_opaque_len;
  uint8_t test_opaque[16];

  If only 4 elements are added to test_opaque[] then test_opaque_len must be
  set to 4 before sending the message.  When decoding, the _len value is set
  by the decode routine and should be checked so that the correct number of
  elements in the array will be accessed.

*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2013 Qualcomm Technologies, Inc.
  All rights reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.


  $Header$
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
 *THIS IS AN AUTO GENERATED FILE. DO NOT ALTER IN ANY WAY
 *====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/* This file was generated with Tool version 6.14.7
   It was generated on: Tue Oct 27 2015 (Spin 0)
   From IDL File: ql_loc_v01.idl */

/** @defgroup ql_loc_qmi_consts Constant values defined in the IDL */
/** @defgroup ql_loc_qmi_msg_ids Constant values for QMI message IDs */
/** @defgroup ql_loc_qmi_enums Enumerated types used in QMI messages */
/** @defgroup ql_loc_qmi_messages Structures sent as QMI messages */
/** @defgroup ql_loc_qmi_aggregates Aggregate types used in QMI messages */
/** @defgroup ql_loc_qmi_accessor Accessor for QMI service object */
/** @defgroup ql_loc_qmi_version Constant values for versioning information */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ql_loc_qmi_version
    @{
  */
/** Major Version Number of the IDL used to generate this file */
#define QL_LOC_V01_IDL_MAJOR_VERS 0x01
/** Revision Number of the IDL used to generate this file */
#define QL_LOC_V01_IDL_MINOR_VERS 0x01
/** Major Version Number of the qmi_idl_compiler used to generate this file */
#define QL_LOC_V01_IDL_TOOL_VERS 0x06
/** Maximum Defined Message ID */
#define QL_LOC_V01_MAX_MESSAGE_ID 0x0317
/**
    @}
  */


/** @addtogroup ql_loc_qmi_consts
    @{
  */

/**  Maximum generic server address length for the host name. */
#define QL_LOC_MAX_SEVER_ADDR_LENGTH_CONST_V01 255

/**  Maximum generic string length. */
#define QL_LOC_MAX_GENERIC_STR_LENGTH_CONST_V01 255

/**  APN name maximum length. */
#define QL_LOC_MAX_APN_NAME_LENGTH_CONST_V01 100

/**  Maximum length of XTRA data injected. */
#define QL_LOC_MAX_XTRA_DATA_LENGTH_CONST_V01 64512

/**  Location map URL maximum size
 (used for indoor positioning). */
#define QL_LOC_GPS_LOCATION_MAP_URL_SIZE_CONST_V01 399

/**  Location map index maximum size
 (used for indoor positioning). */
#define QL_LOC_GPS_LOCATION_MAP_INDEX_SIZE_CONST_V01 16

/**  Maximum number of satellites in view. */
#define QL_LOC_GPS_MAX_SVS_CONST_V01 32

/**  Maximum SSID (Service Set Identifier) buffer size. */
#define QL_LOC_GPS_SSID_BUF_SIZE_CONST_V01 32

/**  IPv6 address length. */
#define QL_LOC_IPV6_ADDR_LEN_CONST_V01 16

/**  NI short string maximum length. */
#define QL_LOC_GPS_NI_SHORT_STRING_MAXLEN_CONST_V01 255

/**  NI long string maximum length. */
#define QL_LOC_GPS_NI_LONG_STRING_MAXLEN_CONST_V01 2047

/**  Raw data maximum size. */
#define QL_LOC_GPS_RAW_DATA_MAX_SIZE_CONST_V01 256

/**  NMEA string maximum length. */
#define QL_LOC_GPS_NMEA_MAX_LENGTH_CONST_V01 255
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_POSITION_MODE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_POSITION_MODE_STANDALONE_V01 = 0, /**<  Mode for running GPS standalone (no assistance).  */
  QL_LOC_POSITION_MODE_MS_BASED_V01 = 1, /**<  AGPS MS-Based mode.  */
  QL_LOC_POSITION_MODE_MS_ASSISTED_V01 = 2, /**<  AGPS MS-Assisted mode.  */
  QL_GPS_POSITION_MODE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_position_mode_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_POSITION_RECURRENCE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_POSITION_RECURRENCE_PERIODIC_V01 = 0, /**<  Receive GPS fixes on a recurring basis at a specified period.  */
  QL_LOC_POSITION_RECURRENCE_SINGLE_V01 = 1, /**<  Request a single-shot GPS fix.  */
  QL_GPS_POSITION_RECURRENCE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_position_recurrence_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_AIDING_DATA_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_DELETE_EPHEMERIS_V01 = 0x00000001, /**<  Delete ephemeris data.  */
  QL_LOC_DELETE_ALMANAC_V01 = 0x00000002, /**<  Delete almanac data.  */
  QL_LOC_DELETE_POSITION_V01 = 0x00000004, /**<  Delete position data.  */
  QL_LOC_DELETE_TIME_V01 = 0x00000008, /**<  Delete time data.  */
  QL_LOC_DELETE_IONO_V01 = 0x00000010, /**<  Delete IONO data.  */
  QL_LOC_DELETE_UTC_V01 = 0x00000020, /**<  Delete UTC data.  */
  QL_LOC_DELETE_HEALTH_V01 = 0x00000040, /**<  Delete health data.  */
  QL_LOC_DELETE_SVDIR_V01 = 0x00000080, /**<  Delete SVDIR data.  */
  QL_LOC_DELETE_SVSTEER_V01 = 0x00000100, /**<  Delete SVSTEER data.  */
  QL_LOC_DELETE_SADATA_V01 = 0x00000200, /**<  Delete SA data.  */
  QL_LOC_DELETE_RTI_V01 = 0x00000400, /**<  Delete RTI data.  */
  QL_LOC_DELETE_CELLDB_INFO_V01 = 0x00000800, /**<  Delete cell DB information.  */
  QL_LOC_DELETE_ALMANAC_CORR_V01 = 0x00001000, /**<  Delete almanac correction data.  */
  QL_LOC_DELETE_FREQ_BIAS_EST_V01 = 0x00002000, /**<  Delete frequency bias estimate.  */
  QL_LOC_DELETE_EPHEMERIS_GLO_V01 = 0x00004000, /**<  Delete ephemeris GLO data.  */
  QL_LOC_DELETE_ALMANAC_GLO_V01 = 0x00008000, /**<  Delete almanac GLO data.  */
  QL_LOC_DELETE_SVDIR_GLO_V01 = 0x00010000, /**<  Delete SVDIR GLO data.  */
  QL_LOC_DELETE_SVSTEER_GLO_V01 = 0x00020000, /**<  Delete SVSTEER GLO data.  */
  QL_LOC_DELETE_ALMANAC_CORR_GLO_V01 = 0x00040000, /**<  Delete almanac correction GLO data.  */
  QL_LOC_DELETE_TIME_GPS_V01 = 0x00080000, /**<  Delete time GPS data.  */
  QL_LOC_DELETE_TIME_GLO_V01 = 0x00100000, /**<  Delete time GLO data.  */
  QL_LOC_DELETE_ALL_V01 = 0xFFFFFFFF, /**<  Delete all location data.  */
  QL_GPS_AIDING_DATA_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_aiding_data_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_AGPS_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_AGPS_TYPE_INVALID_V01 = -1, /**<  Invalid.  */
  QL_LOC_AGPS_TYPE_ANY_V01 = 0, /**<  Any.  */
  QL_LOC_AGPS_TYPE_SUPL_V01 = 1, /**<  SUPL.  */
  QL_LOC_AGPS_TYPE_C2K_V01 = 2, /**<  C2K.  */
  QL_LOC_AGPS_TYPE_WWAN_ANY_V01 = 3, /**<  WWAN any.  */
  QL_LOC_AGPS_TYPE_WIFI_V01 = 4, /**<  Wi-Fi.  */
  QL_LOC_AGPS_TYPE_SUPL_ES_V01 = 5, /**<  SUPL_ES.  */
  QL_AGPS_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_agps_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_AGPS_BEARER_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_AGPS_APN_BEARER_INVALID_V01 = -1, /**<  Invalid.  */
  QL_LOC_AGPS_APN_BEARER_IPV4_V01 = 0, /**<  IPv4.  */
  QL_LOC_AGPS_APN_BEARER_IPV6_V01 = 1, /**<  IPv6.  */
  QL_LOC_AGPS_APN_BEARER_IPV4V6_V01 = 2, /**<  IPv4/v6.  */
  QL_AGPS_BEARER_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_agps_bearer_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_USER_RESPONSE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_NI_RESPONSE_ACCEPT_V01 = 1, /**<  Accept.  */
  QL_LOC_NI_RESPONSE_DENY_V01 = 2, /**<  Deny.  */
  QL_LOC_NI_RESPONSE_NORESP_V01 = 3, /**<  No response.  */
  QL_GPS_USER_RESPONSE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_user_response_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_LOCATION_FLAG_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_LOCATION_HAS_LAT_LONG_V01 = 0x0001, /**<  GPS location has valid latitude and longitude.  */
  QL_LOC_GPS_LOCATION_HAS_ALTITUDE_V01 = 0x0002, /**<  GPS location has a valid altitude.  */
  QL_LOC_GPS_LOCATION_HAS_SPEED_V01 = 0x0004, /**<  GPS location has a valid speed.  */
  QL_LOC_GPS_LOCATION_HAS_BEARING_V01 = 0x0008, /**<  GPS location has a valid bearing.  */
  QL_LOC_GPS_LOCATION_HAS_ACCURACY_V01 = 0x0010, /**<  GPS location has valid accuracy.  */
  QL_LOC_GPS_LOCATION_HAS_SOURCE_INFO_V01 = 0x0020, /**<  GPS location has valid source information.  */
  QL_LOC_GPS_LOCATION_HAS_IS_INDOOR_V01 = 0x0040, /**<  GPS location has a valid "is indoor?" flag.  */
  QL_LOC_GPS_LOCATION_HAS_FLOOR_NUMBE_V01 = 0x0080, /**<  GPS location has a valid floor number.  */
  QL_LOC_GPS_LOCATION_HAS_MAP_URL_V01 = 0x0100, /**<  GPS location has a valid map URL.  */
  QL_LOC_GPS_LOCATION_HAS_MAP_INDEX_V01 = 0x0200, /**<  GPS location has a valid map index.  */
  QL_GPS_LOCATION_FLAG_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_location_flag_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_STATUS_VALUE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_STATUS_NONE_V01 = 0, /**<  GPS status unknown.  */
  QL_LOC_GPS_STATUS_SESSION_BEGIN_V01 = 1, /**<  GPS has begun navigating.  */
  QL_LOC_GPS_STATUS_SESSION_END_V01 = 2, /**<  GPS has stopped navigating.  */
  QL_LOC_GPS_STATUS_ENGINE_ON_V01 = 3, /**<  GPS has powered on but is not navigating.  */
  QL_LOC_GPS_STATUS_ENGINE_OFF_V01 = 4, /**<  GPS is powered off.  */
  QL_GPS_STATUS_VALUE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_status_value_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_CAPABILITIES_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_CAPABILITY_SCHEDULING_V01 = 0x0000001, /**<  GPS HAL schedules fixes for GPS_POSITION_RECURRENCE_PERIODIC mode.
         If this is not set, the framework uses \n 1000 ms for min_interval
         and will call start() and stop() to schedule the GPS.
      */
  QL_LOC_GPS_CAPABILITY_MSB_V01 = 0x0000002, /**<  GPS supports MS-Based AGPS mode.  */
  QL_LOC_GPS_CAPABILITY_MSA_V01 = 0x0000004, /**<  GPS supports MS-Assisted AGPS mode.  */
  QL_LOC_GPS_CAPABILITY_SINGLE_SHOT_V01 = 0x0000008, /**<  GPS supports single-shot fixes.  */
  QL_LOC_GPS_CAPABILITY_ON_DEMAND_TIME_V01 = 0x0000010, /**<  GPS supports on-demand time injection.  */
  QL_GPS_CAPABILITIES_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_capabilities_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_AGPS_STATUS_VALUE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_REQUEST_AGPS_DATA_CONN_V01 = 1, /**<  GPS requests a data connection for AGPS.  */
  QL_LOC_GPS_RELEASE_AGPS_DATA_CONN_V01 = 2, /**<  GPS releases the AGPS data connection.  */
  QL_LOC_GPS_AGPS_DATA_CONNECTED_V01 = 3, /**<  AGPS data connection is initiated  */
  QL_LOC_GPS_AGPS_DATA_CONN_DONE_V01 = 4, /**<  AGPS data connection is completed.  */
  QL_LOC_GPS_AGPS_DATA_CONN_FAILED_V01 = 5, /**<  AGPS data connection failed.  */
  QL_AGPS_STATUS_VALUE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_agps_status_value_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_NI_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_NI_TYPE_VOICE_V01 = 1, /**<  Voice.  */
  QL_LOC_GPS_NI_TYPE_UMTS_SUPL_V01 = 2, /**<  UMTS SUPL.  */
  QL_LOC_GPS_NI_TYPE_UMTS_CTRL_PLANE_V01 = 3, /**<  UMTS control plane.  */
  QL_GPS_NI_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_ni_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_NI_NOTIFY_FLAGS_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_NI_NEED_NOTIFY_V01 = 0x0001, /**<  NI requires notification.  */
  QL_LOC_GPS_NI_NEED_VERIFY_V01 = 0x0002, /**<  NI requires verification.  */
  QL_LOC_GPS_NI_PRIVACY_OVERRIDE_V01 = 0x0004, /**<  NI requires privacy override; no notification/minimal trace.  */
  QL_GPS_NI_NOTIFY_FLAGS_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_ni_notify_flags_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_NI_ENCODING_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_GPS_ENC_NONE_V01 = 0, /**<  None.  */
  QL_LOC_GPS_ENC_SUPL_GSM_DEFAULT_V01 = 1, /**<  SUPL GSM default.  */
  QL_LOC_GPS_ENC_SUPL_UTF8_V01 = 2, /**<  SUPL UTF8.  */
  QL_LOC_GPS_ENC_SUPL_UCS2_V01 = 3, /**<  SUPL UCS2.  */
  QL_LOC_GPS_ENC_UNKNOWN_V01 = -1, /**<  Unknown.  */
  QL_GPS_NI_ENCODING_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_ni_encoding_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_enums
    @{
  */
typedef enum {
  QL_GPS_POSITION_SOURCE_T_MIN_ENUM_VAL_V01 = -2147483647, /**< To force a 32 bit signed enum.  Do not change or use*/
  QL_LOC_ULP_LOCATION_IS_FROM_HYBRID_V01 = 0x0001, /**<  Position source is ULP.  */
  QL_LOC_ULP_LOCATION_IS_FROM_GNSS_V01 = 0x0002, /**<  Position source is GNSS only.  */
  QL_GPS_POSITION_SOURCE_T_MAX_ENUM_VAL_V01 = 2147483647 /**< To force a 32 bit signed enum.  Do not change or use*/
}ql_gps_position_source_t_v01;
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_gps_location_t. */

  ql_gps_location_flag_t_v01 flags;
  /**<   Contains GPS location flags bits. */

  ql_gps_position_source_t_v01 position_source;
  /**<   Provider indicator for HYBRID or GPS. */

  double latitude;
  /**<   Latitude in degrees. */

  double longitude;
  /**<   Longitude in degrees. */

  double altitude;
  /**<   Altitude in meters above the WGS 84 reference
         ellipsoid. */

  float speed;
  /**<   Speed in meters per second. */

  float bearing;
  /**<   Heading in degrees. */

  float accuracy;
  /**<   Expected accuracy in meters. */

  int64_t timestamp;
  /**<   Timestamp for the location fix. */

  uint32_t raw_data_len;  /**< Must be set to # of elements in raw_data */
  uint8_t raw_data[QL_LOC_GPS_RAW_DATA_MAX_SIZE_CONST_V01];
  /**<   Allows the HAL to pass additional information related to the location. */

  int32_t is_indoor;
  /**<   Location is indoors. */

  float floor_number;
  /**<   Indicates the floor number. */

  char map_url[QL_LOC_GPS_LOCATION_MAP_URL_SIZE_CONST_V01 + 1];
  /**<   Map URL. */

  uint8_t map_index[QL_LOC_GPS_LOCATION_MAP_INDEX_SIZE_CONST_V01];
  /**<   Map index. */
}ql_gps_location_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_gps_status_t. */

  ql_gps_status_value_t_v01 status;
  /**<   Status. */
}ql_gps_status_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_gps_sv_info_t. */

  int32_t prn;
  /**<   Pseudo-random number for the SV. */

  float snr;
  /**<   Signal-to-noise ratio. */

  float elevation;
  /**<   Elevation of the SV in degrees. */

  float azimuth;
  /**<   Azimuth of the SV in degrees. */
}ql_gps_sv_info_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_gps_sv_status_t. */

  int32_t num_svs;
  /**<   Number of SVs currently visible. */

  ql_gps_sv_info_t_v01 sv_list[QL_LOC_GPS_MAX_SVS_CONST_V01];
  /**<   Contains an array of SV information. */

  uint32_t ephemeris_mask;
  /**<   Bitmask indicating which SVs
         have ephemeris data.
     */

  uint32_t almanac_mask;
  /**<   Bitmask indicating which SVs
         have almanac data.
     */

  uint32_t used_in_fix_mask;
  /**<   Bitmask indicating which SVs
         were used for computing the most recent position fix.
     */
}ql_gps_sv_status_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_agps_status_t. */

  ql_agps_t_v01 type;
  /**<   Type. */

  ql_agps_status_value_t_v01 status;
  /**<   Status. */

  int32_t ipv4_addr;
  /**<   IPv4 address. */

  char ipv6_addr[QL_LOC_IPV6_ADDR_LEN_CONST_V01 + 1];
  /**<   IPv6 address. */

  char ssid[QL_LOC_GPS_SSID_BUF_SIZE_CONST_V01 + 1];
  /**<   SSID. */

  char password[QL_LOC_GPS_SSID_BUF_SIZE_CONST_V01 + 1];
  /**<   Password. */
}ql_agps_status_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_aggregates
    @{
  */
typedef struct {

  uint32_t size;
  /**<   Set to the size of ql_gps_ni_notification_t. */

  int32_t notification_id;
  /**<   An ID generated by the HAL to associate NI notifications and UI
         responses.  */

  ql_gps_ni_t_v01 ni_type;
  /**<   An NI type used to distinguish different categories of NI
         events, such as GPS_NI_TYPE_VOICE, GPS_NI_TYPE_UMTS_SUPL, etc.  */

  ql_gps_ni_notify_flags_t_v01 notify_flags;
  /**<   Notification/verification options; combinations of GpsNiNotifyFlags
         constants.  */

  int32_t timeout;
  /**<   Timeout period to wait for a user response.
         Set to 0 for no timeout limit.  */

  ql_gps_user_response_t_v01 default_response;
  /**<   Default response when the response times out.  */

  char requestor_id[QL_LOC_GPS_NI_SHORT_STRING_MAXLEN_CONST_V01 + 1];
  /**<   Requestor ID.  */

  char text[QL_LOC_GPS_NI_LONG_STRING_MAXLEN_CONST_V01 + 1];
  /**<   Notification message. It can also be used to store the client ID in some cases.  */

  ql_gps_ni_encoding_t_v01 requestor_id_encoding;
  /**<   Client ID encoding scheme.  */

  ql_gps_ni_encoding_t_v01 text_encoding;
  /**<   Client name encoding scheme.  */

  char extras[QL_LOC_GPS_NI_LONG_STRING_MAXLEN_CONST_V01 + 1];
  /**<   Pointer to extra data. Format:\n
     - key_1 = value_1
     - key_2 = value_2 @tablebulletend
     */
}ql_gps_ni_notification_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Registers for location indications. */
typedef struct {

  /* Mandatory */
  uint8_t register_location_info_ind;
  /**<   Register for location information indications.*/

  /* Mandatory */
  uint8_t register_status_info_ind;
  /**<   Register for status information indications.*/

  /* Mandatory */
  uint8_t register_sv_info_ind;
  /**<   Register for SV information indications.*/

  /* Mandatory */
  uint8_t register_nmea_info_ind;
  /**<   Register for NMEA information indications.*/

  /* Mandatory */
  uint8_t register_capabilities_info_ind;
  /**<   Register for capabilities information indications.*/

  /* Mandatory */
  uint8_t register_utc_time_req_ind;
  /**<   Register for UTC time request indications.*/

  /* Mandatory */
  uint8_t register_xtra_data_req_ind;
  /**<   Register for XTRA data request indications.*/

  /* Mandatory */
  uint8_t register_agps_data_conn_cmd_req_ind;
  /**<   Register for AGPS data connection command request indications.*/

  /* Mandatory */
  uint8_t register_ni_notify_user_response_req_ind;
  /**<   Register for NI notify user response request indications.*/

  /* Optional */
  uint8_t register_xtra_report_server_ind_valid;  /**< Must be set to true if register_xtra_report_server_ind is being passed */
  uint8_t register_xtra_report_server_ind;
  /**<   Register for XTRA report server indications.*/
}ql_loc_set_indications_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_common_enums
    @{
  */
typedef enum {
  QL_RESULT_T_MIN_ENUM_VAL_V01 = -2147483647, /* To force a 32 bit signed enum.  Do not change or use*/
  QL_RESULT_SUCCESS_V01 = 0, /**<  Success.   */
  QL_RESULT_FAILURE_V01 = 1, /**<  Failure. */
  QL_RESULT_T_MAX_ENUM_VAL_V01 = 2147483647 /* To force a 32 bit signed enum.  Do not change or use*/
}ql_result_t_v01;
/**
    @}
  */

/** @addtogroup ql_common_enums
    @{
  */
typedef enum {
  QL_ERROR_T_MIN_ENUM_VAL_V01 = -2147483647, /* To force a 32 bit signed enum.  Do not change or use*/
  QL_SUCCESS_V01 = 0, /**<  Success. */
  QL_SUCCESS_CONDITIONAL_SUCCESS_V01 = 1, /**<  Conditional success. */
  QL_ERROR_QL_SERVICES_NOT_AVAILABLE_V01 = 2, /**<  MCM services not available. */
  QL_ERROR_GENERIC_V01 = 3, /**<  Generic error. */
  QL_ERROR_BADPARM_V01 = 4, /**<  Bad parameter. */
  QL_ERROR_MEMORY_V01 = 5, /**<  Memory error. */
  QL_ERROR_INVALID_STATE_V01 = 6, /**<  Invalid state. */
  QL_ERROR_MALFORMED_MSG_V01 = 7, /**<  Malformed message. */
  QL_ERROR_NO_MEMORY_V01 = 8, /**<  No memory. */
  QL_ERROR_INTERNAL_V01 = 9, /**<  Internal error. */
  QL_ERROR_ABORTED_V01 = 10, /**<  Action was aborted. */
  QL_ERROR_CLIENT_IDS_EXHAUSTED_V01 = 11, /**<  Client IDs have been exhausted. */
  QL_ERROR_UNABORTABLE_TRANSACTION_V01 = 12, /**<  Unabortable transaction. */
  QL_ERROR_INVALID_CLIENT_ID_V01 = 13, /**<  Invalid client ID. */
  QL_ERROR_NO_THRESHOLDS_V01 = 14, /**<  No thresholds. */
  QL_ERROR_INVALID_HANDLE_V01 = 15, /**<  Invalid handle. */
  QL_ERROR_INVALID_PROFILE_V01 = 16, /**<  Invalid profile. */
  QL_ERROR_INVALID_PINID_V01 = 17, /**<  Invalid PIN ID. */
  QL_ERROR_INCORRECT_PIN_V01 = 18, /**<  Incorrect PIN. */
  QL_ERROR_NO_NETWORK_FOUND_V01 = 19, /**<  No network found. */
  QL_ERROR_CALL_FAILED_V01 = 20, /**<  Call failed. */
  QL_ERROR_OUT_OF_CALL_V01 = 21, /**<  Out of call. */
  QL_ERROR_NOT_PROVISIONED_V01 = 22, /**<  Not provisioned. */
  QL_ERROR_MISSING_ARG_V01 = 23, /**<  Missing argument. */
  QL_ERROR_ARG_TOO_LONG_V01 = 24, /**<  Argument is too long. */
  QL_ERROR_INVALID_TX_ID_V01 = 25, /**<  Invalid Tx ID. */
  QL_ERROR_DEVICE_IN_USE_V01 = 26, /**<  Device is in use. */
  QL_ERROR_OP_NETWORK_UNSUPPORTED_V01 = 27, /**<  OP network is not supported. */
  QL_ERROR_OP_DEVICE_UNSUPPORTED_V01 = 28, /**<  OP device is not supported. */
  QL_ERROR_NO_EFFECT_V01 = 29, /**<  No effect. */
  QL_ERROR_NO_FREE_PROFILE_V01 = 30, /**<  No free profile. */
  QL_ERROR_INVALID_PDP_TYPE_V01 = 31, /**<  Invalid PDP type. */
  QL_ERROR_INVALID_TECH_PREF_V01 = 32, /**<  Invalid technical preference. */
  QL_ERROR_INVALID_PROFILE_TYPE_V01 = 33, /**<  Invalid profile type. */
  QL_ERROR_INVALID_SERVICE_TYPE_V01 = 34, /**<  Invalid service type. */
  QL_ERROR_INVALID_REGISTER_ACTION_V01 = 35, /**<  Invalid register action. */
  QL_ERROR_INVALID_PS_ATTACH_ACTION_V01 = 36, /**<  Invalid PS attach action. */
  QL_ERROR_AUTHENTICATION_FAILED_V01 = 37, /**<  Authentication failed. */
  QL_ERROR_PIN_BLOCKED_V01 = 38, /**<  PIN is blocked. */
  QL_ERROR_PIN_PERM_BLOCKED_V01 = 39, /**<  PIN is permanently blocked. */
  QL_ERROR_SIM_NOT_INITIALIZED_V01 = 40, /**<  SIM is not initialized. */
  QL_ERROR_MAX_QOS_REQUESTS_IN_USE_V01 = 41, /**<  Maximum QoS requests are in use. */
  QL_ERROR_INCORRECT_FLOW_FILTER_V01 = 42, /**<  Incorrect flow filter. */
  QL_ERROR_NETWORK_QOS_UNAWARE_V01 = 43, /**<  Network QoS is unaware. */
  QL_ERROR_INVALID_ID_V01 = 44, /**<  Invalid ID. */
  QL_ERROR_INVALID_QOS_ID_V01 = 45, /**<  Invalid QoS ID. */
  QL_ERROR_REQUESTED_NUM_UNSUPPORTED_V01 = 46, /**<  Requested number is not supported. */
  QL_ERROR_INTERFACE_NOT_FOUND_V01 = 47, /**<  Interface was not found. */
  QL_ERROR_FLOW_SUSPENDED_V01 = 48, /**<  Flow is suspended. */
  QL_ERROR_INVALID_DATA_FORMAT_V01 = 49, /**<  Invalid data format. */
  QL_ERROR_GENERAL_V01 = 50, /**<  General error. */
  QL_ERROR_UNKNOWN_V01 = 51, /**<  Unknown error. */
  QL_ERROR_INVALID_ARG_V01 = 52, /**<  Invalid argument. */
  QL_ERROR_INVALID_INDEX_V01 = 53, /**<  Invalid index. */
  QL_ERROR_NO_ENTRY_V01 = 54, /**<  No entry. */
  QL_ERROR_DEVICE_STORAGE_FULL_V01 = 55, /**<  Device storage is full. */
  QL_ERROR_DEVICE_NOT_READY_V01 = 56, /**<  Device is not ready. */
  QL_ERROR_NETWORK_NOT_READY_V01 = 57, /**<  Network is not ready. */
  QL_ERROR_CAUSE_CODE_V01 = 58, /**<  Cause code error. */
  QL_ERROR_MESSAGE_NOT_SENT_V01 = 59, /**<  Message was not sent. */
  QL_ERROR_MESSAGE_DELIVERY_FAILURE_V01 = 60, /**<  Message delivery failure. */
  QL_ERROR_INVALID_MESSAGE_ID_V01 = 61, /**<  Invalid message ID. */
  QL_ERROR_ENCODING_V01 = 62, /**<  Encoding error. */
  QL_ERROR_AUTHENTICATION_LOCK_V01 = 63, /**<  Authentication lock error. */
  QL_ERROR_INVALID_TRANSITION_V01 = 64, /**<  Invalid transition. */
  QL_ERROR_NOT_A_MCAST_IFACE_V01 = 65, /**<  Not an MCast interface. */
  QL_ERROR_MAX_MCAST_REQUESTS_IN_USE_V01 = 66, /**<  Maximum MCast requests are in use. */
  QL_ERROR_INVALID_MCAST_HANDLE_V01 = 67, /**<  Invalid MCast handle. */
  QL_ERROR_INVALID_IP_FAMILY_PREF_V01 = 68, /**<  Invalid IP family preference. */
  QL_ERROR_SESSION_INACTIVE_V01 = 69, /**<  Session is inactive. */
  QL_ERROR_SESSION_INVALID_V01 = 70, /**<  Session is invalid. */
  QL_ERROR_SESSION_OWNERSHIP_V01 = 71, /**<  Session ownership error. */
  QL_ERROR_INSUFFICIENT_RESOURCES_V01 = 72, /**<  Insufficient resources. */
  QL_ERROR_DISABLED_V01 = 73, /**<  Disabled. */
  QL_ERROR_INVALID_OPERATION_V01 = 74, /**<  Invalid operation. */
  QL_ERROR_INVALID_CMD_V01 = 75, /**<  Invalid command. */
  QL_ERROR_TPDU_TYPE_V01 = 76, /**<  Transfer Protocol data unit type error. */
  QL_ERROR_SMSC_ADDR_V01 = 77, /**<  Short message service center address error. */
  QL_ERROR_INFO_UNAVAILABLE_V01 = 78, /**<  Information is not available. */
  QL_ERROR_SEGMENT_TOO_LONG_V01 = 79, /**<  Segment is too long. */
  QL_ERROR_SEGMENT_ORDER_V01 = 80, /**<  Segment order error. */
  QL_ERROR_BUNDLING_NOT_SUPPORTED_V01 = 81, /**<  Bundling is not supported. */
  QL_ERROR_OP_PARTIAL_FAILURE_V01 = 82, /**<  OP partial failure. */
  QL_ERROR_POLICY_MISMATCH_V01 = 83, /**<  Policy mismatch. */
  QL_ERROR_SIM_FILE_NOT_FOUND_V01 = 84, /**<  SIM file was not found. */
  QL_ERROR_EXTENDED_INTERNAL_V01 = 85, /**<  Extended internal error. */
  QL_ERROR_ACCESS_DENIED_V01 = 86, /**<  Access is denied. */
  QL_ERROR_HARDWARE_RESTRICTED_V01 = 87, /**<  Hardware is restricted. */
  QL_ERROR_ACK_NOT_SENT_V01 = 88, /**<  Acknowledgement was not sent. */
  QL_ERROR_INJECT_TIMEOUT_V01 = 89, /**<  Inject timeout error. */
  QL_ERROR_INCOMPATIBLE_STATE_V01 = 90, /**<  Incompatible state. */
  QL_ERROR_FDN_RESTRICT_V01 = 91, /**<  Fixed dialing number restrict error. */
  QL_ERROR_SUPS_FAILURE_CAUSE_V01 = 92, /**<  SUPS failure cause. */
  QL_ERROR_NO_RADIO_V01 = 93, /**<  No radio. */
  QL_ERROR_NOT_SUPPORTED_V01 = 94, /**<  Not supported. */
  QL_ERROR_NO_SUBSCRIPTION_V01 = 95, /**<  No subscription. */
  QL_ERROR_CARD_CALL_CONTROL_FAILED_V01 = 96, /**<  Card call control failed. */
  QL_ERROR_NETWORK_ABORTED_V01 = 97, /**<  Network was aborted. */
  QL_ERROR_MSG_BLOCKED_V01 = 98, /**<  Message was blocked. */
  QL_ERROR_INVALID_SESSION_TYPE_V01 = 99, /**<  Invalid session type. */
  QL_ERROR_INVALID_PB_TYPE_V01 = 100, /**<  Invalid phonebook type. */
  QL_ERROR_NO_SIM_V01 = 101, /**<  No SIM was found. */
  QL_ERROR_PB_NOT_READY_V01 = 102, /**<  Phonebook not ready. */
  QL_ERROR_PIN_RESTRICTION_V01 = 103, /**<  PIN restriction. */
  QL_ERROR_PIN2_RESTRICTION_V01 = 104, /**<  PIN2 restriction. */
  QL_ERROR_PUK_RESTRICTION_V01 = 105, /**<  PIN unlocking key restriction. */
  QL_ERROR_PUK2_RESTRICTION_V01 = 106, /**<  PIN unlocking key2 restriction. */
  QL_ERROR_PB_ACCESS_RESTRICTED_V01 = 107, /**<  Phonebook access is restricted. */
  QL_ERROR_PB_DELETE_IN_PROG_V01 = 108, /**<  Phonebook delete is in progress. */
  QL_ERROR_PB_TEXT_TOO_LONG_V01 = 109, /**<  Phonebook text is too long. */
  QL_ERROR_PB_NUMBER_TOO_LONG_V01 = 110, /**<  Phonebook number is too long. */
  QL_ERROR_PB_HIDDEN_KEY_RESTRICTION_V01 = 111, /**<  Phonebook hidden key restriction. */
  QL_ERROR_PB_NOT_AVAILABLE_V01 = 112, /**<  Phonebook is not available. */
  QL_ERROR_DEVICE_MEMORY_ERROR_V01 = 113, /**<  Device memory error. */
  QL_ERROR_SIM_PIN_BLOCKED_V01 = 114, /**<  SIM PIN is blocked. */
  QL_ERROR_SIM_PIN_NOT_INITIALIZED_V01 = 115, /**<  SIM PIN is not initialized. */
  QL_ERROR_SIM_INVALID_PIN_V01 = 116, /**<  SIM PIN is invalid. */
  QL_ERROR_SIM_INVALID_PERSO_CK_V01 = 117, /**<  SIM invalid personalization CK. */
  QL_ERROR_SIM_PERSO_BLOCKED_V01 = 118, /**<  SIM personalization blocked. */
  QL_ERROR_SIM_PERSO_INVALID_DATA_V01 = 119, /**<  SIM personalization contains invalid data. */
  QL_ERROR_SIM_ACCESS_DENIED_V01 = 120, /**<  SIM access is denied. */
  QL_ERROR_SIM_INVALID_FILE_PATH_V01 = 121, /**<  SIM file path is invalid. */
  QL_ERROR_SIM_SERVICE_NOT_SUPPORTED_V01 = 122, /**<  SIM service is not supported. */
  QL_ERROR_SIM_AUTH_FAIL_V01 = 123, /**<  SIM authorization failure. */
  QL_ERROR_SIM_PIN_PERM_BLOCKED_V01 = 124, /**<  SIM PIN is permanently blocked. */
  QL_ERROR_RADIO_RESET_V01 = 125, /**<  SSR happen, device not in proper state. */
  QL_ERROR_T_MAX_ENUM_VAL_V01 = 2147483647 /* To force a 32 bit signed enum.  Do not change or use*/
}ql_error_t_v01;
/**
    @}
  */

/** @addtogroup ql_common_aggregates
    @{
  */
typedef struct {

  ql_result_t_v01 result;
  /**<   Result code: \n
         - QL_RESULT_SUCCESS
         - QL_RESULT_FAILURE @tablebulletend
                          */

  ql_error_t_v01 error;
  /**<   Error code. Possible error code values are
                            described in the error codes section of each
                            message definition.
                          */
}ql_response_t_v01;  /* Type */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Registers for location indications. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_set_indications_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Sets the position mode. */
typedef struct {

  /* Mandatory */
  /*  Mode */
  ql_gps_position_mode_t_v01 mode;
  /**<   Position mode.*/

  /* Mandatory */
  /*  Recurrence */
  ql_gps_position_recurrence_t_v01 recurrence;
  /**<   Recurrence.*/

  /* Mandatory */
  /*  Minimum Interval */
  uint32_t min_interval;
  /**<   Minimum interval.*/

  /* Mandatory */
  /*  Preferred Accuracy */
  uint32_t preferred_accuracy;
  /**<   Preferred accuracy.*/

  /* Mandatory */
  /*  Preferred Time */
  uint32_t preferred_time;
  /**<   Preferred time.*/
}ql_loc_set_position_mode_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Sets the position mode. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_set_position_mode_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Starts navigation. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ql_loc_start_nav_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Starts navigation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_start_nav_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Stops navigation. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ql_loc_stop_nav_req_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Stops navigation. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_stop_nav_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Deletes location-aiding data. */
typedef struct {

  /* Mandatory */
  /*  Aiding Data Flags */
  ql_gps_aiding_data_t_v01 flags;
  /**<   Aiding data flags.*/
}ql_loc_delete_aiding_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Deletes location-aiding data. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_delete_aiding_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Injects time. */
typedef struct {

  /* Mandatory */
  /*  Time */
  int64_t time;
  /**<   Inject time.*/

  /* Mandatory */
  /*  Time Reference */
  int64_t time_reference;
  /**<   Time reference.*/

  /* Mandatory */
  /*  Uncertainty */
  int32_t uncertainty;
  /**<   Uncertainty.*/
}ql_loc_inject_time_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Injects time. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_inject_time_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Injects a location. */
typedef struct {

  /* Mandatory */
  /*  Latitude */
  double latitude;
  /**<   Latitude.*/

  /* Mandatory */
  /*  Longitude */
  double longitude;
  /**<   Longitude.*/

  /* Mandatory */
  /*  Accuracy */
  float accuracy;
  /**<   Accuracy.*/
}ql_loc_inject_location_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Injects a location. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_inject_location_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Injects XTRA data.
           Since the IPC mechanism puts a limit on the size of the data
           transferable in one message at 64 KB, the application using this
           command must break the data down into chunks of a smaller
           size and repeatedly call this API until all the data has
           been injected. */
typedef struct {

  /* Mandatory */
  /*  Xtra Data */
  uint32_t data_len;  /**< Must be set to # of elements in data */
  uint8_t data[QL_LOC_MAX_XTRA_DATA_LENGTH_CONST_V01];
  /**<   XTRA data.*/
}ql_loc_xtra_inject_data_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Injects XTRA data.
           Since the IPC mechanism puts a limit on the size of the data
           transferable in one message at 64 KB, the application using this
           command must break the data down into chunks of a smaller
           size and repeatedly call this API until all the data has
           been injected. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_xtra_inject_data_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Indicates that the AGPS data connection is open. */
typedef struct {

  /* Mandatory */
  /*  AGPS Type */
  ql_agps_t_v01 agps_type;
  /**<   AGPS type.*/

  /* Mandatory */
  /*  APN */
  char apn[QL_LOC_MAX_APN_NAME_LENGTH_CONST_V01 + 1];
  /**<   APN.*/

  /* Mandatory */
  /*  AGPS Bearer Type */
  ql_agps_bearer_t_v01 bearer_type;
  /**<   Bearer type.*/
}ql_loc_agps_data_conn_open_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Indicates that the AGPS data connection is open. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_agps_data_conn_open_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Indicates that the AGPS data connection is closed. */
typedef struct {

  /* Mandatory */
  /*  AGPS Type */
  ql_agps_t_v01 agps_type;
  /**<   AGPS type.*/
}ql_loc_agps_data_conn_closed_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Indicates that the AGPS data connection is closed. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_agps_data_conn_closed_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Indicates that the AGPS data connection failed to start. */
typedef struct {

  /* Mandatory */
  /*  AGPS Type */
  ql_agps_t_v01 agps_type;
  /**<   AGPS type.*/
}ql_loc_agps_data_conn_failed_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Indicates that the AGPS data connection failed to start. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_agps_data_conn_failed_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Sets the AGPS server. */
typedef struct {

  /* Mandatory */
  /*  AGPS Type */
  ql_agps_t_v01 agps_type;
  /**<   AGPS type.*/

  /* Mandatory */
  /*  Host Name */
  char host_name[QL_LOC_MAX_SEVER_ADDR_LENGTH_CONST_V01 + 1];
  /**<   Host name.*/

  /* Mandatory */
  /*  Port */
  uint32_t port;
  /**<   Port.*/
}ql_loc_agps_set_server_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Sets the AGPS server. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_agps_set_server_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Sends a user response for NI. */
typedef struct {

  /* Mandatory */
  /*  Notification ID */
  int32_t notif_id;
  /**<   Notification ID.*/

  /* Mandatory */
  /*  User Response */
  ql_gps_user_response_t_v01 user_response;
  /**<   User response.*/
}ql_loc_ni_respond_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Sends a user response for NI. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_ni_respond_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Request Message; Updates the network availability status. */
typedef struct {

  /* Mandatory */
  /*  Available */
  int32_t available;
  /**<   Available.*/

  /* Mandatory */
  /*  APN */
  char apn[QL_LOC_MAX_APN_NAME_LENGTH_CONST_V01 + 1];
  /**<   APN.*/
}ql_loc_agps_ril_update_network_availability_req_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Response Message; Updates the network availability status. */
typedef struct {

  /* Mandatory */
  /*  Result Code */
  ql_response_t_v01 resp;
  /**<   Result code.*/
}ql_loc_agps_ril_update_network_availability_resp_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the location payload. */
typedef struct {

  /* Mandatory */
  /*  Location Info */
  ql_gps_location_t_v01 location;
  /**<   Location information.*/
}ql_loc_location_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the location status payload. */
typedef struct {

  /* Mandatory */
  /*  Gps Status */
  ql_gps_status_t_v01 status;
  /**<   GPS status.*/
}ql_loc_status_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the satellites in view payload. */
typedef struct {

  /* Mandatory */
  /*  Gps SV status */
  ql_gps_sv_status_t_v01 sv_info;
  /**<   GPS SV status.*/
}ql_loc_sv_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the NMEA string payload. */
typedef struct {

  /* Mandatory */
  /*  Timestamp */
  int64_t timestamp;
  /**<   Timestamp.*/

  /* Mandatory */
  /*  NMEA String */
  char nmea[QL_LOC_GPS_NMEA_MAX_LENGTH_CONST_V01 + 1];
  /**<   NMEA string.*/

  /* Mandatory */
  /*  Length */
  int32_t length;
  /**<   NMEA string length. @newpagetable*/
}ql_loc_nmea_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the location capabilities. */
typedef struct {

  /* Mandatory */
  /*  Capabilities */
  ql_gps_capabilities_t_v01 capabilities;
  /**<   Capabilities.*/
}ql_loc_capabilities_info_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication request for SNTP time. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ql_loc_utc_time_req_ind_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication request to download XTRA data. */
typedef struct {
  /* This element is a placeholder to prevent the declaration of
     an empty struct.  DO NOT USE THIS FIELD UNDER ANY CIRCUMSTANCE */
  char __placeholder;
}ql_loc_xtra_data_req_ind_msg_v01;

  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the AGPS status. */
typedef struct {

  /* Mandatory */
  /*  AGPS Status */
  ql_agps_status_t_v01 status;
  /**<   AGPS status.*/
}ql_loc_agps_status_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the NI notification payload. */
typedef struct {

  /* Mandatory */
  /*  NI Notification */
  ql_gps_ni_notification_t_v01 notification;
  /**<   NI notification.*/
}ql_loc_ni_notification_ind_msg_v01;  /* Message */
/**
    @}
  */

/** @addtogroup ql_loc_qmi_messages
    @{
  */
/** Indication Message; Indication with the reported XTRA server URLs. */
typedef struct {

  /* Mandatory */
  /*  Server1 */
  char server1[QL_LOC_MAX_SEVER_ADDR_LENGTH_CONST_V01 + 1];
  /**<   server1.*/

  /* Mandatory */
  /*  Server2 */
  char server2[QL_LOC_MAX_SEVER_ADDR_LENGTH_CONST_V01 + 1];
  /**<   server2.*/

  /* Mandatory */
  /*  Server3 */
  char server3[QL_LOC_MAX_SEVER_ADDR_LENGTH_CONST_V01 + 1];
  /**<   server3.*/
}ql_loc_xtra_report_server_ind_msg_v01;  /* Message */
/**
    @}
  */

/* Conditional compilation tags for message removal */
//#define REMOVE_QL_LOC_AGPS_DATA_CONN_CLOSED_V01
//#define REMOVE_QL_LOC_AGPS_DATA_CONN_FAILED_V01
//#define REMOVE_QL_LOC_AGPS_DATA_CONN_OPEN_V01
//#define REMOVE_QL_LOC_AGPS_RIL_UPDATE_NETWORK_AVAILABILITY_V01
//#define REMOVE_QL_LOC_AGPS_SET_SERVER_V01
//#define REMOVE_QL_LOC_AGPS_STATUS_IND_V01
//#define REMOVE_QL_LOC_CAPABILITIES_INFO_IND_V01
//#define REMOVE_QL_LOC_DELETE_AIDING_DATA_V01
//#define REMOVE_QL_LOC_INJECT_LOCATION_V01
//#define REMOVE_QL_LOC_INJECT_TIME_V01
//#define REMOVE_QL_LOC_LOCATION_INFO_IND_V01
//#define REMOVE_QL_LOC_NI_NOTIFICATION_IND_V01
//#define REMOVE_QL_LOC_NI_RESPOND_V01
//#define REMOVE_QL_LOC_NMEA_INFO_IND_V01
//#define REMOVE_QL_LOC_SET_INDICATIONS_V01
//#define REMOVE_QL_LOC_SET_POSITION_MODE_V01
//#define REMOVE_QL_LOC_START_NAV_V01
//#define REMOVE_QL_LOC_STATUS_INFO_IND_V01
//#define REMOVE_QL_LOC_STOP_NAV_V01
//#define REMOVE_QL_LOC_SV_INFO_IND_V01
//#define REMOVE_QL_LOC_UTC_TIME_REQ_IND_V01
//#define REMOVE_QL_LOC_XTRA_DATA_REQ_IND_V01
//#define REMOVE_QL_LOC_XTRA_INJECT_DATA_V01
//#define REMOVE_QL_LOC_XTRA_REPORT_SERVER_IND_V01

/*Service Message Definition*/
/** @addtogroup ql_loc_qmi_msg_ids
    @{
  */
#define QL_LOC_SET_INDICATIONS_REQ_V01 0x0300
#define QL_LOC_SET_INDICATIONS_RESP_V01 0x0300
#define QL_LOC_SET_POSITION_MODE_REQ_V01 0x0301
#define QL_LOC_SET_POSITION_MODE_RESP_V01 0x0301
#define QL_LOC_START_NAV_REQ_V01 0x0302
#define QL_LOC_START_NAV_RESP_V01 0x0302
#define QL_LOC_STOP_NAV_REQ_V01 0x0303
#define QL_LOC_STOP_NAV_RESP_V01 0x0303
#define QL_LOC_DELETE_AIDING_DATA_REQ_V01 0x0304
#define QL_LOC_DELETE_AIDING_DATA_RESP_V01 0x0304
#define QL_LOC_INJECT_TIME_REQ_V01 0x0305
#define QL_LOC_INJECT_TIME_RESP_V01 0x0305
#define QL_LOC_INJECT_LOCATION_REQ_V01 0x0306
#define QL_LOC_INJECT_LOCATION_RESP_V01 0x0306
#define QL_LOC_XTRA_INJECT_DATA_REQ_V01 0x0307
#define QL_LOC_XTRA_INJECT_DATA_RESP_V01 0x0307
#define QL_LOC_AGPS_DATA_CONN_OPEN_REQ_V01 0x0308
#define QL_LOC_AGPS_DATA_CONN_OPEN_RESP_V01 0x0308
#define QL_LOC_AGPS_DATA_CONN_CLOSED_REQ_V01 0x0309
#define QL_LOC_AGPS_DATA_CONN_CLOSED_RESP_V01 0x0309
#define QL_LOC_AGPS_DATA_CONN_FAILED_REQ_V01 0x030A
#define QL_LOC_AGPS_DATA_CONN_FAILED_RESP_V01 0x030A
#define QL_LOC_AGPS_SET_SERVER_REQ_V01 0x030B
#define QL_LOC_AGPS_SET_SERVER_RESP_V01 0x030B
#define QL_LOC_NI_RESPOND_REQ_V01 0x030C
#define QL_LOC_NI_RESPOND_RESP_V01 0x030C
#define QL_LOC_AGPS_RIL_UPDATE_NETWORK_AVAILABILITY_REQ_V01 0x030D
#define QL_LOC_AGPS_RIL_UPDATE_NETWORK_AVAILABILITY_RESP_V01 0x030D

/* Indication Messages Definition */
#define QL_LOC_LOCATION_INFO_IND_V01 0x030E
#define QL_LOC_STATUS_INFO_IND_V01 0x030F
#define QL_LOC_SV_INFO_IND_V01 0x0310
#define QL_LOC_NMEA_INFO_IND_V01 0x0311
#define QL_LOC_CAPABILITIES_INFO_IND_V01 0x0312
#define QL_LOC_UTC_TIME_REQ_IND_V01 0x0313
#define QL_LOC_XTRA_DATA_REQ_IND_V01 0x0314
#define QL_LOC_AGPS_STATUS_IND_V01 0x0315
#define QL_LOC_NI_NOTIFICATION_IND_V01 0x0316
#define QL_LOC_XTRA_REPORT_SERVER_IND_V01 0x0317

/**
    @}
  */

#ifdef __cplusplus
}
#endif
#endif

