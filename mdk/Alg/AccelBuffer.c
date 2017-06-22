/**************************************************************************
 Module:         AccelBuffer.c
 
 Revision:       1.0
 
 Description:    AccelBuffer module that:
 1.) accepts new 3-axis accelerometer data via AccelBuffer_Write()
 2.) buffers the data in a circular buffer
 3.) provides access to stored accelrometer data via AccelBuffer_Read()
 
 Notes:          ---
 
 History:
 Date            Name            Changes
 -----------     --------        -------------------------------------------
 05/20/15          JPO             Initial Revision
 
 
 **************************************************************************/

//#########################################################################
//      #INCLUDES
//#########################################################################
#include "typedef.h"
#include <string.h>
#include "AccelBuffer.h"
#include "scalgtypes.h"

//#########################################################################
//      TESTING #DEFINES
//#########################################################################



//#########################################################################
//      CONSTANT #DEFINES
//#########################################################################

//the algorithm window is 90 samples, the algorithm gets triggered
//after a complete transfer of 12.5 low frequency samples.  So, WORST
//CASE, there are 89 samples in the buffer and we receive 13 new samples
//before the algorithm runs.  89+13 = 102 plus a ~5% margin = 115
#warning "Why was the buffer overflowing when it was set to a max of 115 samples"
#define ACCEL_BUF_LEN 250   //115
#define ACCEL_BUF_LEN_IS_POWER_OF_2 0

#if ACCEL_BUF_LEN_IS_POWER_OF_2
#define ACCEL_BUF_WRAP_MASK ACCEL_BUF_LEN-1
#endif
//check to make sure that ACCEL_BUF_LEN is really a power of two!!
#if ACCEL_BUF_LEN_IS_POWER_OF_2 && ((ACCEL_BUF_LEN & (ACCEL_BUF_LEN - 1)) != 0)
#error "ACCEL_BUF_LEN is NOT a power of two but ACCEL_BUF_LEN_IS_POWER_OF_2 is set to 1"
#endif


//#########################################################################
//      MODULE TYPES
//#########################################################################



//#########################################################################
//      CONSTANTS
//#########################################################################



//#########################################################################
//      MODULE LEVEL VARIABLES
//#########################################################################
typedef struct
{
  uint32_t volatile head;
  uint32_t volatile totalSamples;
  accel_value_t data[N_ACCEL_AXES][ACCEL_BUF_LEN];
  uint32_t volatile minuteMarker;
  uint16_t volatile highWaterMark;
} AccelBuffer_T;

static AccelBuffer_T AB;

//#########################################################################
//      MACROS
//#########################################################################



//#########################################################################
//      PRIVATE FUNCTION PROTOTYPES
//#########################################################################



//#########################################################################
//      PUBLIC FUNCTIONS
//#########################################################################

/****************************************************************
 Function:
 XXX( )
 
 Input Parameters:
 ---
 
 Output Parameters:
 ---
 
 Description:
 ---
 
 Notes:
 ---
 
 Author, Date:
 MRM, MM/DD/YYYY
 */
void AccelBuffer_Init(void)
{
  AB.head = 0;
  AB.totalSamples = 0;
  AB.minuteMarker = UINT32_MAX;
  AB.highWaterMark = 0;
}

uint16_t AccelBuffer_GetHighWaterMark(bool resetAfterGet)
{
  uint32_t retVal = AB.highWaterMark;
  if(resetAfterGet)
  {
    AB.highWaterMark = 0;
  }
  return retVal;
}

void AccelBuffer_StampMinute(void)
{
  AB.minuteMarker = AB.totalSamples;
}

uint32_t AccelBuffer_GetCurrentMinuteSample(void)
{
  return AB.minuteMarker;
}

uint32_t AccelBuffer_GetTotalSamplesSinceInit(void)
{
  return AB.totalSamples;
}

void AccelBuffer_WriteOneSample(const accel_value_t data[N_ACCEL_AXES])
{
  const accel_value_t *pData = &data[0];
  
  AB.data[0][AB.head] = *pData++;
  AB.data[1][AB.head] = *pData++;
  AB.data[2][AB.head] = *pData++;
  
  //    int32_t dim;
  //    for(dim = 0; dim < N_ACCEL_AXES; dim++)
  //    {
  //        AB.data[dim][AB.head] = data[dim];
  //    }
  
#if ACCEL_BUF_LEN_IS_POWER_OF_2
  AB.head = (AB.head+1) & ACCEL_BUF_WRAP_MASK;
#else
  AB.head = (AB.head >= ACCEL_BUF_LEN-1 ? 0 : AB.head+1);
#endif
  AB.totalSamples++;
}

