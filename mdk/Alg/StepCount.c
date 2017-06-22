//
//  StepCount.c
//  stepcount
//
//  Created by james ochs on 5/9/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "scalgtypes.h"
#include "stats.h"
#include "StepCount.h"
#include "StepPointCounter.h"
//#include "config.h"


#define IS_MOTION_WINDOW    17
#define MOTION_VAR_THRES        1320
#define REP_RANGE_THRES         100
#define CORRELATION_SUM_THRES   0.2f
#define CORR_SUM_MINMAX_GAP_THRES   0.3f
#define PEAK_RANGE_THRES        450
#define MIN_REP_LENGTH      7
#define STD_OFFSET          6 // standard offset to move current pointer
#define REP_DETECT_WINDOW_PDA_DEFAULT 50 // to accomodate longer rep that auto_corr can't reach







#define REP_DETECT_WINDOW_PDA_FAST 21

#define SAMPLES_PER_MINUTE 750


typedef struct
{
  //the window size and correlation length that is used when
  //step count results are below FAST_WINDOW_STEPS_PER_MIN_THRESH
  uint8_t repDetectWindowDefault;
  uint8_t correlationLengthDefault;
  
  //window and correlation length when > FAST_WINDOW_STEPS_PER_MIN_THRESH
  uint8_t repDetectWindowFast;
  uint8_t correlationLengthFast;
  
  //when an autocorr cycle is detected, this defines how many cycles we skip
  //ahead before running the algorithm again
  uint8_t autocorrCycleSkip;
  
  uint8_t fastWindowStepsPerMinThresh;
  
  
}mode_dependent_variables_t;




const mode_dependent_variables_t NORMAL_MODE_VARIABLES =
{
  .repDetectWindowDefault = 75,
  .correlationLengthDefault = 50,
  .repDetectWindowFast = 42,
  .correlationLengthFast = 24,
  .autocorrCycleSkip = 1,//2,
  .fastWindowStepsPerMinThresh = 100,
};

const mode_dependent_variables_t SWIM_MODE_VARIABLES =
{
  .repDetectWindowDefault = 90,
  .correlationLengthDefault = 90,
  .repDetectWindowFast = 50,
  .correlationLengthFast = 33,
  .autocorrCycleSkip = 1,
  .fastWindowStepsPerMinThresh = 85,
};
//the maximum of the correlationLengthXXX variables above
#define MAX_CORRELATION_LENGTH 90
//the maximum of the repDetectionWindowXXX variables above
#define MAX_REP_DETECT_WINDOW 90




#if(SC_ALG_MAX_WINDOW_LENGTH != MAX_REP_DETECT_WINDOW)
#error "SC_ALG_MAX_WINDOW_LENGTH, which is defined in StepCount.h must equal to REP_DETECT_WINDOW_DEFAULT"
#endif


typedef struct
{
    uint32_t  RunVariances;
    uint32_t  RunAutoCorrelations;
    uint32_t  RunPeakDetections;

}   SC_Counts_t;

static SC_Counts_t SC_Counts;

void SC_RamLost( void )
{
  memset( &SC_Counts, 0, sizeof( SC_Counts ));
}

typedef struct
{
  uint32_t repDetectWindow;
  uint32_t correlationLength;
  uint32_t repDetectWindowPDA;
  
  uint32_t samplesThisMinute;
  uint32_t stepsThisMinute;
  
  const mode_dependent_variables_t *modeVariables;
  
  
}StepCountAlg;

static StepCountAlg SCA;

static widereal_t correlationMultiDim[MAX_CORRELATION_LENGTH];
static real_t realCorr[MAX_CORRELATION_LENGTH];
static real_t zeroMeanSignal[MAX_REP_DETECT_WINDOW];



