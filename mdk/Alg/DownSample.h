//
//  SCDownSample.h
//  stepcount
//
//  Created by james ochs on 5/8/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#ifndef stepcount_SCDownSample_h
#define stepcount_SCDownSample_h

#include "typedef.h"
#include "scalgtypes.h"

extern void DownSample_100to12dot5_Reset(void);
extern bool DownSample_100to12dot5(const accel_value_t inputSamples[N_ACCEL_AXES], accel_value_t outputSamples[N_ACCEL_AXES]);

#endif
