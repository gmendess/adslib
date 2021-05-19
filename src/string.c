#include "../include/string.h"

#ifdef ADS_STRING_EXTENDED
#include "../include/list.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <ctype.h>

static inline void
ads_string_init_optimized(ads_string_t* str) {
  str->capacity = BASIC_SIZE; // capacity is the size of basic_str
  str->buf = str->basic_str;
}

static inline char* 
expand(ads_string_t* str, size_t new_capacity) {

  char* new_buf = calloc(new_capacity + 1, sizeof(char));
  if(new_buf) {
    memcpy(new_buf, str->buf, str->size + 1);

    if(!ads_string_is_optimized(str))
      free(str->buf);

    str->buf = new_buf;
    str->capacity = new_capacity;
  }

  return new_buf;
  
  /*
    Note: this function can't be just a realloc, because str->buf can point
    to str->basic_str. Thus, would be undefined behaviour to try to realloc a
    region of memory on the stack.
  */
}

static ads_status_t
ads_string_internal_copy(ads_string_t* restrict dest,
                         const char*   restrict src_buf,
                         size_t        src_size,
                         size_t        src_capacity)
{
  if(dest->buf == src_buf)
    return ADS_SUCCESS;

  // the size of src_buf' string is bigger than dest's capacity
  if(src_size > dest->capacity) {    
    // get a new region of memory that fits src_buf's string
    char* new_buf = calloc(src_capacity + 1, sizeof(char));
    if(new_buf == NULL)
      return ADS_NOMEM;

    // if dest's string is in the free store(heap), we must free it before the assignment
    if(!ads_string_is_optimized(dest))
      free(dest->buf);

    // points to the new region of memory that will store a copy of src_buf's string
    dest->buf = new_buf;
  }
  // dest is not optimized, but src_buf's string can be optimized (stored in dest->basic_str)
  else if(!ads_string_is_optimized(dest) && src_capacity == BASIC_SIZE) {
    free(dest->buf);
    dest->buf = dest->basic_str;
  }

  // copy :)
  memcpy(dest->buf, src_buf, src_size + 1);
  dest->capacity = src_capacity;
  dest->size     = src_size;

  return ADS_SUCCESS;
}

static ads_status_t
ads_string_concat_internal(ads_string_t* restrict dest,
                           const char*   restrict src_buf,
                           size_t        src_size)
{
  size_t new_size = dest->size + src_size;

  // check to expand the buffer if necessary
  if(new_size > dest->capacity) {
    if( (src_buf = expand(dest, new_size * 2)) == NULL)
      return ADS_NOMEM;
  }

  // concatenate src in dest
  // &dest->buf[dest->size] = address of `\0` in dest->buf
  memcpy(&dest->buf[dest->size], src_buf, src_size + 1);
  dest->size = new_size;

  return ADS_SUCCESS;
}

void ads_string_clear(ads_string_t* str) {
  if(!ads_string_is_optimized(str))
    free(str->buf);

  memset(str, 0, sizeof(ads_string_t));
  ads_string_init_optimized(str);
}

ads_status_t ads_string_init(ads_string_t* restrict str, const char* restrict init_str) {
  memset(str, 0, sizeof(ads_string_t));

  if(init_str == NULL)
    init_str = "";

  str->size = strlen(init_str);

  // optimized string, because the string fits in the basic_str buffer 
  if(str->size <= BASIC_SIZE)
    ads_string_init_optimized(str);
  else {
    // string in the free store, alloc memory
    str->capacity = str->size;
    str->buf = calloc(str->capacity + 1, sizeof(char)); // +1 = '\0'
    if(!str->buf)
      return ADS_NOMEM;
  }

  memcpy(str->buf, init_str, str->size + 1);

  return ADS_SUCCESS;
}

void ads_string_destroy(ads_string_t* str) {
  // if the string is stored in heap, free the memory
  if(!ads_string_is_optimized(str))
    free(str->buf);

  memset(str, 0, sizeof(ads_string_t));
}

ads_status_t ads_string_concat(ads_string_t* dest, const ads_string_t* src) {
  return ads_string_concat_internal(dest, src->buf, src->size);
}

ads_status_t ads_string_concat_cstr(ads_string_t* dest, const char* src) {
  return ads_string_concat_internal(dest, src, strlen(src));
}

const char* ads_string_contains(const ads_string_t* haystack, const ads_string_t* needle) {
  return strstr(haystack->buf, needle->buf);
}