static void clearCountStepResult(step_count_result_t *result);
static int findMaximumSimilarity(widereal_t *correlation);
static int detectPeak(real_t *windowData[N_ACCEL_AXES],uint32_t len);
uint32_t countSteps(real_t *data[N_ACCEL_AXES],uint32_t lenData,step_count_result_t *result);
static void changeWindowSize(void);
static uint32_t findAutocorrCycle(step_count_result_t *result, real_t** data);

void SC_Init(void)
{
  SCA.repDetectWindowPDA = REP_DETECT_WINDOW_PDA_DEFAULT;
  SCA.samplesThisMinute = 0;
  SCA.stepsThisMinute = 0;
  StepPointCounter_Init();
  SC_SetAlgMode(SC_NORMAL_MODE);
}


void SC_SetAlgMode(step_count_alg_mode_t mode)
{
  switch(mode)
  {
    case SC_NORMAL_MODE:
      SCA.modeVariables = &NORMAL_MODE_VARIABLES;
      break;
    case SC_SWIM_MODE:
      SCA.modeVariables = &SWIM_MODE_VARIABLES;
      break;
    default:
      SCA.modeVariables = &NORMAL_MODE_VARIABLES;
      break;
  }
  
  SCA.repDetectWindow = SCA.modeVariables->repDetectWindowDefault;
  SCA.correlationLength = SCA.modeVariables->correlationLengthDefault;
  
}

typedef enum
{
    SC_LogAlgorithms    = 0x91,

}   SC_Log_t;

void SC_NotifyNewDay( void )
{

    SC_Counts.RunVariances         /= 8;
    SC_Counts.RunAutoCorrelations  /= 2;
    SC_Counts.RunPeakDetections    /= 2;

    if( SC_Counts.RunVariances         >= 0x10000 ) SC_Counts.RunVariances         = 0xFFFF;
    if( SC_Counts.RunAutoCorrelations  >= 0x10000 ) SC_Counts.RunAutoCorrelations  = 0xFFFF;
    if( SC_Counts.RunPeakDetections    >= 0x10000 ) SC_Counts.RunPeakDetections    = 0xFFFF;

    #define NUMBYTES_LogAlgorithms  7
    u8 bytes[NUMBYTES_LogAlgorithms];
    u8 *p = bytes;
    //UINT8_TO_BSTREAM (p, SC_LogAlgorithms); 
    //UINT16_TO_BSTREAM(p, SC_Counts.RunVariances        ); 
    //UINT16_TO_BSTREAM(p, SC_Counts.RunAutoCorrelations ); 
    //UINT16_TO_BSTREAM(p, SC_Counts.RunPeakDetections   ); 
    //hwLog_WriteLogEntry( log_algorithm, bytes, NUMBYTES_LogAlgorithms);

    SC_Counts.RunVariances         = 0;
    SC_Counts.RunAutoCorrelations  = 0;
    SC_Counts.RunPeakDetections    = 0;
}

void SC_AggregateMinute(minute_summary_t *minuteData)
{
#ifdef _SC_ALG_PC_TEST_LEGACY_COMPARE
  SCA.stepsThisMinute = minuteData->steps;
  changeWindowSize();
#else
  StepPointCounter_NewMinute(minuteData);
#endif
}



int32_t SC_GetWindowSize(void)
{
  return SCA.repDetectWindow;
}




