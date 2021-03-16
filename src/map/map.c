#include <stdlib.h>
#include <string.h>
#include "map.h"

/* ----- STRING KEY  ----- */

size_t ADS_MAP_HASH_STRING(void* key_string) {
  size_t hash = 5381;
  char* str = key_string;
  
  int c;
  while (c = *str++)
    hash = ((hash << 5) + hash) + c;

  return hash;
}

int ADS_MAP_COMPARE_STRING(void* key_string1, void* key_string2) {
  return strcmp(key_string1, key_string2) == 0;
}

/* ----- UNSIGNED INTEGER64 KEY  ----- */

size_t ADS_MAP_HASH_UINT64(void* key_uint64) {
  size_t hash = (size_t) key_uint64;
  
  hash ^= (hash >> 33);
  hash *= 0xff51afd7ed558ccd;
  hash ^= (hash >> 33);
  hash *= 0xc4ceb9fe1a85ec53;
  hash ^= (hash >> 33);

  return hash;
}

int ADS_MAP_COMPARE_UINT64(void* key_uint64a, void* key_uint64b) {
  return ((size_t) key_uint64a) == ((size_t) key_uint64b);
}

/* ---------- */

ads_status_t
ads_map_init(ads_map_t* map,
             size_t buckets,
             void   (*destroy)(void* value),
             int    (*compare)(void* key1, void* key2),
             size_t (*hash)(void* key))
{
  // allocate memory for each bucket
  map->htable = calloc(buckets, sizeof(ads_dlist_t));
  if(!map->htable)
    return ADS_NOMEM;

  // create a linked lists in each bucket
  for(size_t i = 0; i < buckets; i++)
    ads_dlist_init(&map->htable[i], free);

  map->buckets = buckets;
  map->destroy = destroy;
  map->compare = compare;
  map->hash    = hash;

  return ADS_SUCCESS;
}

static ads_map_entry_t* 
ads_map_create_entry(void* key, void* value) {
  ads_map_entry_t* entry = calloc(1, sizeof(ads_map_entry_t));
  if(entry) {
    entry->key = key;
    entry->value = value;
  }

  return entry;
}

ads_status_t 
ads_map_insert(ads_map_t*  map,
               void*       key,
               void*       value)
{
  ads_status_t status = ADS_SUCCESS;

  // try to find the key in the map
  ads_map_entry_t* entry = ads_map_get(map, key, NULL);
  
  // key already exist, so we need to update with the new value, and free the old one
  if(entry) {
    if(map->destroy) map->destroy(entry->value);
    entry->value = value;
  }
  else { // key doens't exist, so we need to create an entry
    entry = ads_map_create_entry(key, value);
    if(entry == NULL)
      return ADS_NOMEM; // failed to create the entry
  
    size_t index = ads_map_get_index(map, key);
    ads_status_t status = ads_dlist_push_back(&map->htable[index], entry);
    if(status != ADS_SUCCESS)
      free(entry); // failed to push entry in the list
    else
      map->size++;
  }

  return status;
}

void ads_map_destroy(ads_map_t* map) {
  
  for(size_t i = 0; i < map->buckets && map->size > 0; i++) {
    ads_dlist_t* dlist = &map->htable[i];
    size_t dlist_size = ads_dlist_get_size(dlist);

    ads_dlist_node_t* node = ads_dlist_get_head(dlist);
    while(node) {
      ads_map_entry_t* entry = ads_dlist_get_data_as(node, ads_map_entry_t*);
      if(map->destroy)
        map->destroy(entry->value);

      map->size--;
      node = ads_dlist_get_next(node);
    }
    ads_dlist_destroy(dlist);
  }

  free(map->htable);
}

static ads_dlist_node_t*
ads_map_get_key_node(ads_map_t* map, void* key) {
  size_t index = ads_map_get_index(map, key);
  ads_dlist_t* dlist = &map->htable[index];

  ads_dlist_node_t* node = ads_dlist_get_head(dlist);
  while(node) {

    ads_map_entry_t* entry = ads_dlist_get_data_as(node, ads_map_entry_t*);
    if(map->compare(key, entry->key))
      return node;

    node = ads_dlist_get_next(node);
  }

  return NULL;
}

ads_map_entry_t*
ads_map_get(ads_map_t* map, void* key, void** out) {

  ads_map_entry_t* entry = NULL;
  ads_dlist_node_t* key_node = ads_map_get_key_node(map, key);
  if(key_node) {
    entry = ads_dlist_get_data_as(key_node, ads_map_entry_t*);
    if(out) *out = entry->value;
  }

  return entry;
}

ads_status_t ads_map_remove(ads_map_t* map, void* key, void** out) {
  ads_dlist_node_t* key_node = ads_map_get_key_node(map, key);
  if(!key_node)
    return ADS_NOTFOUND;
  
  ads_dlist_node_t* prev = key_node->prev;

  ads_map_entry_t* entry = NULL;
  ads_dlist_remove_next(&map->htable[ads_map_get_index(map, key)], prev, (void*) &entry);
  
  void* value = entry->value;
  if(out)
    *out = value;
  else
    map->destroy(value);
  
  map->size--;
  free(entry);

  return ADS_SUCCESS;
}