/*
 * accel_spi.c
 *
 *  Created on: Sep 6, 2016
 *      Author: huydinh
 */
#include "accel_spi.h"
#include "spi_for_accel.h"
#include "ADXL362.h"
#include "adxl362_spi.h"
#include "algtypes.h"
#include "string.h"
#include "stdio.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"

#define SIZE_OF_ACCEL_BUFFER    (ACCEL_NUM_SAMPLES_PER_WTM * ACCEL_NUM_AXIS * sizeof(accel_value_t) + 1)
#define USE_FAKE_DATA   0

static uint8_t accel_tmp_buf[SIZE_OF_ACCEL_BUFFER];


#if USE_DMA_SPI_ACCEL
static uint32_t accel_tmp32_buf[SIZE_OF_ACCEL_BUFFER];
static uint32_t cmd[SIZE_OF_ACCEL_BUFFER];
#else
static uint8_t cmd[SIZE_OF_ACCEL_BUFFER];
#endif

static accel_value_t accel_sample_buf[ACCEL_NUM_SAMPLES_PER_WTM * ACCEL_NUM_AXIS];

volatile static uint8_t keeping_this_sample = 0;
volatile static uint16_t sample_count = 0;
volatile static uint8_t current_minute = 0;
volatile static uint8_t last_minute = 0;

static void accelPowerInit(void);
void wtm_tx_callback(void *user_data, uint16_t transferred);
static int16_t getAccelSample(int16_t raw_accel_data);

void accelInit(void)
{
    accel_spi_init();
    accelPowerInit();
    accel_SelfTest();
    //accel_ChipInit();
}

static void accelPowerInit(void)
{
    //accel_power_off();
    //accel_power_on();
}

void accel_wtm_int_handler()
{
    volatile uint16_t fifo_sample;
    volatile uint8_t i = 0;
    spi_transfer_t transferConfig = {0};
    
    NVIC_DisableIRQ(EXT_GPIO_WAKEUP_IRQn);
    NVIC_ClearPendingIRQ(EXT_GPIO_WAKEUP_IRQn);
    
#if USE_DMA_SPI_ACCEL
    //memset(cmd,0xAA, SIZE_OF_ACCEL_BUFFER*sizeof(uint32_t));
    //for (i = 0; i < SIZE_OF_ACCEL_BUFFER; i++)
    //{
    //    cmd[i] = 0x000000AA;
    //}
#else
    //memset(cmd,0xAA, SIZE_OF_ACCEL_BUFFER);
#endif
    cmd[0] = ADXL362_WRITE_FIFO;
    
    sample_count++;
    if (sample_count >= 480)
    {
        current_minute++;
        sample_count = 0;
    }
    i++;
    
    transferConfig.txData = (uint8_t *) cmd;
    transferConfig.configFlags |= kSPI_FrameAssert;
#if USE_DMA_SPI_ACCEL
    transferConfig.rxData = (uint8_t *) accel_tmp32_buf;
    transferConfig.dataSize = SIZE_OF_ACCEL_BUFFER*sizeof(uint32_t);
    accel_dma_transfer(&transferConfig);
#else
    transferConfig.rxData = (uint8_t *) accel_tmp_buf;
    transferConfig.dataSize = SIZE_OF_ACCEL_BUFFER;
	accel_nondma_transfer(&transferConfig);

#endif
}

void wtm_tx_callback(void *user_data, uint16_t transferred)
{
    NVIC_ClearPendingIRQ(EXT_GPIO_WAKEUP_IRQn);
    NVIC_EnableIRQ(EXT_GPIO_WAKEUP_IRQn);
}

int16_t getAccelSample(int16_t raw_accel_data)
{
    //raw_accel_data &= ADXL362_BIT_RESOLUTION;                                           // set bit resolution
    raw_accel_data &= 0x3FFF;                                  // drop axis bits
    raw_accel_data |= ((raw_accel_data << 2) & 0xC000);    // sign extend sample
    
    return raw_accel_data;
}
