//
//  TapBuffer.c
//  tapdetect
//
//  Created by james ochs on 5/3/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//


#include "TapSampleBuffer.h"

tap_detect_sample_t TapBuffer[TAP_BUF_LEN];
uint32_t TapBufHead;

//all functions are inlined in .h file
