//
//  TapDetection.h
//  tapdetect
//
//  Created by james ochs on 5/4/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#ifndef __tapdetect__TapDetection__
#define __tapdetect__TapDetection__

#include "TapTypes.h"


extern void TapDetection_Init(void);

#ifdef _MEX
 #ifdef _TD_USE_SHINE_INTERFACE
extern uint32_t TapDetection_Exec(const tap_detect_sample_t *currentSample, tap_t *potentialTapReturn, tap_group_t *tapGroupReturn);
 #else
extern uint32_t TapDetection_Exec(accel_value_t currentSample[N_ACCEL_AXES], tap_t *potentialTapReturn, tap_group_t *tapGroupReturn);
 #endif
#else
 #ifdef _TD_USE_SHINE_INTERFACE
  extern uint32_t TapDetection_Exec(const tap_detect_sample_t *currentSample);
#else
  extern uint32_t TapDetection_Exec(accel_value_t currentSample[N_ACCEL_AXES]);
 #endif
#endif

#endif /* defined(__tapdetect__TapDetection__) */
