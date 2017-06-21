/*
 * ADXL362.c
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */

#include "ADXL362.h"
#include "spi_for_accel.h"
#include "stdio.h"

void ADXL362_SetRegisterValue(unsigned short registerValue,
                              unsigned char  registerAddress,
                              unsigned char  bytesNumber)
{
    unsigned char buffer[4] = {0, 0, 0, 0};
    
    buffer[0] = ADXL362_WRITE_REG;
    buffer[1] = registerAddress;
    buffer[2] = (registerValue & 0x00FF);
    buffer[3] = (registerValue >> 8);
    ADXL362_SPI_Write(ADXL362_SLAVE_ID, buffer, bytesNumber + 2);
}

void ADXL362_SetOutputRate(unsigned char outRate)
{
    unsigned char oldFilterCtl = 0;
    unsigned char newFilterCtl = 0;
    
    ADXL362_GetRegisterValue(&oldFilterCtl, ADXL362_REG_FILTER_CTL, 1);
    newFilterCtl = oldFilterCtl & ~ADXL362_FILTER_CTL_ODR(0x7);
    newFilterCtl = newFilterCtl | ADXL362_FILTER_CTL_ODR(outRate);
    ADXL362_SetRegisterValue(newFilterCtl, ADXL362_REG_FILTER_CTL, 1);
}

void ADXL362_GetRegisterValue(unsigned char* pReadData,
                              unsigned char  registerAddress,
                              unsigned char  bytesNumber)
{
    unsigned char buffer[32];
    unsigned char index = 0;
    
    buffer[0] = ADXL362_READ_REG;
    buffer[1] = registerAddress;
    for(index = 0; index < bytesNumber; index++)
    {
        buffer[index + 2] = pReadData[index];
    }
    ADXL362_SPI_Read(ADXL362_SLAVE_ID, buffer, bytesNumber + 2);
    for(index = 0; index < bytesNumber; index++)
    {
        pReadData[index] = buffer[index + 2];
    }
}

void ADXL362_SetRange(unsigned char gRange)
{
    unsigned char oldFilterCtl = 0;
    unsigned char newFilterCtl = 0;
    
    ADXL362_GetRegisterValue(&oldFilterCtl, ADXL362_REG_FILTER_CTL, 1);
    newFilterCtl = oldFilterCtl & ~ADXL362_FILTER_CTL_RANGE(0x3);
    newFilterCtl = newFilterCtl | ADXL362_FILTER_CTL_RANGE(gRange);
    ADXL362_SetRegisterValue(newFilterCtl, ADXL362_REG_FILTER_CTL, 1);
}

void ADXL362_SetFilterBW(unsigned char filter_bw)
{
    unsigned char oldFilterCtl = 0;
    unsigned char newFilterCtl = 0;
    
    ADXL362_GetRegisterValue(&oldFilterCtl, ADXL362_REG_FILTER_CTL, 1);
    newFilterCtl = oldFilterCtl & ~ADXL362_FILTER_CTL_HALF_BW;
    if (filter_bw)
    {
        newFilterCtl = newFilterCtl | ADXL362_FILTER_CTL_HALF_BW;
    }
    ADXL362_SetRegisterValue(newFilterCtl, ADXL362_REG_FILTER_CTL, 1);
}

void ADXL362_FifoSetup(unsigned char  mode,
                       unsigned short waterMarkLvl,
                       unsigned char  enTempRead)
{
    unsigned char writeVal = 0;
    
    writeVal = ADXL362_FIFO_CTL_FIFO_MODE(mode) |
    (enTempRead * ADXL362_FIFO_CTL_FIFO_TEMP);                 // We are running the FIFO between 0-255 TODO:JEE make it a conditional for setting AH bit
    ADXL362_SetRegisterValue(writeVal, ADXL362_REG_FIFO_CTL, 1);
    ADXL362_SetRegisterValue(waterMarkLvl, ADXL362_REG_FIFO_SAMPLES, 1);
}

void ADXL362_SetPowerMode(unsigned char pwrMode)
{
    unsigned char oldPowerCtl = 0;
    unsigned char newPowerCtl = 0;
    
    ADXL362_GetRegisterValue(&oldPowerCtl, ADXL362_REG_POWER_CTL, 1);
    newPowerCtl = oldPowerCtl & ~ADXL362_POWER_CTL_MEASURE(0x3);
    newPowerCtl = newPowerCtl |
    (pwrMode * ADXL362_POWER_CTL_MEASURE(ADXL362_MEASURE_ON));
    ADXL362_SetRegisterValue(newPowerCtl, ADXL362_REG_POWER_CTL, 1);
}

void ADXL362_GetFifoValue(unsigned char* pBuffer, unsigned short bytesNumber)
{
    unsigned char  buffer[512];
    unsigned short index = 0;
    
    buffer[0] = ADXL362_WRITE_FIFO;
    for(index = 0; index < bytesNumber; index++)
    {
        buffer[index + 1] = pBuffer[index];
    }
    ADXL362_SPI_Read(ADXL362_SLAVE_ID, buffer, bytesNumber + 1);
    for(index = 0; index < bytesNumber; index++)
    {
        pBuffer[index] = buffer[index + 1];
    }
}
