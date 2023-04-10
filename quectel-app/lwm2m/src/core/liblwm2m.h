/*******************************************************************************
 *
 * Copyright (c) 2013, 2014 Intel Corporation and others.
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
 *    Fabien Fleutot - Please refer to git log
 *    Simon Bernard - Please refer to git log
 *    Toby Jaffey - Please refer to git log
 *    Julien Vermillard - Please refer to git log
 *    Bosch Software Innovations GmbH - Please refer to git log
 *    Pascal Rieux - Please refer to git log
 *    Ville Skyttä - Please refer to git log
 *    Scott Bertin, AMETEK, Inc. - Please refer to git log
 *
 *******************************************************************************/

/*
 Copyright (c) 2013, 2014 Intel Corporation

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.
     * Neither the name of Intel Corporation nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.

 David Navarro <david.navarro@intel.com>

*/

#ifndef _LWM2M_CLIENT_H_
#define _LWM2M_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "mbedtls/ssl.h"

#ifdef LWM2M_SERVER_MODE
#ifndef LWM2M_SUPPORT_JSON
#define LWM2M_SUPPORT_JSON
#endif
#ifndef LWM2M_VERSION_1_0
#ifndef LWM2M_SUPPORT_SENML_JSON
#define LWM2M_SUPPORT_SENML_JSON
#endif
#endif
#endif

#ifdef LWM2M_BOOTSTRAP_SERVER_MODE
#ifndef LWM2M_VERSION_1_0
#ifndef LWM2M_SUPPORT_SENML_JSON
#define LWM2M_SUPPORT_SENML_JSON
#endif
#endif
#endif

#if defined(LWM2M_BOOTSTRAP) && defined(LWM2M_BOOTSTRAP_SERVER_MODE)
#error "LWM2M_BOOTSTRAP and LWM2M_BOOTSTRAP_SERVER_MODE cannot be defined at the same time!"
#endif

//add by joe start
#define ESUCCESS    0
#define EFAILURE    -1

#define LWM2M_FALSE	false
#define LWM2M_TRUE 	true


#define LWM2M_PERSITENT_OBSERVATION_STORAGE   "/data/lwm2m/observation_pr"
#define SERVER_PERSISTENCE_FILE               "/data/lwm2m/server_obj_pr"
#define ACL_PERSISTENCE_FILE                  "/data/lwm2m/acl_obj_pr"
#define SECURITY_PERSISTENCE_FILE             "/data/lwm2m/security_obj_pr"
#define FIRMWARE_PERSISTENCE_FILE              "/data/lwm2m/firmware_obj_pr"
#define CONN_MON_PERSISTENCE_FILE             "/data/lwm2m/conn_mon_obj_pr"
#define DEVICE_PERSISTENCE_FILE             "/data/lwm2m/device_obj_pr"
#define LWM2M_LOCATION_OBJ_PERSISTENCE_PATH "/data/lwm2m/location_obj_delete_pr"
#define LWM2M_DEVICE_OBJ_PERSISTENCE_PATH "/data/lwm2m/device_obj_delete_pr"
#define LWM2M_CONN_MON_OBJ_PERSISTENCE_PATH "/data/lwm2m/conn_mon_obj_obj_delete_pr"
#define LWM2M_CONN_STAT_OBJ_PERSISTENCE_PATH "/data/lwm2m/conn_stat_obj_delete_pr"
//add by joe for vzw network start
#define QUEC_LWM2M_PHONE_NUMBER_SAVE     "/data/lwm2m/phoneNumber.dat"
#define QUEC_LWM2M_PHONE_NUMBER_SAVE_TEMP     "/data/lwm2m/phoneNumberTemp.dat"
#define QUEC_LWM2M_FULL_REGIST_FALG_SAVE     "/data/lwm2m/fullregistflag.dat"
#define QUEC_LWM2M_FOTA_UPDATE_FILE          "/data/update_ext4.zip"
#define QUEC_LWM2M_FIRMWARE_VERSION_SAVE     "/data/lwm2m/firmwareversion.dat"
#define QUEC_LWM2M_FIRMWARE_VERSION_SAVE_TEMP     "/data/lwm2m/firmwareversionTemp.dat"
#define QUEC_LWM2M_FW_UPDATE_EXECUTE_FALG_SAVE     "/data/lwm2m/firmwareupexecuteflag.dat"
//add by joe for vzw network end
#define LWM2M_FIRMWARE_OBJ_PERSISTENCE_PATH "/data/lwm2m/firmware_obj_delete_pr"
#define REGISTRATION_STATUS_PERSISTENCE_FILE  "/data/lwm2m/m2m_regstatus"
/* Application extensible object persistence file */ 
#define LWM2M_APP_OBJ_PERSISTENCE_PATH "/data/lwm2m/application_obj_pr" 
/* Application persist re-bootstrapping configuration */
#define LWM2M_APP_REBOOTSTRAP_PERSISTENCE_PATH "/data/lwm2m/application_rebootstrap_pr"
#define BOOTSTRAP_STATUS_PERSISTENCE_FILE     "/data/lwm2m/m2m_bststus"
#define SMS_RATE_LIMIT_FILE                   "/data/lwm2m/sms_rate"
/* ExtObj that is registered*/
#define LWM2M_REGISTERED_EXTOBJ_PERSISTENCE_FILE  "/data/lwm2m/registed_ext_obj_pr"

#define QUEC_LWM2M_VZW_100_SERVER_URL_FILE     "/data/lwm2m/100_url_server.url"
#define QUEC_LWM2M_VZW_101_SERVER_URL_FILE     "/data/lwm2m/101_url_server.url"
#define QUEC_LWM2M_VZW_102_SERVER_URL_FILE     "/data/lwm2m/102_url_server.url"
#define QUEC_LWM2M_VZW_1000_SERVER_URL_FILE     "/data/lwm2m/1000_url_server.url"


#define LWM2M_RETRY_TIMEOUT_MAX_VAL   (1+2+4+8+16)*60   
#define LWM2M_RETRY_TIMEOUT_INIT_VAL 60//unit:sec
#define LWM2M_RETRY_TIMEOUT_EXP_VAL  2

#define MAX_RES_INSTANCES 100
#define INVALID_RES_INST_ID 0xFF

#define LWM2M_APN_CLASS2    2
#define LWM2M_APN_CLASS3    3
#define LWM2M_APN_CLASS6    6
#define LWM2M_APN_CLASS7    7

#define VZW_CLASS2_APN_IFNAME "rmnet_data0"
#define VZW_CLASS3_APN_IFNAME "rmnet_data1"

#define MAX_STRING_LEN 64
#define GPS_TO_UNIX_TIME_OFFSET   315964800
#define DEFAULT_SESSION_RESUMPTION_TIME 59

#define LWM2M_MAX_LOG_SIZE 20480 //20M
#define LWM2M_MAX_LOG_COUNT 4

#define LWM2M_SERVER_DEFAULT_DISABLE_TIMEOUT  86400

#define LWM2M_VZW_DIAGNOSTIC_SERVER_INSTANCEID 2
#define LWM2M_VZW_DIAGNOSTIC_SHORT_SERVERID 101

#define LWM2M_SERVER_BS_RETRY_TIMEOUT  86400

