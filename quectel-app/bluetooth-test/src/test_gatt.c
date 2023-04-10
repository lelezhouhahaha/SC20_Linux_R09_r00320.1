#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_bt.h"
#include "ql_error.h"

extern menu_node_group_t gatt_group_test;

menu_node_item_t gatt_items[] = {
    {0,"QL_BT_init"},
    {1,"QL_BT_AddRxIndMsgHandler"},
    {2,"QL_BT_enable_with_ble_scan"},
    {3,"QL_BT_enable"},
    {4,"QL_BT_disable"},
    {5,"QL_BT_register_client"},
    {6,"QL_BT_ble_scan"},
    {7,"QL_BT_ble_unscan"},
    {8,"QL_BT_pair"},
    {9,"QL_BT_unpair"},
    {10,"QL_BT_ssp_reply"},
    {11,"QL_BT_conn_gatt"},
    {12,"QL_BT_search_service"},
    {13,"QL_BT_btgattc_register_for_notification"},
    {14,"QL_BT_write_descriptor"},

    {-1,"Return to main menu"}
};

ssp_result_t *ssp_result;
gattc_context_t gattc_context;

bool uuid_equal(bt_uuid_t src,bt_uuid_t dest){
    for(int i = 0;i < 16;i ++){
            if(src.uu[i] != dest.uu[i]){
                        return false;
                    }
        }
    return true;
}

bt_uuid_t ancs_uuid = {
    .uu = {
          0xD0,0x00,0x2D,0x12,0x1E,0x4B,
          0x0F,0xA4,0x99,0x4E,
          0xCE,0xB5,
          0x31,0xF4,0x05,0x79
        }
};

bt_uuid_t notification_source_uuid = {
    .uu = {
           0xBD,0x1D,0xA2,0x99,0xE6,0x25,
           0x58,0x8C,
           0xD9,0x42,
           0x01,0x63,
           0x0D,0x12,0xBF,0x9F
        }
};

bt_uuid_t control_point_uuid = {
    .uu = {
            0xD9,0xD9,0xAA,0xFD,0xBD,0x9B,
            0x21,0x98,
            0xA8,0x49,
            0xE1,0x45,
            0xF3,0xD8,0xD1,0x69
        }
};

bt_uuid_t data_source_uuid = {
    .uu = {
            0xFB,0x7B,0x7C,0xCE,0x6A,0xB3,
            0x44,0xBE,
            0xB5,0x4B,
            0xD6,0x24,
            0xE9,0xC6,0xEA,0x22
        }
};

/* btgatt_db_element_t *data_source_db;         */
/* bool data_source_is_last = -1;               */
/* btgatt_db_element_t *data_source_descriptor; */

btgatt_db_element_t *notification_source_db;
int notification_source_is_last = -1;
btgatt_db_element_t *notification_source_descriptor[5];

