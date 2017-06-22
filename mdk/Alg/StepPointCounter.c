//
//  AssignStepPoints.c
//  stepcount
//
//  Created by james ochs on 5/26/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#include "StepPointCounter.h"
#include "StepCount.h"
#include <string.h>


typedef struct
{
  uint32_t stepsThisMinute;
  uint32_t pointsThisMinute;
  uint32_t varianceThisMinute;
  uint32_t numWinThisMinute;
#if USE_NEW_BTLE_FORMAT_4
  uint32_t maxVarianceThisMinute;
#endif
} StepPointCounter_t;

static StepPointCounter_t SPC;

void StepPointCounter_Init(void)
{
  memset(&SPC,0,sizeof(StepPointCounter_t));
}

void StepPointCounter_NewWindow(step_count_result_t *windowData)
{
  
  uint32_t pts;
  (void)pts;
  real_t maxVariance;
  
  SPC.numWinThisMinute++;
  SPC.stepsThisMinute+=windowData->numSteps;
  
  
  maxVariance = windowData->variance[0];
  for(int32_t i=1; i<N_ACCEL_AXES; i++)
  {
    if(windowData->variance[i] > maxVariance)
    {
      maxVariance = windowData->variance[i];
    }
  }
  
  SPC.varianceThisMinute += maxVariance;
  
#if USE_NEW_BTLE_FORMAT_4
  if (maxVariance > SPC.maxVarianceThisMinute)
    SPC.maxVarianceThisMinute = maxVariance;
#else
  if(windowData->numSteps == 0)
  {
  
  //TODO: magic numbers, low priority for production release
  if (maxVariance > 2500000)
  {
    pts = 101;
  }
  else if (maxVariance > 50000)
  {
    pts = 27 + (maxVariance / 34000); 
  }
  else if (maxVariance > 15000)
  {
    pts = 23 + (maxVariance / 10000); 
  }
  else if (maxVariance > 10000)
  {
    pts = 19 + (maxVariance / 2400); 
  }
  else // maxvariance <= 1000
  {
    pts = 0;
  }
  
  SPC.pointsThisMinute += (pts >> 3);
  }
#endif
}

void StepPointCounter_NewMinute(minute_summary_t *result)
{
  
  //TODO: magic numbers, low priority for production release
  uint32_t pointsPerStep = 2500;
  
  if(SPC.stepsThisMinute > 250)
  {
    SPC.stepsThisMinute = 250;
  }
  
  
  //assign points per minute as a function of steps per minute
  if (SPC.stepsThisMinute <= 105)
  {
    pointsPerStep = 2500;
  }
  else if (SPC.stepsThisMinute <= 125)
  {
    pointsPerStep = 2500 + 25 * (SPC.stepsThisMinute - 105);
  }
  else if (SPC.stepsThisMinute <= 130)
  {
    pointsPerStep = 3000 + 400 * (SPC.stepsThisMinute - 125);
  }
  else
  {
    pointsPerStep = 5000 + 40 * (SPC.stepsThisMinute - 130);
  }
  
  SPC.pointsThisMinute += (uint32_t)((pointsPerStep * 256 * SPC.stepsThisMinute) / 10000);

#if USE_NEW_BTLE_FORMAT_4
  uint32_t pts = 0;
  uint32_t avgVar = SPC.varianceThisMinute / SPC.numWinThisMinute;
  //TODO: magic numbers, low priority for production release
  if (avgVar > 2500000)
  {
    pts = 101;
  }
  else if (avgVar > 50000)
  {
    pts = 27 + (avgVar / 34000);
  }
  else if (avgVar > 15000)
  {
    pts = 23 + (avgVar / 10000);
  }
  else if (avgVar > 10000)
  {
    pts = 19 + (avgVar / 2400);
  }
  else // maxvariance <= 1000
  {
    pts = 0;
  }
  
  SPC.pointsThisMinute += (pts >> 3);
#endif
  
  result->accumVariance = SPC.varianceThisMinute;
  result->steps = SPC.stepsThisMinute;
  //remove the remainder from the points that we report to the outside world
  result->activityPoints = (SPC.pointsThisMinute & 0xFFFFFFFF00);
  result->numWindows = SPC.numWinThisMinute;

#if USE_NEW_BTLE_FORMAT_4
  result->maxVariance = SPC.maxVarianceThisMinute;
#endif
  
  //reset for the next windows
  SPC.stepsThisMinute = 0;
  SPC.numWinThisMinute = 0;
  SPC.varianceThisMinute = 0;
#if USE_NEW_BTLE_FORMAT_4
  SPC.maxVarianceThisMinute = 0;
#endif
  
 //carry the remainder internally
  SPC.pointsThisMinute = SPC.pointsThisMinute & 0xFF;
  
}