time_t g_atc_apn_changed_time ;

#define ATC_REQ_CMD_MAX_LEN     100
#define ATC_RESP_CMD_MAX_LEN    2048

#define SEC_TO_MSEC(s)    ((s) * 1000ULL)
#define USEC_TO_MSEC(s)   ((s) / 1000ULL)
#define NSEC_TO_MSEC(s)   ((s) / 1000000ULL)

//add by joe end

/*
 * Platform abstraction functions to be implemented by the user
 */
#define LWM2M_STRCPY(d,s,n) strlcpy(d,s,n)
#define FALSE	false
#define TRUE 	true
#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef memscpy
#define memscpy(dst, dst_size, src, bytes_to_copy) (void) memcpy(dst, src, MIN(dst_size, bytes_to_copy))
#endif



#ifndef LWM2M_MEMORY_TRACE
// Allocate a block of size bytes of memory, returning a pointer to the beginning of the block.
void * lwm2m_malloc(size_t s);
// Deallocate a block of memory previously allocated by lwm2m_malloc() or lwm2m_strdup()
void lwm2m_free(void * p);
// Allocate a memory block, duplicate the string str in it and return a pointer to this new block.
char * lwm2m_strdup(const char * str);
#else
// same functions as above with caller location for debugging purposes
char * lwm2m_trace_strdup(const char * str, const char * file, const char * function, int lineno);
void * lwm2m_trace_malloc(size_t size, const char * file, const char * function, int lineno);
void    lwm2m_trace_free(void * mem, const char * file, const char * function, int lineno);

#define lwm2m_strdup(S) lwm2m_trace_strdup(S, __FILE__, __FUNCTION__, __LINE__)
#define lwm2m_malloc(S) lwm2m_trace_malloc(S, __FILE__, __FUNCTION__, __LINE__)
#define lwm2m_free(M)   lwm2m_trace_free(M, __FILE__, __FUNCTION__, __LINE__)
#endif
// Compare at most the n first bytes of s1 and s2, return 0 if they match
int lwm2m_strncmp(const char * s1, const char * s2, size_t n);
// This function must return the number of seconds elapsed since origin.
// The origin (Epoch, system boot, etc...) does not matter as this
// function is used only to determine the elapsed time since the last
// call to it.
// In case of error, this must return a negative value.
// Per POSIX specifications, time_t is a signed integer.
time_t lwm2m_gettime(void);

#ifdef LWM2M_WITH_LOGS
// Same usage as C89 printf()
void lwm2m_printf(const char * format, ...);
#endif

//add by joe start
int lwm2m_read_from_efs_file(    const char * name, void ** buffer_ptr, size_t * buffer_size);

bool lwm2m_string2hex(char *str, unsigned char **hex_array, int *hex_array_len);
//add by joe end
// communication layer
#ifdef LWM2M_CLIENT_MODE
// Returns a session handle that MUST uniquely identify a peer.
// secObjInstID: ID of the Securty Object instance to open a connection to
// userData: parameter to lwm2m_init()
void * lwm2m_connect_server(uint16_t secObjInstID, void * userData);
// Close a session created by lwm2m_connect_server()
// sessionH: session handle identifying the peer (opaque to the core)
// userData: parameter to lwm2m_init()
void lwm2m_close_connection(void * sessionH, void * userData);
#endif
// Send data to a peer
// Returns COAP_NO_ERROR or a COAP_NNN error code
// sessionH: session handle identifying the peer (opaque to the core)
// buffer, length: data to send
// userData: parameter to lwm2m_init()
uint8_t lwm2m_buffer_send(void * sessionH, uint8_t * buffer, size_t length, void * userData);
// Compare two session handles
// Returns true if the two sessions identify the same peer. false otherwise.
// userData: parameter to lwm2m_init()
bool lwm2m_session_is_equal(void * session1, void * session2, void * userData);

#define LWM2M_MEMCPY(d,dn,s,cn) memscpy(d,dn,s,cn)
/*
 * Error code
 */

#define COAP_NO_ERROR                   (uint8_t)0x00
#define COAP_IGNORE                     (uint8_t)0x01

#define COAP_201_CREATED                (uint8_t)0x41
#define COAP_202_DELETED                (uint8_t)0x42
#define COAP_204_CHANGED                (uint8_t)0x44
#define COAP_205_CONTENT                (uint8_t)0x45
#define COAP_231_CONTINUE               (uint8_t)0x5F
#define COAP_400_BAD_REQUEST            (uint8_t)0x80
#define COAP_401_UNAUTHORIZED           (uint8_t)0x81
#define COAP_402_BAD_OPTION             (uint8_t)0x82
#define COAP_404_NOT_FOUND              (uint8_t)0x84
#define COAP_405_METHOD_NOT_ALLOWED     (uint8_t)0x85
#define COAP_406_NOT_ACCEPTABLE         (uint8_t)0x86
#define COAP_408_REQ_ENTITY_INCOMPLETE  (uint8_t)0x88
#define COAP_412_PRECONDITION_FAILED    (uint8_t)0x8C
#define COAP_413_ENTITY_TOO_LARGE       (uint8_t)0x8D
#define COAP_500_INTERNAL_SERVER_ERROR  (uint8_t)0xA0
#define COAP_501_NOT_IMPLEMENTED        (uint8_t)0xA1
#define COAP_503_SERVICE_UNAVAILABLE    (uint8_t)0xA3

/*
 * Standard Object IDs
 */
#define LWM2M_SECURITY_OBJECT_ID            0
#define LWM2M_SERVER_OBJECT_ID              1
#define LWM2M_ACL_OBJECT_ID                 2
#define LWM2M_DEVICE_OBJECT_ID              3
#define LWM2M_CONN_MONITOR_OBJECT_ID        4
#define LWM2M_FIRMWARE_UPDATE_OBJECT_ID     5
#define LWM2M_LOCATION_OBJECT_ID            6
#define LWM2M_CONN_STATS_OBJECT_ID          7
#define LWM2M_SOFTWARE_MGNT_OBJECT_ID       9
#define LWM2M_OSCORE_OBJECT_ID             21
#define LWM2M_APN_CONN_PROFILE_ID          11
#define LWM2M_DEVICE_CAP_OBJECT_ID          15
#define LWM2M_STANDARD_OBJECT_ID_MAX        LWM2M_DEVICE_CAP_OBJECT_ID
#define LWM2M_HOST_DEVICE_OBJECT_ID	       16//10241//10255	//Added for host device object
#define LWM2M_VZW_HOST_DEVICE_OBJECT_ID         10241
#define LWM2M_CONN_EXTEN_OBJECT_ID	        10308				//Added for Connectivity Extension object
#define LWM2M_IS_STANDARD_OBJECT(object_id) ((((int32_t)object_id >= 0x00) && \
                                                (object_id <= 0x07)) || \
                                              (LWM2M_SOFTWARE_MGNT_OBJECT_ID == object_id) || \
                                              (LWM2M_APN_CONN_PROFILE_ID == object_id) || \
                                              (LWM2M_HOST_DEVICE_OBJECT_ID == object_id) || \
										      (LWM2M_CONN_EXTEN_OBJECT_ID == object_id))