static void bt_event_handler(E_QL_BT_IND_EVENT_TYPE_T event,void *pv_data,void *contextPtr){
    int i = 0;
    bond_state_t *state;
    gattc_context_t *temp_context;
    gattc_service_db_t *service_db;
    gattc_com_t *com_result;
    ble_scan_result_t *scan_result;
    notify_data_t *notify_data;
    switch(event){   
        case E_QL_BT_ENABLE_STATE_EV:
            printf("E_QL_BT_ENABLE_STATE_EV : %s\n",*((int *)(pv_data)) == 0 ? "OFF":"ON");
            break;
        case E_QL_BLE_DEVICE_FOUND_EV:
            scan_result = (ble_scan_result_t *)pv_data;
            char c_address[32];
            snprintf(c_address,sizeof(c_address), "%02X:%02X:%02X:%02X:%02X:%02X",
                    scan_result->bda.address[0], scan_result->bda.address[1], scan_result->bda.address[2],
                    scan_result->bda.address[3], scan_result->bda.address[4], scan_result->bda.address[5]);
            printf("E_QL_BLE_DEVICE_FOUND_EV rssi (%d) bda (%s)\n",scan_result->rssi,c_address);
            break;
        case E_QL_BT_BOND_STATE_EV:
            state = (bond_state_t *)pv_data;
            printf("E_QL_BT_BOND_STATE_EV : %s --> %d\n",state->address,state->status);
            break;
        case E_QL_BT_SSP_RESULT_EV:
            ssp_result = (ssp_result_t *)pv_data;
            printf("E_QL_BT_SSP_RESULT_EV : %06d\n",ssp_result->pass_key);
            break;
        case E_QL_BLE_REGISTER_CLIENT_EV:
            temp_context = (gattc_context_t *)pv_data;
            if(temp_context->status == 0){
                gattc_context.client_if = temp_context->client_if;
                memcpy(&gattc_context.uuid,&temp_context->uuid,sizeof(bt_uuid_t));
            }
            printf("E_QL_BLE_REGISTER_CLIENT_EV status : %d client_if : %d\n",temp_context->status,gattc_context.client_if);
            break;
        case E_QL_BLE_CONNECTION_STATE_EV:
            temp_context = (gattc_context_t *)pv_data;
            if(temp_context->status == 0){
                gattc_context.client_if = temp_context->client_if;
                gattc_context.conn_id = temp_context->conn_id;
                memcpy(&gattc_context.bda,&temp_context->bda,sizeof(bt_bdaddr_t));
            }
            printf("E_QL_BLE_CONNECTION_STATE_EV status : %d client_if : %d conn_id : %d\n",temp_context->status,temp_context->client_if,temp_context->conn_id);
            break;
        case E_QL_BLE_SEARCH_SERVICE_EV:
            service_db = (gattc_service_db_t *)pv_data;
            for(int i = 0; i < service_db->count ; i++) {                                                                                                                                                                        
                btgatt_db_element_t curr = service_db->db[i];
                /* fprintf(stdout,"curr type is %d\n", curr.type); */
                switch(curr.type) {
                    case BTGATT_DB_PRIMARY_SERVICE:
                        if(uuid_equal(ancs_uuid,curr.uuid)){
                            fprintf(stdout,"curr type is %d\n", curr.type);
                            fprintf(stdout," id is %d \n", curr.id);
                            fprintf(stdout," type is BTGATT_DB_PRIMARY_SERVICE  %d \n", curr.type);
                            fprintf(stdout," uuid is ");
                            for (int j = sizeof(curr.uuid) - 1; j >= 0;j--) {
                                fprintf(stdout,  "%02x", curr.uuid.uu[j]);
                            }
                            fprintf(stdout," \n");
                            fprintf(stdout," \n");
                        }
                        /*
                         * fprintf(stdout," attribute handle is %d \n", curr.attribute_handle);
                         * fprintf(stdout," start handle is %d \n", curr.start_handle);
                         * fprintf(stdout," end handle is %d \n", curr.end_handle);
                         * fprintf(stdout," properties are %d \n", curr.properties);
                         */

                        break;
                    case BTGATT_DB_SECONDARY_SERVICE:
                        /*
                         * fprintf(stdout," id is %d \n", curr.id);
                         * fprintf(stdout," type is BTGATT_DB_SECONDARY_SERVICE %d \n", curr.type);
                         * fprintf(stdout," uuid is \n");
                         * for (int j = 0; j < sizeof(curr.uuid); j++) {
                         *     fprintf(stdout,  "%02x", curr.uuid.uu[j]);
                         *     fprintf(stdout,  "%02x", curr.uuid.uu[j]);
                         * }
                         * fprintf(stdout," \n");
                         */
                        break;
                    case BTGATT_DB_CHARACTERISTIC:
                        /* if(uuid_equal(notification_source_uuid,curr.uuid) || uuid_equal(control_point_uuid,curr.uuid) || uuid_equal(data_source_uuid,curr.uuid)){ */
                        if(uuid_equal(notification_source_uuid,curr.uuid)){
                            notification_source_is_last = notification_source_is_last + 1;
                            notification_source_db = &service_db->db[i];
                            fprintf(stdout,"curr type is %d\n", curr.type);
                            fprintf(stdout," id is %d \n", curr.id);
                            fprintf(stdout," type is  BTGATT_DB_CHARACTERISTIC %d \n", curr.type);
                            fprintf(stdout," uuid is ");
                            for (int j = sizeof(curr.uuid) - 1; j >= 0;j--) {
                                fprintf(stdout,  "%02x", curr.uuid.uu[j]);
                            }
                            fprintf(stdout," \n");
                            fprintf(stdout," properties is %u\n",curr.properties);
                            fprintf(stdout," \n");
                        }else{
                            notification_source_is_last = -1;
                        }
                        break;
                    case BTGATT_DB_DESCRIPTOR:
                        if(notification_source_is_last != -1){
                            notification_source_descriptor[notification_source_is_last] = &service_db->db[i];
                            notification_source_is_last = notification_source_is_last + 1;

                            fprintf(stdout,"curr type is %d\n", curr.type);
                            fprintf(stdout," id is %d \n", curr.id);
                            fprintf(stdout," type is BTGATT_DB_DESCRIPTOR %d \n", curr.type);
                            fprintf(stdout," uuid is ");
                            for (int j = sizeof(curr.uuid) - 1; j >= 0; j--) {
                                fprintf(stdout,  "%02x", curr.uuid.uu[j]);
                            }
                            fprintf(stdout," \n");
                            fprintf(stdout," \n");
                        }
                        break;
                /* } */
                }
            }
            break;
        case E_QL_BLE_REGISTER_NOTIFICATION_EV:
            com_result = (gattc_com_t *)pv_data;
            printf("E_QL_BLE_REGISTER_NOTIFICATION_EV status : %d registered : %d conn_id : %d handle : %d\n",com_result->status,com_result->registered,com_result->conn_id,com_result->handle);
            break;
        case E_QL_BLE_WRITE_DESCTIPTOR_EV:
            com_result = (gattc_com_t *)pv_data;
            printf("E_QL_BLE_WRITE_DESCTIPTOR_EV status : %d conn_id : %d handle : %d\n",com_result->status,com_result->conn_id,com_result->handle);
            break;
        case E_QL_BLE_NOTIFY_DATA_EV:
            notify_data = (notify_data_t *)pv_data;
            for(i = 0;i < notify_data->data->len;i ++){                                                                 
                printf("%02X ",notify_data->data->value[i]);                                                            
            }
            printf("\n"); 
            break;

        default:
            printf("E_QL_BT_IND_EVENT_TYPE_T : %x\n",event);
            break;
    }
}

