#include <stdlib.h>
#include <memory.h>
#include "vector.h"

#define ADS_VECTOR_PRE_ALLOCATE 4

static inline int
double_capacity(ads_vector_t* vec)
{
  void* new_buf = realloc(vec->buf, (vec->capacity * vec->data_size) * 2);
  if(new_buf == NULL)
    return 0;

  vec->buf = new_buf;
  vec->capacity *= 2;

  return 1;
}

ads_status_t
ads_vector_init(ads_vector_t*        vec,
                size_t               data_size,
                ads_vector_copy_f    copy,
                ads_vector_destroy_f destroy)
{

  // pre-allocate ADS_VECTOR_PRE_ALLOCATE elements of size = data_size
  vec->buf = calloc(ADS_VECTOR_PRE_ALLOCATE, data_size);
  if(vec->buf == NULL)
    return ADS_NOMEM;

  vec->data_size = data_size;
  vec->size = 0;
  vec->capacity = ADS_VECTOR_PRE_ALLOCATE;

  vec->copy = copy ? copy : memcpy;
  vec->destroy = destroy;

  return ADS_SUCCESS;
}

void ads_vector_clear(ads_vector_t* vec) {
  if(vec->destroy) {
    for(int i = 0; i < vec->size; i++)
      vec->destroy(ads_vector_get_idx_address(vec, i));
  }
  memset(vec->buf, 0, vec->size * vec->data_size);
  vec->size = 0;
}

void ads_vector_destroy(ads_vector_t* vec) {
  ads_vector_clear(vec);
  free(vec->buf);    
}

ads_status_t ads_vector_push_back(ads_vector_t* vec, void* data) {

  if(ads_vector_is_full(vec)) {
    if(!double_capacity(vec))
      return ADS_NOMEM;
  }

  void* end = ads_vector_get_idx_address(vec, vec->size);
  vec->copy(end, data, vec->data_size); // memcpy or custom function
  ++vec->size;

  return ADS_SUCCESS;
}

ads_status_t ads_vector_push_front(ads_vector_t* vec, void* data) {
  
  if(ads_vector_is_full(vec)) {
    if(!double_capacity(vec))
      return ADS_NOMEM;
  }

  void* start = ads_vector_get_idx_address(vec, 1);
  memmove(start, vec->buf, vec->size * vec->data_size);
  vec->copy(vec->buf, data, vec->data_size); // memcpy or custom function
  ++vec->size;

  return ADS_SUCCESS;
}

ads_status_t ads_vector_insert_at(ads_vector_t* vec, ssize_t index, void* data) {
  if(index < 0 || index > vec->size)
    return ADS_OUTOFBOUNDS;
  
  if(index == 0)
    return ads_vector_push_front(vec, data);
  else if(index == vec->size)
    return ads_vector_push_back(vec, data);
  else {
    
    if(ads_vector_is_full(vec)) {
      if(!double_capacity(vec))
        return ADS_NOMEM;
    }

    void* idx_start = ads_vector_get_idx_address(vec, index);
    void* dest_start = ads_vector_get_idx_address(vec, index + 1);
    size_t bytes_idx_up_to_end = (vec->size - index) * vec->data_size; 
    
    memmove(dest_start, idx_start, bytes_idx_up_to_end);
    vec->copy(idx_start, data, vec->data_size); // memcpy or custom function

    return ADS_SUCCESS;

  }
}

ads_status_t ads_vector_get_at(ads_vector_t* vec, ssize_t index, void** out) {
  if(index < 0 || index >= vec->size)
    return ADS_OUTOFBOUNDS;

  if(out)
   *out = ads_vector_get_idx_address(vec, index);

  return ADS_SUCCESS;
}

ads_status_t ads_vector_copy(ads_vector_t* dest, const ads_vector_t* src) {
  dest->buf = calloc(src->capacity, src->data_size);
  if(dest->buf == NULL)
    return ADS_NOMEM;

  dest->capacity  = src->capacity;
  dest->size      = src->size;
  dest->data_size = src->data_size;

  dest->copy      = src->copy;
  dest->destroy   = src->destroy;

  memcpy(dest->buf, src->buf, src->size * src->data_size);
  return ADS_SUCCESS;
}

static inline void 
ads_vector_remove_internal(ads_vector_t* vec, size_t index) {
  void* rem_idx = ads_vector_get_idx_address(vec, index);
  if(vec->destroy)
    vec->destroy(rem_idx);

  memset(rem_idx, 0, sizeof(vec->data_size));
  --vec->size;
}

void ads_vector_pop_back(ads_vector_t* vec) {
  if(vec->size > 0)
    ads_vector_remove_internal(vec, vec->size - 1);
}

void ads_vector_pop_front(ads_vector_t* vec) {
  if(vec->size > 0)
    ads_vector_remove_internal(vec, 0);
  
  if(vec->size > 0) {
    void* src = ads_vector_get_idx_address(vec, 1);
    memcpy(vec->buf, src, vec->size * vec->data_size);
  }
}