const char* ads_string_contains_cstr(const ads_string_t* restrict haystack, const char* restrict needle) {
  return strstr(haystack->buf, needle);
}

ads_status_t
ads_string_substr(const ads_string_t* src,
                  size_t              pos,
                  int                 count,
                  ads_string_t*       dest)
{
  if(pos >= src->size)
    return ADS_OUTOFBOUNDS;

  // calculate `dest` new size
  if( count == -1 || ( (size_t)count > src->size - pos) )
    dest->size = src->size - pos; // the whole string starting at pos
  else
    dest->size = count;

  if(dest->size > dest->capacity) {
    if(expand(dest, dest->size) == NULL)
      return ADS_NOMEM;
  }

  memcpy(dest->buf, &src->buf[pos], dest->size);
  dest->buf[dest->size] = '\0';

  return ADS_SUCCESS;
}

ads_status_t ads_string_copy(ads_string_t* dest, const ads_string_t* src) {
  return ads_string_internal_copy(dest, src->buf, src->size, src->capacity);
}

ads_status_t ads_string_copy_cstr(ads_string_t* dest, const char* src) {
  size_t src_size = strlen(src);
  size_t src_capacity = (src_size > BASIC_SIZE) ? src_size : BASIC_SIZE; 

  return ads_string_internal_copy(dest, src, src_size, src_capacity);
}

void ads_string_move(ads_string_t* restrict dest, ads_string_t* restrict src) {
  if(dest == src) return;

  memcpy(dest, src, sizeof(ads_string_t));

  if(ads_string_is_optimized(dest))
    dest->buf = dest->basic_str;

  src->buf = NULL;
  ads_string_clear(src);
}

// the string will lost characters at the end of the process
static int
ads_string_replace_lose_char(ads_string_t* restrict str,
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

  char* save_str_buf = str->buf; // "12hey345"; this pointer will be manipulated
  char* found = NULL;

  int count_replaces = 0;
  size_t size_diff = old_str_size - new_str_size; // 3("hey") - 2("hi") = 1

  // while old_str is found in save_str_buf
  while( save_str_buf[0] && (found = strstr(save_str_buf, old_str)) ) {
    ++count_replaces;

    /*
      in the example, `found` will be the address where "hey" starts in `save_str_buf`
      "hey" starts at the position 3 in the string
    */

    char* rest = found + old_str_size; // rest of text (in the ex. = "345")
    size_t rest_size = str->size - (rest - str->buf); // size of rest, in the ex. = 3

    memcpy(found, new_str, new_str_size); // copy "hi" where "hey" is (str = "12hiy345")

    // move the `rest` of the string backward, also deleting the characters of `old_str` that stayed behind
    memcpy(found + new_str_size, rest, rest_size); // now, str = "12hi345", note that "345" overlapped the char "y"

    str->size -= size_diff; // decrements the number of chars lost ("hey" - "hi" = 1)
    str->buf[str->size] = '\0'; // reajust the end of string

    // move forward in the string (in the ex. save_str_buf will be "345"; "12hi" has already been processed)
    save_str_buf = found + new_str_size;
  }

  return count_replaces;
}

// the string will gain characters at the end of the process
static int
ads_string_replace_gain_char(ads_string_t* restrict str,
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

  // save_str_buf = "12hi345"; this pointer will be manipulated to iterate over the string
  char* save_str_buf = str->buf;
  char* found = NULL;

  int count_replaces = 0;
  size_t size_diff = new_str_size - old_str_size; // 3("hey") - 2("hi") = 1

  // while old_str is found in save_str_buf
  while( save_str_buf[0] && (found = strstr(save_str_buf, old_str)) ) {
    ++count_replaces;

    /*
      in the example, `found` will be the address where "hey" starts in `save_str_buf`
      "hey" starts at the position 3 in the string
    */

    size_t new_size = str->size + size_diff;
    if(new_size > str->capacity) {
      size_t found_offset = found - str->buf;
      if(expand(str, new_size * 2) == NULL)
        return -1;
      found = &str->buf[found_offset]; // reajust found pointer after `expand`
    }

    char* rest = found + old_str_size; // rest of text (in the ex. = "345")
    size_t rest_size = str->size - (rest - str->buf); // size of rest, in the ex. = 3

    /* move the text forward to fit the new_str
       str->buf = "12hi3345" */
    memmove(found + new_str_size, found + old_str_size, rest_size);

    /* copy new_str where the old_str was
       str->buf = "12hey345"; note that the duplicated "3" disappear*/
    memcpy(found, new_str, new_str_size);

    save_str_buf = found + new_str_size;
    str->size += size_diff;
  }

  str->buf[str->size] = '\0';

  return count_replaces;
}

