//
//  TapProfiler.h
//  tapdetect
//
//  Created by james ochs on 5/7/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#ifndef __tapdetect__TapProfiler__
#define __tapdetect__TapProfiler__


#ifdef _MEX
#define _ENABLE_TAP_STATE_PROFILING
#endif

#ifdef _ENABLE_TAP_STATE_PROFILING

#include<stdint.h>

typedef struct
{
  uint32_t isMotion;
  uint32_t foundUpSlope;
  uint32_t qualifyTap;
  uint32_t qualifyDoubleTap;
  uint32_t qualifyMoreThanDoubleTap;
}tap_state_profile_t;

extern void TapStateProfiler_Reset(void);
extern void TapStateProfiler_IsMotion(void);
extern void TapStateProfiler_UpSlopeFound(void);
extern void TapStateProfiler_QualifyTap(void);
extern void TapStateProfiler_QualifyDoubleTap(void);
extern void TapStateProfiler_QualifyMoreThanDoubleTap(void);


extern const tap_state_profile_t *TapStateProfiler_GetStats(void);


#else

#define TapStateProfiler_Reset()
#define TapStateProfiler_IsMotion()
#define TapStateProfiler_UpSlopeFound()
#define TapStateProfiler_QualifyTap()
#define TapStateProfiler_QualifyDoubleTap()
#define TapStateProfiler_QualifyMoreThanDoubleTap()

#endif


#endif /* defined(__tapdetect__TapProfiler__) */

