//
//  TapQualifier.c
//  tapdetect
//
//  Created by james ochs on 4/18/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//
#include "TapQualifier.h"
#include "TapSampleBuffer.h"
#include "TapConfig.h"
#include "TapStateProfiler.h"
#include <string.h>

#ifdef _MEX
#include "mex.h"
#endif

//2nd derivative needs 3 samples
#define NUM_SAMPLES_FOR_INIT 3
#define LPF_SHIFT 2 //divide by 4, which is equivalent to a filter weight of .25

#define DOWN_SLOPE_THRESH 200
#define CONF_THRESH  -300000
#define GROUP_THRESH 0
#define GROUP_THRESH_2 -500000 //-1000000
#define DOUBLE_THRESH -2000000
#define MAX_DISTANCE 95
#define MAG_BEFORE_THRESH 600
#define MAX_TIME_TO_FIND_UPSLOPE 40


static inline int32_t ABS(int32_t value)
{
  uint32_t temp = value >> 31;     // make a mask of the sign bit
  value ^= temp;                   // toggle the bits if value is negative
  value += temp & 1;               // add one if value was negative
  return value;
}



#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

typedef enum
{
  INITIAL_STATE,
  FIND_UP_SLOPE,
  FIND_DOWN_SLOPE,
}tap_qual_state_t;

static struct
{
  uint8_t initCounter;

  tap_detect_sample_t lastSampleBeforeTap;
  uint32_t secondToLastSampleIndexBeforeTap;
  //previous value of first derivative
  
  //filtered absolute sum of the 2nd derivate of 3 acess accel signals
  uint32_t magD2Filtered;
  uint32_t magD2;
  
  point_t startPoint;
  point_t currentPoint;
  point_t previousPoint;
  point_t maxPoint;
  
  tap_qual_state_t state;
  
  //TBD probably better to feed sample
  //counter into TapQualifier to avoid having two counts
  uint32_t sampleCounter;
  uint32_t samplesSinceLastQualifiedTap;
  uint32_t samplesSinceInit;
  
  tap_t previousTap;
  
  tap_group_t tapGroup;
  
  int32_t magnitudeBefore;
  
  
}TapQual;


static void calcMag2ndDerivative(void);
static bool findPotentialTap(tap_t *potentialTap);
static bool qualifyPotentialTap(tap_t *potentialTap);
static void updateTapGroup(tap_t *tapToAdd);
static uint32_t qualifyTapGroup(void);
void calcMagnitudeBeforeTap(void);


static inline void filterMagD2(void)
{
  int32_t tmp = TapQual.magD2 - TapQual.magD2Filtered;
  TapQual.magD2Filtered += (tmp >> LPF_SHIFT);
  
  
}


//#include <stdio.h>

//static FILE *fp = NULL;

void TapQualifier_Init(uint32_t currentSampleCount)
{
  memset(&TapQual,0,sizeof(TapQual));
  TapQual.state = INITIAL_STATE;
  TapQual.tapGroup.minDist = UINT32_MAX;
  TapQual.tapGroup.minPwr = UINT32_MAX;
  TapQual.sampleCounter = currentSampleCount;
  
}

bool TapQualifier_IsProcessingTaps(void)
{
  
  if(TapQual.tapGroup.numTaps != 0)
  {
    //have detected at least one tap, so the module will
    //be returing the result (1,2,3, or more taps shortly
    return true; //is processing
  }
  else if(TapQual.state == FIND_UP_SLOPE)
  {
    
    //searching for the first threshold crossing to allow us to
    //qualify the tap, this should happen not to long after the
    //the module is initialized (isMotion passes in higher level TapDetection)
    return TapQual.samplesSinceInit < MAX_TIME_TO_FIND_UPSLOPE;
  }
  else if(TapQual.samplesSinceInit < MAX_DISTANCE)
  {
    //we have found a potential tap (crossed UP_SLOPE threshold)
    //now wait the full time time qualify it
    return true;
  }
  else
  {
    return false;
  }
  
}


