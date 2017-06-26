#ifndef ACCEL_SPI_FOR_ACCEL_H_
#define ACCEL_SPI_FOR_ACCEL_H_
#include "fsl_spi.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_dma.h"
#include "fsl_spi_dma.h"
#include <stdint.h>

extern void accel_spi_init(void);

extern void accel_power_off(void);
extern void accel_power_on(void);

extern uint8_t ADXL362_SPI_Read(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber);
extern uint8_t ADXL362_SPI_Write(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber);
extern void accel_SelfTest(void);
extern void accel_ChipInit(void);
extern void accel_spi_pins_disable(void);
extern void accel_dma_transfer(spi_transfer_t * inputTransferConfig);
extern void accel_nondma_transfer(spi_transfer_t * inputTransferConfig);
extern void accel_spi_deinit(void);
#endif /* ACCEL_SPI_FOR_ACCEL_H_ */
