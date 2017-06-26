/*
 * adcl362_spi.h
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */

#ifndef ACCEL_ADXL362_SPI_H_
#define ACCEL_ADCL362_SPI_H_

#include "accel_spi.h"

//#######################################################################################
// DEFINED CONSTANTS
//#######################################################################################

#define ACCEL_DATA_RATE_HZ                    ODR_100Hz //ODR_200Hz   //ODR_100Hz  //ODR_50Hz   //ODR_25Hz
#define ADXL362_INITIAL_POWER_MODE            NORMAL      //POWER_DOWN
#define ADXL362_FULLSCALE_RANGE               FULLSCALE_4
#define ADXL362_FILTER_BW                     ADXL362_BW_FILTER_HALF
#define ADXL362_BIT_RESOLUTION                ADXL362_BIT_DEPTH_FILTER_8   // 10 //12
#define ADXL362_FIFO_MODE                     FIFO_STREAM_MODE  //FIFO_MODE

#define ADXL362_FIFO_WATERMARK_SAMPLES        23//24//4    //NOTE:  The number of samples collected will be this number + 1, since the interrupt fires when it has collected more than this number of samples

#define ADXL362_FIFO_WATERMARK_SAMPLES_ADXL   (ADXL362_FIFO_WATERMARK_SAMPLES * 3)  // samples are seperated into x,y, and z for adxl chips
#define ACCEL_NUM_SAMPLES_PER_WTM             (ADXL362_FIFO_WATERMARK_SAMPLES + 1)

#define ADXL362_CHECK_WTM_FIFO_FOR_ALIGNMENT     1

//#######################################################################################
// DEFINED TYPES
//#######################################################################################

//#######################################################################################
// FUNCTION PROTOTYPES
//#######################################################################################

uint8_t adxl362CheckAlignment(uint16_t sample);
void adxl362FixAlignment(uint8_t fix_alignment);

uint8_t adxl362ReadReg(uint8_t Reg, uint8_t* Data);
uint8_t adxl362WriteReg(uint8_t Reg, uint8_t Data);

uint8_t adxl362AlignFIFO(void);
void accel_SelfTest(void);
void accel_ChipInit(void);

#endif /* ACCEL_ADXL362_SPI_H_ */
