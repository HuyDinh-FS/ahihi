/*
 * accel_spi.h
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */

#ifndef ACCEL_ACCEL_SPI_H_
#define ACCEL_ACCEL_SPI_H_

#define USE_ADXL362 1

#define ACCEL_USE_FIFO

#define ACCEL_NUM_AXIS                 3


#include <stdint.h>

#define USE_DMA_SPI_ACCEL						0
#define USE_NONBLOCKING_SPI_ACCEL		0

extern void accelInit(void);
extern void accel_wtm_int_handler();
extern void wtm_tx_callback(void *user_data, uint16_t transferred);
#endif /* ACCEL_ACCEL_SPI_H_ */
