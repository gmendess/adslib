#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

static inline void
ads_string_init_optimize(ads_string_t* str) {
  str->capacity = BASIC_SIZE; // capacity is the size of basic_str
  str->ptr = str->basic_str;
}

static inline char* 
expand(ads_string_t* str, size_t new_capacity) {
  char* new_ptr = calloc(new_capacity + 1, sizeof(char));
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

static inline int
double_capacity(ads_string_t* str) {
  size_t old_capacity = str->capacity;

  // double the capacity
  char* new_ptr = expand(str, str->capacity * 2);
  if(new_ptr == NULL)
    return -1;

  if(old_capacity > BASIC_SIZE)
    free(str->ptr);

  str->ptr = new_ptr;
  return 0;
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
    char* new_ptr = calloc(src_capacity + 1, sizeof(char));
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
    if(double_capacity(dest) != 0)
      return -1;
  }
  
  // concatenate src in dest
  // &dest->ptr[dest->size] = address of `\0` in dest->ptr
  memcpy(&dest->ptr[dest->size], src_ptr, src_size + 1);
  dest->size = new_size;

  return 0;
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
  else {
    // string in the free store, alloc memory
    str->capacity = str->size;
    str->ptr = calloc(str->capacity + 1, sizeof(char)); // +1 = '\0'
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

// the string will lost characters at the end of the process
static int
ads_string_replace_lost_char(ads_string_t* str,
                             const char*   old_str,
                             size_t        old_str_size,
                             const char*   new_str,
                             size_t        new_str_size)
{
  /*
    Example:
    str = "12hey345"
    old_str = "hey"
    new_str = "hi"
  */

  char* save_str_ptr = str->ptr; // "12hey345"; this pointer will be manipulated
  char* found = NULL;

  int count_replaces = 0;
  size_t size_diff = old_str_size - new_str_size; // 3("hey") - 2("hi") = 1

  // while old_str is found in save_str_ptr
  while( (found = strstr(save_str_ptr, old_str)) ) {
    ++count_replaces;

    /*
      in the example, `found` will be the address where "hey" starts in `save_str_ptr`
      "hey" starts at the position 3 in the string
    */

    char* rest = found + old_str_size; // rest of text (in the ex. = "345")
    size_t rest_size = str->size - (rest - str->ptr); // size of rest, in the ex. = 3

    memcpy(found, new_str, new_str_size); // copy "hi" where "hey" is (str = "12hiy345")

    // move the `rest` of the string backward, also deleting the characters of `old_str` that stayed behind
    memcpy(found + new_str_size, rest, rest_size); // now, str = "12hi345", note that "345" overlapped the char "y"

    str->size -= size_diff; // decrements the number of chars lost ("hey" - "hi" = 1)
    str->ptr[str->size] = '\0'; // reajust the end of string

    // move forward in the string (in the ex. save_str_ptr will be "345"; "12hi" has already been processed)
    save_str_ptr = found + new_str_size;
  }

  return count_replaces;
}

// the string will gain characters at the end of the process
static int
ads_string_replace_gain_char(ads_string_t* str,
                             const char*   old_str,
                             size_t        old_str_size,
                             const char*   new_str,
                             size_t        new_str_size)
{
  /*
    Example:
    str = "12hi345"
    old_str = "hi"
    new_str = "hey"
  */

  // save_str_ptr = "12hi345"; this pointer will be manipulated to iterate over the string
  char* save_str_ptr = str->ptr;
  char* found = NULL;

  int count_replaces = 0;
  size_t size_diff = new_str_size - old_str_size; // 3("hey") - 2("hi") = 1

  // while old_str is found in save_str_ptr
  while( (found = strstr(save_str_ptr, old_str)) ) {
    ++count_replaces;

    /*
      in the example, `found` will be the address where "hey" starts in `save_str_ptr`
      "hey" starts at the position 3 in the string
    */

    size_t new_size = str->size + size_diff;
    if(new_size > str->capacity) {
      size_t found_offset = found - str->ptr;
      if(double_capacity(str) != 0)
        return -1;
      found = &str->ptr[found_offset]; // reajust found pointer after realloc
    }

    char* rest = found + old_str_size; // rest of text (in the ex. = "345")
    size_t rest_size = str->size - (rest - str->ptr); // size of rest, in the ex. = 3

    /* move the text forward to fit the new_str
       str->ptr = "12hi3345" */
    memmove(found + new_str_size, found + old_str_size, rest_size);

    /* copy new_str where the old_str was
       str->ptr = "12hey345"; note that the duplicated "3" disappear*/
    memcpy(found, new_str, new_str_size);

    save_str_ptr = found + new_str_size;
    str->size += size_diff;
  }

  str->ptr[str->size] = '\0';

  return count_replaces;
}

static int
ads_string_replace_equal_char(ads_string_t* str,
                             const char*   old_str,
                             const char*   new_str,
                             size_t        size)
{
  char* save_str_ptr = str->ptr;
  char* found = NULL;

  int count_replaces = 0;

  // while old_str is found in save_str_ptr
  while( (found = strstr(save_str_ptr, old_str)) ) {
    ++count_replaces;
    memcpy(found, new_str, size);
    save_str_ptr = found + size;
  }

  return count_replaces;
}

int ads_string_replace(ads_string_t* str, const char* old_str, const char* new_str) {
  size_t old_str_size = strlen(old_str);
  if(old_str_size > str->size)
    return 0;

  size_t new_str_size = strlen(new_str);
  if(new_str_size < old_str_size)
    return ads_string_replace_lost_char(str, old_str, old_str_size, new_str, new_str_size);
  else if(new_str_size > old_str_size)
    return ads_string_replace_gain_char(str, old_str, old_str_size, new_str, new_str_size);
  else
    return ads_string_replace_equal_char(str, old_str, new_str, new_str_size);

  return 0;

}