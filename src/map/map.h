#ifndef ADS_MAP_H
#define ADS_MAP_H

#include <stdlib.h>
#include "../common/error.h"
#include "../dlist/dlist.h"

// implementation of a map based on a chained hash table

typedef struct ads_map_entry {
  void* key;
  void* value;
} ads_map_entry_t;

typedef struct ads_map {
  ads_dlist_t* htable; // each bucket is a doubly linked list
  size_t buckets;
  size_t size;

  void   (*destroy)(void* value); // destroy the value store into the map
  int    (*compare)(void* key1, void* key2); // function to compare two keys
  size_t (*hash)(void* key); // hash function
} ads_map_t;

#define ads_map_get_size(map)    ((map)->size)
#define ads_map_get_buckets(map) ((map)->buckets)
#define ads_map_is_empty(map)    (ads_map_get_size((map)) == 0)
#define ads_map_get_index(map, key) ((map)->hash((key)) % (map)->buckets)

#define ads_map_uint64_key(key) ((void*) ((size_t)(key)))

ads_status_t
ads_map_init(ads_map_t* map,
             size_t buckets,
             void   (*destroy)(void* value),
             int    (*compare)(void* key1, void* key2),
             size_t (*hash)(void* key));

void ads_map_destroy(ads_map_t* map);

size_t ADS_MAP_HASH_STRING(void* key_string);
int ADS_MAP_COMPARE_STRING(void* key_string1, void* key_string2);

size_t ADS_MAP_HASH_UINT64(void* key_uint64);
int ADS_MAP_COMPARE_UINT64(void* key_uint64a, void* key_uint64b);

ads_status_t ads_map_insert(ads_map_t* map, void* key, void* value);
ads_status_t ads_map_remove(ads_map_t* map, void* key, void** out);
ads_map_entry_t* ads_map_get(ads_map_t* map, void* key, void** out);

#endif