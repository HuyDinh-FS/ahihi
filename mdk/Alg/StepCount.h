//
//  StepCount.h
//  stepcount
//
//  Created by james ochs on 5/9/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#ifndef stepcount_StepCount_h
#define stepcount_StepCount_h

#include "scalgtypes.h"

typedef struct
{
    uint16_t steps;
    uint32_t accumVariance;
    uint16_t activityPoints;  //carries over the last 8 bit to the next minute
    uint16_t numWindows;
#if USE_NEW_BTLE_FORMAT_4
    uint32_t maxVariance;
#endif
}minute_summary_t;


typedef enum {
    REP_NO_MOTION_DETECTED = 0,  //is motion failed
    REP_AUTOCORR_PASSED = 1,     //autocorrelation cycle found and passed range check
    REP_AUTOCORR_FAILED = 2,     //autocorrelation cycle found but failed the range check
    REP_PEAK_DETECT_PASSED = 3,  //autocorr cycle not found, so peak detection was run, peak detection passed
    REP_PEAK_DETECT_FAILED = 4,  //autocorr cycyle not found, peak detection was run, but a peak was not found
    REP_ALG_DID_NOT_RUN = 0xFF //algorithm was not run because there was not enough data to run it (this should never occur on Shine_J)
} repinfo_t;

static inline bool repInfoMotionDetected(repinfo_t info)
{
    return (info > REP_NO_MOTION_DETECTED && info < REP_ALG_DID_NOT_RUN);
}

typedef struct
{
    real_t variance[N_ACCEL_AXES];
    real_t power[N_ACCEL_AXES];
    repinfo_t repInfo;
    uint8_t numSteps;
    uint32_t multiDimCycleLength;
    
}step_count_result_t;


typedef enum
{
  SC_NORMAL_MODE,
  SC_SWIM_MODE
}step_count_alg_mode_t;

extern void SC_Init(void);
extern uint32_t SC_Execute(real_t *data[N_ACCEL_AXES], int32_t len, step_count_result_t *result);
extern void SC_AggregateMinute(minute_summary_t *minuteData);
extern int32_t SC_GetWindowSize(void);
extern void SC_SetAlgMode(step_count_alg_mode_t mode);
void SC_NotifyNewDay ( void );
void SC_RamLost ( void );
#define SC_ALG_MAX_WINDOW_LENGTH 90

#define STEP_COUNT_ALG_SAMPLES_PER_SECOND 12.5
#define STEP_COUNT_ALG_SAMPLES_PER_MINUTE 750

#endif
