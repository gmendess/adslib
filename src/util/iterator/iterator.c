#include <memory.h>
#include "iterator.h"
#include "../../list/list.h"
#include "../../dlist/dlist.h"
#include "../../string/string.h"


/**           DEFAULT ITERATORS            **/

static int
ads_iterator_list(ads_iterator_t* it) {
  ads_list_t* list = it->data_structure;
  
  if(it->curr_position == NULL)
    it->curr_position = ads_list_get_head(list);
  else
    it->curr_position = ads_list_get_next( (ads_list_node_t*) it->curr_position);

  return it->curr_position == NULL ? 0 : 1; 
}

static int
ads_iterator_string(ads_iterator_t* it) {
  ads_string_t* str = it->data_structure;

  if(it->curr_position == NULL)
    it->curr_position = &str->buf[0];
  else
    it->curr_position = ((char*) it->curr_position) + 1;
  
  return ( *((char*) it->curr_position) == '\0') ? 0 : 1;
}

static int
ads_iterator_dlist(ads_iterator_t* it) {
  ads_dlist_t* dlist = it->data_structure;

  if(it->curr_position == NULL)
    it->curr_position = ads_dlist_get_head(dlist);
  else
    it->curr_position = ads_dlist_get_next( (ads_dlist_node_t*) it->curr_position);

  return it->curr_position == NULL ? 0 : 1;
}

/** * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/

void ads_iterator_init(ads_iterator_t* it,
                       void*           data_structure,
                       it_function_t   it_func)
{
  it->data_structure = data_structure;
  it->it_func = it_func;
  it->curr_position = NULL;

  if     (it_func == ADS_ITERATOR_LIST)    it->it_func = ads_iterator_list;
  else if(it_func == ADS_ITERATOR_DLIST)   it->it_func = ads_iterator_list;
  else if(it_func == ADS_ITERATOR_STRING)  it->it_func = ads_iterator_string;
  else                                     it->it_func = it_func;
}

int ads_iterator_iterate(ads_iterator_t* it, void** value) {
  int ret = it->it_func(it);

  if(value != NULL)
    *value = it->curr_position;
  
  return ret;
}

void ads_iterator_destroy(ads_iterator_t* it) {
  memset(it, 0, sizeof(ads_iterator_t));
}