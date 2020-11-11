#include "dlist.h"
#include <stdlib.h>
#include <memory.h>

void ads_dlist_init(ads_dlist_t* dlist, void (*destroy)(void*)) {
  dlist->destroy = destroy;
  dlist->head    = NULL;
  dlist->tail    = NULL;
  dlist->size    = 0;
}

static inline 
ads_dlist_node_t* ads_dlist_new_node(void* data) {
  ads_dlist_node_t* new_node = malloc(sizeof(ads_dlist_node_t));
  if(new_node) {
    new_node->data = data;
    new_node->next = NULL;
    new_node->prev = NULL;
  }

  return new_node;
}

int ads_dlist_add_front(ads_dlist_t* dlist, void* data) {
  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return -1;

  if(ads_dlist_is_empty(dlist)) {
    dlist->head = new_node;
    dlist->tail = new_node;
  }
  else {
    dlist->head->prev = new_node;
    new_node->next = dlist->head;
    dlist->head = new_node;
  }

  dlist->size++;

  return 0;
}

int ads_dlist_add_back(ads_dlist_t* dlist, void* data) {
  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return -1;
  
  if(ads_dlist_is_empty(dlist)) {
    dlist->head = new_node;
    dlist->tail = new_node;
  }
  else {
    new_node->prev = dlist->tail;
    dlist->tail->next = new_node;
    dlist->tail = new_node;
  }

  dlist->size++;

  return 0;
}

int ads_dlist_add_next(ads_dlist_t* dlist,
                       ads_dlist_node_t* node,
                       void* data)
{
  if(node == NULL)
    return ads_dlist_add_front(dlist, data);
  else if(node == ads_dlist_get_tail(dlist))
    return ads_dlist_add_back(dlist, data);

  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return -1;

  new_node->next = node->next;
  new_node->prev = node;
 
  node->next->prev = new_node;
  node->next = new_node;

  dlist->size++;
  
  return 0;
}

int ads_dlist_add_prev(ads_dlist_t* dlist,
                       ads_dlist_node_t* node,
                       void* data)
{
  if(node == ads_dlist_get_head(dlist) || node == NULL)
    return ads_dlist_add_front(dlist, data);

  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return -1;

  new_node->next = node;
  new_node->prev = node->prev;

  node->prev->next = new_node;
  node->prev = new_node;

  dlist->size++;

  return 0;
}