uint32_t SC_Execute(real_t *data[N_ACCEL_AXES],int32_t lenData,step_count_result_t *result)
{
  uint32_t dim;
  real_t *localPointers[N_ACCEL_AXES];
  uint32_t totalSamplesProcessed=0;
  uint32_t currentSamplesProcessed;
  
  clearCountStepResult(result);
  
  if(lenData >= (int32_t)SCA.repDetectWindow)
  {
    //move pointer ahead so that countSteps sees last unprocessed
    //sample at index 0.
    for(dim = 0; dim<N_ACCEL_AXES; dim++)
    {
      localPointers[dim] = data[dim]+totalSamplesProcessed;
    }
    
    //run step counting algorithm
    currentSamplesProcessed = countSteps(localPointers,lenData,result);
    totalSamplesProcessed+=currentSamplesProcessed;
    lenData -= totalSamplesProcessed;
    
    
    //assign points
#ifndef _SC_ALG_PC_TEST_LEGACY_COMPARE
    StepPointCounter_NewWindow(result);
#endif
    
    
    //keep track steps per minute and change window size
    SCA.samplesThisMinute+=currentSamplesProcessed;
    SCA.stepsThisMinute+=result->numSteps;
    if(SCA.samplesThisMinute >= SAMPLES_PER_MINUTE)
    {
      SCA.samplesThisMinute = SCA.samplesThisMinute-SAMPLES_PER_MINUTE;
#ifndef _SC_ALG_PC_TEST_LEGACY_COMPARE
      changeWindowSize();
#endif
      SCA.stepsThisMinute = 0;
    }
    
  }
  return totalSamplesProcessed;
}

#define USE_DOUBLE_ALGO 1 // HND: 12 May 2016 , on PLUTO based product, we use floating point here to improve accuracy.

uint32_t findAutocorrCycle(step_count_result_t *result, real_t** data)
{
  int32_t i;
  int32_t dim;
  uint32_t cycleLength;
  for (i = 0; i < SCA.correlationLength; i++)
  {
    correlationMultiDim[i] = 0;
  }
#if USE_DOUBLE_ALGO
  float first_dim_strength = 0;
#endif
  
  for (dim = 0; dim < N_ACCEL_AXES; ++dim)
  {
    //samples are 12-bit, autocorr is a sum of squares with len = 75.   Autocorr out = 75*24-bit = 31-bit
    removeDCCopy(data[dim],zeroMeanSignal,SCA.repDetectWindow);
    autocorr(zeroMeanSignal, realCorr, SCA.correlationLength,SCA.repDetectWindow);
    
    result->power[dim] = realCorr[0];
    
    if (dim == 0)
    {
      for (i = SCA.correlationLength - 1; i >= 0; --i)
      {
        correlationMultiDim[i] = realCorr[i];
      }
      
#if USE_DOUBLE_ALGO
      first_dim_strength = realCorr[0];
#endif
    }
    else
    {
#if USE_DOUBLE_ALGO
      
      float strength_ratio = 1.0f * first_dim_strength / realCorr[0];
      for (i = SCA.correlationLength - 1; i >= 0; --i)
      {
        correlationMultiDim[i] += (widereal_t)(strength_ratio * realCorr[i]);
      }
#else
      
      widereal_t strengthRatio = correlationMultiDim[0];
      
      if (dim == 2) strengthRatio >>= 1;
      for (i = SCA.correlationLength - 1; i >= 0; --i)
      {
        //for i=1: 31-bit * 31-bit = 62-bit
        //for i=2: 31-bit * 42-bit = 73-bit (overflow)
        correlationMultiDim[i] *= (widereal_t)realCorr[0];
        //for i=1: 62-bit + 62-bit = 63-bit number
        correlationMultiDim[i] += strengthRatio * (widereal_t)realCorr[i];
        //for i=1: down to 42-bit
        correlationMultiDim[i] >>= 20;  // for dim == 2, this will do a redundant shift but
        // that does not affect the accuracy while running
        // much faster than 'if (dim == 1) ... '
      }
#endif
    }
  }
  
  cycleLength = findMaximumSimilarity(correlationMultiDim);
  return cycleLength;
}

void changeWindowSize()
{
  if(SCA.stepsThisMinute >= SCA.modeVariables->fastWindowStepsPerMinThresh)
  {
    SCA.repDetectWindow = SCA.modeVariables->repDetectWindowFast;
    SCA.repDetectWindowPDA = REP_DETECT_WINDOW_PDA_FAST;
    SCA.correlationLength = SCA.modeVariables->correlationLengthFast;
    
  }
  else
  {
    SCA.repDetectWindow = SCA.modeVariables->repDetectWindowDefault;
    SCA.repDetectWindowPDA = REP_DETECT_WINDOW_PDA_DEFAULT;
    SCA.correlationLength = SCA.modeVariables->correlationLengthDefault;
  }
}

