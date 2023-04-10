#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "main.h"

static void add_node_group(menu_node_group_t *group){
    for(int i = 0;i < MAX_GROUP;i ++){
        if(groups[i].group_name == NULL){
            groups[i].group_name = group->group_name;
            groups[i].items = group->items;
            groups[i].action = group->action;
            break;
        }
    }
}

static void show_groups(){
    printf("Main menu:\n");
    for(int i = 0;i < MAX_GROUP;i ++){
        if(groups[i].group_name != NULL){
            printf("%d\t%s\n",i,groups[i].group_name);
        }
    }
}

void show_group_help(menu_node_group_t *group){
    printf("Group Name:%s, Supported test cases:\n",group->group_name);
    for(int i = 0;;i++){
        if(group->items[i].index == -1){
            break;
        }
        printf("%d\t%s\n",group->items[i].index,group->items[i].name);
    }
}

void main(){
    int cmdIdx = 0,caseIdx = 0;
    add_node_group(&adapter_group_test);
    add_node_group(&gatt_group_test);
    show_groups();
    
    while(1){
        printf("please input command index(-1 exit): ");
        scanf("%d", &cmdIdx);
        if(cmdIdx == -1){
            break;
        }
        if((cmdIdx >= MAX_GROUP) || groups[cmdIdx].group_name == NULL){
            show_groups();
        }else{
            groups[cmdIdx].action();
            show_groups();
        }
    }
}
