#include "dlist.h"
#include <stdlib.h>
#include <memory.h>

void ads_dlist_init(ads_dlist_t* dlist, void (*destroy)(void*)) {
  dlist->destroy = destroy;
  dlist->head    = NULL;
  dlist->tail    = NULL;
  dlist->size    = 0;
}

void ads_dlist_clean(ads_dlist_t* list) {
  while(!ads_dlist_is_empty(list))
    ads_dlist_pop_front(list, NULL);
}

void ads_dlist_destroy(ads_dlist_t* list) {
  if(list->destroy == NULL)
    ads_dlist_clean(list);
  else {
    void* data = NULL;

    while(!ads_dlist_is_empty(list)) {
      ads_dlist_pop_front(list, &data);
      list->destroy(data);
    }
  }

  memset(list, 0, sizeof(ads_dlist_t));
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

ads_status_t ads_dlist_push_front(ads_dlist_t* dlist, void* data) {
  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return ADS_NOMEM;

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

  return ADS_SUCCESS;
}

ads_status_t ads_dlist_push_back(ads_dlist_t* dlist, void* data) {
  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return ADS_NOMEM;
  
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

  return ADS_SUCCESS;
}

ads_status_t
ads_dlist_add_next(ads_dlist_t* dlist,
                   ads_dlist_node_t* node,
                   void* data)
{
  if(node == NULL)
    return ads_dlist_push_front(dlist, data);
  else if(node == ads_dlist_get_tail(dlist))
    return ads_dlist_push_back(dlist, data);

  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return ADS_NOMEM;

  new_node->next = node->next;
  new_node->prev = node;
 
  node->next->prev = new_node;
  node->next = new_node;

  dlist->size++;
  
  return ADS_SUCCESS;
}

ads_status_t
ads_dlist_add_prev(ads_dlist_t* dlist,
                   ads_dlist_node_t* node,
                   void* data)
{
  if(node == ads_dlist_get_head(dlist) || node == NULL)
    return ads_dlist_push_front(dlist, data);

  ads_dlist_node_t* new_node = ads_dlist_new_node(data);
  if(new_node == NULL)
    return ADS_NOMEM;

  new_node->next = node;
  new_node->prev = node->prev;

  node->prev->next = new_node;
  node->prev = new_node;

  dlist->size++;

  return ADS_SUCCESS;
}

void ads_dlist_pop_front(ads_dlist_t* dlist, void** ret_data) {
  if(ads_dlist_is_empty(dlist))
    return;

  ads_dlist_node_t* old_head = ads_dlist_get_head(dlist);
  if(ret_data)
    *ret_data = old_head->data;

  dlist->head = old_head->next;

  dlist->size--;
  free(old_head);

  if(ads_dlist_is_empty(dlist))
    dlist->tail = NULL;
  else
    dlist->head->prev = NULL;
}

void ads_dlist_pop_back(ads_dlist_t* dlist, void** ret_data) {

}

void ads_dlist_remove_next(ads_dlist_t* dlist,
                           ads_dlist_node_t* node,
                           void** ret_data)
{
  if(node == NULL)
    ads_dlist_pop_front(dlist, ret_data);
  else if(node != ads_dlist_get_tail(dlist)) {
    ads_dlist_node_t* rem_node = ads_dlist_get_next(node);
    if(ret_data)
      *ret_data = rem_node->data;

    node->next = rem_node->next;
    if(node->next == NULL)
      dlist->tail = node;
    else
      node->next->prev = node;

    dlist->size--;
    free(rem_node);
  }
}

static inline
ads_dlist_node_t* ads_dlist_look_backward(ads_dlist_node_t* node, int steps) {
  for(int i = 0; i != steps && node; i++)
    node = node->prev;
  return node;
}

static inline
ads_dlist_node_t* ads_dlist_look_forward(ads_dlist_node_t* node, int steps) {
  for(int i = 0; i != steps && node; i++)
    node = node->next;
  return node;
}

ads_dlist_node_t* ads_dlist_get_at(ads_dlist_t* dlist, int index) {
  size_t dlist_size = ads_dlist_get_size(dlist);

  if(index < 0 || index >= dlist_size)
    return NULL;

  int middle = dlist_size / 2;

  if(index >= middle)
    return ads_dlist_look_backward(dlist->tail, (dlist_size - 1) - index);
  else
    return ads_dlist_look_forward(dlist->head, index);
}