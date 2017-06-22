/*
 * Copyright (c) 2016, NXP Semiconductors, N.V.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "app_config.h"
#include "app_ble.h"
#include "led.h"
#include "button.h"
#include "power_manager.h"
#include "accel_spi.h"

#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_calibration.h"
#include "fsl_power.h"
#include "fsl_rf.h"
#include "fsl_clock.h"
#include "fsl_usart.h"
#include "uart_adapter.h"
#include "adxl362_accel.h"
#include "TapHandler.h"
#include "AlgExec.h"
#include "StepCount.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define HIGH_ALERT_PERIOD (TA_MILLISECOND(200)) /* 200ms */
#define MILD_ALERT_PERIOD (TA_MILLISECOND(500)) /* 500ms */

// Custom config HDINH
#define ACCEL_INT_PIN           (18)
#define USE_UART_DEBUG          (1)
#define ADV_INV_IN_SEC          (2)
#define ADV_INV_INPUT           (ADV_INV_IN_SEC*1600)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t s_usrCharCode;
uint8_t s_usrAlertLvl;
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_WakeupRestore(void)
{
    BUTTON_EnableInterrupt();
#if USE_UART_DEBUG
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.stopBitCount = kUART_OneStopBit;
     * config.loopback = false;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    usart_config_t config;
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = DEMO_USART_BAUDRATE;
    config.enableTx = true;
    config.enableRx = true;
    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
#endif
    accel_spi_init();
}


/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t primask = DisableGlobalIRQ();

    POWER_EnableDCDC(true);
    RF_ConfigRxMode(SYSCON, kRxModeHighPerformance);

    CALIB_SystemCalib();

    BOARD_InitPins();
    BOARD_BootClockRUN();
#if USE_UART_DEBUG
    BOARD_InitDebugConsole();
#endif

    /* Shut down higher 64K RAM for lower power consumption */
    POWER_EnablePD(kPDRUNCFG_PD_MEM9);
    POWER_EnablePD(kPDRUNCFG_PD_MEM8);
    POWER_EnablePD(kPDRUNCFG_PD_MEM7);
    POWER_EnablePD(kPDRUNCFG_PD_MEM6);

    PM_Init();

    PM_SetReq(kPmPowerDown1, kPmReqUserPolicy);

    /* Enable GPIO clock */
    CLOCK_EnableClock(kCLOCK_Gpio);
    /* Use Low Frequency Clock source as wdt's clock source */
    CLOCK_AttachClk(k32K_to_WDT_Clk);
    /* Config BLE's high frequency clock to 8MHz */
    CLOCK_AttachClk(k8M_to_BLE_Clk);

    TM_Init();
    LED_Init();

    BUTTON_Init();
    BUTTON_SetGpioWakeupLevel(BOARD_SW_GPIO, BOARD_SW1_GPIO_PIN, 1U);
    adxl362_accel_config_and_init();
    TapHandler_Init();
    SC_Init();
    Alg_Init();
    accelInit();

    BLE_Init();

    APP_Init();

    EnableGlobalIRQ(primask);;
    while (1)
    {
        KE_Schedule();

        /* TODO: Feed watchdog */

        PM_PowerManagement();
    }
}

__STATIC_INLINE void APP_ProxrHighAlert(void)
{
    LED_Blink(BOARD_LED_GREEN_GPIO_PIN_MASK, HIGH_ALERT_PERIOD, HIGH_ALERT_PERIOD);
}

__STATIC_INLINE void APP_ProxrMildAlert(void)
{
    LED_Blink(BOARD_LED_GREEN_GPIO_PIN_MASK, MILD_ALERT_PERIOD, MILD_ALERT_PERIOD);
}

__STATIC_INLINE void APP_ProxrStopAlert(void)
{
    LED_Blink(BOARD_LED_GREEN_GPIO_PIN_MASK, 0, 0);
}

