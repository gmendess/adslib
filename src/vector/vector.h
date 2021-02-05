#ifndef ADS_VECTOR_H
#define ADS_VECTOR_H

#include <stdlib.h>

typedef struct ads_vector ads_vector_t;

typedef void* (*ads_vector_copy_f)(void* restrict dest, const void* restrict src, size_t src_size);
typedef void (*ads_vector_destroy_f)(void* data);

typedef struct ads_vector {
  size_t data_size;
  size_t size;
  size_t capacity;
  void* buf;

  ads_vector_copy_f    copy;
  ads_vector_destroy_f destroy;
} ads_vector_t;

#define ads_vector_is_full(vec) ((vec)->size == (vec)->capacity)
#define ads_vector_get_idx_address(vec, index) ( &((char*)(vec)->buf)[(index) * (vec)->data_size] )
#define ads_vector_get_as(vec, type) ((type)(vec)->buf)

int ads_vector_init(ads_vector_t*       vec, 
                    size_t              data_size,
                    ads_vector_copy_f copy,
                    ads_vector_destroy_f destroy);

void ads_vector_clear(ads_vector_t* vec);
void ads_vector_destroy(ads_vector_t* vec);

int ads_vector_push_back(ads_vector_t* vec, void* data);
int ads_vector_push_front(ads_vector_t* vec, void* data);

void ads_vector_pop_back(ads_vector_t* vec);
void ads_vector_pop_front(ads_vector_t* vec);

int ads_vector_insert_at(ads_vector_t* vec, ssize_t index, void* data);
int ads_vector_get_at(ads_vector_t* vec, ssize_t index, void** out);
int ads_vector_copy(ads_vector_t* dest, const ads_vector_t* src);

#endif