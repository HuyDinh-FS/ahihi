/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
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
#ifndef _CLOCK_CONFIG_H_
#define _CLOCK_CONFIG_H_

#include "system_QN908X.h"
#include "board.h"

/*******************************************************************************
 * DEFINITION
 ******************************************************************************/

/* Configure high frequency XTAL according to board design: 16MHz or 32MHz */
#if ((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_DK))
#define BOARD_XTAL0_CLK_HZ 32000000U
#elif((defined(CFG_BOARD_TYPE)) && (CFG_BOARD_TYPE == BOARD_QN908X_EVB))
#define BOARD_XTAL0_CLK_HZ 16000000U
#endif

/* Configure low frequency XTAL according to board design: 32768U or 0U (no 32K crystal on board) */
#define BOARD_XTAL1_CLK_HZ 32768U

/* Select RCO 32K if no 32K crystal on board */
#if (BOARD_XTAL1_CLK_HZ != 0)
#define CLK_32K_CFG CLK_XTAL_32KHZ
#else
#define CLK_32K_CFG CLK_RCO_32KHZ
#endif

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* AHB clock = 8MHz */
void BOARD_BootClockLSRUN(void);
/* AHB clock = 16MHz */
void BOARD_BootClockRUN(void);
/* AHB clock = crystal */
void BOARD_BootClockHSRUN(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _CLOCK_CONFIG_H_ */
