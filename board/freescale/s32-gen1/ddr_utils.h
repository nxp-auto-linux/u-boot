/*
 * Copyright 2020 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DDR_UTILS_H_
#define DDR_UTILS_H_

#include <asm/io.h>

/* Possible errors */
#define NO_ERR              0x00000000
#define UNSUPPORTED_ERR     0x00000001
#define TIMEOUT_ERR         0x00000002
#define TRAINING_FAILED     0x00000003

/* DDRC related */
#define    DDRC_BASE_ADDR                   0x403C0000
#define    OFFSET_DDRC_SWCTL                0x320
#define    OFFSET_DDRC_DFIMISC              0x1b0
#define    OFFSET_DDRC_DFISTAT              0x1bc
#define    OFFSET_DDRC_PWRCTL               0x30
#define    OFFSET_DDRC_SWSTAT               0x324
#define    OFFSET_DDRC_STAT                 0x04
#define    OFFSET_DDRC_DBG1                 0x304

/* uMCTL2 Multi-Port Registers */
#define	   DDRC_UMCTL2_MP_BASE_ADDR         0x403C03F8
#define    OFFSET_DDRC_PCTRL_0              0x98
#define    OFFSET_DDRC_PCTRL_1              0x148
#define    OFFSET_DDRC_PCTRL_2              0x1f8

/* PHY related */
#define    DDR_PHYA_MASTER0_CALBUSY                 0x4038165C
#define    DDR_PHYA_APBONLY_UCTSHSADOWREGS          0x40380404
#define    DDR_PHYA_MASTER_PUBMODE                  0x403815d0
#define    UCT_WRITE_PROT_SHADOW_MASK               0x1
#define    DDR_PHYA_APBONLY_DCTWRITEPROT            0x4038040C
#define    DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW      0x40380410
#define    OFFSET_DDRC_RFSHCTL3                     0x60

/* DDR Subsystem */
#define    DDR_SS_REG                    0x403D0000

/* Reset Generation Module */
#define    MC_RGM_PRST_0                 0x40078040

/* Default timeout for DDR PHY operations */
#define DEFAULT_TIMEOUT 1000000

/**
 * @brief Set default AXI parity.
 */
uint32_t set_axi_parity(void);
/**
 * @brief Post PHY train setup - complementary settings
 * that needs to be performed after running the firmware.
 */
uint32_t post_train_setup(void);

uint32_t wait_firmware_execution(void);

uint32_t load_register_array(uint32_t reg_writes,
			     uint32_t array[reg_writes][2]);

#endif /* DDR_UTILS_H_ */
