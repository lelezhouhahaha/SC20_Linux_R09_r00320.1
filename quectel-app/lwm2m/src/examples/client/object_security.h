/**/
#ifndef LWM2M_OBJECT_SECURITY_H_
#define LWM2M_OBJECT_SECURITY_H_
typedef enum
{
  CERT_SOURCE_UNKNOWN  = 0,
  CERT_SOURCE_PRELOAD,
  CERT_SOURCE_NETWORK,
} cert_source_enum;

typedef struct 
{
    uint32_t psk_Size;  
    /**< PSK table buffer size. */
    uint8_t *psk_Buf;      
    /**< PSK table buffer. */ 
} ssl_psk_struct_t; 

typedef struct _security_instance_
{
    struct _security_instance_ * next;        // matches lwm2m_list_t::next
    uint16_t                     instanceId;  // matches lwm2m_list_t::id
    char *                       uri;
    bool                         isBootstrap;    
    uint8_t                      securityMode;
    char *                       publicIdentity;
    uint16_t                     publicIdLen;
    char *                       serverPublicKey;
    uint16_t                     serverPublicKeyLen;
    char *                       secretKey;
    uint16_t                     secretKeyLen;
    uint8_t                      smsSecurityMode;
    char *                       smsParams; // SMS binding key parameters
    uint16_t                     smsParamsLen;
    char *                       smsSecret; // SMS binding secret key
    uint16_t                     smsSecretLen;
    uint16_t                     shortID;
    uint32_t                     clientHoldOffTime;
    uint32_t                     bootstrapServerAccountTimeout;
    char *                       sms_number;
#if defined (LWM2M_BOOTSTRAP)
    resource_instance_int_list_t*    custom30000List;
#endif  
    cert_source_enum             cert_source;
} security_instance_t;

#if defined (LWM2M_BOOTSTRAP)
#define LWM2M_SECURITY_RESOURCE_INST_IS_HOLD_OFF_TIMER       0
#define LWM2M_SECURITY_RESOURCE_INST_IS_BOOTSTRAPPED         1
#endif

#endif

