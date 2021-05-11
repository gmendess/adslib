#ifndef ADS_ALGORITHM_H
#define ADS_ALGORITHM_H

#include "iterator.h"

typedef void (*ads_callback)(void* data);

void ads_foreach(ads_iterator_t* it, ads_callback func);

#endif