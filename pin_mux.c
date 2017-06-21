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

#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_iocon.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_InitPins(void)
{
    /* USART0 RX/TX pin */
    IOCON_PinMuxSet(IOCON, 0, 16, IOCON_FUNC4);
    IOCON_PinMuxSet(IOCON, 0, 17, IOCON_FUNC4);

    /* IO drive capability (0:low, 1:high) */
    SYSCON->PIO_DRV_CFG0 = 0;
    SYSCON->PIO_DRV_CFG1 = 0;

    /* IO pull control (00:High-Z, 01:Pull-down, 10:Pull-up, 11:Reserved) */
    SYSCON->PIO_PULL_CFG0 = 0xAAAAAAAA;
    SYSCON->PIO_PULL_CFG1 = 0xAAAA9A9A; /* PA22 pull-down for swd clk */ //PA 18 High-Z for Accel Interrupt
    SYSCON->PIO_PULL_CFG2 = 0x0000002A;
}

