/*
 * adxl326_spi.c
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */
#include "ADXL362.h"
#include "spi_for_accel.h"
#include "stdio.h"
#include "adxl362_spi.h"

u8_t adxl362ReadReg(u8_t Reg, u8_t* Data)
{
    
    ADXL362_GetRegisterValue(Data, Reg, 1);
    return 1; //(i2cCheckError() == 0);
}

u8_t adxl362WriteReg(u8_t Reg, u8_t Data)
{
    
    ADXL362_SetRegisterValue(Data, Reg, 1);
    return 1; //(i2cCheckError() == 0);
}

void accel_SelfTest(void)
{
    volatile unsigned char nID = 0;
    
    adxl362ReadReg(ADXL362_REG_PARTID, &nID);
    //TODO: JEE: investigate throwing an error if the read reg fails
    //printf("nId = %.2x\n", nID);
    if (nID != ADXL362_PART_ID) {
        //TODO: JEE: add hardware log here or throw error
        return;
    }
}

void accel_ChipInit(void)
{
    volatile uint8_t success = 0;
    //set OuputRate
    ADXL362_SetOutputRate(ACCEL_DATA_RATE_HZ);
    
    //set Fullscale
    ADXL362_SetRange(ADXL362_FULLSCALE_RANGE);
    
    //set Filter
    ADXL362_SetFilterBW(ADXL362_FILTER_BW);
    
    //********FIFO WTM Configuration********
#ifdef ACCEL_USE_FIFO
    //Setup Fifo
    ADXL362_FifoSetup(ADXL362_FIFO_MODE, ADXL362_FIFO_WATERMARK_SAMPLES_ADXL, 0);
    
    //Set Int1 to Watermark
    adxl362WriteReg(ADXL362_REG_INTMAP1, ADXL362_STATUS_FIFO_WATERMARK);
#endif
    
    //Set Activity/Inactivity control - Set Loop mode
    adxl362WriteReg(ADXL362_REG_ACT_INACT_CTL,
                    ADXL362_ACT_INACT_CTL_LINKLOOP(ADXL362_MODE_DEFAULT));
    
    //set PowerMode - Enable Measurement
    ADXL362_SetPowerMode(ADXL362_INITIAL_POWER_MODE);
    
    success = adxl362AlignFIFO();
    if (success)
    {
        printf("Init successful\n");
    }
    else
    {
        printf("Init failed\n");
    }
    
}

uint8_t adxl362AlignFIFO(void)
{
    int i = 0;
    int j = 0;
    uint8_t reg_data[18];
    uint8_t loop_active = 1;
    uint16_t fifo_count;
    uint8_t success = 0;
    
    while (loop_active)
    {
        if (160000 < i++)
        {
            break;
        }
        
        //wait for at least 12 samples to accumulate in FIFO
        adxl362ReadReg(ADXL362_REG_FIFO_L, &reg_data[0]);       // check entry count in fifo
        adxl362ReadReg(ADXL362_REG_FIFO_H, &reg_data[1]);       // check entry count in fifo
        fifo_count = reg_data[1] << 8;
        fifo_count |= reg_data[0];
        if (12 <= fifo_count)                                      // if 3 or more
        {
            ADXL362_GetFifoValue(&reg_data[0], 18); // discard the first 9 samples, these are just junks
            
            for (j = 0; j < 3; j++)
            {
                ADXL362_GetFifoValue(&reg_data[0], 2);              // grab a sample
                reg_data[1] >>= 6; // right shift out all bits except axis bits (from high byte)
                if (ADXL362_Z_AXIS_DES == reg_data[1]) // check if z-axis sample was read last
                {
                    loop_active = 0; // the pump is primed for x-axis data next.
                    success = 1;
                    break;
                }
            }
            break;
        }
    }
    return success;
}

