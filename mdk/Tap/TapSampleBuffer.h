#ifndef _TAP_BUFFER_H
#define _TAP_BUFFER_H

#include "TapTypes.h"

#define TAP_BUF_LEN 32
#define TAP_BUF_WRAP_MASK 31u

//check to make sure that TAP_BUF_LEN is really a power of two!!
#if ((TAP_BUF_LEN & (TAP_BUF_LEN - 1)) != 0)
#error "TAP_BUF_LEN must be a power of two"
#endif


typedef struct
{
  uint32_t headIdx;
  tap_detect_sample_t data[TAP_BUF_LEN];
  
}tap_sample_buffer_t;

extern tap_detect_sample_t TapBuffer[TAP_BUF_LEN];
extern uint32_t TapBufHead;

//extern tap_sample_buffer_t TSB;

static inline void TapSampleBuffer_Init(void)
{
  TapBufHead = 0;
}

#ifdef _TD_USE_SHINE_INTERFACE
static inline void TapSampleBuffer_WriteOneSample(const tap_detect_sample_t *currentSample)
{
  tap_detect_sample_t *dest = TapBuffer + TapBufHead;
  
  //doing a "manual" memberwise copy like this is significantly more
  //efficient that *dest = *data, which results in the compiler branching to
  //memcpy
  dest->x = currentSample->x;
  dest->y = currentSample->y;
  dest->z = currentSample->z;
  TapBufHead++;
  TapBufHead &= TAP_BUF_WRAP_MASK;
}
#else
static inline void TapSampleBuffer_WriteOneSample(const accel_value_t currentSample[N_ACCEL_AXES])
{
  tap_detect_sample_t *dest = TapBuffer + TapBufHead;
  
  //doing a "manual" memberwise copy like this is significantly more
  //efficient that *dest = *data, which results in the compiler branching to
  //memcpy
  dest->x = currentSample[0];
  dest->y = currentSample[1];
  dest->z = currentSample[2];
  TapBufHead++;
  TapBufHead &= TAP_BUF_WRAP_MASK;
}
#endif

static inline const tap_detect_sample_t *TapSampleBuffer_Read(uint32_t sampleIndex)
{
  return TapBuffer + (sampleIndex & TAP_BUF_WRAP_MASK);
}

#endif