/*
 * Resource IDs for the LWM2M Security Object
 */
#define LWM2M_SECURITY_URI_ID                 0
#define LWM2M_SECURITY_BOOTSTRAP_ID           1
#define LWM2M_SECURITY_SECURITY_ID            2
#define LWM2M_SECURITY_PUBLIC_KEY_ID          3
#define LWM2M_SECURITY_SERVER_PUBLIC_KEY_ID   4
#define LWM2M_SECURITY_SECRET_KEY_ID          5
#define LWM2M_SECURITY_SMS_SECURITY_ID        6
#define LWM2M_SECURITY_SMS_KEY_PARAM_ID       7
#define LWM2M_SECURITY_SMS_SECRET_KEY_ID      8
#define LWM2M_SECURITY_SMS_SERVER_NUMBER_ID   9
#define LWM2M_SECURITY_SHORT_SERVER_ID        10
#define LWM2M_SECURITY_HOLD_OFF_ID            11
#define LWM2M_SECURITY_BOOTSTRAP_TIMEOUT_ID   12
#define LWM2M_SECURITY_RESOURCE_30000         30000 //add by joe for vzw

/*
 * Resource IDs for the LWM2M Server Object
 */
#define LWM2M_SERVER_SHORT_ID_ID              0
#define LWM2M_SERVER_LIFETIME_ID              1
#define LWM2M_SERVER_MIN_PERIOD_ID            2
#define LWM2M_SERVER_MAX_PERIOD_ID            3
#define LWM2M_SERVER_DISABLE_ID               4
#define LWM2M_SERVER_TIMEOUT_ID               5
#define LWM2M_SERVER_STORING_ID               6
#define LWM2M_SERVER_BINDING_ID               7
#define LWM2M_SERVER_UPDATE_ID                8
#define LWM2M_SERVER_BOOTSTRAP_ID             9
#define LWM2M_SERVER_APN_ID                  10
#define LWM2M_SERVER_TLS_ALERT_CODE_ID       11
#define LWM2M_SERVER_LAST_BOOTSTRAP_ID       12
#define LWM2M_SERVER_REG_ORDER_ID            13
#define LWM2M_SERVER_INITIAL_REG_DELAY_ID    14
#define LWM2M_SERVER_REG_FAIL_BLOCK_ID       15
#define LWM2M_SERVER_REG_FAIL_BOOTSTRAP_ID   16
#define LWM2M_SERVER_COMM_RETRY_COUNT_ID     17
#define LWM2M_SERVER_COMM_RETRY_TIMER_ID     18
#define LWM2M_SERVER_SEQ_DELAY_TIMER_ID      19
#define LWM2M_SERVER_SEQ_RETRY_COUNT_ID      20
#define LWM2M_SERVER_TRIGGER_ID              21
#define LWM2M_SERVER_PREFERRED_TRANSPORT_ID  22
#define LWM2M_SERVER_MUTE_SEND_ID            23
#define LWM2M_SERVER_RESOURCE_30000         30000  //add by joe for vzw

#define LWM2M_SECURITY_MODE_PRE_SHARED_KEY  0
#define LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY  1
#define LWM2M_SECURITY_MODE_CERTIFICATE     2
#define LWM2M_SECURITY_MODE_NONE            3

#define MAX_STRING_LEN 64

#define RES_M_REBOOT                4
#define RES_O_FACTORY_RESET         5

#define VZW_REGIST_HOLD_OFF_TIMER 30

/*
 * Utility functions for sorted linked list
 */

typedef struct _lwm2m_list_t
{
    struct _lwm2m_list_t * next;
    uint16_t    id;
} lwm2m_list_t;

// defined in list.c
// Add 'node' to the list 'head' and return the new list
lwm2m_list_t * lwm2m_list_add(lwm2m_list_t * head, lwm2m_list_t * node);
// Return the node with ID 'id' from the list 'head' or NULL if not found
lwm2m_list_t * lwm2m_list_find(lwm2m_list_t * head, uint16_t id);
// Remove the node with ID 'id' from the list 'head' and return the new list
lwm2m_list_t * lwm2m_list_remove(lwm2m_list_t * head, uint16_t id, lwm2m_list_t ** nodeP);
// Return the lowest unused ID in the list 'head'
uint16_t lwm2m_list_newId(lwm2m_list_t * head);
// Free a list. Do not use if nodes contain allocated pointers as it calls lwm2m_free on nodes only.
// If the nodes of the list need to do more than just "free()" their instances, don't use lwm2m_list_free().
void lwm2m_list_free(lwm2m_list_t * head);

#define LWM2M_LIST_ADD(H,N) lwm2m_list_add((lwm2m_list_t *)H, (lwm2m_list_t *)N);
#define LWM2M_LIST_RM(H,I,N) lwm2m_list_remove((lwm2m_list_t *)H, I, (lwm2m_list_t **)N);
#define LWM2M_LIST_FIND(H,I) lwm2m_list_find((lwm2m_list_t *)H, I)
#define LWM2M_LIST_FREE(H) lwm2m_list_free((lwm2m_list_t *)H)

/*
 * URI
 *
 * objectId is always set
 * instanceId or resourceId are set according to the flag bit-field
 *
 */

#define LWM2M_MAX_ID   ((uint16_t)0xFFFF)

//add by joe start
#define LWM2M_URI_FLAG_RESOURCE_INST_ID  (uint8_t)0x08

#define LWM2M_URI_FLAG_OBJECT_ID         (uint8_t)0x01
#define LWM2M_URI_FLAG_INSTANCE_ID       (uint8_t)0x02
#define LWM2M_URI_FLAG_RESOURCE_ID       (uint8_t)0x04
#define LWM2M_URI_FLAG_RESOURCE_INST_ID  (uint8_t)0x08
//add by joe end

#define LWM2M_URI_IS_SET_OBJECT(uri) ((uri)->objectId != LWM2M_MAX_ID)
#define LWM2M_URI_IS_SET_INSTANCE(uri) ((uri)->instanceId != LWM2M_MAX_ID)
#define LWM2M_URI_IS_SET_RESOURCE(uri) ((uri)->resourceId != LWM2M_MAX_ID)
#ifndef LWM2M_VERSION_1_0
#define LWM2M_URI_IS_SET_RESOURCE_INSTANCE(uri) ((uri)->resourceInstanceId != LWM2M_MAX_ID)
#endif

typedef struct
{
    uint8_t     flag;           // indicates which segments are set
    uint16_t    objectId;
    uint16_t    instanceId;
    uint16_t    resourceId;
//#ifndef LWM2M_VERSION_1_0
    uint16_t    resourceInstanceId;
//#endif
} lwm2m_uri_t;

#define LWM2M_URI_RESET(uri) memset((uri), 0xFF, sizeof(lwm2m_uri_t))

#define LWM2M_STRING_ID_MAX_LEN 6

// Parse an URI in LWM2M format and fill the lwm2m_uri_t.
// Return the number of characters read from buffer or 0 in case of error.
// Valid URIs: /1, /1/, /1/2, /1/2/, /1/2/3
// Invalid URIs: /, //, //2, /1//, /1//3, /1/2/3/, /1/2/3/4
int lwm2m_stringToUri(const char * buffer, size_t buffer_len, lwm2m_uri_t * uriP);

