/**/
#ifndef LWM2M_OBJECT_SERVER_H_
#define LWM2M_OBJECT_SERVER_H_

typedef struct _server_instance_
{
    struct _server_instance_ * next;   // matches lwm2m_list_t::next
    uint16_t    instanceId;            // matches lwm2m_list_t::id
    uint16_t    shortServerId;
    uint32_t    lifetime;
    uint32_t    defaultMinPeriod;
    uint32_t    defaultMaxPeriod;
    uint32_t    disableTimeout;
    bool        storing;
    char        binding[4];
#ifndef LWM2M_VERSION_1_0
    int         registrationPriorityOrder; // <0 when it doesn't exist
    int         initialRegistrationDelayTimer; // <0 when it doesn't exist
    int8_t      registrationFailureBlock; // <0 when it doesn't exist, 0 for false, > 0 for true
    int8_t      bootstrapOnRegistrationFailure; // <0 when it doesn't exist, 0 for false, > 0 for true
    int         communicationRetryCount; // <0 when it doesn't exist
    int         communicationRetryTimer; // <0 when it doesn't exist
    int         communicationSequenceDelayTimer; // <0 when it doesn't exist
    int         communicationSequenceRetryCount; // <0 when it doesn't exist
#endif
    resource_instance_int_list_t*    custom30000List;
} server_instance_t;

void update_isRegistered_for_server(lwm2m_object_t * serv_object,uint16_t ssid, uint8_t value);

#endif