#ifdef _MEX
uint32_t TapQualifier_Exec(tap_t *potentialTapReturn, tap_group_t *tapGroupReturn)
{
  potentialTapReturn->startPoint.x = 0;
  tapGroupReturn->minDist = 0;
#else
  uint32_t TapQualifier_Exec(void)
  {
#endif
    
    
    uint32_t numTapsDetected = 0;
    
    
    
    calcMag2ndDerivative();
    
    tap_t potentialTap;
    
    if(findPotentialTap(&potentialTap)) //detect a potential tap
    {
      
      if(qualifyPotentialTap(&potentialTap)) //qualify it
      {
        
        //tap was qualified
        updateTapGroup(&potentialTap);
        
#ifdef _MEX
        *potentialTapReturn = potentialTap;
#endif
      }
    }
    
    
    if(TapQual.tapGroup.numTaps)
    {
      TapQual.samplesSinceLastQualifiedTap++;
      //if we have not qualifed a tap in ~.5sec (at 200Hz) and signal is below the lower threshold (meaning we are probably not in the middle of a tap
      if(TapQual.samplesSinceLastQualifiedTap > MAX_DISTANCE && TapQual.currentPoint.y < DOWN_SLOPE_THRESH)
      {
        numTapsDetected = qualifyTapGroup();
#ifdef _MEX
        *tapGroupReturn = TapQual.tapGroup;
#endif
        
        TapQual.tapGroup.numTaps = 0;
      }
    }
    
    TapQual.sampleCounter++;
    TapQual.samplesSinceInit++;
    
    return numTapsDetected;
    
  }
  
  
  
  
  void calcMag2ndDerivative(void)
  {
    
    int32_t tmpD2;
    TapQual.magD2 = 0;
    const tap_detect_sample_t *currentSample = TapSampleBuffer_Read(TapQual.sampleCounter);
    const tap_detect_sample_t *n1 = TapSampleBuffer_Read(TapQual.sampleCounter-1);
    const tap_detect_sample_t *n2 = TapSampleBuffer_Read(TapQual.sampleCounter-2);
    
    
    tmpD2 = currentSample->z  - (n1->z << 1) + n2->z;
    TapQual.magD2 += ABS(tmpD2);
    
    //2nd derivative, x
    //y[n] = ((x[n] - x[n-1]) - (x[n-1] - x[n-2])) = x[n] - 2*x[n-1] + x[n-2]
    tmpD2 = currentSample->x  - (n1->x << 1) + n2->x;
    //sum of absolute value
    TapQual.magD2 += ABS(tmpD2);
    
    //same for y
    tmpD2 = currentSample->y  - (n1->y << 1) + n2->y;
    TapQual.magD2 += ABS(tmpD2);
    
    
  }
  
  void calcMagnitudeBeforeTap(void)
  {
    
    
    
    uint32_t idxStart = TapQual.sampleCounter-12+2;
    uint32_t idxEnd = TapQual.secondToLastSampleIndexBeforeTap-2;
    
    
    const tap_detect_sample_t *n2;
    const tap_detect_sample_t *n3;
    const tap_detect_sample_t *n4;
    tap_detect_sample_t globalMin,lastLocalMin;
    
   
    lastLocalMin.x = ACCEL_VALUE_MAX;
    lastLocalMin.y = ACCEL_VALUE_MAX;
    lastLocalMin.z = ACCEL_VALUE_MAX;
    
    n2 = TapSampleBuffer_Read(idxStart);
    n3 = TapSampleBuffer_Read(idxStart-1);
    n4 = TapSampleBuffer_Read(idxStart-2);
    
    globalMin.x = MIN(n4->x,n3->x);
    globalMin.y = MIN(n4->y,n3->y);
    globalMin.z = MIN(n4->z,n3->z);
 
    
    do
    {
      globalMin.x = MIN(globalMin.x,n2->x);
      globalMin.y = MIN(globalMin.y,n2->y);
      globalMin.z = MIN(globalMin.z,n2->z);
      
      
      if(n3->x < n4->x && n3->x < n2->x)
      {
        lastLocalMin.x = n3->x;
      }
      
      if(n3->y < n4->y && n3->y < n2->y)
      {
        lastLocalMin.y = n3->y;
      }
      
      if(n3->z < n4->z && n3->z < n2->z)
      {
        lastLocalMin.z = n3->z;
      }
      
      n4=n3;
      n3=n2;
      idxStart++;
      n2 = TapSampleBuffer_Read(idxStart);
      
      
    }while(idxStart<=idxEnd);
    
    
    if(lastLocalMin.x == ACCEL_VALUE_MAX)
    {
      lastLocalMin.x = globalMin.x;
    }
    if(lastLocalMin.y == ACCEL_VALUE_MAX)
    {
      lastLocalMin.y = globalMin.y;
    }
    if(lastLocalMin.z == ACCEL_VALUE_MAX)
    {
      lastLocalMin.z = globalMin.z;
    }
    
    TapQual.magnitudeBefore =
    ABS(TapQual.lastSampleBeforeTap.x - lastLocalMin.x) +
    ABS(TapQual.lastSampleBeforeTap.y  - lastLocalMin.y) +
    ABS(TapQual.lastSampleBeforeTap.z -lastLocalMin.z);
    
    
    
  }
  
  
  bool findPotentialTap(tap_t *potentialTap)
  {
    
    bool foundTap = false;
    point_t prevPrevPoint;
    
    switch(TapQual.state)
    {
        
        
      case FIND_UP_SLOPE:
        
        filterMagD2();
        
        prevPrevPoint = TapQual.previousPoint;
        TapQual.previousPoint = TapQual.currentPoint;
        TapQual.currentPoint.x = TapQual.sampleCounter;
        TapQual.currentPoint.y = TapQual.magD2Filtered;
        
        
        //local for local minimum
        if(prevPrevPoint.y < DOWN_SLOPE_THRESH)
        {
          
          TapQual.secondToLastSampleIndexBeforeTap = TapQual.sampleCounter;
          
        }
        
        
        if(TapQual.previousPoint.y < DOWN_SLOPE_THRESH)
        {
          TapQual.startPoint = TapQual.previousPoint;
          TapQual.maxPoint = TapQual.previousPoint;
          TapQual.lastSampleBeforeTap = *TapSampleBuffer_Read(TapQual.sampleCounter-3);
        }
        
        if(TapQual.previousPoint.y > TapQual.maxPoint.y)
        {
          TapQual.maxPoint = TapQual.previousPoint;
        }
        
        if(TapQual.currentPoint.y > UP_SLOPE_THRESH && TapQual.previousPoint.y <= UP_SLOPE_THRESH && TapQual.secondToLastSampleIndexBeforeTap != 0)
        {
          TapQual.state = FIND_DOWN_SLOPE;
          
          TapStateProfiler_UpSlopeFound();
          calcMagnitudeBeforeTap();
          
          #ifdef CONFIG_DEMO_UX
            #if FORCE_TOUCH_ON_UP_SLOPE
              //If this is the start of the first tap
              if (TapQual.tapGroup.numTaps == 0)
              {
                  //For UX demo, force tap will start clock/progress animation if it has not been started
                  uint64_t time_since_last_touch = sysTime_get_time_full() - captouch_time_of_last_touch_get();
                  if(( time_since_last_touch < FORCE_TOUCH_CAP_WINDOW_TIME_TICKS) && !Animation_Is_Running())
                  {
                    Events_Handler(EVT_SINGLE_TOUCH);
                  }
              }
            #endif
          #endif
          
        }
        break;
      case FIND_DOWN_SLOPE:
        
        filterMagD2();
        
        TapQual.previousPoint = TapQual.currentPoint;
        TapQual.currentPoint.x = TapQual.sampleCounter;
        TapQual.currentPoint.y = TapQual.magD2Filtered;
        
        
        if(TapQual.previousPoint.y > TapQual.maxPoint.y)
        {
          TapQual.maxPoint = TapQual.previousPoint;
        }
        
        if(TapQual.currentPoint.y < DOWN_SLOPE_THRESH && TapQual.previousPoint.y >= DOWN_SLOPE_THRESH)
        {
          TapQual.state = FIND_UP_SLOPE;
          
          potentialTap->endPoint = TapQual.currentPoint;
          potentialTap->startPoint = TapQual.startPoint;
          potentialTap->maxPoint = TapQual.maxPoint;
          potentialTap->magnitudeBefore = TapQual.magnitudeBefore;
          
          if(TapQual.previousTap.startPoint.x > 0)
          {
            potentialTap->distMax = potentialTap->maxPoint.x - TapQual.previousTap.maxPoint.x + 1;
            potentialTap->distEndToStart = (potentialTap->startPoint.x - TapQual.previousTap.endPoint.x + 1);
          }
          else
          {
            potentialTap->distMax = 0;
            potentialTap->distEndToStart = 0;
          }
          
          #if FORCE_TOUCH_ON_DOWN_SLOPE
            //If this is the start of the first tap
            if (TapQual.tapGroup.numTaps == 0)
            {
              //force tap will start clock/progress animation if it has not been started
              uint64_t time_since_last_touch = sysTime_get_time_full() - captouch_time_of_last_touch_get();
              if(( time_since_last_touch < FORCE_TOUCH_CAP_WINDOW_TIME_TICKS) && !Animation_Is_Running())
              {
                Events_Handler(EVT_SINGLE_TOUCH);
              }
            }
          #endif
          foundTap = true;
          
        }
        break;
        
      case INITIAL_STATE:
        TapQual.initCounter++;
        if(TapQual.initCounter == NUM_SAMPLES_FOR_INIT-1)
        {
          
          //initialization when we get the first value
          //TODO: is there a way to NOT check this every sample!!!
          //if(TapQual.sampleCounter == NUM_SAMPLES_FOR_INIT)
          {
            TapQual.startPoint.x = TapQual.magD2Filtered;
            TapQual.startPoint.y = TapQual.sampleCounter;
            TapQual.maxPoint = TapQual.startPoint;
            TapQual.currentPoint = TapQual.startPoint;
            TapQual.previousPoint = TapQual.startPoint;
          }
          
          TapQual.state = FIND_UP_SLOPE;
          
        }
        
        break;
    }
    return foundTap;
  }
  
  bool qualifyPotentialTap(tap_t *potentialTap)
  {
    
    TapStateProfiler_QualifyTap();
    
#define NUM_TAP_QUAL_PARAMS 7
    static const int32_t W[NUM_TAP_QUAL_PARAMS] = {161,19,-4269,3730,19,13,-47};
    static const int32_t b = 20950;
    int32_t P[NUM_TAP_QUAL_PARAMS];
    int32_t upWidth,downWidth,totalWidth,pwrUp,pwrDown,conf,i;
    bool tapQualified = false;
    
    
    if(potentialTap->magnitudeBefore >= MAG_BEFORE_THRESH)
    {
      return false;
    }
    
    
#ifdef _TD_ACCEL_ADXL
    potentialTap->maxPoint.y += (potentialTap->maxPoint.y >> 1) + (potentialTap->maxPoint.y >> 3);
#endif
    
    upWidth = potentialTap->maxPoint.x - potentialTap->startPoint.x + 1;
    
    
    downWidth = potentialTap->endPoint.x - potentialTap->maxPoint.x + 1;
    totalWidth = upWidth + downWidth;
    
    pwrUp = potentialTap->maxPoint.y - potentialTap->startPoint.y;
    pwrDown = potentialTap->maxPoint.y - potentialTap->endPoint.y;
    
    
    P[0] = potentialTap->startPoint.y << 4;
    P[1] = potentialTap->maxPoint.y >> 1;  //max power
    P[2] = upWidth;
    P[3] = totalWidth;
    P[4] = pwrUp >> 1;
    P[5] = pwrDown >> 1;
    P[6] = ((pwrUp + (upWidth >> 1))/upWidth) << 4;
    
    conf=b;
    for(i=0; i<NUM_TAP_QUAL_PARAMS; i++)
    {
      conf += W[i]*P[i];
    }
    potentialTap->conf = -conf;
    
    
    // tapQualified = true;
    //  TapQual.previousTap = *potentialTap;
    //  TapQual.samplesSinceLastQualifiedTap = 0;
    //  return tapQualified;
   
    
    
    //if we have detected at least one previous tap
    if(TapQual.previousTap.startPoint.x ==  0)
    {
      if(potentialTap->conf >= CONF_THRESH)
      {
        tapQualified = true;
        TapQual.previousTap = *potentialTap;
        TapQual.samplesSinceLastQualifiedTap = 0;
        
        
      }
    }
    else
    {
      
      if(potentialTap->conf >= CONF_THRESH && potentialTap->distEndToStart >= 3 && 5*potentialTap->maxPoint.y > TapQual.previousTap.maxPoint.y)
      {
        tapQualified = true;
        TapQual.previousTap = *potentialTap;
        TapQual.samplesSinceLastQualifiedTap = 0;
        
        
      }
    }
    
    return tapQualified;
    
  }
  
  void updateTapGroup(tap_t *tapToAdd)
  {
    
    
    TapQual.tapGroup.endIndex = tapToAdd->endPoint.x;
    
    TapQual.tapGroup.numTaps++;
    
    
    
    if(TapQual.tapGroup.numTaps == 1)
    {
      TapQual.tapGroup.minPwr = tapToAdd->maxPoint.y;
      TapQual.tapGroup.maxPwr = tapToAdd->maxPoint.y;
      TapQual.tapGroup.minDist = tapToAdd->distMax;
      TapQual.tapGroup.maxDist = tapToAdd->distMax;
      TapQual.tapGroup.minConf = tapToAdd->conf;
      TapQual.tapGroup.maxConf = tapToAdd->conf;
      TapQual.tapGroup.minMagBefore = tapToAdd->magnitudeBefore;
      TapQual.tapGroup.maxMagBefore = tapToAdd->magnitudeBefore;
      
    }
    else if(TapQual.tapGroup.numTaps == 2)
    {
      TapQual.tapGroup.minDist = tapToAdd->distMax;
      TapQual.tapGroup.maxDist = tapToAdd->distMax;
      
      if(tapToAdd->maxPoint.y < TapQual.tapGroup.minPwr)
      {
        TapQual.tapGroup.minPwr = tapToAdd->maxPoint.y;
      }
      
      if(tapToAdd->maxPoint.y > TapQual.tapGroup.maxPwr)
      {
        TapQual.tapGroup.maxPwr = tapToAdd->maxPoint.y;
      }
      
      TapQual.tapGroup.minConf = MIN(TapQual.tapGroup.minConf,tapToAdd->conf);
      TapQual.tapGroup.maxConf = MAX(TapQual.tapGroup.maxConf,tapToAdd->conf);
      TapQual.tapGroup.minMagBefore = MIN(TapQual.tapGroup.minMagBefore,tapToAdd->magnitudeBefore);
      TapQual.tapGroup.maxMagBefore = MAX(TapQual.tapGroup.maxMagBefore,tapToAdd->magnitudeBefore);
      
      
      
    }
    else
    {
      if(tapToAdd->maxPoint.y < TapQual.tapGroup.minPwr)
      {
        TapQual.tapGroup.minPwr = tapToAdd->maxPoint.y;
      }
      
      if(tapToAdd->maxPoint.y > TapQual.tapGroup.maxPwr)
      {
        TapQual.tapGroup.maxPwr = tapToAdd->maxPoint.y;
      }
      
      if(tapToAdd->distMax < TapQual.tapGroup.minDist)
      {
        TapQual.tapGroup.minDist = tapToAdd->distMax;
      }
      
      if(tapToAdd->distMax > TapQual.tapGroup.maxDist)
      {
        TapQual.tapGroup.maxDist = tapToAdd->distMax;
      }
      
      TapQual.tapGroup.minConf = MIN(TapQual.tapGroup.minConf,tapToAdd->conf);
      TapQual.tapGroup.maxConf = MAX(TapQual.tapGroup.maxConf,tapToAdd->conf);
      TapQual.tapGroup.minMagBefore = MIN(TapQual.tapGroup.minMagBefore,tapToAdd->magnitudeBefore);
      TapQual.tapGroup.maxMagBefore = MAX(TapQual.tapGroup.maxMagBefore,tapToAdd->magnitudeBefore);
      
    }
  }
  
  uint32_t qualifyTapGroup(void)
  {
#define NUM_GROUP_QUAL_PARAMS1 9
    static const int32_t W1[NUM_GROUP_QUAL_PARAMS1] = {-48,19,42,62,46,103,-14,-67,-22};
    static const int32_t b1 = 2527801;
    int32_t P1[NUM_GROUP_QUAL_PARAMS1];
    
    
#define NUM_GROUP_QUAL_PARAMS2 5
    static const int32_t W2[NUM_GROUP_QUAL_PARAMS2] = {36,66,34,15,-56};
    static const int32_t b2 = 1470399;
    int32_t P2[NUM_GROUP_QUAL_PARAMS2];
    
    
#define NUM_GROUP_QUAL_PARAMS3 3
    static const int32_t W3[NUM_GROUP_QUAL_PARAMS3] = {1249,17,-722};
    static const int32_t b3 = 515;
    int32_t P3[NUM_GROUP_QUAL_PARAMS3];
    int32_t i;
    
    
   // return TapQual.tapGroup.numTaps > 4 ? 4 : TapQual.tapGroup.numTaps;
    
     TapQual.tapGroup.conf = 0;
    
    if(TapQual.tapGroup.numTaps == 1)
    {
      return 1;  //no qualification for single or double taps
    }
    
    else if(TapQual.tapGroup.numTaps == 2)
    {
      TapStateProfiler_QualifyDoubleTap();
      
      P1[0] = TapQual.tapGroup.minPwr << 4;
      P1[1] = TapQual.tapGroup.maxPwr << 4;
      P1[2] = (TapQual.tapGroup.maxPwr - TapQual.tapGroup.minPwr) << 6;
      P1[3] = TapQual.tapGroup.minDist << 10;
      P1[4] = TapQual.tapGroup.minMagBefore << 10;
      P1[5] = (TapQual.tapGroup.maxMagBefore + TapQual.tapGroup.minMagBefore) << 7;
      P1[6] = TapQual.tapGroup.minConf >> 1;
      P1[7] = TapQual.tapGroup.maxConf >> 1;
      P1[8] = (TapQual.tapGroup.minConf + TapQual.tapGroup.maxConf) >> 1;
      
      //TapQual.tapGroup.conf = b1;
      TapQual.tapGroup.conf  = b1;
      for(i=0; i<NUM_GROUP_QUAL_PARAMS1; i++)
      {
        TapQual.tapGroup.conf  += W1[i]*P1[i];
      }
      TapQual.tapGroup.conf  = -TapQual.tapGroup.conf ;
      
      
      //return 2;
      return TapQual.tapGroup.conf  < DOUBLE_THRESH ? 0 : TapQual.tapGroup.numTaps;
      
    }
    
    else
    {
      TapStateProfiler_QualifyMoreThanDoubleTap();
      //return 0;
      //return TapQual.tapGroup.numTaps > 4 ? 4 : TapQual.tapGroup.numTaps;
      
      
      P3[0] = TapQual.tapGroup.maxDist - TapQual.tapGroup.minDist;
      P3[1] = TapQual.tapGroup.maxPwr - TapQual.tapGroup.minPwr;
      P3[1] = P3[1] >> 3;  //divide by 8, required to scale param to others
      P3[2] = TapQual.tapGroup.minDist;
      
      TapQual.tapGroup.conf = b3;
      for(i=0; i<NUM_GROUP_QUAL_PARAMS3; i++)
      {
        TapQual.tapGroup.conf += W3[i]*P3[i];
      }
      TapQual.tapGroup.conf = -TapQual.tapGroup.conf;
      
      
      if(TapQual.tapGroup.conf < GROUP_THRESH)
      {
        
        return 2;
      }
      else
      {
         // return TapQual.tapGroup.numTaps > 4 ? 4 : TapQual.tapGroup.numTaps;
        
        P2[0] = (TapQual.tapGroup.maxPwr - TapQual.tapGroup.minPwr) << 4;
        P2[1] = (TapQual.tapGroup.maxDist - TapQual.tapGroup.minDist) << 10;
        P2[2] = TapQual.tapGroup.minMagBefore << 9;
        P2[3] = TapQual.tapGroup.minConf >> 3;
        P2[4] = TapQual.tapGroup.maxConf >> 2;
        
        
        TapQual.tapGroup.conf = b2;
        for(i=0; i<NUM_GROUP_QUAL_PARAMS2; i++)
        {
          TapQual.tapGroup.conf += W2[i]*P2[i];
        }
        TapQual.tapGroup.conf = -TapQual.tapGroup.conf;
        
      
        if(TapQual.tapGroup.conf < GROUP_THRESH_2)
        {
          return 0;
        }
        else
        {
          return TapQual.tapGroup.numTaps > 4 ? 4 : TapQual.tapGroup.numTaps;
          
        }
        
        
      }
    }
    
    
  }
  
  
  
  