bool autocorrCycleRangeCheck(real_t** data, uint32_t cycleLength)
{
  int32_t i;
  int32_t dim;
  bool rangePassed=false;
  for (dim = 0; dim < N_ACCEL_AXES; ++dim)
  {
    // version 5e: double check for raw data range
    real_t maxVal = data[dim][0];
    real_t minVal = maxVal;
    
    for (i = 0; i < cycleLength; ++i)
    {
      maxVal = (maxVal > data[dim][i]) ? maxVal : data[dim][i];
      minVal = (minVal < data[dim][i]) ? minVal : data[dim][i];
    }
    
    if (maxVal - minVal >= REP_RANGE_THRES)
    {
      rangePassed = true;
      break;
    }
  }
  return rangePassed;
}

uint32_t countSteps(real_t *data[N_ACCEL_AXES],uint32_t lenData,step_count_result_t *result)
{
  
  int32_t dim;
  uint32_t cycleLength;
  bool isMotion = false;
  uint8_t cycleFound = 0;
  uint32_t numSamplesProcessed;
  
  
  result->repInfo = REP_NO_MOTION_DETECTED;
  
    SC_Counts.RunVariances++;
  
  //Check for motion
  for (dim = 0; dim < N_ACCEL_AXES; dim++)
  {
    result->variance[dim] = variance(data[dim], IS_MOTION_WINDOW);
    if(result->variance[dim] > MOTION_VAR_THRES)
    {
      isMotion = true;
    }
  }
  
  //if no motion detected, just return immediately, and move the processed sample # by
  //the standard offset (currently 6 samples at 12.5Hz)
  if(isMotion == false)
  {
    numSamplesProcessed = STD_OFFSET;
    return numSamplesProcessed;
  }
  
  //RUN FULL ALGORITHM....

    SC_Counts.RunAutoCorrelations++;

  //1.) see if there is a autocorr based step
  cycleLength = findAutocorrCycle(result, data);
  result->multiDimCycleLength = cycleLength;
  
  //2.) if we did not find a POTENTIAL autocorr step, set the cycle length to the default offset,
  //which moves the algorithm forward a fixed amount on steps we don't find
  if (cycleLength <= 0)
  {
    cycleFound = 0;
    cycleLength = STD_OFFSET;
  }
  else
  {
    cycleFound = 1;
  }
  
  //3.) found a POTENTIAL autocorr step, go back to the real data and make sure the step was large enough
  if (cycleFound)
  {
    
    bool rangePassed = autocorrCycleRangeCheck(data, cycleLength);
    
    if (rangePassed)
    {
      cycleLength *= SCA.modeVariables->autocorrCycleSkip;
      
//      cycleLength -= 1;   // HND: 12 May 2016,
      // we should offset a value = (2*cycleLen - 1) since two consecutive cycles always share 1 sample
      
      //found a autocorr cycle, and is was large enough to be counted as a step
      result->repInfo = REP_AUTOCORR_PASSED;
    }
    else // not passing the repRange check
    {
      cycleLength = STD_OFFSET;
      //found a autocorr cycle, but it was NOT large enough
      result->repInfo = REP_AUTOCORR_FAILED;
    }
  }
  else // 4.) cycle not found, run peak detection
  {
    
    SC_Counts.RunPeakDetections++;

    // new in version 10: we don't go back one step but do peak detection right here
    // the best thing is we don't need to care about previous steps anymore
    // in experiments, these helps to recover a lot of step from 'not so rythmic' data
    // on which autocorrelation fails to find a step
    
    cycleLength = detectPeak(data,SCA.repDetectWindowPDA);
    
    if (cycleLength)
    {
      result->repInfo = REP_PEAK_DETECT_PASSED;
    }
    else
    {
      cycleLength = SCA.repDetectWindowPDA;
      result->repInfo = REP_PEAK_DETECT_FAILED;
    }
  } //end cycle not found, so do peak detect
  
  numSamplesProcessed = cycleLength;
  
  
  
  if(result->repInfo == REP_AUTOCORR_PASSED)
  {
    result->numSteps = 2*SCA.modeVariables->autocorrCycleSkip;
  }
  else if(result->repInfo == REP_PEAK_DETECT_PASSED)
  {
    result->numSteps = 2;
  }
  
  
  return numSamplesProcessed;
}