void APP_GapSetDevCfgCallback(void)
{
    /* set device configuration */
    APP_GapmSetDevConfigCmd(CFG_GAP_ROLE, GAPM_CFG_ADDR_PUBLIC, NULL, &g_AppEnv.irk);
}

void APP_BleReadyCallback(void)
{   
#if defined(CFG_APP_AUTO)
    APP_GapmStartAdvertiseCmd(GAPM_ADV_UNDIRECT, GAPM_STATIC_ADDR, GAP_GEN_DISCOVERABLE, ADV_INV_INPUT,
                                 ADV_INV_INPUT, g_AppEnv.adv_data_len, g_AppEnv.adv_data, g_AppEnv.scan_rsp_data_len,
                                 g_AppEnv.scan_rsp_data, NULL);
#endif
}

void APP_AdvertisingStopCallback(uint8_t status)
{
    if (status == GAP_ERR_NO_ERROR)
    {
        QPRINTF("Device connected by peer device.\r\n");
    }
    else if (status == GAP_ERR_CANCELED)
    {
        QPRINTF("Advertising is cancelled.\r\n");
    }
    else if (status == GAP_ERR_TIMEOUT)
    {
        QPRINTF("Advertising is complete.\r\n");
    }
    else
    {
        QPRINTF("Advertising is failed(0x%02x).\r\n", status);
    }
}

void APP_ConnectionEstablishedCallback(struct gapc_connection_req_ind const *param)
{
    QPRINTF("Connected with %02x%02x%02x%02x%02x%02x.\r\n", param->peer_addr.addr[5], param->peer_addr.addr[4],
            param->peer_addr.addr[3], param->peer_addr.addr[2], param->peer_addr.addr[1], param->peer_addr.addr[0]);

    /* Stop link lost alert when the connection is re-established */
    if (s_usrAlertLvl != PROXR_ALERT_NONE)
    {
        APP_ProxrStopAlert();
    }
}

void APP_ConnectionTerminatedCallback(struct gapc_disconnect_ind const *param)
{
    if (g_AppEnv.state != APP_OP_ADVERTISING)
    {
        APP_GapmStartAdvertiseCmd(GAPM_ADV_UNDIRECT, GAPM_STATIC_ADDR, GAP_GEN_DISCOVERABLE, CFG_ADV_INT_MIN,
                                     CFG_ADV_INT_MAX, g_AppEnv.adv_data_len, g_AppEnv.adv_data, g_AppEnv.scan_rsp_data_len,
                                     g_AppEnv.scan_rsp_data, NULL);
    }

    /* Stop immediate alert when the connection is lost */
    if (s_usrAlertLvl != PROXR_ALERT_NONE)
    {
        APP_ProxrStopAlert();
    }
}

void APP_TkExchangeCallback(uint16_t conhdl, uint8_t tk_type, uint32_t pin_code)
{
    /* TK shall be displayed by local device */
    if (tk_type == GAP_TK_DISPLAY)
    {
        QPRINTF("TK:%d\r\n", pin_code);
    }
    /* TK shall be entered by user */
    else if (tk_type == GAP_TK_KEY_ENTRY)
    {
        /* add by user */
    }
}

void APP_ButtonDownCallback(uint32_t pin_mask)
{
    if (pin_mask & BOARD_SW1_GPIO_PIN_MASK)
    {
        while (GPIO_ReadPinInput(GPIOA, ACCEL_INT_PIN))
        {
            adxl362_WtmIsr();
        }
    }
}

void APP_ProxrAlertCallback(uint8_t char_code, uint8_t alert_lvl)
{
    s_usrCharCode = char_code;
    s_usrAlertLvl = alert_lvl;

    if (s_usrAlertLvl == PROXR_ALERT_NONE)
    {
        APP_ProxrStopAlert();
    }
    else if (s_usrAlertLvl == PROXR_ALERT_MILD)
    {
        APP_ProxrMildAlert();
    }
    else if (s_usrAlertLvl == PROXR_ALERT_HIGH)
    {
        APP_ProxrHighAlert();
    }
}