bool CopyClientUUID(bt_uuid_t *uuid){
    uuid->uu[0] = 0xff;
    for (int i = 1; i < 16; i++) {
        uuid->uu[i] = 0x30;
    }   
    return true;
}

int test_gatt(){
    int cmdIdx = 0;
    int ret = 0;
    int num = 0;
    char address[20];
    bt_uuid_t client_uuid;
    char data[2] = {
        0x01, 0x00
    };
    int i = 0;

    show_group_help(&gatt_group_test);
    while(1){
        printf("please input cmd index(-1 exit): ");
        ret = scanf("%d", &cmdIdx);
        if(ret != 1){
            char c;
            while(((c=getchar()) != '\n') && (c != EOF)){
                ;
            }
            continue;
        }
        if(cmdIdx == -1){
            break;
        }
        switch(cmdIdx){
            case 0:
                ret = QL_BT_init();
                printf("QL_BT_init : %d\n",ret);
                break;
            case 1:
                ret = QL_BT_AddRxIndMsgHandler(bt_event_handler,NULL);
                printf("QL_BT_AddRxIndMsgHandler : %d\n",ret);
                break;
            case 2:
                ret = QL_BT_enable_with_ble_scan();
                printf("QL_BT_scan : %d\n",ret);
                break;
            case 3:
                ret = QL_BT_enable();
                printf("QL_BT_enable : %d\n",ret);
                break;
            case 4:
                ret = QL_BT_disable();
                printf("QL_BT_disable : %d\n",ret);
                break;
            case 5:
                CopyClientUUID(&client_uuid);
                ret = QL_BT_register_client(&client_uuid);
                printf("QL_BT_register_client : %d\n",ret);
                break;
            case 6:
                ret = QL_BT_ble_scan();
                printf("QL_BT_scan : %d\n",ret);
                break;
            case 7:
                ret = QL_BT_ble_unscan();
                printf("QL_BT_unscan : %d\n",ret);
                break;
            case 8:
                printf("please input remote device address:");
                scanf("%s",address);
                if(ret != 1){
                    char c;
                    while(((c=getchar()) != '\n') && (c != EOF)){
                        ;
                    }
                    continue;
                }
                ret = QL_BT_pair(address,BT_TRANSPORT_LE);
                break;
            case 9:
                printf("please input remote device address:");
                scanf("%s",address);
                ret = QL_BT_unpair(address);
                break;
            case 10:
                printf("0.no 1.yes please input yes or no:");
                scanf("%d",&num);
                if(ssp_result){
                    ret = QL_BT_ssp_reply(ssp_result->remote_bd_addr,ssp_result->pairing_variant,num,ssp_result->pass_key);
                }else{
                    ret = -1;
                }
                printf("QL_BT_ssp_reply : %d\n",ret);
                break;
            case 11:
                printf("please input remote device address:");
                scanf("%s",address);
                ret = QL_BT_conn_gatt(gattc_context.client_if,address);
                printf("QL_BT_conn_gatt ret : %d\n",ret);
                break;
            case 12:
                ret = QL_BT_search_service(gattc_context.conn_id);
                printf("QL_BT_search_service ret : %d\n",ret);
                break;
            case 13:
                ret = QL_BT_btgattc_register_for_notification(gattc_context.client_if,&gattc_context.bda,notification_source_db->id);
                printf("QL_BT_btgattc_register_for_notification ret : %d\n",ret);
                break;
            case 14:
                for(i = 0;i < 5;i ++){
                    if(notification_source_descriptor[i]){
                        ret = QL_BT_write_descriptor(gattc_context.conn_id,notification_source_descriptor[i]->id,sizeof(data),data);
                        printf("QL_BT_write_descriptor ret : %d\n",ret);
                    }
                }
                break;
            default:
                show_group_help(&gatt_group_test);
                break;
        }
    }
    return 1;
}

menu_node_group_t gatt_group_test = {"gatt",gatt_items,test_gatt};
