#ifndef ALPHA_QL_BLUETOOTH
#define ALPHA_QL_BLUETOOTH
#include <hardware/hardware.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>
#include <hardware/bt_pan.h>
#include <hardware/bt_common_types.h>
#include <hardware/bt_gatt_client.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef BT_MAX
#define BT_MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef BT_MIN
#define BT_MIN(x, y) ((x) < (y) ? (x) : (y)) 
#endif

typedef enum{
    BT_TRANSPORT_INVALID = 0,
    BT_TRANSPORT_BR_EDR,
    BT_TRANSPORT_LE
}transport_type_t;

typedef struct {
    char *ap;
    char *address;
}scan_result_t;

typedef struct {
    int status;
    char *address;
}bond_state_t;

typedef struct {
    int status;
    int client_if;
    int conn_id;
    bt_uuid_t uuid;
    bt_bdaddr_t bda;
}gattc_context_t;

typedef struct {
    int conn_id;
    btgatt_db_element_t *db;
    int count;
}gattc_service_db_t;

typedef struct {
    int conn_id;
    int registered;
    int status;
    int handle;
}gattc_com_t;

typedef struct {
    uint32_t pass_key;
    bt_bdaddr_t remote_bd_addr;
    bt_bdname_t bd_name;
    bt_ssp_variant_t pairing_variant;
    uint32_t cod;
}ssp_result_t;

typedef struct {
    bt_bdaddr_t bda;
    int rssi;
    uint8_t* adv_data;
}ble_scan_result_t;

typedef struct {
    btgatt_notify_params_t *data;
    int conn_id;
}notify_data_t;

typedef enum{
    E_QL_BT_UNKNOWN_EV = -1,
    E_QL_BT_ENABLE_STATE_EV = 0x8000,
    E_QL_BT_DISCOVERY_STATE_EV = 0x8001,
    E_QL_BT_DEVICE_FOUND_EV = 0x8002,
    E_QL_BT_BOND_STATE_EV = 0x8003,
    E_QL_BT_SSP_RESULT_EV = 0x8004,

    E_QL_BLE_DEVICE_FOUND_EV = 0x9002,
    E_QL_BLE_REGISTER_CLIENT_EV = 0x9003,
    E_QL_BLE_CONNECTION_STATE_EV = 0x9004,
    E_QL_BLE_SEARCH_SERVICE_EV = 0x9005,
    E_QL_BLE_REGISTER_NOTIFICATION_EV = 0x9006,
    E_QL_BLE_WRITE_DESCTIPTOR_EV = 0x9007,
    E_QL_BLE_NOTIFY_DATA_EV = 0x9008,

}E_QL_BT_IND_EVENT_TYPE_T;

typedef enum{
    E_QL_BT_PROPERTY_BDNAME = 0x1,
    E_QL_BT_PROPERTY_BDADDR,
}E_QL_BT_PROPERTY_TYPE_T;

typedef void (*QL_BT_RxIndNotify_Func_t)(E_QL_BT_IND_EVENT_TYPE_T event,void *pv_data,void *contextPtr);

int QL_BT_AddRxIndMsgHandler(QL_BT_RxIndNotify_Func_t handlerPtr,void *contextPtr);

int QL_BT_init();

int QL_BT_enable_with_scan();

int QL_BT_enable_with_ble_scan();

int QL_BT_enable();

int QL_BT_disable();

int QL_BT_get_adapter_property(E_QL_BT_PROPERTY_TYPE_T type);

int QL_BT_scan();

int QL_BT_unscan();

int QL_BT_pair(char *remote_address,int transport);

int QL_BT_ssp_reply(const bt_bdaddr_t bd_addr, bt_ssp_variant_t variant,uint8_t accept, uint32_t passkey);

int QL_BT_unpair(char *remote_address);

int QL_BT_register_client(bt_uuid_t *uuid);

int QL_BT_unregister_client(int client_if);

int QL_BT_conn_gatt(int client_if,char *remote_address);

int QL_BT_disconn_gatt(int client_if, const bt_bdaddr_t *bd_addr,int conn_id);

int QL_BT_search_service(int conn_id); 

int QL_BT_btgattc_register_for_notification(int client_if,const bt_bdaddr_t *bd_addr, uint16_t id);

int QL_BT_write_descriptor(int conn_id, uint16_t handle,int len,char* p_value);




int QL_BT_socket_connect(char *remote_address);

int QL_BT_socket_listen(char *server_name);

void QL_BT_set_recv_cb(void (*p)(const char *data));

#ifdef __cplusplus
}
#endif


#endif
