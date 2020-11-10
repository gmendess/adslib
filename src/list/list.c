#include "list.h"
#include <stdlib.h>
#include <memory.h>

void ads_list_init(ads_list_t* list, void (*destroy)(void*)) {
  list->destroy = destroy;
  list->head    = NULL;
  list->tail    = NULL;
  list->size    = 0;
}

void ads_list_clean(ads_list_t* list) {
  while(!ads_list_is_empty(list))
    ads_list_remove_front(list, NULL);
}

void ads_list_destroy(ads_list_t* list) {
  if(list->destroy == NULL)
    ads_list_clean(list);
  else {
    void* data = NULL;

    while(!ads_list_is_empty(list)) {
      ads_list_remove_front(list, &data);
      list->destroy(data);
    }
  }

  memset(list, 0, sizeof(ads_list_t));
}

static inline 
ads_list_node_t* ads_list_new_node(void* data) {
  ads_list_node_t* new_node = malloc(sizeof(ads_list_node_t));
  if(new_node) {
    new_node->data = data;
    new_node->next = NULL;
  }

  return new_node;
}

int ads_list_add_back(ads_list_t* list, void* data) {
  ads_list_node_t* new_node = ads_list_new_node(data);
  if(new_node == NULL)
    return -1;

  if(ads_list_is_empty(list))
    list->head = new_node;
  else
    list->tail->next = new_node;

  list->tail = new_node;

  list->size++;

  return 0;
}

int ads_list_add_front(ads_list_t* list, void* data) {
  ads_list_node_t* new_node = ads_list_new_node(data);
  if(new_node == NULL)
    return -1;

  if(ads_list_is_empty(list))
    list->tail = new_node;

  new_node->next = list->head;
  list->head = new_node;

  list->size++;

  return 0;
}

int ads_list_add_next(ads_list_t*      list, 
                      ads_list_node_t* node, 
                      void*            data)
{
  if(node == NULL) 
    return ads_list_add_front(list, data);
  else if(node == ads_list_get_tail(list))
    return ads_list_add_back(list, data);

  ads_list_node_t* new_node = ads_list_new_node(data);
  if(new_node == NULL)
    return -1;
  
  new_node->next = node->next;
  node->next = new_node;  

  list->size++;

  return 0;
}

ads_list_node_t* ads_list_get_at(ads_list_t* list, int index) {
  if(index < 0 || index >= list->size)
    return NULL;

  ads_list_node_t* aux = ads_list_get_head(list);
  for(int i = 0; i < index; i++)
    aux = ads_list_get_next(aux);

  return aux;
}

int ads_list_remove_front(ads_list_t* list, void** ret_data) {
  if(ads_list_is_empty(list))
    return -1;

  ads_list_node_t* node = ads_list_get_head(list);
  if(ret_data)
    *ret_data = node->data;

  list->head = node->next;

  free(node);
  list->size--;

  if(ads_list_get_size(list) == 0)
    list->tail = NULL;

  return 0;
}

int ads_list_remove_next(ads_list_t*      list, 
                         ads_list_node_t* node,
                         void**           ret_data)
{
  if(node == NULL)
    return ads_list_remove_front(list, ret_data);
  else if(node == ads_list_get_tail(list))
    return -1;

  ads_list_node_t* rem_node = node->next;
  if(ret_data)
    *ret_data = rem_node->data;

  node->next = rem_node->next;
  if(node->next == NULL)
    list->tail = node;

  free(rem_node);
  list->size--;

  return 0;
}

int ads_list_remove_back(ads_list_t* list, void** ret_data) {
  if(ads_list_is_empty(list))
    return -1;
  else if(ads_list_get_size(list) == 1)
    return ads_list_remove_front(list, ret_data);

  ads_list_node_t* node = ads_list_get_at(list, list->size - 2);
  return ads_list_remove_next(list, node, ret_data);
}