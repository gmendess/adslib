#ifndef ADS_STRING_H
#define ADS_STRING_H

#include <stdlib.h>

/*
  STRING HEADER
*/

#define BASIC_SIZE 15

typedef struct ads_string {
  size_t size;     // number of characters
  size_t capacity; // number of characters the string can hold at all
  char* ptr;       // pointer to basic_str(strings up to 15 chars) or to a region in free store(heap)

  /* optimization: the string will be store in this buffer if the numbers of characters in the
     string is up to 15. Small strings can be stored on the stack */
  char basic_str[BASIC_SIZE + 1]; // +1 = '\0'
} ads_string_t;

#define ads_string_get_buffer(str) ((const char* const) (str)->ptr)
#define ads_string_get_size(str) ((str)->size)
#define ads_string_is_empty(str) ((str)->size == 0)
#define ads_string_is_optimized(str) ((str)->capacity == BASIC_SIZE)

int ads_string_init(ads_string_t* str, const char* init_str);
void ads_string_destroy(ads_string_t* str);

int ads_string_concat(ads_string_t* dest, const ads_string_t* src);
int ads_string_concat_literal(ads_string_t* dest, const char* src);

const char* ads_string_contains(const ads_string_t* haystack, const ads_string_t* needle);

int ads_string_substr(const ads_string_t* str, size_t pos, int count, ads_string_t* dest);

int ads_string_copy(ads_string_t* dest, const ads_string_t* src);
int ads_string_copy_literal(ads_string_t* dest, const char* src);

void ads_string_move(ads_string_t* dest, ads_string_t* src);

void ads_string_clear(ads_string_t* str);

#endif