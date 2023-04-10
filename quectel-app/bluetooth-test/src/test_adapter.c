#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_bt.h"
#include "ql_error.h"

extern menu_node_group_t adapter_group_test;
menu_node_item_t adapter_items[] = {
    {0,"QL_BT_init"},
    {1,"QL_BT_AddRxIndMsgHandler"},
    {2,"QL_BT_enable_with_scan"},
    {3,"QL_BT_enable"},
    {4,"QL_BT_disable"},
    {5,"QL_BT_scan"},
    {6,"QL_BT_unscan"},
    {7,"QL_BT_pair"},
    {8,"QL_BT_ssp_reply"},
    {9,"QL_BT_unpair"},
    {10,"QL_BT_get_adapter_property"},
    {11,"get_device_name"},
    {12,"set_device_name"},
    {-1,"Return to main menu"}
};

ssp_result_t *ssp_result;
static void bt_event_handler(E_QL_BT_IND_EVENT_TYPE_T event,void *pv_data,void *contextPtr){
    bond_state_t *state;
    scan_result_t *scan_result;
    switch(event){   
        case E_QL_BT_ENABLE_STATE_EV:
            printf("E_QL_BT_ENABLE_STATE_EV : %s\n",*((int *)(pv_data)) == 0 ? "OFF":"ON");
            break;
        case E_QL_BT_DISCOVERY_STATE_EV:
            printf("E_QL_BT_DISCOVERY_STATE_EV : %s\n",*((int *)pv_data) == 0 ? "STOPPED":"STARTED");
            break;
        case E_QL_BT_DEVICE_FOUND_EV:
            scan_result = (scan_result_t *)pv_data;
            printf("E_QL_BT_DEVICE_FOUND_EV : %s --> %s\n",scan_result->ap,scan_result->address);
            break;
        case E_QL_BT_BOND_STATE_EV:
            state = (bond_state_t *)pv_data;
            printf("E_QL_BT_BOND_STATE_EV : %s --> %d\n",state->address,state->status);
            break;
        case E_QL_BT_SSP_RESULT_EV:
            ssp_result = (ssp_result_t *)pv_data;
            printf("E_QL_BT_SSP_RESULT_EV : %06d\n",ssp_result->pass_key);
            break;
        default:
            printf("E_QL_BT_IND_EVENT_TYPE_T : %x\n",event);
            break;
    }
}

static int test_adapter(){
    int cmdIdx = 0;
    int ret = 0;
    int num = 0;
    char address[20];

    show_group_help(&adapter_group_test);
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
                ret = QL_BT_enable_with_scan();
                printf("QL_BT_enable_with_scan : %d\n",ret);
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
                ret = QL_BT_scan();
                printf("QL_BT_scan : %d\n",ret);
                break;
            case 6:
                ret = QL_BT_unscan();
                printf("QL_BT_unscan : %d\n",ret);
                break;
            case 7:
                printf("please input remote device address:");
                scanf("%s",address);
                if(ret != 1){
                    char c;
                    while(((c=getchar()) != '\n') && (c != EOF)){
                        ;
                    }
                    continue;
                }
                ret = QL_BT_pair(address,BT_TRANSPORT_BR_EDR);
                break;
            case 8:
                printf("0.no 1.yes please input yes or no:");
                scanf("%d",&num);
                if(ssp_result){
                    ret = QL_BT_ssp_reply(ssp_result->remote_bd_addr,ssp_result->pairing_variant,num,ssp_result->pass_key);
                }else{
                    ret = -1;
                }
                printf("QL_BT_ssp_reply : %d\n",ret);
                break;
            case 9:
                printf("please input remote device address:");
                scanf("%s",address);
                ret = QL_BT_unpair(address);
                break;
            case 10:
                printf("1.name 2.address 3.uuids");
                printf("please first input number:");
                scanf("%d",&num);
                ret = QL_BT_get_adapter_property((E_QL_BT_PROPERTY_TYPE_T)num);
                printf("QL_BT_get_adapter_property : %d\n",ret);
                break;
            default:
                show_group_help(&adapter_group_test);
                break;
        }
    }
    return 1;
}

menu_node_group_t adapter_group_test = {"adpater",adapter_items,test_adapter};