static int
ads_string_replace_equal_char(ads_string_t* restrict str,
                             const char*   old_str,
                             const char*   new_str,
                             size_t        size)
{
  char* save_str_buf = str->buf;
  char* found = NULL;

  int count_replaces = 0;

  // while old_str is found in save_str_buf
  while( (found = strstr(save_str_buf, old_str)) ) {
    ++count_replaces;
    memcpy(found, new_str, size);
    save_str_buf = found + size;
  }

  return count_replaces;
}

int ads_string_replace(ads_string_t* restrict str, const char* old_str, const char* new_str) {
  size_t old_str_size = strlen(old_str);
  if(old_str_size > str->size)
    return 0;

  size_t new_str_size = strlen(new_str);
  if(new_str_size < old_str_size)
    return ads_string_replace_lose_char(str, old_str, old_str_size, new_str, new_str_size);
  else if(new_str_size > old_str_size)
    return ads_string_replace_gain_char(str, old_str, old_str_size, new_str, new_str_size);
  else
    return ads_string_replace_equal_char(str, old_str, new_str, new_str_size);
}

void ads_string_trim(ads_string_t* str) {
  ads_string_ltrim(str);
  ads_string_rtrim(str);
}

void ads_string_ltrim(ads_string_t* str) {
  if(ads_string_is_empty(str))
    return;

  char* c = str->buf;
  while(isspace(*c))
    ++c;

  str->size = str->size - (c - str->buf);
  memcpy(str->buf, c, str->size + 1); // + 1 = also copy the '\0'
}

void ads_string_rtrim(ads_string_t* str) {
  if(ads_string_is_empty(str))
    return;

  char* c = &str->buf[str->size - 1];
  while(c >= str->buf && isspace(*c))
    --c;
  ++c;

  str->size = c - str->buf;
  *c = '\0';
}

// compile with -DADS_STRING_EXTENDED option
#ifdef ADS_STRING_EXTENDED

static void
ads_string_list_destroy(void* data) {
  ads_string_t* _data = data;

  ads_string_destroy(_data);
  free(_data);
}

static ads_string_t*
ads_string_split_create_data(const char* str, size_t str_size) {

  ads_string_t* data = calloc(1, sizeof(ads_string_t));
  if(data == NULL)
    goto nomem_err_1; // just return NULL

  if(str_size > BASIC_SIZE) {
    data->buf = calloc(str_size, sizeof(char));
    if(data->buf == NULL)
      goto nomem_err_2; // free `data` and return NULL
    data->capacity = str_size;
  }
  else {
    data->buf = data->basic_str;
    data->capacity = BASIC_SIZE;
  }

  data->size = str_size;
  memcpy(data->buf, str, str_size);
  return data; // success, return data

// on error, go to one of these labels, free memory(if needed) and return NULL
nomem_err_2:
  free(data);
nomem_err_1:
  return NULL;
}

int ads_string_split(ads_string_t* str, const char* delimiter, ads_list_t* out) {
  out->destroy = ads_string_list_destroy;

  if(delimiter[0] == '\0')
    return 0;
  size_t delimiter_size = strlen(delimiter);

  char* save_str_buf  = str->buf,
      * found         = NULL;

  // data that will be pushed into the list
  ads_string_t* data = NULL;

  while( (found = strstr(save_str_buf, delimiter)) != NULL ) {
    size_t found_size = found - save_str_buf;

    // create an ads_string_t to be pushed into the list
    data = ads_string_split_create_data(save_str_buf, found_size);
    if(data == NULL)
      return -1;

    if(ads_list_push_back(out, data) != ADS_SUCCESS) // push data into the list
      return -1;
    save_str_buf = found + delimiter_size;
  }

  size_t rest_size = str->size - (save_str_buf - str->buf);
  if(rest_size > 0 && rest_size != str->size) {
    // create an ads_string_t to be pushed into the list
    data = ads_string_split_create_data(save_str_buf, rest_size);
    if(data == NULL)
      return -1;

    if(ads_list_push_back(out, data) != ADS_SUCCESS) // push data into the list
      return -1;
  }

  return out->size;
}
#endif