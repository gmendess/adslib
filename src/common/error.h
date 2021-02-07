#ifndef ADS_ERROR_H
#define ADS_ERROR_H

typedef enum {
  ADS_SUCCESS = 0,
  ADS_NOMEM,
  ADS_OUTOFBOUNDS,
} ads_status_t;

const char* ads_status_message(ads_status_t status);

#endif