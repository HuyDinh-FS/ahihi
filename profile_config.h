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
#ifndef _PROFILE_CONFIG_H_
#define _PROFILE_CONFIG_H_

/*! @brief Enable the profiles */
/* 
 * Supported Profiles include:
 *
 * CFG_PRF_ANPC / CFG_PRF_ANPS
 * CFG_PRF_BASC / CFG_PRF_BASS
 * CFG_PRF_BLPC / CFG_PRF_BLPS
 * CFG_PRF_CPPC / CFG_PRF_CPPS
 * CFG_PRF_CSCPC / CFG_PRF_CSCPS
 * CFG_PRF_DISC / CFG_PRF_DISS
 * CFG_PRF_FMPL / CFG_PRF_FMPT
 * CFG_PRF_GLPC / CFG_PRF_GLPS
 * CFG_PRF_HOGPD / CFG_PRF_HOGPBH / CFG_PRF_HOGPRH
 * CFG_PRF_HRPC / CFG_PRF_HRPS
 * CFG_PRF_HTPC / CFG_PRF_HTPT
 * CFG_PRF_LANC / CFG_PRF_LANS
 * CFG_PRF_PASPC / CFG_PRF_PASPS
 * CFG_PRF_PXPM / CFG_PRF_PXPR 
 * CFG_PRF_QPPC / CFG_PRF_QPPS 
 * CFG_PRF_RSCPC / CFG_PRF_RSCPS
 * CFG_PRF_SCPPC / CFG_PRF_SCPPS
 * CFG_PRF_TIPC / CFG_PRF_TIPS
 */

#define CFG_PRF_BASS
#define CFG_PRF_PXPR

/*
 * BASS Configuration
 ****************************************************************************************
 */

/*! @brief Maximal number of BAS that can be added in the DB */
#define CFG_BASS_NB_BAS_INS_MAX                 (2)
/*! @brief Duration before recheck the battery level */
#define CFG_BASS_BATT_LVL_CHK_DUR               (500)

/*
 * DISS Configuration
 ****************************************************************************************
 */

/*! @brief DISS supported characteristics @ref enum diss_features */
#define CFG_DIS_FEAT_SUP                        (DIS_MANUFACTURER_NAME_CHAR_SUP \
                                                |DIS_MODEL_NB_STR_CHAR_SUP \
                                                |DIS_FIRM_REV_STR_CHAR_SUP \
                                                |DIS_SW_REV_STR_CHAR_SUP)

/*! @brief Manufacturer name string */
#define CFG_DIS_MANU_NAME_VAL                   "NXP"
/*! @brief Model number string */
#define CFG_DIS_MODEL_NB_VAL                    "QN908x"
/*! @brief Serial number string */
#define CFG_DIS_SERIAL_NB_VAL                   "2.0"
/*! @brief Hardware revision string */
#define CFG_DIS_HW_REV_VAL                      "QN9080A"
/*! @brief Firmware revision string */
#define CFG_DIS_FW_REV_VAL                      "0.2.2"
/*! @brief Software revision string */
#define CFG_DIS_SW_REV_VAL                      "8.3.8"
/*! @brief System ID */
#define CFG_DIS_SYS_ID_VAL                      "\x00\x00\x00\x00\x00\xBE\x7C\x08"
/*! @brief IEEE Certification Data List */
#define CFG_DIS_IEEE_CERTIF_VAL                 "\x01\x02\x00\x00\x00\x00"
/*! @brief PnP ID (NXP Semiconductors 0x0025(former Philips) or 0x008E(former Quintic Corp)) */
#define CFG_DIS_PNP_ID_VAL                      "\x01\x25\x00\x00\x00\x00\x00"

/*
 * PROXR Configuration
 ****************************************************************************************
 */

/*! @brief Proximity reporter supported feature @ref enum proxr_feature */
#define CFG_PROXR_FEATURE                       PROXR_IAS_TXPS_SUP

/*
 * QPPS Configuration
 ****************************************************************************************
 */

/*! @brief Enable QPPS throuthput statistics */
#define CFG_QPP_SHOW_THROUGHPUT

#endif

