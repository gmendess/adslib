#ifndef ADS_ITERATOR_H
#define ADS_ITERATOR_H

typedef struct ads_iterator ads_iterator_t;
typedef int (*it_function_t)(ads_iterator_t*);

typedef struct ads_iterator {
  void* data_structure;
  void* curr_position;
  it_function_t it_func;
} ads_iterator_t;


#define ADS_ITERATOR_LIST   ( (it_function_t) 1) 
#define ADS_ITERATOR_DLIST  ( (it_function_t) 2) 
#define ADS_ITERATOR_STRING ( (it_function_t) 3)

#define ads_iterator_reset(it) ((it)->curr_position = NULL)

void ads_iterator_init(ads_iterator_t* it, void* data_structure, it_function_t it_func);
int ads_iterator_iterate(ads_iterator_t* it, void** value);
void ads_iterator_destroy(ads_iterator_t* it);

#endif