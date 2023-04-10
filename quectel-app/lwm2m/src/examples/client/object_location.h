#ifndef OBJECT_LOCATION_H_
#define OBJECT_LOCATION_H_

#include "liblwm2m.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VELOCITY_OCTETS                      5  // for HORIZONTAL_VELOCITY_WITH_UNCERTAINTY 
typedef struct
{
    float    latitude;
    float    longitude;
    float    altitude;
    float    radius;
    uint8_t  velocity   [VELOCITY_OCTETS];        //3GPP notation 1st step: HORIZONTAL_VELOCITY_WITH_UNCERTAINTY
    unsigned long timestamp;
    float    speed;
    float    bearing;
} location_data_t;

#endif