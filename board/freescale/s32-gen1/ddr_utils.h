/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020-2021 NXP
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

#ifndef CONFIG_S32_GEN1
#include "io.h"
#else
#include <asm/io.h>
#endif

#include <stdbool.h>

/* Possible errors */
#define NO_ERR              0x00000000
#define UNSUPPORTED_ERR     0x00000001
#define TIMEOUT_ERR         0x00000002
#define TRAINING_FAILED     0x00000003
#define BITFIELD_EXCEEDED   0x00000004

/* DDRC related */
#define    DDRC_BASE_ADDR                   0x403C0000
#define    OFFSET_DDRC_SWCTL                0x320
#define    OFFSET_DDRC_DFIMISC              0x1b0
#define    OFFSET_DDRC_DFISTAT              0x1bc
#define    OFFSET_DDRC_PWRCTL               0x30
#define    OFFSET_DDRC_SWSTAT               0x324
#define    OFFSET_DDRC_STAT                 0x04
#define    OFFSET_DDRC_DBG1                 0x304
#define    OFFSET_DDRC_ECCCFG0              0x70
#define    OFFSET_DDRC_ECCCFG1              0x74
#define    OFFSET_DDRC_SBRCTL               0xf24
#define    OFFSET_DDRC_SBRSTAT              0xf28
#define    OFFSET_DDRC_SBRWDATA0            0xf2c
#define    OFFSET_DDRC_MRSTAT               0x18
#define    OFFSET_DDRC_MRCTRL0              0x10
#define    OFFSET_DDRC_MRCTRL1              0x14
#define    OFFSET_DDRC_DERATEEN             0x20
#define    OFFSET_DDRC_RFSHTMG              0x64
#define    OFFSET_DDRC_DRAMTMG0             0x100
#define    OFFSET_DDRC_DRAMTMG1             0x104
#define    OFFSET_DDRC_DRAMTMG4             0x110

/* DDRC masks and values */
#define    DDRC_RFSHTMG_VAL_SHIFT           16
#define    DDRC_RFSHTMG_VAL                 0xfff
#define    DDRC_RFSHTMG_MASK                (DDRC_RFSHTMG_VAL << \
					     DDRC_RFSHTMG_VAL_SHIFT)
#define    DDRC_RFSHCTL3_UPDATE_SHIFT       1
#define    DDRC_RFSHCTL3_AUTO_REFRESH_VAL   0x1
#define    DDRC_RFSHCTL3_MASK               (DDRC_RFSHCTL3_AUTO_REFRESH_VAL \
					     << DDRC_RFSHCTL3_UPDATE_SHIFT)
#define    DDRC_DERATEEN_ENABLE             0x1
#define    DDRC_SWCTL_SWDONE_ENABLE         0x0
#define    DDRC_SWSTAT_SWDONE_ACK_MASK      0x1
#define    DDRC_DRAMTMG4_TRCD_POS           24
#define    DDRC_DRAMTMG5_TRCD_MASK          0x1f
#define    DDRC_DRAMTMG4_TRRD_POS           8
#define    DDRC_DRAMTMG5_TRRD_MASK          0xf
#define    DDRC_DRAMTMG0_TRAS_POS           0
#define    DDRC_DRAMTMG0_TRAS_MASK          0x3f
#define    DDRC_DRAMTMG4_TRP_POS            0
#define    DDRC_DRAMTMG4_TRP_MASK           0x1f
#define    DDRC_DRAMTMG1_TRC_POS            0
#define    DDRC_DRAMTMG1_TRC_MASK           0x7f
#define    DDRC_SWCTL_SWDONE_DONE           0x1
#define    SUCCESSIVE_READ                  2
#define    DDRC_DERATEEN_MASK_DISABLE       0x1

/* Performance monitoring registers */
#define    PERF_BASE_ADDR                   0x403E0000
#define    OFFSET_MRR_0_DATA_REG_ADDR       0x40
#define    OFFSET_MRR_1_DATA_REG_ADDR       0x44

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
#ifndef MC_CGM5_BASE_ADDR
#define    MC_CGM5_BASE_ADDR             0x40068000
#endif
#define    OFFSET_MUX_0_CSS              0x304
#define    OFFSET_MUX_0_CSC              0x300
#define    MC_CGM_MUX_0_DIV_UPD_STAT     0x33C
#define    FIRC_CLK_SRC                  0x0
#define    DDR_PHI0_PLL                  0x24

/* Default timeout for DDR PHY operations */
#define DEFAULT_TIMEOUT 1000000

/* Start addresses of IMEM and DMEM memory areas */
#define IMEM_START_ADDR 0x403A0000
#define DMEM_START_ADDR 0x403B0000

/* ERR050543 related defines */
#define MR4 4
#define TUF_THRESHOLD 3
#define REQUIRED_OK_CHECKS 3

/**
 * @brief Set default AXI parity.
 */
u32 set_axi_parity(void);

/**
 * @brief Post PHY train setup - complementary settings
 * that needs to be performed after running the firmware.
 */
u32 post_train_setup(void);

u32 wait_firmware_execution(void);

u32 load_register_array(u32 reg_writes,
			u32 array[][2]);

/* Initialize memory with the ecc scrubber */
u32 init_memory_ecc_scrubber(void);

/*
 * Set the ddr clock source, FIRC or DDR_PLL_PHI0.
 * @param clk_src - requested clock source
 * @return - true whether clock source has been changed, false otherwise
 */
bool sel_clk_src(u32 clk_src);

/* Read lpddr4 mode register.
 * @param mr_index - index of mode register to be read
 */
u32 read_lpddr4_mr(u8 mr_index);

/* Write lpddr4 mode register
 * @param mr_index - index of mode register to be read
 * @param mr_data - data to be written
 */
u32 write_lpddr4_mr(u8 mr_index, u8 mr_data);

/* Read Temperature Update Flag from lpddr4 MR4 register. */
u8 read_tuf(void);

/*
 * Enable ERR050543 errata workaround.
 * If the system is hot or cold prior to enabling derating, Temperature Update
 * Flag might not be set in MR4 register, causing incorrect refresh period and
 * derated timing parameters (tRCD, tRAS, rRP, tRRD being used.
 * Software workaround requires reading MR register and adjusting timing
 * parameters, if necessary.
 */
u32 enable_derating_temp_errata(void);

/*
 * Periodically read Temperature Update Flag in MR4 and undo changes made by
 * ERR050543 workaround if no longer needed. Refresh rate is updated and auto
 * derating is turned on.
 * @param traffic_halted - if ddr traffic was halted, restore also timing
 * parameters
 * @return - Returns 1, if the errata changes are reverted, 0 otherwise
 */
int poll_derating_temp_errata(bool traffic_halted);

#endif /* DDR_UTILS_H_ */
