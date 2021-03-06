/*
 * ADXL362.h
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */

#ifndef ACCEL_ADXL362_H_
#define ACCEL_ADXL362_H_

/***************************************************************************//**
                                                                              *   @file   ADXL362.h
                                                                              *   @brief  Header file of ADXL362 Driver.
                                                                              *   @author DNechita(Dan.Nechita@analog.com)
                                                                              ********************************************************************************
                                                                              * Copyright 2012(c) Analog Devices, Inc.
                                                                              *
                                                                              * All rights reserved.
                                                                              *
                                                                              * Redistribution and use in source and binary forms, with or without
                                                                              * modification, are permitted provided that the following conditions are met:
                                                                              *  - Redistributions of source code must retain the above copyright
                                                                              *    notice, this list of conditions and the following disclaimer.
                                                                              *  - Redistributions in binary form must reproduce the above copyright
                                                                              *    notice, this list of conditions and the following disclaimer in
                                                                              *    the documentation and/or other materials provided with the
                                                                              *    distribution.
                                                                              *  - Neither the name of Analog Devices, Inc. nor the names of its
                                                                              *    contributors may be used to endorse or promote products derived
                                                                              *    from this software without specific prior written permission.
                                                                              *  - The use of this software may or may not infringe the patent rights
                                                                              *    of one or more patent holders.  This license does not release you
                                                                              *    from the requirement that you obtain separate licenses from these
                                                                              *    patent holders to use this software.
                                                                              *  - Use of the software either in source or binary form, must be run
                                                                              *    on or directly connected to an Analog Devices Inc. component.
                                                                              *
                                                                              * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
                                                                              * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
                                                                              * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
                                                                              * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
                                                                              * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
                                                                              * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
                                                                              * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
                                                                              * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
                                                                              * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
                                                                              * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
                                                                              *
                                                                              ********************************************************************************
                                                                              *   SVN Revision: 797
                                                                              *******************************************************************************/

#ifndef __ADXL362_H__
#define __ADXL362_H__

#include "stdint.h"
/******************************************************************************/
/********************************* ADXL362 ************************************/
/******************************************************************************/

#define ADXL362_SLAVE_ID    1

/* ADXL362 communication commands */
#define ADXL362_WRITE_REG           0x0A
#define ADXL362_READ_REG            0x0B
#define ADXL362_WRITE_FIFO          0x0D

/* Registers */
#define ADXL362_REG_DEVID_AD            0x00
#define ADXL362_REG_DEVID_MST           0x01
#define ADXL362_REG_PARTID              0x02
#define ADXL362_REG_REVID               0x03
#define ADXL362_REG_XDATA               0x08
#define ADXL362_REG_YDATA               0x09
#define ADXL362_REG_ZDATA               0x0A
#define ADXL362_REG_STATUS              0x0B
#define ADXL362_REG_FIFO_L              0x0C
#define ADXL362_REG_FIFO_H              0x0D
#define ADXL362_REG_XDATA_L             0x0E
#define ADXL362_REG_XDATA_H             0x0F
#define ADXL362_REG_YDATA_L             0x10
#define ADXL362_REG_YDATA_H             0x11
#define ADXL362_REG_ZDATA_L             0x12
#define ADXL362_REG_ZDATA_H             0x13
#define ADXL362_REG_TEMP_L              0x14
#define ADXL362_REG_TEMP_H              0x15
#define ADXL362_REG_SOFT_RESET          0x1F
#define ADXL362_REG_THRESH_ACT_L        0x20
#define ADXL362_REG_THRESH_ACT_H        0x21
#define ADXL362_REG_TIME_ACT            0x22
#define ADXL362_REG_THRESH_INACT_L      0x23
#define ADXL362_REG_THRESH_INACT_H      0x24
#define ADXL362_REG_TIME_INACT_L        0x25
#define ADXL362_REG_TIME_INACT_H        0x26
#define ADXL362_REG_ACT_INACT_CTL       0x27
#define ADXL362_REG_FIFO_CTL            0x28
#define ADXL362_REG_FIFO_SAMPLES        0x29
#define ADXL362_REG_INTMAP1             0x2A
#define ADXL362_REG_INTMAP2             0x2B
#define ADXL362_REG_FILTER_CTL          0x2C
#define ADXL362_REG_POWER_CTL           0x2D
#define ADXL362_REG_SELF_TEST           0x2E

/* ADXL362_REG_STATUS definitions */
#define ADXL362_STATUS_ERR_USER_REGS        (1 << 7)
#define ADXL362_STATUS_AWAKE                (1 << 6)
#define ADXL362_STATUS_INACT                (1 << 5)
#define ADXL362_STATUS_ACT                  (1 << 4)
#define ADXL362_STATUS_FIFO_OVERRUN         (1 << 3)
#define ADXL362_STATUS_FIFO_WATERMARK       (1 << 2)
#define ADXL362_STATUS_FIFO_RDY             (1 << 1)
#define ADXL362_STATUS_DATA_RDY             (1 << 0)

