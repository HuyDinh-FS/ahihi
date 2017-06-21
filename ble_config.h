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
#ifndef _BLE_CONFIG_H_
#define _BLE_CONFIG_H_

/*! @brief GAP role: CFG_CENTRAL, CFG_PERIPHERAL, CFG_OBSERVER, CFG_BROADCASTER, CFG_ALLROLES */
#define CFG_PERIPHERAL

/*! @brief Maximum connected devices number (Link number). It's up to 16. 
    Select the appropriate number for your application and get optimized memory usage. */
#define CFG_CON_DEV_MAX (2)

/*! @brief Maximum scanned devices number */
#define CFG_SCAN_DEV_MAX (20)

/*! @brief Maximum bonded devices number */
#define CFG_BOND_DEV_MAX (10)

/*! @brief Number of devices in the white list */
#define CFG_BLE_WHITELIST_MAX (CFG_CON_DEV_MAX + 2)

/*! @brief Number of devices in the Resolution Address List */
#define CFG_BLE_RESOL_ADDR_LIST_MAX (3)

/*! @brief Number of devices capacity for the scan filtering */
#define CFG_BLE_DUPLICATE_FILTER_MAX (10)

/*! @brief TX buffer size */
#define CFG_REG_BLE_EM_TX_BUFFER_DATA_SIZE          (260)

/*! @brief RX buffer size */
#define CFG_REG_BLE_EM_RX_BUFFER_SIZE (260)

/*! @brief Clock correction delay */
#define CFG_EA_CLOCK_CORR_LAT (0)
/*! @brief EA programming latency for only 1 activity */
#define CFG_BLE_PROG_LATENCY_DFT (2)

/*! @brief Device Appearance */
#define CFG_DEV_APPEARANCE (0x0000)

/*! @brief Duration before regenerate device address, unit(second) */
#define CFG_ADDR_RENEW_DUR (150)

/*! @brief Maximum device MTU size (23 ~ ) */
#define CFG_MAX_MTU (2048)
/*! @brief Maximum device MPS size (23 ~ QN_MAX_MTU) */
#define CFG_MAX_MPS (23)

/*! @brief Suggested value for the Controller's maximum transmitted number of payload octets to be used (27 ~ 251) */
#define CFG_SUGG_MAX_TX_SIZE (251)
/*! @brief Suggested value for the Controller's maximum packet transmission time to be used */
#define CFG_SUGG_MAX_TX_TIME (CFG_SUGG_MAX_TX_SIZE * 8 + 112)

/*! @brief Advertising channel map */
#define CFG_ADV_CHMAP (ADV_ALL_CHNLS_EN)
/*! @brief Advertising minimum interval */
#define CFG_ADV_INT_MIN (0x1E0)
/*! @brief Advertising maximum interval */
#define CFG_ADV_INT_MAX (0x1E0)
/*! @brief Non-connectable advertising minimum interval */
#define CFG_ADV_NONCONN_INT_MIN (0xA0)
/*! @brief Non-connectable advertising maximum interval */
#define CFG_ADV_NONCONN_INT_MAX (0xA0)

/*! @brief Advertising data excludes Flags and Local Name
    Flags will be added by GAP layer and Local Name will be tried to add in APP_ConstructAdvData() */
#define CFG_ADV_DATA                                ""
#define CFG_ADV_DATA_LEN                            (0)

/*! @brief Scan response data */
/*                                                    Length
 *                                                        Complete list of 16-bit Service UUIDs
 *                                                            GAP service
 *                                                                    GATT service
 *                                                                            Battery service
 *                                                                                    Link loss service
 *                                                                                            Immediate alert service
 *                                                                                                    Tx power service
 */
#define CFG_SCAN_RSP_DATA                           "\x0D\x03\x00\x18\x01\x18\x0F\x18\x03\x18\x02\x18\x04\x18"
#define CFG_SCAN_RSP_DATA_LEN                       (14)


/*! @brief Scan interval, Value Time = N * 0.625 ms */
#define CFG_SCAN_INTV (GAP_INQ_SCAN_INTV)
/*! @brief Scan window, Value Time = N * 0.625 ms */
#define CFG_SCAN_WIND (GAP_INQ_SCAN_WIND)

/*! @brief Minimum connection interval, Value Time = N * 1.25 ms */
#define CFG_CONN_MIN_INTV (0x00F0) // 300ms interval
/*! @brief Maximum connection interval, Value Time = N * 1.25 ms */
#define CFG_CONN_MAX_INTV (0x00F0) // 300ms interval
/*! @brief Connection latency */
#define CFG_CONN_LATENCY (GAP_CONN_LATENCY)
/*! @brief Connection supervision timeout */
#define CFG_CONN_SUPERV_TIMEOUT (GAP_CONN_SUPERV_TIMEOUT)

/*! @brief Scan interval in any discovery or connection establishment, Value Time = N * 0.625 ms */
#define CFG_SCAN_FAST_INTV (GAP_SCAN_FAST_INTV)
/*! @brief Scan window in any discovery or connection establishment, Value Time = N * 0.625 ms */
#define CFG_SCAN_FAST_WIND (GAP_SCAN_FAST_WIND)

/*! @brief Minimum connection event */
#define CFG_CONN_MIN_CE (GAP_CONN_MIN_CE)
/*! @brief Maximum connection event */
#define CFG_CONN_MAX_CE (GAP_CONN_MAX_CE)

/*! @brief Device security requirements (minimum security level)
    The pairing procedure will use this parameter to check whether key distribution and security mode match the reqirement */
#define CFG_SEC_MODE_LEVEL (GAP_SEC1_NOAUTH_PAIR_ENC)

/*! @brief IO capability for pairing */
#define CFG_IO_CAPABILITY (GAP_IO_CAP_DISPLAY_YES_NO)

/*! @brief Authentication requirements for pairing */
#define CFG_AUTH_REQ                                (GAP_AUTH_REQ_NO_MITM_BOND)

#endif
