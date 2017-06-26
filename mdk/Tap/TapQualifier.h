//
//  TapQualifier.h
//  tapdetect
//
//  Created by james ochs on 4/18/15.
//  Copyright (c) 2015 Misfit. All rights reserved.
//

#ifndef __tapdetect__TapQualifier__
#define __tapdetect__TapQualifier__

#include "scalgtypes.h"
#include "TapTypes.h"






extern void TapQualifier_Init(uint32_t currentSampleCounter);

#ifdef _MEX
extern uint32_t TapQualifier_Exec(tap_t *potentialTapReturn, tap_group_t *tapGroupReturn);
#else
extern uint32_t TapQualifier_Exec(void);
#endif


extern bool TapQualifier_IsProcessingTaps(void);



#endif /* defined(__tapdetect__TapQualifier__) */