/*
 * The lwm2m_data_t is used to store LWM2M resource values in a hierarchical way.
 * Depending on the type the value is different:
 * - LWM2M_TYPE_OBJECT, LWM2M_TYPE_OBJECT_INSTANCE, LWM2M_TYPE_MULTIPLE_RESOURCE: value.asChildren
 * - LWM2M_TYPE_STRING, LWM2M_TYPE_OPAQUE, LWM2M_TYPE_CORE_LINK: value.asBuffer
 * - LWM2M_TYPE_INTEGER, LWM2M_TYPE_TIME: value.asInteger
 * - LWM2M_TYPE_UNSIGNED_INTEGER: value.asUnsigned
 * - LWM2M_TYPE_FLOAT: value.asFloat
 * - LWM2M_TYPE_BOOLEAN: value.asBoolean
 *
 * LWM2M_TYPE_STRING is also used when the data is in text format.
 */

typedef enum
{
    LWM2M_TYPE_UNDEFINED = 0,
    LWM2M_TYPE_OBJECT,
    LWM2M_TYPE_OBJECT_INSTANCE,
    LWM2M_TYPE_MULTIPLE_RESOURCE,

    LWM2M_TYPE_STRING,
    LWM2M_TYPE_OPAQUE,
    LWM2M_TYPE_INTEGER,
    LWM2M_TYPE_UNSIGNED_INTEGER,
    LWM2M_TYPE_FLOAT,
    LWM2M_TYPE_BOOLEAN,

    LWM2M_TYPE_OBJECT_LINK,
    LWM2M_TYPE_CORE_LINK
} lwm2m_data_type_t;

typedef struct _lwm2m_data_t lwm2m_data_t;

struct _lwm2m_data_t
{
    lwm2m_data_type_t type;
    uint16_t    id;
    union
    {
        bool        asBoolean;
        int64_t     asInteger;
        uint64_t    asUnsigned;
        double      asFloat;
        struct
        {
            size_t    length;
            uint8_t * buffer;
			uint8_t   block1_more;   /**< More blocks to be received. */
      		uint32_t  block1_num;    /**< Block number */                                             
      		uint16_t  block1_size;   /**< Block size */                         
      		uint32_t  block1_offset; /**< Block offset */
      		uint32_t  size1;         /** < Size indication */             
        } asBuffer;
        struct
        {
            size_t         count;
            lwm2m_data_t * array;
        } asChildren;
        struct
        {
            uint16_t objectId;
            uint16_t objectInstanceId;
        } asObjLink;
    } value;
};

typedef enum
{
    LWM2M_CONTENT_TEXT       = 0,        // Also used as undefined
    LWM2M_CONTENT_LINK       = 40,
    LWM2M_CONTENT_OPAQUE     = 42,
    LWM2M_CONTENT_TLV_OLD    = 1542,     // Keep old value for backward-compatibility
    LWM2M_CONTENT_TLV        = 11542,
    LWM2M_CONTENT_JSON_OLD   = 1543,     // Keep old value for backward-compatibility
    LWM2M_CONTENT_JSON       = 11543,
    LWM2M_CONTENT_SENML_JSON = 110
} lwm2m_media_type_t;

lwm2m_data_t * lwm2m_data_new(int size);
int lwm2m_data_parse(lwm2m_uri_t * uriP, const uint8_t * buffer, size_t bufferLen, lwm2m_media_type_t format, lwm2m_data_t ** dataP);
int lwm2m_data_serialize(lwm2m_uri_t * uriP, int size, lwm2m_data_t * dataP, lwm2m_media_type_t * formatP, uint8_t ** bufferP);
void lwm2m_data_free(int size, lwm2m_data_t * dataP);

int lwm2m_data_decode_string(const lwm2m_data_t * dataP, char * string, size_t length);

void lwm2m_data_encode_string(const char * string, lwm2m_data_t * dataP);
void lwm2m_data_encode_nstring(const char * string, size_t length, lwm2m_data_t * dataP);
void lwm2m_data_encode_opaque(const uint8_t * buffer, size_t length, lwm2m_data_t * dataP);
void lwm2m_data_encode_int(int64_t value, lwm2m_data_t * dataP);
int lwm2m_data_decode_int(const lwm2m_data_t * dataP, int64_t * valueP);
void lwm2m_data_encode_uint(uint64_t value, lwm2m_data_t * dataP);
int lwm2m_data_decode_uint(const lwm2m_data_t * dataP, uint64_t * valueP);
void lwm2m_data_encode_float(double value, lwm2m_data_t * dataP);
int lwm2m_data_decode_float(const lwm2m_data_t * dataP, double * valueP);
void lwm2m_data_encode_bool(bool value, lwm2m_data_t * dataP);
int lwm2m_data_decode_bool(const lwm2m_data_t * dataP, bool * valueP);
void lwm2m_data_encode_objlink(uint16_t objectId, uint16_t objectInstanceId, lwm2m_data_t * dataP);
void lwm2m_data_encode_corelink(const char * corelink, lwm2m_data_t * dataP);
void lwm2m_data_encode_instances(lwm2m_data_t * subDataP, size_t count, lwm2m_data_t * dataP);
void lwm2m_data_include(lwm2m_data_t * subDataP, size_t count, lwm2m_data_t * dataP);


/*
 * Utility function to parse TLV buffers directly
 *
 * Returned value: number of bytes parsed
 * buffer: buffer to parse
 * buffer_len: length in bytes of buffer
 * oType: (OUT) type of the parsed TLV record. can be:
 *          - LWM2M_TYPE_OBJECT
 *          - LWM2M_TYPE_OBJECT_INSTANCE
 *          - LWM2M_TYPE_MULTIPLE_RESOURCE
 *          - LWM2M_TYPE_OPAQUE
 * oID: (OUT) ID of the parsed TLV record
 * oDataIndex: (OUT) index of the data of the parsed TLV record in the buffer
 * oDataLen: (OUT) length of the data of the parsed TLV record
 */

#define LWM2M_TLV_HEADER_MAX_LENGTH 6

int lwm2m_decode_TLV(const uint8_t * buffer, size_t buffer_len, lwm2m_data_type_t * oType, uint16_t * oID, size_t * oDataIndex, size_t * oDataLen);

typedef uint32_t objlink_t;

typedef struct lwm2m_id_info_t_s
{
  struct lwm2m_id_info_t_s *next;  /**< Pointer to the next ID information. */
  uint8_t id_set;       /**< ID category defined in qapi_lwm2m_id. */
  uint16_t object_ID;   /**< Object ID. */
  uint16_t instance_ID;  /**< Object instance ID. */
  uint16_t resource_ID;  /**< Resource ID. */
} lwm2m_id_info_t;

typedef struct lwm2m_object_info_t_s
{
  uint16_t no_object_info;             /**< Number of object information blocks. */
  lwm2m_id_info_t *id_info;  /**< Pointer to the ID information. */
} lwm2m_object_info_t;