/* ADXL362_REG_ACT_INACT_CTL definitions */
#define ADXL362_ACT_INACT_CTL_LINKLOOP(x)   (((x) & 0x3) << 4)
#define ADXL362_ACT_INACT_CTL_INACT_REF     (1 << 3)
#define ADXL362_ACT_INACT_CTL_INACT_EN      (1 << 2)
#define ADXL362_ACT_INACT_CTL_ACT_REF       (1 << 1)
#define ADXL362_ACT_INACT_CTL_ACT_EN        (1 << 0)

/* ADXL362_ACT_INACT_CTL_LINKLOOP(x) options */
#define ADXL362_MODE_DEFAULT        0
#define ADXL362_MODE_LINK           1
#define ADXL362_MODE_LOOP           3

/* ADXL362_REG_FIFO_CTL */
#define ADXL362_FIFO_CTL_AH                 (1 << 3)
#define ADXL362_FIFO_CTL_FIFO_TEMP          (1 << 2)
#define ADXL362_FIFO_CTL_FIFO_MODE(x)       (((x) & 0x3) << 0)

/* ADXL362_FIFO_CTL_FIFO_MODE(x) options */
#define ADXL362_FIFO_DISABLE              0
#define ADXL362_FIFO_OLDEST_SAVED         1
#define ADXL362_FIFO_STREAM               2
#define ADXL362_FIFO_TRIGGERED            3

typedef enum {
    FIFO_DISABLE                  =               0x00,
    FIFO_OLDEST_SAVED             =               0x01,
    FIFO_STREAM_MODE              =               0x02,
    FIFO_TRIGGER_MODE             =               0x03,
} FifoMode_t;

/* ADXL362_REG_INTMAP1 */
#define ADXL362_INTMAP1_INT_LOW             (1 << 7)
#define ADXL362_INTMAP1_AWAKE               (1 << 6)
#define ADXL362_INTMAP1_INACT               (1 << 5)
#define ADXL362_INTMAP1_ACT                 (1 << 4)
#define ADXL362_INTMAP1_FIFO_OVERRUN        (1 << 3)
#define ADXL362_INTMAP1_FIFO_WATERMARK      (1 << 2)
#define ADXL362_INTMAP1_FIFO_READY          (1 << 1)
#define ADXL362_INTMAP1_DATA_READY          (1 << 0)

/* ADXL362_REG_INTMAP2 definitions */
#define ADXL362_INTMAP2_INT_LOW             (1 << 7)
#define ADXL362_INTMAP2_AWAKE               (1 << 6)
#define ADXL362_INTMAP2_INACT               (1 << 5)
#define ADXL362_INTMAP2_ACT                 (1 << 4)
#define ADXL362_INTMAP2_FIFO_OVERRUN        (1 << 3)
#define ADXL362_INTMAP2_FIFO_WATERMARK      (1 << 2)
#define ADXL362_INTMAP2_FIFO_READY          (1 << 1)
#define ADXL362_INTMAP2_DATA_READY          (1 << 0)

/* ADXL362_REG_FILTER_CTL definitions */
#define ADXL362_FILTER_CTL_RANGE(x)         (((x) & 0x3) << 6)
#define ADXL362_FILTER_CTL_RES              (1 << 5)
#define ADXL362_FILTER_CTL_HALF_BW          (1 << 4)
#define ADXL362_FILTER_CTL_EXT_SAMPLE       (1 << 3)
#define ADXL362_FILTER_CTL_ODR(x)           (((x) & 0x7) << 0)

/* ADXL362_FILTER_CTL_HALF_BW options */
#define ADXL362_BW_FILTER_HALF          0
#define ADXL362_BW_FILTER_QUARTER       1

/* ADXL362_FILTER_CTL_RANGE(x) options */
#define ADXL362_RANGE_2G                0 /* +-2 g */
#define ADXL362_RANGE_4G                1 /* +-4 g */
#define ADXL362_RANGE_8G                2 /* +-8 g */

typedef enum {
    FULLSCALE_2                   =               0x00,
    FULLSCALE_4                   =               0x01,
    FULLSCALE_8                   =               0x02,
} Fullscale_t;

/* ADXL362_BIT_DEPTH_FILTER */
// this is different from shine to venus
//#define ADXL362_BIT_DEPTH_FILTER_8       0xFF00
//#define ADXL362_BIT_DEPTH_FILTER_10      0xFFC0
//#define ADXL362_BIT_DEPTH_FILTER_12      0xFFF0
#define ADXL362_BIT_DEPTH_FILTER_8       0xFFF0
#define ADXL362_BIT_DEPTH_FILTER_10      0xFFFC
#define ADXL362_BIT_DEPTH_FILTER_12      0xFFFF


/* ADXL362_FILTER_CTL_ODR(x) options */
#define ADXL362_ODR_12_5_HZ             0 /* 12.5 Hz */
#define ADXL362_ODR_25_HZ               1 /* 25 Hz */
#define ADXL362_ODR_50_HZ               2 /* 50 Hz */
#define ADXL362_ODR_100_HZ              3 /* 100 Hz */
#define ADXL362_ODR_200_HZ              4 /* 200 Hz */
#define ADXL362_ODR_400_HZ              5 /* 400 Hz */

