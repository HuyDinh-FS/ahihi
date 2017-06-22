//
//  DownSample.c
//  stepcount
//
//  Created by james ochs on 5/8/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//



#include <stdio.h>
#include "typedef.h"
#include "scalgtypes.h"

#define FILTER_LENGTH 15
#define SECOND_STAGE_INIT_VAL 8 //Start the second stage counter at 8 to offset it from the stage 1 counter.
#define NUM_STAGES 2

// original values were created with
// H = design(fdesign.lowpass('N,Fp,Fst',14,.05,.0625), 'ALLFIR');
// b = H(2).Numerator;
// the values here are multiplied by 2^19
// Huy's note: b's range is 15 bit, data's range is 12 bit, the filter length is 4 bit -> values fit in 31 bit
#ifdef USE_DOUBLE
static const double B_Coeffs[FILTER_LENGTH] = {0.042923, 0.046497, 0.049654, 0.052331, 0.054472, 0.056035, 0.056985, 0.057304, 0.056985, 0.056035, 0.054472, 0.052331, 0.049654, 0.046497, 0.042923};
#else
static const int32_t B_Coeffs[FILTER_LENGTH] = {22504, 24378, 26033, 27436, 28559, 29378, 29877, 30044, 29877, 29378, 28559, 27436, 26033, 24378, 22504};
#endif


typedef struct
{
    int32_t cnt[NUM_STAGES];
    int32_t sumX[NUM_STAGES];
    int32_t sumY[NUM_STAGES];
    int32_t sumZ[NUM_STAGES];

}DownSample_T;


static DownSample_T SCDS = 
{
  .cnt = {0}, 
  .sumX = {0}, 
  .sumY = {0}, 
  .sumZ = {0}
};

void DownSample_100to12dot5_Reset(void){
    
    SCDS.cnt[0] = 0;
    SCDS.cnt[1] = SECOND_STAGE_INIT_VAL;
    
    for (uint32_t i=0; i<NUM_STAGES; i++)
    {
        SCDS.sumX[i] = 0;
        SCDS.sumY[i] = 0;
        SCDS.sumZ[i] = 0;
    }
    
}

bool DownSample_100to12dot5(const accel_value_t inputSamples[N_ACCEL_AXES], accel_value_t outputSamples[N_ACCEL_AXES])
{
    //Pseudo Code for filter sample-by-sample
    //
    //  For each filter stage       (we have two filter stages to keep track of the sums of the overlapped low frequency sample data)
    //    If we are still collecting samples for this stage
    //      Scale this sample and add it to the sum for this stage
    //      Increment the samplecounter var
    //    Else (we have already collected all the samples for this stage) so prepare the filtered sample to be returned as the next low frequency sample
    //      Scale the sample values down since we had scaled up the filter coefficients to work with integers instead of floats (the float version of the coefficients are all less than one)
    //      Set the pointer of the sample to be returned
    //      Clear the filter stage values to prepare for the next sample round
    //      Indicate that we are returning a valid filtered sample pointer
    
    
    bool filterSampleReturn = false;
    
    for (u8 i = 0; i<2; i++){        //loop through the filter stages
        
        if (SCDS.cnt[i] < FILTER_LENGTH)
        {
            SCDS.sumX[i] += B_Coeffs[SCDS.cnt[i]] * inputSamples[X_AXIS];
            SCDS.sumY[i] += B_Coeffs[SCDS.cnt[i]] * inputSamples[Y_AXIS];
            SCDS.sumZ[i] += B_Coeffs[SCDS.cnt[i]] * inputSamples[Z_AXIS];
            SCDS.cnt[i]++;
        }
        else      //Have reached the last element of this filter stage, so prepare the filtered sample to be returned as the next low frequency sample
        {
            
            //Scale sample values
            SCDS.sumX[i] >>= 19;//16;
            SCDS.sumY[i] >>= 19;//16;
            SCDS.sumZ[i] >>= 19;//16;
            
            //Store filtered sample
            //p_outsample = &sumSample[i];
            outputSamples[X_AXIS] = (accel_value_t)SCDS.sumX[i];
            outputSamples[Y_AXIS] = (accel_value_t)SCDS.sumY[i];
            outputSamples[Z_AXIS] = (accel_value_t)SCDS.sumZ[i];
            
            //Reset the filter stage vars
            SCDS.cnt[i] = 0;
            SCDS.sumX[i] = 0;
            SCDS.sumY[i] = 0;
            SCDS.sumZ[i] = 0;
            
            //Indicate that we are returning a valid filtered sample pointer
            filterSampleReturn = true;
        }
    }
    
    return filterSampleReturn;
}
