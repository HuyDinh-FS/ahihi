//
//  algtypes.h
//  stepcount
//
//  Created by james ochs on 5/9/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#ifndef stepcount_algtypes_h
#define stepcount_algtypes_h

#include "stdint.h"

#ifdef _SC_ALG_USE_FLOAT
typedef float real_t;
typedef double real_t;
#else
typedef int32_t     real_t;
typedef int64_t     widereal_t;
#endif

typedef enum {X_AXIS=0,
    Y_AXIS,
    Z_AXIS} dimension_t;

#define N_ACCEL_AXES ((uint32_t)Z_AXIS+1)

typedef int16_t accel_value_t;

typedef struct
{
  accel_value_t x;
  accel_value_t y;
  accel_value_t z;
} accelData_t;

#define ACCEL_VALUE_MAX INT16_MAX


#endif