int detectPeak(real_t* windowData[N_ACCEL_AXES],uint32_t len)
{
  real_t ddot_prev = 0;
  real_t ddot = 0;
  int peakCount = 1;
  int peak0, peak1 = 0, peak2 = 0; // current peak, previous peak, previous previous peak
  int i;
  real_t strength01=0, strength02=0, strength12=0;
  
  for (i = 1; i < len; i++)
  {
    // 1st derivative
    ddot_prev = ddot;
    ddot = windowData[0][i] + windowData[1][i] + windowData[2][i] - windowData[0][i - 1] - windowData[1][i - 1] - windowData[2][i - 1];
    
    if ((ddot_prev < 0 && ddot >= 0) || (ddot_prev > 0 && ddot <= 0))
    {
      // change sign in 1st derivative, we have peak at pos i-1
      
      // at the very beginning, just need to assign the previous peak and go on
      if (peak1 == 0)
      {
        peak1 = i-1;
        strength12 = (windowData[0][peak1] + windowData[1][peak1] + windowData[2][peak1] - windowData[0][peak2] - windowData[1][peak2] - windowData[2][peak2]);
        
        if (ABS(strength12) >= PEAK_RANGE_THRES)
        {
          peakCount++;
        }
        continue;
      }
      
      peak0 = i-1;
      
      // the peak strength is the difference between current peak and previous peaks
      strength01 = (windowData[0][peak0] + windowData[1][peak0] + windowData[2][peak0] - windowData[0][peak1] - windowData[1][peak1] - windowData[2][peak1]);
      strength02 = (windowData[0][peak0] + windowData[1][peak0] + windowData[2][peak0] - windowData[0][peak2] - windowData[1][peak2] - windowData[2][peak2]);
      
      //HND: 12 May 2016 : // we need to check if strength01 and strength12 have opposite sign.
      //since (strength01 * strength12) could be overflow, 
      
      if ((strength01 ^ strength12) < 0)
      {
        if (ABS(strength01) >= PEAK_RANGE_THRES)
        {
          if ((peakCount & 1)  || (peak0 - peak2 + 1 > MIN_REP_LENGTH))// there's a step between peak0 and peak2
          {
            peak2 = peak1;
            peak1 = peak0;
            strength12 = strength01;
            peakCount++;
            
            // printf("Peak %d at %d\n", peakCount, peak0);
            if (peakCount >= 4) break;
          }
          else
          {
            // peak0 is too close to peak 1, which implies a pulse, we'd better ignore peak0
          }
        }
        else
        {
          // this is a tricky case
          if (ABS(strength01) > ABS(strength12))
          {
            peak2 = peak1;
            peak1 = peak0;
            strength12 = strength01;
          }
        }
      }
      else // if strength01 and strength12 are the same sign, just need to update peak1 to current peak
      {
        peak1 = peak0;
        if (ABS(strength12) < PEAK_RANGE_THRES && ABS(strength02) >= PEAK_RANGE_THRES) peakCount++; // if previously we haven't counted this peak
        strength12 = strength02;
      }
    }
  }
  
  if (peakCount == 4)
  {
    return peak2;
  }
  
  return 0;
}

void clearCountStepResult(step_count_result_t *result)
{
  int32_t i;
  result->repInfo = REP_ALG_DID_NOT_RUN;
  for(i=0;i<N_ACCEL_AXES; i++)
  {
    result->variance[i]=0;
    result->power[i]=0;
  }
  result->numSteps = 0;
  result->multiDimCycleLength = 0;
}


