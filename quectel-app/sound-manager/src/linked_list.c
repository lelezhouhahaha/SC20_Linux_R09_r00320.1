/*
 * Copyright: (C) 2014 EAST fulinux <fulinux@sina.com>
 * source from: http://blog.chinaunix.net/uid-20696246-id-1892194.html
 *              https://github.com/dxtr/list
 */
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"

/*****************Signal linked list definition**********************/

/* Creates a list (node) and returns it
 * Arguments: The data the list will contain or NULL to create an empty
 * list/node
 */
list_node_t* list_create(void *data)
{
	list_node_t *l = malloc(sizeof(list_node_t));
	if (l != NULL) {
		l->next = NULL;
		l->data = data;
	}

	return l;
}

/* Completely destroys a list
 * Arguments: A pointer to a pointer to a list
 */
void list_destroy(list_node_t **list)
{
	if (list == NULL) return;
	while (*list != NULL) {
		list_remove(list, *list);
	}
}

/* Creates a list node and inserts it after the specified node
 * Arguments: A node to insert after and the data the new node will contain
 */
list_node_t* list_insert_after(list_node_t *node, void *data)
{
	list_node_t *new_node = list_create(data);
	if (new_node) {
		new_node->next = node->next;
		node->next = new_node;
	}
	return new_node;
}

/* Creates a new list node and inserts it in the beginning of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node_t* list_insert_beginning(list_node_t *list, void *data)
{
	list_node_t *new_node = list_create(data);
	if (new_node != NULL) { new_node->next = list; }
	return new_node;
}

/* Creates a new list node and inserts it at the end of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
list_node_t* list_insert_end(list_node_t *list, void *data)
{
    list_node_t *it;
	list_node_t *new_node = list_create(data);

	if (new_node != NULL) {
        for (it = list; it != NULL; it = it->next) {
            if (it->next == NULL) {
                it->next = new_node;
                break;
            }
        }
	}

	return new_node;
}

/* Removes a node from the list
 * Arguments: The list and the node that will be removed
 */
void list_remove(list_node_t **list, list_node_t *node)
{
	list_node_t *tmp = NULL;
	if (list == NULL || *list == NULL || node == NULL) return;

	if (*list == node) {
		*list = (*list)->next;
		free(node);
		node = NULL;
	} else {
		tmp = *list;
		while (tmp->next && tmp->next != node) tmp = tmp->next;
		if (tmp->next) {
			tmp->next = node->next;
			free(node);
			node = NULL;
		}
	}
}

/* Removes an element from a list by comparing the data pointers
 * Arguments: A pointer to a pointer to a list and the pointer to the data
 */
void list_remove_by_data(list_node_t **list, void *data)
{
	if (list == NULL || *list == NULL || data == NULL) return;
	list_remove(list, list_find_by_data(*list, data));
}

/* Find an element in a list by the pointer to the element
 * Arguments: A pointer to a list and a pointer to the node/element
 */
list_node_t* list_find_node(list_node_t *list, list_node_t *node)
{
	while (list) {
		if (list == node) break;
		list = list->next;
	}
	return list;
}

/* Finds an elemt in a list by the data pointer
 * Arguments: A pointer to a list and a pointer to the data
 */
list_node_t* list_find_by_data(list_node_t *list, void *data)
{
	while (list) {
		if (list->data == data) break;
		list = list->next;
	}
	return list;
}

/* Finds an element in the list by using the comparison function
 * Arguments: A pointer to a list, the comparison function and a pointer to the
 * data
 */
list_node_t* list_find(list_node_t *list, int(*func)(list_node_t*,void*), void *data)
{
	if (!func) return NULL;
	while(list) {
		if (func(list, data)) break;
		list = list->next;
	}
	return list;
}

/*****************Double linked list definition**********************/

/* 
 * 初始化链表头指针
 */
void list_init_head(dlist_node_t *head)
{
    head->prev = head->next = head;
}

/* 
 * 判断链表是否为空 
 */
int list_is_empty(const dlist_node_t *head)
{
    return (head->prev == head) && (head->next == head);
}

static void __list_insert(dlist_node_t *node, 
        dlist_node_t *prev, dlist_node_t *next)
{
    prev->next = next->prev = node;
    node->prev = prev;
    node->next = next;
}

/* 
 * 插入头结点 
 */
void list_push_front(dlist_node_t *head, dlist_node_t *node)
{
    __list_insert(node, head, head->next);
}

/* 
 * 插入尾结点 
 */
void list_push_back(dlist_node_t *head, dlist_node_t *node)
{
    __list_insert(node, head->prev, head);
}

static void __list_delete(dlist_node_t *prev, 
        dlist_node_t *next)
{
    prev->next = next;
    next->prev = prev;
}

/* 
 * 移除节点 
 */
void list_remove_node(dlist_node_t *node)
{
    __list_delete(node->prev, node->next);
    node->prev = node->next = NULL;
}

/* 
 * 链表结合，将list结合到head, 
 * 组合后的链表头结点仍为head 
 */
void list_splice(dlist_node_t *head, dlist_node_t *list)
{
    if(list_is_empty(list) == 0){
        list->next->prev = head;
        head->prev = list->next;

        list->prev->next = head->next;
        head->next->prev = list->prev;
    }
}