void AccelBuffer_WriteSamples(const accel_value_t *pData, uint32_t NumSets, bool Stamp )
{
  while( NumSets-- )
  {
    AB.data[0][AB.head] = *pData++;
    AB.data[1][AB.head] = *pData++;
    AB.data[2][AB.head] = *pData++;
    
#if ACCEL_BUF_LEN_IS_POWER_OF_2
    AB.head = (AB.head+1) & ACCEL_BUF_WRAP_MASK;
#else
    AB.head = (AB.head >= ACCEL_BUF_LEN-1 ? 0 : AB.head+1);
#endif
    
    AB.totalSamples++;
    
    if( Stamp )
    {
      Stamp = false;
      AccelBuffer_StampMinute();
    }
  }
}

bool AccelBuffer_Read(real_t *dataOut[N_ACCEL_AXES],uint32_t lenToRead, uint32_t sampleIndex)
{
  
  bool retVal;
  uint32_t totalSamples;
  uint32_t currentDepth;
#if !ACCEL_BUF_LEN_IS_POWER_OF_2
  uint32_t head;
#endif
  totalSamples = AB.totalSamples;
#if !ACCEL_BUF_LEN_IS_POWER_OF_2
  head = AB.head;
#endif
  currentDepth = totalSamples - sampleIndex;
  if(currentDepth > AB.highWaterMark)
  {
    AB.highWaterMark = currentDepth;
  }
  
  if(sampleIndex+lenToRead > totalSamples || sampleIndex>totalSamples) //not enough data
  {
    retVal=false;
  }
  else if(currentDepth > ACCEL_BUF_LEN)
  {
    //buffer overflow!! asking for data that is older than the buffer can support
    // ASSERT unavailable on STM32
    while(1);
    retVal=false;
  }
  else
  {
#if ACCEL_BUF_LEN_IS_POWER_OF_2
    uint32_t readStartIdx = sampleIndex & ACCEL_BUF_WRAP_MASK;
#else
    int32_t readStartIdx = head - (totalSamples - sampleIndex);
    if(readStartIdx < 0)
    {
      readStartIdx += ACCEL_BUF_LEN;
    }
#endif
    
    int32_t numSamplesOverflowed = readStartIdx + lenToRead - ACCEL_BUF_LEN;
    int32_t dim,i,copyLen;
    accel_value_t *src;
    real_t *dest;
    
    if(numSamplesOverflowed > 0)
    {
      for(dim = 0; dim<N_ACCEL_AXES; dim++)
      {
        //copy mem chunk #1, before the overflow
        //memcpy(&dataOut[dim][0],&AB.data[dim][readStartIdx],(lenToRead-numSamplesOverflowed)*sizeof(dataOut[0][0]));
        dest = &dataOut[dim][0];
        src = &AB.data[dim][readStartIdx];
        copyLen = lenToRead-numSamplesOverflowed;
        for(i=0; i<copyLen; i++)
        {
          dest[i] = (real_t)src[i];
        }
        //copy mem chunk #2, after the overflow
        //memcpy(&dataOut[dim][lenToRead-numSamplesOverflowed],&AB.data[dim][0],numSamplesOverflowed*sizeof(dataOut[0][0]));
        dest = &dataOut[dim][lenToRead-numSamplesOverflowed];
        src = &AB.data[dim][0];
        copyLen = numSamplesOverflowed;
        for(i=0; i<copyLen; i++)
        {
          dest[i] = (real_t)src[i];
        }
      }
    }
    else //nothing overflowed, single memcpy
    {
      for(dim = 0; dim<N_ACCEL_AXES; dim++)
      {
        dest = &dataOut[dim][0];
        src = &AB.data[dim][readStartIdx];
        copyLen = lenToRead;
        for(i=0; i<copyLen; i++)
        {
          dest[i] = (real_t)src[i];
        }
        //memcpy(dataOut[dim],&AB.data[dim][readStartIdx],lenToRead*sizeof(dataOut[0][0]));
      }
    }
    retVal = true;
  }
  return retVal;
}


//#########################################################################
//      PRIVATE FUNCTIONS
//#########################################################################




//#########################################################################
//      INTERRUPTS
//#########################################################################



//#########################################################################
//      TEST HARNESSES
//#########################################################################




//#########################################################################
//      END OF XXX.c
//#########################################################################


