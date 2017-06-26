//
//  TapTypes.h
//  tapdetect
//
//  Created by james ochs on 5/4/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#ifndef tapdetect_TapTypes_h
#define tapdetect_TapTypes_h

#include "scalgtypes.h"

#ifdef _TD_USE_SHINE_INTERFACE
#include "accel_data.h"
typedef accel_sample_basic_t tap_detect_sample_t;
#else
typedef struct
{
  accel_value_t z;
  accel_value_t y;
  accel_value_t x;
}tap_detect_sample_t;
#endif

//TODO this should be renamed to not confuse things with
//x,y accel axies
typedef struct
{
  uint32_t x;
  uint32_t y;
}point_t;

typedef struct
{
  //start end and maximum point of tap
  point_t startPoint;
  point_t endPoint;
  point_t maxPoint;
  
  //confidence
  int32_t conf;
  
  //distance between this tap's maximum and the previous
  //maximum
  uint32_t distMax;
  
  //distance from the end of the previous tap to the start
  //of this tap
  uint32_t distEndToStart;
  
  int32_t magnitudeBefore;
  
}tap_t;

typedef struct
{
  uint32_t numTaps;
  uint32_t minDist;
  uint32_t maxDist;
  uint32_t minPwr;
  uint32_t maxPwr;
  uint32_t endIndex;
  uint32_t maxMagBefore;
  uint32_t minMagBefore;
  int32_t  minConf;
  int32_t  maxConf;
  int32_t conf;
  
}tap_group_t;

#endif
