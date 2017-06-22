/****************************************************************************************
 Module:		    AccelBuffer.h
 
 Revision:	    1.0
 
 Description:	---
 
 Notes:			  ---
 
 History:
 Date		      Name	    Changes
 -----------	  --------	-------------------------------------------
 
 
 ****************************************************************************************/

#ifndef _ACCEL_BUFFER_H
#define _ACCEL_BUFFER_H

//#######################################################################################
// #INCLUDES
//#######################################################################################

#include "typedef.h"
#include "scalgtypes.h"




//#######################################################################################
// FUNCTION PROTOTYPES
//#######################################################################################

extern void AccelBuffer_Init(void);
extern void AccelBuffer_WriteOneSample(const accel_value_t data[N_ACCEL_AXES]);
extern void AccelBuffer_WriteSamples  (const accel_value_t *pData, uint32_t NumSets, bool StampMinute );
extern bool AccelBuffer_Read(real_t *dataOut[N_ACCEL_AXES],uint32_t lenToRead, uint32_t sampleIndex);
extern void AccelBuffer_StampMinute(void);
extern uint32_t AccelBuffer_GetCurrentMinuteSample(void);
extern uint32_t AccelBuffer_GetTotalSamplesSinceInit(void);
extern uint16_t AccelBuffer_GetHighWaterMark(bool resetAfterGet);

//#######################################################################################
// END OF XXX.h
//#######################################################################################
#endif
