/************************************************************************************************************
Module:       adxl362_accel.h

Revision:     1.0

Description:  ---

Notes:        ---

History:
Date          Name      Changes
-----------   ----      -------------------------------------------------------------------------------------
07/31/2015    JOA       Began Coding    (JOA = John Alabi)

COPYRIGHT (c) 2015 Misfit.  ALL RIGHTS RESERVED.

THIS SOURCE IS MISFIT PROPRIETARY AND CONFIDENTIAL!  NO PART OF THIS SOURCE MAY BE DISCLOSED IN ANY MANNTER
TO A THIRD PARTY WITHOUT PRIOR WRITTEN CONSENT OF MISFIT WEARABLES CORPORATION INC.
************************************************************************************************************/

#ifndef _ADXL362_ACCEL_H
#define _ADXL362_ACCEL_H

//#ifdef __cplusplus
//extern "C" {
//#endif

//###########################################################################################################
// #INCLUDES
//###########################################################################################################
#include "stdint.h"
#include "stdbool.h"

//###########################################################################################################
// DEFINED CONSTANTS
//###########################################################################################################
#define SPI_READ_THRESHOLD    42 //60 // Changed to 42 in order to prevent the IOM fifo from 
                                 // filling before servicing the spi-trxn. This allows for
                                 // a continuous read of the accelerometer samples. 
#define ACCEL_IOM_NUM         1
#define ACCEL_NUM_OF_AXES     3
#define BYTES_PER_SAMPLE      2

//###########################################################################################################
// DEFINED TYPES
//###########################################################################################################
typedef enum
{
  adxl362_fifty_hz       = 2,
  adxl362_one_hundred_hz = 3,
  adxl362_two_hundred_hz = 4,
  adxl362_four_hundred_hz = 5
} adxl362_rate_enum_t;

typedef enum
{
  adxl362_two_Gs_range = 0,
  adxl362_four_Gs_range = 1,
  adxl362_eight_Gs_range = 2
} adxl362_range_enum_t;

typedef struct
{
  int16_t x_val;
  int16_t y_val;
  int16_t z_val;
} adxl362_sample_set_t;

//###########################################################################################################
// DEFINED MACROS
//###########################################################################################################
#define ADXL362_FIFO_ENTRIES_L_REG    0x0C
#define FIFO_ENTRIES_MASK             0x3FF
#define ADXL362_PARTID_REG            0x02
#define ADXL362_PARTID_VAL            0xF2
#define BYTES_IN_IOM_FIFO             64
#define ADXL362_AXIS_MASK             0xC000
#define ADXL362_STATUS_REG            0x0B
#define ADXL362_DATA_READY_MASK       0x01
#define ADXL362_XDATA_L_REG           0x0E
#define ADXL362_BYTES_PER_SAMPLE_SET  6
//###########################################################################################################
// FUNCTION PROTOTYPES
//###########################################################################################################
void adxl362_accel_config_and_init (void);
void adxl362_accel_enable_interrupts (void);
void adxl362_accel_disable_interrupts (void);
void adxl362_accel_config (bool b_sync_mode, uint16_t fifo_watermark, \
                 adxl362_rate_enum_t sample_rate, adxl362_range_enum_t range, \
                 bool b_set_half_bandwidth);
void adxl362_accel_power_on (void);
void adxl362_accel_power_off (void);
void adxl362_accel_power_init (void);
bool adxl362_accel_is_IOM_idle (void);
void adxl362_accel_spi_init (void);
uint8_t adxl362_self_test_combo_BT (void);
bool adxl362_accel_get_orientation_data_ready (void);
void adxl362_accel_get_sample_set (adxl362_sample_set_t *sample_set, \
  uint8_t sample_set_index);
void adxl362_accel_get_sample_set_reference (const adxl362_sample_set_t **pp_sample_set, \
  uint8_t sample_set_index);
void adxl362_accel_vibe_calibration_init (void);
bool adxl362_GetAxes (int16_t samples[]);
void adxl362_clear_flags (void);
void adxl362_accel_alg_data_handler (void * p_event_data, uint16_t event_size);
void adxl362_WtmIsr( void );
void adxl362_accel_fifo_receive_handler(void);
//#ifdef __cplusplus
//}
//#endif

//###########################################################################################################
// END OF accel_hal.h
//###########################################################################################################
#endif /* _ADXL362_ACCEL_H */