/* ADXL362_FILTER_CTL_ODR(x) enum */
/* Added by josh@misfit */
typedef enum {
    ODR_12_5Hz  = 0x00,  /* 12.5 Hz */
    ODR_25Hz    = 0x01,  /* 25 Hz */
    ODR_50Hz    = 0x02,  /* 50 Hz */
    ODR_100Hz   = 0x03,  /* 100 Hz */
    ODR_200Hz   = 0x04,  /* 200 Hz */
    ODR_400Hz   = 0x05   /* 400 Hz */
} ODR_t;

/* ADXL362_REG_POWER_CTL definitions */
#define ADXL362_POWER_CTL_RES               (1 << 7)
#define ADXL362_POWER_CTL_EXT_CLK           (1 << 6)
#define ADXL362_POWER_CTL_LOW_NOISE(x)      (((x) & 0x3) << 4)
#define ADXL362_POWER_CTL_WAKEUP            (1 << 3)
#define ADXL362_POWER_CTL_AUTOSLEEP         (1 << 2)
#define ADXL362_POWER_CTL_MEASURE(x)        (((x) & 0x3) << 0)

/* ADXL362_POWER_CTL_LOW_NOISE(x) options */
#define ADXL362_NOISE_MODE_NORMAL           0
#define ADXL362_NOISE_MODE_LOW              1
#define ADXL362_NOISE_MODE_ULTRALOW         2

/* ADXL362_POWER_CTL_MEASURE(x) options */
#define ADXL362_MEASURE_STANDBY         0
#define ADXL362_MEASURE_ON              2

typedef enum {
    POWER_DOWN  =   0x00,
    // LOW_POWER   =   0x01,     // this is reserved don't use it
    NORMAL      =   0x01
} Power_mode_t;

/* ADXL362_REG_SELF_TEST */
#define ADXL362_SELF_TEST_ST            (1 << 0)

/* ADXL362 device information */
#define ADXL362_DEVICE_AD               0xAD
#define ADXL362_DEVICE_MST              0x1D
#define ADXL362_PART_ID                 0xF2

/* ADXL362 Reset settings */
#define ADXL362_RESET_KEY               0x52

/* ADXL362 axis designators */
#define ADXL362_X_AXIS_DES              0x00
#define ADXL362_Y_AXIS_DES              0x01
#define ADXL362_Z_AXIS_DES              0x02
#define ADXL362_TEMP_DES                0x03

//these could change accordingly with the architecture
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef int8_t i8_t;
typedef int16_t i16_t;

typedef u8_t IntPinConf_t;
typedef u8_t Axis_t;
typedef u8_t Int1Conf_t;

typedef enum {
    MEMS_SUCCESS    =   0x01,
    MEMS_ERROR      =   0x00
} statuss_t;

typedef struct {
    i16_t AXIS_X;
    i16_t AXIS_Y;
    i16_t AXIS_Z;
} AccAxesRaw_t;

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Writes data into a register. */
void ADXL362_SetRegisterValue(unsigned short registerValue,
                              unsigned char  registerAddress,
                              unsigned char  bytesNumber);

/*! Performs a burst read of a specified number of registers. */
void ADXL362_GetRegisterValue(unsigned char *pReadData,
                              unsigned char  registerAddress,
                              unsigned char  bytesNumber);

/*! Reads multiple bytes from the device's FIFO buffer. */
void ADXL362_GetFifoValue(unsigned char *pBuffer, unsigned short bytesNumber);

/*! Resets the device via SPI communication bus. */
void ADXL362_SoftwareReset(void);

/*! Sets or clears the AH bit in the FIFO control register. */
void ADXL362_SetFIFO_AH(unsigned char above_half);

/*! Places the device into standby/measure mode. */
void ADXL362_SetPowerMode(unsigned char pwrMode);

void ADXL362_SetFifoMode(unsigned char mode);

/*! Selects the filter bandwidth range. */
void ADXL362_SetFilterBW(unsigned char filter_bw);

/*! Selects the measurement range. */
void ADXL362_SetRange(unsigned char gRange);

/*! Selects the Output Data Rate of the device. */
void ADXL362_SetOutputRate(unsigned char outRate);

/*! Reads the 3-axis raw data from the accelerometer. */
void ADXL362_GetXyz(short *x, short *y, short *z);

/*! Reads the temperature of the device. */
float ADXL362_ReadTemperature(void);

/*! Configures the FIFO feature. */
void ADXL362_FifoSetup(unsigned char  mode,
                       unsigned short waterMarkLvl,
                       unsigned char  enTempRead);

/*! Configures activity detection. */
void ADXL362_SetupActivityDetection(unsigned char  refOrAbs,
                                    unsigned short threshold,
                                    unsigned char  time);

/*! Configures inactivity detection. */
void ADXL362_SetupInactivityDetection(unsigned char  refOrAbs,
                                      unsigned short threshold,
                                      unsigned short time);

#endif  /* __ADXL362_H__ */


#endif /* ACCEL_ADXL362_H_ */
