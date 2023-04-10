/*
 * Copyright: (C) 2014 EAST fulinux <fulinux@sina.com>
 * source from: http://blog.chinaunix.net/uid-20696246-id-1892194.html
 *              https://github.com/dxtr/list
 */
#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

typedef struct list_node {
	void *data;
	struct list_node *next;
} list_node_t;

typedef struct dlist_node{

    struct dlist_node *prev;

    struct dlist_node *next;

} dlist_node_t;

/* Signal linked list API */

void list_destroy(struct list_node **list);

void list_remove_by_data(struct list_node **list, void *data);

void list_remove(struct list_node **list, struct list_node *node);

struct list_node *list_create(void *data);

struct list_node *list_insert_end(struct list_node *list, void *data);

struct list_node *list_insert_after(struct list_node *node, void *data);

struct list_node *list_find_by_data(struct list_node *list, void *data);

struct list_node *list_find_node(struct list_node *list, struct list_node *node);

struct list_node *list_insert_beginning(struct list_node *list, void *data);

struct list_node *list_find(struct list_node *list, int(*func)(struct list_node *,void *), void *data);


/* Double linked list API */

/* 链表遍历 */
#define list_for_each(iter, head) \
    for(iter = (head)->next; iter != (head); iter = iter->next)

/* 链表逆序遍历 */
#define list_for_each_reverse(iter, head) \
    for(iter = (head)->prev; iter != (head); iter = iter->prev)

/* 遍历链表，支持链表删除操作 */
#define list_for_each_remove(iter, n, head) \
    for(iter = (head)->next, n = iter->next; iter != (head); iter = n, n = iter->next)

/* 求包含此链表的结构体指针 */
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/* 初始化链表头指针 */

int list_is_empty(const struct dlist_node *head);

void list_init_head(struct dlist_node *head);

void list_remove_node(struct dlist_node *node);

void list_splice(struct dlist_node *head, struct dlist_node *list);

void list_push_back(struct dlist_node *head, struct dlist_node *node);

void list_push_front(struct dlist_node *head, struct dlist_node *node);

#endif /* __LINKED_LIST_H_ */


