//
//  TapDetection.c
//  tapdetect
//
//  Created by james ochs on 5/4/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#include "TapDetection.h"
#include "TapSampleBuffer.h"
#include "TapQualifier.h"
#include "TapConfig.h"
#include "TapStateProfiler.h"

#ifdef _MEX
#include "mex.h"
#endif

#define LOOK_BACK_NUM_SAMPLES (TAP_BUF_LEN-1)



typedef enum
{
  INITIALIZATION,
  IDLE,
  PROCESSING_OLD_SAMPLES,
  PROCESSING_CURRENT_SAMPLES
}tap_detect_state_t;


static accel_value_t prevZ = 0;

static struct
{
  tap_detect_state_t state;
  uint32_t sampleCounter;
  uint32_t processedSampleCounter;
}TapDetect;


void TapDetection_Init(void)
{
  TapDetect.sampleCounter = 0;
  TapDetect.state = IDLE;
  TapSampleBuffer_Init();
  TapQualifier_Init(0);
}

static inline int32_t ABS(int32_t value)
{
  uint32_t temp = value >> 31;     // make a mask of the sign bit
  value ^= temp;                   // toggle the bits if value is negative
  value += temp & 1;               // add one if value was negative
  return value;
}

#ifdef _MEX
static inline uint32_t processOldSamples(tap_t *potentialTapReturn, tap_group_t *tapGroupReturn)
#else
static inline uint32_t processOldSamples(void)
#endif
{
  uint32_t tapsDetected = 0;
  while(TapDetect.processedSampleCounter <= TapDetect.sampleCounter && tapsDetected==0)
  {
#ifdef _MEX
    tapsDetected=TapQualifier_Exec(potentialTapReturn,tapGroupReturn);
#else
    tapsDetected=TapQualifier_Exec();
#endif
    TapDetect.processedSampleCounter++;
  }
  
  if(TapDetect.processedSampleCounter > TapDetect.sampleCounter)
  {
    TapDetect.state = PROCESSING_CURRENT_SAMPLES;
  }
  
  return tapsDetected;
}

#ifdef _MEX
  #ifdef _TD_USE_SHINE_INTERFACE
uint32_t TapDetection_Exec(const tap_detect_sample_t *currentSample, tap_t *potentialTapReturn, tap_group_t *tapGroupReturn)
  #else
uint32_t TapDetection_Exec(accel_value_t currentSample[N_ACCEL_AXES], tap_t *potentialTapReturn, tap_group_t *tapGroupReturn)
  #endif
{
  potentialTapReturn->startPoint.x = 0;
  tapGroupReturn->minDist = 0;
#else
  #ifdef _TD_USE_SHINE_INTERFACE
uint32_t TapDetection_Exec(const tap_detect_sample_t *currentSample)
  #else
uint32_t TapDetection_Exec(accel_value_t currentSample[N_ACCEL_AXES])
  #endif
{
#endif
  
  TapSampleBuffer_WriteOneSample(currentSample);

  
  uint32_t tapsDetected  = 0;

  
  //this is more efficient than switch since we can force
  //the compiler to always look for the IDLE BRANCH, which we will be
  //in most of the time
  /** IDLE - looking for is motion ***/
  if(TapDetect.state == IDLE)
  {
     // const tap_detect_sample_t *prevSample = TapSampleBuffer_Read(TapDetect.sampleCounter-1);
#ifdef _TD_USE_SHINE_INTERFACE
    accel_value_t tmp = currentSample->z - prevZ;
#else
    accel_value_t tmp = currentSample[Z_AXIS] - prevZ;
#endif
      if(tmp > IS_MOTION_THRESH || tmp < -IS_MOTION_THRESH)
      {
        //passed a very simple (and sensitive) check for is motion, now do a little more
        //work before running the full algorithm
        const tap_detect_sample_t *prevSample = TapSampleBuffer_Read(TapDetect.sampleCounter-1);
        int32_t d1[N_ACCEL_AXES];
#ifdef _TD_USE_SHINE_INTERFACE
        d1[0] = prevSample->x - currentSample->x;
        d1[1] = prevSample->y - currentSample->y;
#else
        d1[0] = prevSample->x - currentSample[0];
        d1[1] = prevSample->y - currentSample[1];
#endif
        d1[2] = tmp;
        int32_t magD1 = ABS(d1[0]) + ABS(d1[1]) + ABS(d1[2]);
        
        if(magD1 > IS_MOTION_2_THRESH)
        {
        TapStateProfiler_IsMotion();
        TapQualifier_Init(TapDetect.sampleCounter - LOOK_BACK_NUM_SAMPLES);
        TapDetect.processedSampleCounter = TapDetect.sampleCounter - LOOK_BACK_NUM_SAMPLES;
        TapDetect.state = PROCESSING_OLD_SAMPLES;
#ifdef _MEX
        processOldSamples(potentialTapReturn, tapGroupReturn);
#else
        processOldSamples();
#endif
        }
        
      }
  }
  else if(TapDetect.state == PROCESSING_CURRENT_SAMPLES)
  {
#ifdef _MEX
    tapsDetected=TapQualifier_Exec(potentialTapReturn,tapGroupReturn);
#else
    tapsDetected=TapQualifier_Exec();
#endif
    if(tapsDetected || !TapQualifier_IsProcessingTaps())
    {
      TapDetect.state = IDLE;
    }
  }
  else if(TapDetect.state == PROCESSING_OLD_SAMPLES)
  {
#ifdef _MEX
    processOldSamples(potentialTapReturn, tapGroupReturn);
#else
    processOldSamples();
#endif
  }
  else if(TapDetect.state == INITIALIZATION)
  {
    if(TapDetect.sampleCounter == LOOK_BACK_NUM_SAMPLES)
    {
      TapDetect.state = IDLE;
    }
    
  }
  
  TapDetect.sampleCounter++;
#ifdef _TD_USE_SHINE_INTERFACE
  prevZ = currentSample->z;
#else
  prevZ = currentSample[Z_AXIS];
#endif
  return tapsDetected;
}
