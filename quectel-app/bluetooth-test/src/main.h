#ifndef __MENU_NODE__
#define __MENU_NODE__

#define MAX_GROUP 20
typedef int (*FUNC)(void);

typedef struct menu_node_item{
    int index;
    char *name;
}menu_node_item_t;

typedef struct menu_node_group{
    char *group_name;
    menu_node_item_t *items; 
    FUNC action;
}menu_node_group_t;

static menu_node_group_t groups[MAX_GROUP] = {0};

extern menu_node_group_t adapter_group_test;
extern menu_node_group_t gatt_group_test;

void show_group_help(menu_node_group_t *group);

#endif
