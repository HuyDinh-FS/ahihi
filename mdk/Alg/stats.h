//
//  stats.h
//  stepcount
//
//  Created by james ochs on 5/9/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#ifndef stepcount_stats_h
#define stepcount_stats_h

#include "scalgtypes.h"

extern real_t mean(const real_t *windowData, int32_t windowLen);
extern real_t variance(const real_t *windowData, int windowLen);
extern real_t max(real_t A, real_t B);
void autocorr(const real_t *wdata, real_t *correlation, int32_t corrLen, int32_t dataLen);
void removeDCCopy(const real_t *x, real_t *y, int32_t windowLen);


extern float mad(int32_t *v, int32_t len);
extern float median(int32_t *x, int n);

#define ABS(x) ((x > 0) ? (x) : (-(x)))

#endif
