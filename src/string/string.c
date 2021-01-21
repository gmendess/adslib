#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <string.h>

static inline void
ads_string_init_optimize(ads_string_t* str) {
  str->capacity = BASIC_SIZE; // capacity is the size of basic_str
  str->ptr = str->basic_str;
}

static inline char* 
expand(ads_string_t* str, size_t new_capacity) {
  char* new_ptr = malloc(new_capacity + 1);
  if(new_ptr != NULL)
    memcpy(new_ptr, str->ptr, str->size + 1);

  str->capacity = new_capacity;
  return new_ptr;
  
  /*
    Note: this function can't be just a realloc, because str->ptr can point
    to str->basic_str. Thus, would be undefined behaviour to try to realloc a
    region of memory on the stack.
  */
}

static int
ads_string_internal_copy(ads_string_t* dest,
                         const char*   src_ptr,
                         size_t        src_size,
                         size_t        src_capacity)
{

  // the size of src_ptr' string is bigger than dest's capacity
  if(src_size > dest->capacity) {    
    // get a new region of memory that fits src_ptr's string
    char* new_ptr = malloc(src_capacity + 1);
    if(new_ptr == NULL)
      return -1;
    
    // if dest's string is in the free store(heap), we must free it before the assignment
    if(!ads_string_is_optimized(dest))
      free(dest->ptr);
  
    // points to the new region of memory that will store a copy of src_ptr's string
    dest->ptr = new_ptr;
  }
  // dest is not optimized, but src_ptr's string can be optimized (stored in dest->basic_str)
  else if(!ads_string_is_optimized(dest) && src_capacity == BASIC_SIZE) {
    free(dest->ptr);
    dest->ptr = dest->basic_str;
  }

  // copy :)
  memcpy(dest->ptr, src_ptr, src_size + 1);
  dest->capacity = src_capacity;
  dest->size     = src_size;

  return 0;
}

static int
ads_string_concat_internal(ads_string_t* dest,
                           const char*   src_ptr,
                           size_t        src_size,
                           size_t        src_capacity)
{
  size_t new_size = dest->size + src_size;

  // check if the new_size is greater than the current capacity. This check includes optimized strings
  if(new_size > dest->capacity) {
    size_t old_capacity = dest->capacity;

    // double the capacity
    char* new_ptr = expand(dest, dest->capacity * 2);
    if(new_ptr == NULL)
      return -1;
    
    if(old_capacity > BASIC_SIZE)
      free(dest->ptr);

    dest->ptr = new_ptr;
  }
  
  // concatenate src in dest
  memcpy(&dest->ptr[dest->size], src_ptr, src_size + 1);
  dest->size = new_size;

  return 0;
}

// auxiliary function to ads_boyer_moore_search
static const char*
ads_last_occurence(const char* str, size_t str_size, char c) {
  const char* last = &str[str_size - 1];

  while(last >= str) {
    if(*last == c)
      return last;
    *last--;
  }

  return NULL;
}

void ads_string_clear(ads_string_t* str) {
  if(!ads_string_is_optimized(str))
    free(str->ptr);

  memset(str, 0, sizeof(ads_string_t));
  ads_string_init_optimize(str);
}

int ads_string_init(ads_string_t* str, const char* init_str) {
  // create an empty, and optimized, string
  if(init_str == NULL) {
    memset(str, 0, sizeof(ads_string_t));
    ads_string_init_optimize(str);
    return 0;
  }

  str->size = strlen(init_str);

  // optimized string, because the string fits on the basic_str buffer 
  if(str->size <= BASIC_SIZE)
    ads_string_init_optimize(str);
  else { // string in the free store
    str->capacity = str->size;
    str->ptr = malloc(str->capacity + 1); // +1 = '\0'
    if(!str->ptr)
      return -1;
  }

  memcpy(str->ptr, init_str, str->size + 1);

  return 0;
}

void ads_string_destroy(ads_string_t* str) {
  // if the string is stored in heap, free the memory
  if(str->size > BASIC_SIZE)
    free(str->ptr);

  memset(str, 0, sizeof(ads_string_t));
}

int ads_string_concat(ads_string_t* dest, const ads_string_t* src) {
  if(dest == src)
    return 0;

  return ads_string_concat_internal(dest, src->ptr, src->size, src->capacity);
}

int ads_string_concat_literal(ads_string_t* restrict dest, const char* restrict  src) {
  size_t src_size = strlen(src);
  size_t src_capacity = (src_size > BASIC_SIZE) ? src_size : BASIC_SIZE; 

  return ads_string_concat_internal(dest, src, src_size, src_capacity);
}

const char* ads_string_contains(const ads_string_t* haystack, const ads_string_t* needle) {
  return strstr(haystack->ptr, needle->ptr);
}

int ads_string_substr(const ads_string_t* str, 
                      size_t              pos,
                      int                 count,
                      ads_string_t*       dest)
{
  if(pos >= str->size)
    return -1;
  
  if( count == -1 || (count > str->size - pos))
    dest->size = str->size - pos; // copy the whole string starting at pos
  else
    dest->size = count;

  if(dest->size <= BASIC_SIZE)
    ads_string_init_optimize(dest);
  else {
    dest->capacity = dest->size;
    dest->ptr = malloc(dest->capacity + 1);
    if(dest->ptr == NULL)
      return -1;
  }

  memcpy(dest->ptr, &str->ptr[pos], dest->size + 1);
  return 0;
}

int ads_string_copy(ads_string_t* dest, const ads_string_t* src) {
  return ads_string_internal_copy(dest, src->ptr, src->size, src->capacity);
}

int ads_string_copy_literal(ads_string_t* dest, const char* src) {
  size_t src_size = strlen(src);
  size_t src_capacity = (src_size > BASIC_SIZE) ? src_size : BASIC_SIZE; 

  return ads_string_internal_copy(dest, src, src_size, src_capacity);
}

void ads_string_move(ads_string_t* dest, ads_string_t* src) {
  memcpy(dest, src, sizeof(ads_string_t));

  if(ads_string_is_optimized(dest))
    dest->ptr = dest->basic_str;
  
  src->ptr = NULL;
  ads_string_clear(src);
}