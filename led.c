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

#include "led.h"
#include "timer_manager.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief LED blinking configuration structure */
typedef struct _led_blink_config
{
    uint32_t ledOnTime;
    uint32_t ledOffTime;
} led_blink_config_t;

static led_blink_config_t s_ledBlinkConfig[BOARD_LED_NUM];

static tm_timer_id_t s_ledTimerID[BOARD_LED_NUM];

#if ((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_DK))
static const uint32_t s_ledGpioPin[BOARD_LED_NUM] = {BOARD_LED_RED_GPIO_PIN, BOARD_LED_GREEN_GPIO_PIN,
                                                     BOARD_LED_BLUE_GPIO_PIN};
static const uint32_t s_ledGpioPinMask[BOARD_LED_NUM] = {BOARD_LED_RED_GPIO_PIN_MASK, BOARD_LED_GREEN_GPIO_PIN_MASK,
                                                         BOARD_LED_BLUE_GPIO_PIN_MASK};
#elif((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_EVB))
static const uint32_t s_ledGpioPin[BOARD_LED_NUM] = {BOARD_LED1_GPIO_PIN, BOARD_LED2_GPIO_PIN, BOARD_LED3_GPIO_PIN,
                                                     BOARD_LED4_GPIO_PIN};
static const uint32_t s_ledGpioPinMask[BOARD_LED_NUM] = {BOARD_LED1_GPIO_PIN_MASK, BOARD_LED2_GPIO_PIN_MASK,
                                                         BOARD_LED3_GPIO_PIN_MASK, BOARD_LED4_GPIO_PIN_MASK};
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void LED_Init(void)
{
    for (int i = 0; i < BOARD_LED_NUM; i++)
    {
        GPIO_PinInit(BOARD_LED_GPIO, s_ledGpioPin[i], &(gpio_pin_config_t){kGPIO_DigitalOutput, LOGIC_LED_OFF});
        s_ledTimerID[i] = TM_AllocateTimer();
    }
}

/*!
 * @brief Handles the LED blink timer.
 */
static void LED_BlinkTimerHandler(uint32_t ledIdx)
{
    uint32_t dur;
    if (LOGIC_LED_ON == !(BOARD_LED_GPIO->DATAOUT & s_ledGpioPinMask[ledIdx]))
    {
        dur = s_ledBlinkConfig[ledIdx].ledOnTime;
        if (dur)
        {
            LED_ON_MASK(s_ledGpioPinMask[ledIdx]);
        }
    }
    else
    {
        dur = s_ledBlinkConfig[ledIdx].ledOffTime;
        if (dur)
        {
            LED_OFF_MASK(s_ledGpioPinMask[ledIdx]);
        }
    }
    if (dur)
    {
        TM_UpdateInterval(s_ledTimerID[ledIdx], dur);
    }
    else
    {
        TM_ClearTimer(s_ledTimerID[ledIdx]);
    }
}

void LED_Blink(uint32_t pin_mask, uint32_t time_on, uint32_t time_off)
{
    uint32_t ledIdx = BOARD_LED_NUM;
    for (int i = 0; i < BOARD_LED_NUM; i++)
    {
        if (pin_mask == s_ledGpioPinMask[i])
        {
            ledIdx = i;
            break;
        }
    }

    assert(ledIdx < BOARD_LED_NUM);
    s_ledBlinkConfig[ledIdx].ledOnTime = time_on;
    s_ledBlinkConfig[ledIdx].ledOffTime = time_off;

    if (time_on == 0U) // first priority
    {
        LED_OFF_MASK(pin_mask);
    }
    else if (time_off == 0U)
    {
        LED_ON_MASK(pin_mask);
    }
    else
    {
        TM_SetTimer(s_ledTimerID[ledIdx], kTM_IntervalTimer, time_off, (tm_callback_t)LED_BlinkTimerHandler,
                    (void *)(ledIdx));
        LED_OFF_MASK(pin_mask);
    }
}
