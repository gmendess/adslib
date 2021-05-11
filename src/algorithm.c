#include "../include/algorithm.h"
#include "../include/iterator.h"
#include <stdlib.h>

void ads_foreach(ads_iterator_t* it, ads_callback func) {
  void* data = NULL;
  while(ads_iterator_iterate(it, &data))
    func(data);
}