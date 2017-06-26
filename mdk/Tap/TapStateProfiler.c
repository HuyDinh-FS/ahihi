//
//  TapProfiler.c
//  tapdetect
//
//  Created by james ochs on 5/7/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#include "TapStateProfiler.h"
#include <string.h>

#ifdef _ENABLE_TAP_STATE_PROFILING

static tap_state_profile_t TSP;

void TapStateProfiler_Reset(void)
{
  memset(&TSP,0,sizeof(TSP));
}

void TapStateProfiler_IsMotion(void)
{
  TSP.isMotion++;
}

void TapStateProfiler_UpSlopeFound(void)
{
  TSP.foundUpSlope++;
}


void TapStateProfiler_QualifyTap(void)
{
  TSP.qualifyTap++;
}

void TapStateProfiler_QualifyDoubleTap(void)
{
  TSP.qualifyDoubleTap++;
}

void TapStateProfiler_QualifyMoreThanDoubleTap(void)
{
  TSP.qualifyMoreThanDoubleTap++;
}

const tap_state_profile_t *TapStateProfiler_GetStats(void)
{
  return &TSP;
}









#endif