typedef struct _lwm2m_resource_info_t
{
  struct _lwm2m_resource_info_t *next;  /**< Pointer to the next resource information. */
  uint16_t resource_ID;                         /**< Resource ID. */
  lwm2m_data_type_t type;             /**< Type of resource. */  
  /** Union of resource values. */
  union
  {
    bool asBoolean;      /**< Value in Boolean format. */
    int64_t asInteger;   /**< Value as an integer. */
    double asFloat;      /**< Value as a floating point. */
    objlink_t asObjLink; /**< Value as a object link. */    
    struct
    {
      size_t length;          /**< String length. */
      uint8_t *buffer;        /**< Pointer to the string buffer. */
      uint8_t  block1_more;   /**< More blocks to be received. */
      uint32_t block1_num;    /**< Block number */                                             
      uint16_t block1_size;   /**< Block size */                         
      uint32_t block1_offset; /**< Block offset */
      uint32_t size1;         /** < Size indication */
    } asBuffer;  /**< Value as a string. */
    struct
    {
      size_t        count;  /**< Number of resources in the array. */
      lwm2m_data_t  *array;  /**< Array of resources. */ 
    } asChildren;  /**< Value as a multi-resource instance */
  } value;  
} lwm2m_resource_info_t;

typedef struct _lwm2m_instance_info_t
{
  struct _lwm2m_instance_info_t *next;  /**< Pointer to the next object instance. */
  uint16_t instance_ID;    /**< Instance ID. */
  uint16_t no_resources;   /**< Number of resources. */
  lwm2m_resource_info_t * resource_info;  /**< Pointer to the resource information. */
} lwm2m_instance_info_t;

typedef struct _lwm2m_object_data_t
{
  struct _lwm2m_object_data_t *next;  /**< Pointer to the next object data. */
  uint16_t object_ID;    /**< Object ID. */
  uint16_t no_instances;  /**< Number of instances. */
  lwm2m_instance_info_t *instance_info;  /**< Pointer to the instance information. */
} lwm2m_object_data_t;

/*
 * LWM2M Objects
 *
 * For the read callback, if *numDataP is not zero, *dataArrayP is pre-allocated
 * and contains the list of resources to read.
 *
 */

typedef struct _lwm2m_object_t lwm2m_object_t;

