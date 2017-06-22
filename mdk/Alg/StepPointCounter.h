//
//  AssignStepPoints.h
//  stepcount
//
//  Created by james ochs on 5/26/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#ifndef stepcount_AssignStepPoints_h
#define stepcount_AssignStepPoints_h


#include "typedef.h"
#include "scalgtypes.h"
#include "StepCount.h"


extern void StepPointCounter_Init(void);
extern void StepPointCounter_NewWindow(step_count_result_t *windowData);
extern void StepPointCounter_NewMinute(minute_summary_t *result);


#endif