int findMaximumSimilarity(widereal_t *correlation)
{
  widereal_t corrThreshold = (widereal_t)(CORRELATION_SUM_THRES * correlation[0]);
  widereal_t gapThreshold = (widereal_t)(CORR_SUM_MINMAX_GAP_THRES * correlation[0]);
  
  widereal_t ddot_prev, ddot = correlation[1] - correlation[0];
  
  int peakCount = 1;
  int peak0, peak1 = 0, peak2 = 0; // current peak, previous peak, previous previous peak
  int i;
  widereal_t strength01=0, strength02=0, strength12=0;
  int candidate = 0; // candidate rep position
  
  // printf("%lf %lf\n", corrThreshold, gapThreshold);
  // printf("%d\n", CORRELATION_LENGTH);
  for (i = 2; i < SCA.correlationLength; i++)
  {
    // 1st derivative
    ddot_prev = ddot;
    ddot = correlation[i] - correlation[i-1];
    
    // printf("%d %.3lf %.3lf\n", i, ddot_prev, ddot);
    
    if ((ddot_prev < 0 && ddot >= 0) || (ddot_prev > 0 && ddot <= 0))
    {
      // change sign in 1st derivative, we have peak at pos i-1
      // printf("Local max at %d\n", i);
      // at the very beginning, just need to assign the previous peak and go on
      if (peak1 == 0)
      {
        peak1 = i-1;
        strength12 = correlation[peak1] -  correlation[peak2];
        
        // printf("strength: %lf\n", strength12);
        
        if (ABS(strength12) >= gapThreshold)
        {
          peakCount++;
          // printf("Peak %d at %d\n", peakCount, peak1+1);
        }
        continue;
      }
      
      peak0 = i-1;
      
      // the peak strength is the difference between current peak and previous peaks
      strength01 = correlation[peak0] -  correlation[peak1];
      strength02 = correlation[peak0] -  correlation[peak2];
      
      // printf("Peaks %d %d %d, strengths: %.3lf %.3lf %.3lf\n", peak2+1, peak1+1, peak0+1, strength01, strength02, strength12);
      // if strengths are opposite signs, we have a possible rep
      //HND: 12 May 2016 : (strength01 ^ strength12) <= 0
      if ((strength01 ^ strength12) <= 0)
      {
        //printf("Peaks %d %d %d\n", peak2, peak1, peak0);
        if (ABS(strength01) >= gapThreshold)
        {
          peak2 = peak1;
          peak1 = peak0;
          strength12 = strength01;
          peakCount++;
          // printf("Peak %d at %d\n", peakCount, peak0+1);
          if (peakCount >= 6 && candidate) return candidate;
        }
        else
        {
          // this is a tricky case
          if (ABS(strength01) > ABS(strength12))
          {
            peak2 = peak1;
            peak1 = peak0;
            strength12 = strength01;
          }
        }
      }
      else // if strength01 and strength12 are the same sign, just need to update peak1 to current peak
      {
        peak1 = peak0;
        if (ABS(strength12) < PEAK_RANGE_THRES && ABS(strength02) >= PEAK_RANGE_THRES) peakCount++; // if previously we haven't counted this peak
        strength12 = strength02;
      }
      
      // Here we consider all the local maximum, which satisfies all the conditions: max min gap > threshold,
      // correlation value > threshold, position >= min replength, as the rep candidate
      if (ddot_prev > 0  && (peakCount & 1) && correlation[peak0] >= corrThreshold && (strength01 >= gapThreshold || strength02 >= gapThreshold) && peak0 >= MIN_REP_LENGTH)
      {
        candidate = peak0;
        // update the threshold to make sure the next accepted candidate should be better than this
        corrThreshold = correlation[peak0];
      }
      
    }
  }
  
  return candidate;
}