typedef uint8_t (*lwm2m_read_callback_t) (uint16_t instanceId, int * numDataP, lwm2m_data_t ** dataArrayP, lwm2m_object_t * objectP);
typedef uint8_t (*lwm2m_discover_callback_t) (uint16_t instanceId, int * numDataP, lwm2m_data_t ** dataArrayP, lwm2m_object_t * objectP);
typedef uint8_t (*lwm2m_write_callback_t) (uint16_t instanceId, int numData, lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
typedef uint8_t (*lwm2m_execute_callback_t) (uint16_t instanceId, uint16_t resourceId, uint8_t * buffer, int length, lwm2m_object_t * objectP);
typedef uint8_t (*lwm2m_create_callback_t) (uint16_t instanceId, int numData, lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
typedef uint8_t (*lwm2m_delete_callback_t) (uint16_t instanceId, lwm2m_object_t * objectP);

typedef int32_t (*lwm2m_setvalue_callback_t) (void *ctx_p,lwm2m_object_data_t * lwm2m_data , lwm2m_object_t * targetP, lwm2m_uri_t *uriP);

struct _lwm2m_object_t
{
    struct _lwm2m_object_t * next;           // for internal use only.
    uint16_t       objID;
    lwm2m_list_t * instanceList;
    lwm2m_read_callback_t     readFunc;
    lwm2m_write_callback_t    writeFunc;
    lwm2m_execute_callback_t  executeFunc;
    lwm2m_create_callback_t   createFunc;
    lwm2m_delete_callback_t   deleteFunc;
    lwm2m_discover_callback_t discoverFunc;
    lwm2m_setvalue_callback_t setValueFunc;
    void * userData;
    void *ctxP;
};

/*
 * LWM2M Servers
 *
 * Since LWM2M Server Object instances are not accessible to LWM2M servers,
 * there is no need to store them as lwm2m_objects_t
 */

typedef enum
{
    STATE_DEREGISTERED = 0,        // not registered or boostrap not started
    STATE_REG_HOLD_OFF,            // initial registration delay or delay between retries
    STATE_REG_PENDING,             // registration pending
    STATE_REGISTERED,              // successfully registered
    STATE_REG_FAILED,              // last registration failed
    STATE_REG_UPDATE_PENDING,      // registration update pending
    STATE_REG_UPDATE_NEEDED,       // registration update required
    STATE_REG_FULL_UPDATE_NEEDED,  // registration update with objects required
    STATE_DEREG_PENDING,           // deregistration pending
    STATE_BS_HOLD_OFF,             // bootstrap hold off time
    STATE_BS_INITIATED,            // bootstrap request sent
    STATE_BS_PENDING,              // boostrap ongoing
    STATE_BS_FINISHING,            // boostrap finish received
    STATE_BS_FINISHED,             // bootstrap done
    STATE_BS_FAILING,              // bootstrap error occurred
    STATE_BS_FAILED,               // bootstrap failed
} lwm2m_status_t;

typedef enum
{
    VERSION_MISSING = 0,  // Version number not in registration.
    VERSION_UNRECOGNIZED, // Version number in registration not recognized.
    VERSION_1_0,          // LWM2M version 1.0
    VERSION_1_1,          // LWM2M version 1.1
} lwm2m_version_t;

#define BINDING_UNKNOWN 0x01
#define BINDING_U       0x02 // UDP
#define BINDING_T       0x04 // TCP
#define BINDING_S       0x08 // SMS
#define BINDING_N       0x10 // Non-IP
#define BINDING_Q       0x20 // queue mode
/* Legacy bindings */
#define BINDING_UQ (BINDING_U|BINDING_Q) // UDP queue mode
#define BINDING_SQ (BINDING_S|BINDING_Q) // SMS queue mode
#define BINDING_US (BINDING_U|BINDING_S) // UDP plus SMS
#define BINDING_UQS (BINDING_U|BINDING_Q|BINDING_S) // UDP queue mode plus SMS
typedef uint8_t lwm2m_binding_t;

/*
 * LWM2M block1 data
 *
 * Temporary data needed to handle block1 request.
 * Currently support only one block1 request by server.
 */
typedef struct _lwm2m_block1_data_ lwm2m_block1_data_t;

struct _lwm2m_block1_data_
{
    uint8_t *             block1buffer;     // data buffer
    size_t                block1bufferSize; // buffer size
    uint16_t              lastmid;          // mid of the last message received
};

typedef struct _lwm2m_server_
{
    struct _lwm2m_server_ * next;         // matches lwm2m_list_t::next
    uint16_t                secObjInstID; // matches lwm2m_list_t::id
    uint16_t                shortID;      // servers short ID, may be 0 for bootstrap server
    time_t                  lifetime;     // lifetime of the registration in sec or 0 if default value (86400 sec), also used as hold off time for bootstrap servers
    time_t                  registration; // date of the last registration in sec or end of client hold off time for bootstrap servers or end of hold off time for registration holds.
    lwm2m_binding_t         binding;      // client connection mode with this server
    void *                  sessionH;
    lwm2m_status_t          status;
    char *                  location;
    bool                    dirty;
    lwm2m_block1_data_t *   block1Data;   // buffer to handle block1 data, should be replace by a list to support several block1 transfer by server.
    time_t                  disabled;
    bool                    disable;
	time_t			        disable_timeOut;// disable timeout of a server
#ifndef LWM2M_VERSION_1_0
    uint16_t                servObjInstID;// Server object instance ID if not a bootstrap server.
    uint8_t                 attempt;      // Current registration attempt
    uint8_t                 sequence;     // Current registration sequence
#endif
//add by joe start
    bool                    storing;
    uint32_t                defaultMinPeriod;   // used to set Min Period of observation
    uint32_t                defaultMaxPeriod;   //// used to set Max Period of observation
    bool                    isBootstrapped;
    bool                    isRegistered;
    bool                    isRegisterUpdateing;
    bool                    isRegupdate_for_sms;
    bool                    pending_reg_update;
    bool                    network_down ; //add by joe mark network status
    int                     register_retry_count;
    time_t                  register_next_try_timer;
    time_t                  registration_holderoff_timer;
//add by joe end	
    
} lwm2m_server_t;

#define CARRIER_APN_LEN 128 //add by joe

typedef struct _lwm2m_context_ lwm2m_context_t;

/*
 * LWM2M result callback
 *
 * When used with an observe, if 'data' is not nil, 'status' holds the observe counter.
 */
typedef void (*lwm2m_result_callback_t) (uint16_t clientID, lwm2m_uri_t * uriP, int status, lwm2m_media_type_t format, uint8_t * data, int dataLength, void * userData);

/*
 * LWM2M Observations
 *
 * Used to store latest user operation on the observation of remote clients resources.
 * Any node in the observation list means observation was established with client already.
 * status STATE_REG_PENDING means the observe request was sent to the client but not yet answered.
 * status STATE_REGISTERED means the client acknowledged the observe request.
 * status STATE_DEREG_PENDING means the user canceled the request before the client answered it.
 */

typedef struct _lwm2m_observation_
{
    struct _lwm2m_observation_ * next;  // matches lwm2m_list_t::next
    uint16_t                     id;    // matches lwm2m_list_t::id
    struct _lwm2m_client_ * clientP;
    lwm2m_uri_t             uri;
    lwm2m_status_t          status;     // latest user operation
    lwm2m_result_callback_t callback;
    void *                  userData;
} lwm2m_observation_t;

/*
 * LWM2M Link Attributes
 *
 * Used for observation parameters.
 *
 */

#define LWM2M_ATTR_FLAG_MIN_PERIOD      (uint8_t)0x01
#define LWM2M_ATTR_FLAG_MAX_PERIOD      (uint8_t)0x02
#define LWM2M_ATTR_FLAG_GREATER_THAN    (uint8_t)0x04
#define LWM2M_ATTR_FLAG_LESS_THAN       (uint8_t)0x08
#define LWM2M_ATTR_FLAG_STEP            (uint8_t)0x10

typedef enum lwm2m_id_t_e
{
  LWM2M_OBJECT_ID_E             = 0x01,  /**< Object ID. */
  LWM2M_INSTANCE_ID_E           = 0x02,  /**< Instance ID. */
  LWM2M_RESOURCE_ID_E           = 0x04,  /**< Resource ID. */
  LWM2M_RESOURCE_INSTANCE_ID_E  = 0x08,  /**< Resource instance ID. */
} lwm2m_id_t;
// typedef struct
// {
    // uint8_t     toSet;
    // uint8_t     toClear;
    // uint32_t    minPeriod;
    // uint32_t    maxPeriod;
    // double      greaterThan;
    // double      lessThan;
    // double      step;
// } lwm2m_attributes_t;

typedef enum
{
  LWM2M_READ_REQ_E                  = 1,   /**< Read request. */
  LWM2M_WRITE_REPLACE_REQ_E         = 2,   /**< Write replace request. */
  LWM2M_WRITE_PARTIAL_UPDATE_REQ_E  = 3,   /**< Write partial update request. */
  LWM2M_WRITE_ATTR_REQ_E            = 4,   /**< Write attribute request. */
  LWM2M_DISCOVER_REQ_E              = 5,   /**< Discover request. */
  LWM2M_EXECUTE_REQ_E               = 6,   /**< Execute request. */
  LWM2M_DELETE_REQ_E                = 7,   /**< Delete request. */
  LWM2M_OBSERVE_REQ_E               = 8,   /**< Observe request. */
  LWM2M_CANCEL_OBSERVE_REQ_E        = 9,   /**< Cancel observe request. */
  LWM2M_ACK_MSG_E                   = 10,  /**< Acknowledge message. */
  LWM2M_INTERNAL_CLIENT_IND_E       = 11,  /**< Internal client indication. */
  LWM2M_CREATE_REQ_E                = 12,  /**< Create request. */
  LWM2M_DELETE_ALL_REQ_E            = 13,  /**< Delete all request. */
} lwm2m_msg_type;

typedef enum
{
  LWM2M_MIN_PERIOD_E   = 1,	 /**< Minimum period. */
  LWM2M_MAX_PERIOD_E   = 2,	 /**< Maximum period. */
  LWM2M_GREATER_THAN_E = 4,	 /**< Greater than. */
  LWM2M_LESS_THAN_E	  = 8,	 /**< Less than. */
  LWM2M_STEP_E		  = 16,  /**< Step. */
  LWM2M_DIM_E		  = 32,  /**< Dimension. */
} lwm2m_write_attr_t;

typedef enum
{
  LWM2M_TEXT_PLAIN                    = 0,      /**< Plain text. */
  LWM2M_TEXT_XML                      = 1,      /**< XML text. */
  LWM2M_TEXT_CSV                      = 2,      /**< CSV text. */
  LWM2M_TEXT_HTML                     = 3,      /**< HTML text. */
  LWM2M_APPLICATION_LINK_FORMAT       = 40,     /**< Application link format. */
  LWM2M_APPLICATION_XML               = 41,     /**< Application XML. */
  LWM2M_APPLICATION_OCTET_STREAM      = 42,     /**< Application Octet stream. */
  LWM2M_APPLICATION_RDF_XML           = 43,     /**< Application RDF XML. */
  LWM2M_APPLICATION_SOAP_XML          = 44,     /**< Application SOAP XML. */
  LWM2M_APPLICATION_ATOM_XML          = 45,     /**< Application ATOM XML. */
  LWM2M_APPLICATION_XMPP_XML          = 46,     /**< Application XMPP XML. */
  LWM2M_APPLICATION_EXI               = 47,     /**< Application EXI. */
  LWM2M_APPLICATION_FASTINFOSET       = 48,     /**< Application FastInfoSet. */
  LWM2M_APPLICATION_SOAP_FASTINFOSET  = 49,     /**< Application SOAP FastInfoSet. */
  LWM2M_APPLICATION_JSON              = 50,     /**< Application JSON. */
  LWM2M_APPLICATION_X_OBIX_BINARY     = 51,     /**< Application X OBIX binary. */
  LWM2M_M2M_TLV                       = 11542,  /**< M2M TLV. */
  LWM2M_M2M_JSON                      = 11543   /**< M2M JSON. */
}lwm2m_content_type_t;


typedef struct
{
  lwm2m_id_t             obj_mask;         /**< Bitmap indicating valid object fields. */
  uint16_t               obj_id;           /**< Object ID. */
  uint16_t               obj_inst_id;      /**< Object instance ID. */
  uint16_t               res_id;           /**< Resource ID. */
  uint16_t               res_inst_id;      /**< Resource instance ID. */
} lwm2m_obj_info_t;


typedef struct
{
	lwm2m_obj_info_t	  obj_info; 	  /**< LWM2M object information associated with write attributes. */
	lwm2m_write_attr_t    set_attr_mask;  /**< Bitmap indicating valid attribute fields to set. */
	lwm2m_write_attr_t    clr_attr_mask;  /**< Bitmap indicating attribute fields to clear. */
	uint8_t			      dim; 		   /**< Dimension. */

    uint8_t     toSet;
    uint8_t     toClear;
    uint32_t    minPeriod;
    uint32_t    maxPeriod;
    double      greaterThan;
    double      lessThan;
    double      step;
} lwm2m_attributes_t;

#define MAX_LWM2M_MSG_ID_LENGTH  10

typedef struct
{
  lwm2m_msg_type         msg_type;        /**< DL message type (requests, acknowledgements, or internal). */
  lwm2m_obj_info_t          obj_info;        /**< Object information. */
  uint8_t                   msg_id_len;      /**< Message ID length.  */
  uint8_t                   msg_id[MAX_LWM2M_MSG_ID_LENGTH];
  /**< Message ID. \n
   * The message ID is transparent to the application, but is passed to the application for every message
   * received from the server. The expectation is that the application stores the message ID associated
   * with the message and passes it to the LWM2M client when a response or notification must be sent to
   * the server. After the transaction pertaining to the message is complete, the message ID can be
   * discarded from the application. */
  uint16_t                        notification_id;
  /**< Notification ID.\n
   * When a notification is sent using qapi_Net_LWM2M_Send_Message(), the notification ID associated
   * with the message is returned to the caller. It is the caller's responsibility to maintain the
   * notification ID for observation mapping. Later, when the network does a Cancel Observation for
   * a particular notification using RESET, it is indicated using the notification ID to the caller.
   * Using this notification ID, the caller can cancel the observation. If the cancel observation was not using
   * RESET, obj_info should have the information based on the observation that is to be cancelled. */
  lwm2m_content_type_t            content_type;    /**< Current encoded data payload content type. */
  uint32_t                        payload_len;     /**< Encoded data payload length. */
  uint8_t                        *payload;         /**< Encoded data payload. */
  lwm2m_attributes_t             *lwm2m_attr;      /**< Write attributes. */
  bool                            accept_is_valid; /**< Flag to check accept field is set or not. */
  lwm2m_content_type_t            accept;          /**< Intended data payload content type. */
} lwm2m_server_data_t;

/*
 * LWM2M Clients
 *
 * Be careful not to mix lwm2m_client_object_t used to store list of objects of remote clients
 * and lwm2m_object_t describing objects exposed to remote servers.
 *
 */
typedef struct _aclInfo_t
{
  lwm2m_list_t   *objSpecificList;
  lwm2m_list_t   *instSpecificList;
} lwm2m_acl_info;

typedef struct _lwm2m_client_object_
{
    struct _lwm2m_client_object_ * next; // matches lwm2m_list_t::next
    uint16_t                 id;         // matches lwm2m_list_t::id
    lwm2m_list_t *           instanceList;
} lwm2m_client_object_t;

typedef struct _lwm2m_client_
{
    struct _lwm2m_client_ * next;       // matches lwm2m_list_t::next
    uint16_t                internalID; // matches lwm2m_list_t::id
    char *                  name;
    lwm2m_version_t         version;
    lwm2m_binding_t         binding;
    char *                  msisdn;
    char *                  altPath;
    lwm2m_media_type_t      format;
    uint32_t                lifetime;
    time_t                  endOfLife;
    void *                  sessionH;
    lwm2m_client_object_t * objectList;
    lwm2m_observation_t *   observationList;
    uint16_t                observationId;
} lwm2m_client_t;

typedef struct _dm_server_connect_info_
{
    char     endpointname[64];
    char     serverAddr[64];
    char     serverPort[64];
    char     pskId[64];
    char     psk[64];
    uint32_t pskLen;
    uint32_t lifetime;
    time_t   lastRegistrationTime;
    uint32_t diff;
    char                  location[125];
    mbedtls_ssl_session     saved_session;
} dm_server_connect_info;

/*
 * LWM2M transaction
 *
 * Adaptation of Erbium's coap_transaction_t
 */

typedef struct _lwm2m_transaction_ lwm2m_transaction_t;

typedef void (*lwm2m_transaction_callback_t) (lwm2m_context_t * contextP, lwm2m_transaction_t * transacP, void * message);

struct _lwm2m_transaction_
{
    lwm2m_transaction_t * next;  // matches lwm2m_list_t::next
    uint16_t              mID;   // matches lwm2m_list_t::id
    void *                peerH;
    uint8_t               ack_received; // indicates, that the ACK was received
    time_t                response_timeout; // timeout to wait for response, if token is used. When 0, use calculated acknowledge timeout.
    uint8_t  retrans_counter;
    time_t   retrans_time;
    void * message;
    uint16_t buffer_len;
    uint8_t * buffer;
    lwm2m_transaction_callback_t callback;
    void * userData;
};

/*
 * LWM2M observed resources
 */
typedef struct _lwm2m_watcher_
{
    struct _lwm2m_watcher_ * next;

    bool active;
    bool update;
    lwm2m_server_t * server;
    lwm2m_attributes_t * parameters;
    lwm2m_media_type_t format;
    uint8_t token[8];
    size_t tokenLen;
    time_t lastTime;
    uint32_t counter;
    uint16_t lastMid;
    union
    {
        int64_t asInteger;
        uint64_t asUnsigned;
        double  asFloat;
    } lastValue;
} lwm2m_watcher_t;

typedef struct _lwm2m_observed_
{
    struct _lwm2m_observed_ * next;

    lwm2m_uri_t uri;
    lwm2m_watcher_t * watcherList;
} lwm2m_observed_t;

#ifdef LWM2M_CLIENT_MODE

typedef enum
{
    STATE_INITIAL = 0,
    STATE_BOOTSTRAP_REQUIRED,
    STATE_BOOTSTRAPPING,
    STATE_REGISTER_REQUIRED,
    STATE_REGISTERING,
    STATE_READY
} lwm2m_client_state_t;

#endif

typedef struct{
	int     mask;//LSB0:apn_name;LSB1:estab_time; LSB2: estab_result;LSB3:estab_result;LSB4:estab_rej_cause;LSB5:release_time
	char    apn_name[256];
	int     estab_time;
	int     estab_result;
	int     estab_rej_cause;
	int     release_time;
}lwm2m_apn_conn_info_t;

typedef struct pers_resource_instance_int_list_s
{
  bool                    isValid;
  uint16_t                resInstId;
  uint16_t                InstValue;
} pers_resource_instance_int_list_t;

typedef struct resource_instance_int_list_s
{
  struct resource_instance_int_list_s*   next;
  uint16_t                resInstId;
  uint16_t                InstValue;
} resource_instance_int_list_t;

typedef struct resource_instance_string_list_s
{
  struct resource_instance_string_list_s*   next;
  uint16_t                resInstId;
  char                    InstValue[MAX_STRING_LEN];
} resource_instance_string_list_t;

/*
 * LWM2M Context
 */

#ifdef LWM2M_BOOTSTRAP_SERVER_MODE
// In all the following APIs, the session handle MUST uniquely identify a peer.

// LWM2M bootstrap callback
// When a LWM2M client requests bootstrap information, the callback is called with status COAP_NO_ERROR, uriP is nil and
// name is set. The callback must return a COAP_* error code. COAP_204_CHANGED for success.
// After a lwm2m_bootstrap_delete() or a lwm2m_bootstrap_write(), the callback is called with the status returned by the
// client, the URI of the operation (may be nil) and name is nil. The callback return value is ignored.
typedef int (*lwm2m_bootstrap_callback_t) (void * sessionH, uint8_t status, lwm2m_uri_t * uriP, char * name, void * userData);
#endif

struct _lwm2m_context_
{
#ifdef LWM2M_CLIENT_MODE
    lwm2m_client_state_t state;
    char *               endpointName;
    char *               msisdn;
    char *               altPath;
    lwm2m_server_t *     bootstrapServerList;
    lwm2m_server_t *     serverList;
    lwm2m_object_t *     objectList;
    lwm2m_observed_t *   observedList;
    dm_server_connect_info * connect_info;
    bool                 isColdStart;
#endif
#ifdef AUTHORIZATION_SUPPORT
	lwm2m_acl_info       aclInfo;
    lwm2m_server_t *     activeServerP;
#endif
#ifdef LWM2M_SERVER_MODE
    lwm2m_client_t *        clientList;
    lwm2m_result_callback_t monitorCallback;
    void *                  monitorUserData;
#endif
#ifdef LWM2M_BOOTSTRAP_SERVER_MODE
    lwm2m_bootstrap_callback_t bootstrapCallback;
    void *                     bootstrapUserData;
#endif
    uint16_t                nextMID;
    lwm2m_transaction_t *   transactionList;
    void *                  userData;
	int                     fota_fd;//add by joe
	int                     serverListCount;
	int                     bootstrapServerListCount;
};


// initialize a liblwm2m context.
lwm2m_context_t * lwm2m_init(void * userData);
// close a liblwm2m context.
void lwm2m_close(lwm2m_context_t * contextP);

// perform any required pending operation and adjust timeoutP to the maximal time interval to wait in seconds.
int lwm2m_step(lwm2m_context_t * contextP, time_t * timeoutP);
// dispatch received data to liblwm2m
void lwm2m_handle_packet(lwm2m_context_t * contextP, uint8_t * buffer, int length, void * fromSessionH);

#ifdef LWM2M_CLIENT_MODE
// configure the client side with the Endpoint Name, binding, MSISDN (can be nil), alternative path
// for objects (can be nil) and a list of objects.
// LWM2M Security Object (ID 0) must be present with either a bootstrap server or a LWM2M server and
// its matching LWM2M Server Object (ID 1) instance
int lwm2m_configure(lwm2m_context_t * contextP, const char * endpointName, const char * msisdn, const char * altPath, uint16_t numObject, lwm2m_object_t * objectList[]);
int lwm2m_add_object(lwm2m_context_t * contextP, lwm2m_object_t * objectP);
int lwm2m_remove_object(lwm2m_context_t * contextP, uint16_t id);

// send a registration update to the server specified by the server short identifier
// or all if the ID is 0.
// If withObjects is true, the registration update contains the object list.
int lwm2m_update_registration(lwm2m_context_t * contextP, uint16_t shortServerID, bool withObjects);

void lwm2m_resource_value_changed(lwm2m_context_t * contextP, lwm2m_uri_t * uriP);
//add by joe start
resource_instance_int_list_t * find_resource_inst(resource_instance_int_list_t * head, uint16_t id);
resource_instance_string_list_t * find_string_resource_inst(resource_instance_string_list_t * head,
    uint16_t id);

bool prv_write_resource_inst_val(resource_instance_int_list_t** rsList,    uint16_t resId, uint16_t resValue);
bool prv_write_string_resource_inst_val(resource_instance_string_list_t** rsList, uint16_t resId, char* resValue);
resource_instance_int_list_t * lwm2m_reosurce_list_add(resource_instance_int_list_t * head,resource_instance_int_list_t * node);


//add by joe end
#endif

#ifdef LWM2M_SERVER_MODE
// Clients registration/deregistration monitoring API.
// When a LWM2M client registers, the callback is called with status COAP_201_CREATED.
// When a LWM2M client deregisters, the callback is called with status COAP_202_DELETED.
// clientID is the internal ID of the LWM2M Client.
// The callback's parameters uri, data, dataLength are always NULL.
// The lwm2m_client_t is present in the lwm2m_context_t's clientList when the callback is called. On a deregistration, it deleted when the callback returns.
void lwm2m_set_monitoring_callback(lwm2m_context_t * contextP, lwm2m_result_callback_t callback, void * userData);

// Device Management APIs
int lwm2m_dm_read(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_discover(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_write(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_media_type_t format, uint8_t * buffer, int length, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_write_attributes(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_attributes_t * attrP, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_execute(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_media_type_t format, uint8_t * buffer, int length, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_create(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_media_type_t format, uint8_t * buffer, int length, lwm2m_result_callback_t callback, void * userData);
int lwm2m_dm_delete(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);

// Information Reporting APIs
int lwm2m_observe(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);
int lwm2m_observe_cancel(lwm2m_context_t * contextP, uint16_t clientID, lwm2m_uri_t * uriP, lwm2m_result_callback_t callback, void * userData);
#endif

#ifdef LWM2M_BOOTSTRAP_SERVER_MODE
// Clients bootstrap request monitoring API.
// When a LWM2M client sends a bootstrap request, the callback is called with the client's endpoint name.
void lwm2m_set_bootstrap_callback(lwm2m_context_t * contextP, lwm2m_bootstrap_callback_t callback, void * userData);

// Boostrap Interface APIs
// if uriP is nil, a "Delete /" is sent to the client
int lwm2m_bootstrap_delete(lwm2m_context_t * contextP, void * sessionH, lwm2m_uri_t * uriP);
int lwm2m_bootstrap_write(lwm2m_context_t * contextP, void * sessionH, lwm2m_uri_t * uriP, lwm2m_media_type_t format, uint8_t * buffer, size_t length);
int lwm2m_bootstrap_finish(lwm2m_context_t * contextP, void * sessionH);

#endif
bool lwm2m_check_if_observe_and_update_app(uint8_t id_set, uint16_t object_ID, uint16_t instance_ID, uint16_t resource_ID, lwm2m_object_data_t *data);

bool write_Tlv_to_file(int fd, int tag, uint8_t length, uint8_t* value);

int read_Tlv_from_file(int fd, int* tag, uint8_t* length, uint8_t** value);


#ifdef AUTHORIZATION_SUPPORT
lwm2m_client_state_t lwm2m_get_client_state(lwm2m_context_t *ctx_p);
#endif
#ifdef __cplusplus
}
#endif

#endif
