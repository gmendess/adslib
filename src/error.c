#include "../include/error.h"

static const char*
ads_status_description[] = {
  "success",                // ADS_SUCCESS
  "cannot allocate memory", // ADS_NOMEM
  "index out of bounds"     // ADS_OUTOFBOUNDS
};

const char* ads_status_message(ads_status_t status) {
  return ads_status_description[status];
}