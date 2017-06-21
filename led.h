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

#ifndef _LED_H_
#define _LED_H_

#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if ((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_DK))

#define LED_ON_MASK(mask) GPIO_SetPinsOutput(BOARD_LED_GPIO, mask)        /*!< Turn on a group of target LED */
#define LED_OFF_MASK(mask) GPIO_ClearPinsOutput(BOARD_LED_GPIO, mask)     /*!< Turn off a group of target LED */
#define LED_TOGGLE_MASK(mask) GPIO_TogglePinsOutput(BOARD_LED_GPIO, mask) /*!< Toggle a group of target LED */
#define LED_ON(pin) GPIO_SetPinsOutput(BOARD_LED_GPIO, 1U << pin)         /*!< Turn on target LEDx */
#define LED_OFF(pin) GPIO_ClearPinsOutput(BOARD_LED_GPIO, 1U << pin)      /*!< Turn off target LEDx */
#define LED_TOGGLE(pin) GPIO_TogglePinsOutput(BOARD_LED_GPIO, 1U << pin)  /*!< Toggle on target LEDx */

#elif((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_EVB))

#define LED_ON_MASK(mask) GPIO_ClearPinsOutput(BOARD_LED_GPIO, mask)      /*!< Turn on a group of target LED */
#define LED_OFF_MASK(mask) GPIO_SetPinsOutput(BOARD_LED_GPIO, mask)       /*!< Turn off a group of target LED */
#define LED_TOGGLE_MASK(mask) GPIO_TogglePinsOutput(BOARD_LED_GPIO, mask) /*!< Toggle a group of target LED */
#define LED_ON(pin) GPIO_ClearPinsOutput(BOARD_LED_GPIO, 1U << pin)       /*!< Turn on target LEDx */
#define LED_OFF(pin) GPIO_SetPinsOutput(BOARD_LED_GPIO, 1U << pin)        /*!< Turn off target LEDx */
#define LED_TOGGLE(pin) GPIO_TogglePinsOutput(BOARD_LED_GPIO, 1U << pin)  /*!< Toggle on target LEDx */

#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

/*! @brief Initialize the leds */
void LED_Init(void);

/*!
 * @brief To set the specified LEDs in blinking state.
 *
 * @param pin_mask         GPIO pin mask
 * @param[in] time_on      LED on time in a period: TA_MILLISECOND(n)
 * @param[in] time_off     LED off time in a period: TA_MILLISECOND(n)
 * @return void
 */
void LED_Blink(uint32_t pin_mask, uint32_t time_on, uint32_t time_off);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _LED_H_ */
