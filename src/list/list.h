#ifndef ADS_SLINKED_LIST_H
#define ADS_SLINKED_LIST_H

#include <stdlib.h>

/*
  SINGLE LINKED LIST HEADER
*/

// individual node in a single linked list
typedef struct ads_list_node {
  void* data;
  struct ads_list_node* next;
} ads_list_node_t;

// single linked list
typedef struct ads_list {
  ads_list_node_t* head;
  ads_list_node_t* tail;
  size_t size;

  void (*destroy)(void* data);
} ads_list_t;

#define ads_list_get_size(list)  ((list)->size)
#define ads_list_get_head(list)  ((list)->head)
#define ads_list_get_tail(list)  ((list)->tail)
#define ads_list_is_empty(list) (ads_list_get_size((list)) == 0 ? 1 : 0)

#define ads_list_get_next(node) ((node)->next)

void ads_list_init(ads_list_t* list, void (*destroy)(void*));
void ads_list_destroy(ads_list_t* list);
void ads_list_clean(ads_list_t* list);

int ads_list_remove_front(ads_list_t* list, void** ret_data);
int ads_list_remove_back(ads_list_t* list, void** ret_data);
int ads_list_remove_next(ads_list_t* list, ads_list_node_t* node, void** ret_data);

int ads_list_add_front(ads_list_t* list, void* data);
int ads_list_add_back(ads_list_t* list, void* data);
int ads_list_add_next(ads_list_t* list, ads_list_node_t* node, void* data);

ads_list_node_t* ads_list_get_at(ads_list_t* list, int index);

#endif