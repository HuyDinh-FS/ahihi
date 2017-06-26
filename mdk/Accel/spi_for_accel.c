#include "spi_for_accel.h"
#include "fsl_spi.h"
#include "fsl_iocon.h"
#include "fsl_gpio.h"
#include "fsl_dma.h"
#include "fsl_spi_dma.h"
#include "power_manager.h"
#include "accel_spi.h"
#include "fsl_power.h"
#include <stdint.h>

#define POWER_PIN 15

#define DMA_FOR_ACCEL								DMA0
#define DMA_CHANNEL_FOR_ACCEL_RX		4
#define DMA_CHANNEL_FOR_ACCEL_TX		5


#if USE_DMA_SPI_ACCEL
dma_handle_t masterTxHandle;
dma_handle_t masterRxHandle;

spi_dma_handle_t masterHandle;
volatile uint8_t transfer_completed = 0;
#else
spi_master_handle_t masterHandle;
#endif

#if USE_DMA_SPI_ACCEL
void SPI_MasterUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
    }
    transfer_completed = 1;
    wtm_tx_callback(NULL, 69);
    PM_SetReq(kPmPowerDown1, kPmReqUserPolicy);
}

void accel_dma_transfer(spi_transfer_t * inputTransferConfig)
{
    transfer_completed = 0;
    PM_SetReq(kPmSleep, kPmReqUserPolicy);
    SPI_MasterTransferDMA(SPI0, &masterHandle, inputTransferConfig);
}
#else
void SPI_MasterUserCallbackNonDMA(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData)
{
	  if (status == kStatus_Success)
    {
    }
    wtm_tx_callback(NULL, 69);
    PM_SetReq(kPmPowerDown1, kPmReqUserPolicy);
}

void accel_nondma_transfer(spi_transfer_t * inputTransferConfig)
{
#if USE_NONBLOCKING_SPI_ACCEL
    PM_SetReq(kPmSleep, kPmReqUserPolicy);
	SPI_MasterTransferNonBlocking(SPI0, &masterHandle, inputTransferConfig);
#else
	SPI_MasterTransferBlocking(SPI0, inputTransferConfig);
	wtm_tx_callback(NULL, 69);
#endif
}
#endif

void accel_spi_deinit(void)
{
    SPI_Deinit(SPI0);
    GPIO_PinInit(GPIOA, 1, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)}); /* SPI0_SSEL2 */
    GPIO_PinInit(GPIOA, 29, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)}); /* SPI0_SCK */
    GPIO_PinInit(GPIOA, 4, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)}); /* SPI0_MOSI */
    GPIO_PinInit(GPIOA, 5, &(gpio_pin_config_t){kGPIO_DigitalOutput, (0U)}); /* SPI0_MISO */
    GPIO_WritePinOutput(GPIOA, 1, 0);
    GPIO_WritePinOutput(GPIOA, 29, 0);
    GPIO_WritePinOutput(GPIOA, 4, 0);
    GPIO_WritePinOutput(GPIOA, 5, 0);
    
}

void accel_spi_init(void)
{
    spi_master_config_t masterConfig = {0};
    uint32_t sourceClock = 0U;
    
    /* SPI0 pin */
    IOCON_PinMuxSet(IOCON, 0, 1, IOCON_FUNC5);  /* SPI0_SSEL2 */
    IOCON_PinMuxSet(IOCON, 0, 29, IOCON_FUNC4); /* SPI0_SCK */
    IOCON_PinMuxSet(IOCON, 0, 4, IOCON_FUNC5); /* SPI0_MOSI */
    IOCON_PinMuxSet(IOCON, 0, 5, IOCON_FUNC5); /* SPI0_MISO */
#if USE_DMA_SPI_ACCEL
    DMA_Init(DMA_FOR_ACCEL);
#endif
    /* Init SPI master */
    SPI_MasterGetDefaultConfig(&masterConfig);
    sourceClock = CLOCK_GetFreq(kCLOCK_BusClk);
    masterConfig.sselNum = (spi_ssel_t) 2;
    masterConfig.baudRate_Bps = 8000000U;
    masterConfig.dataWidth = kSPI_Data8Bits;
    SPI_MasterInit(SPI0, &masterConfig, sourceClock);
#if USE_DMA_SPI_ACCEL
    DMA_EnableChannel(DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_RX);
    DMA_EnableChannel(DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_TX);
    DMA_SetChannelPriority(DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_TX, kDMA_ChannelPriority1);
    DMA_SetChannelPriority(DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_RX, kDMA_ChannelPriority0);
    DMA_CreateHandle(&masterTxHandle, DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_TX);
    DMA_CreateHandle(&masterRxHandle, DMA_FOR_ACCEL, DMA_CHANNEL_FOR_ACCEL_RX);
    
    SPI_MasterTransferCreateHandleDMA(SPI0, &masterHandle, SPI_MasterUserCallback, NULL, &masterTxHandle,
                                      &masterRxHandle);
    NVIC_EnableIRQ(DMA_IRQn);
#else
#if USE_NONBLOCKING_SPI_ACCEL
		SPI_MasterTransferCreateHandle(SPI0, &masterHandle, SPI_MasterUserCallbackNonDMA, NULL);
#endif
#endif
}

uint8_t ADXL362_SPI_Read(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber)
{
    volatile uint8_t value;
    spi_transfer_t transferConfig = {0};
				
    if (bytesNumber < 2)
    {
        return 0;
    }
    transferConfig.txData = data;
    transferConfig.rxData = data;
    transferConfig.configFlags |= kSPI_FrameAssert;
    transferConfig.dataSize = bytesNumber;
    
    SPI_MasterTransferBlocking(SPI0, &transferConfig);
    return bytesNumber;
}

uint8_t ADXL362_SPI_Write(uint8_t slaveDeviceId, uint8_t* data, uint8_t bytesNumber)
{
    return ADXL362_SPI_Read(slaveDeviceId, data, bytesNumber);
}

void accel_power_off(void)
{
    GPIO_PinInit(GPIOA, POWER_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, 0U});
}
void accel_power_on(void)
{
    GPIO_PinInit(GPIOA, POWER_PIN, &(gpio_pin_config_t){kGPIO_DigitalOutput, 1U});
}
