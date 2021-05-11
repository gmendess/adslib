#ifndef ADS_DLINKED_LIST_H
#define ADS_DLINKED_LIST_H

#include <stdlib.h>
#include "error.h"

/*
  DOUBLE LINKED LIST HEADER
*/

// individual node in a double linked list
typedef struct ads_dlist_node {
  void* data;
  struct ads_dlist_node* next;
  struct ads_dlist_node* prev;
} ads_dlist_node_t;

// double linked list
typedef struct ads_dlist {
  ads_dlist_node_t* head;
  ads_dlist_node_t* tail;
  size_t size;

  void (*destroy)(void* data);
} ads_dlist_t;

#define ads_dlist_get_size(dlist) ((dlist)->size)
#define ads_dlist_get_head(dlist) ((dlist)->head)
#define ads_dlist_get_tail(dlist) ((dlist)->tail)
#define ads_dlist_is_empty(dlist) (ads_dlist_get_size((dlist)) == 0 ? 1 : 0)
#define ads_dlist_get_data_as(node, type) ( (type) node->data)

#define ads_dlist_get_next(node)  ((node)->next)
#define ads_dlist_get_prev(node)  ((node)->prev)

void ads_dlist_init(ads_dlist_t* dlist, void (*destroy)(void*));
void ads_dlist_destroy(ads_dlist_t* dlist);
void ads_dlist_clean(ads_dlist_t* dlist);

void ads_dlist_pop_front(ads_dlist_t* dlist, void** ret_data);
void ads_dlist_pop_back(ads_dlist_t* dlist, void** ret_data);
void ads_dlist_remove_next(ads_dlist_t* dlist, ads_dlist_node_t* node, void** ret_data);
void ads_dlist_remove_prev(ads_dlist_t* dlist, ads_dlist_node_t* node, void** ret_data);

ads_status_t ads_dlist_push_front(ads_dlist_t* dlist, void* data);
ads_status_t ads_dlist_push_back(ads_dlist_t* dlist, void* data);
ads_status_t ads_dlist_add_next(ads_dlist_t* dlist, ads_dlist_node_t* node, void* data);
ads_status_t ads_dlist_add_prev(ads_dlist_t* dlist, ads_dlist_node_t* node, void* data);

ads_dlist_node_t* ads_dlist_get_at(ads_dlist_t* dlist, int index);

#endif