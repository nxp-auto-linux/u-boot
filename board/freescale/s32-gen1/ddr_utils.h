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

#include <asm/io.h>
#include <stdbool.h>

/* Possible errors */
#define NO_ERR              0x00000000U
#define TIMEOUT_ERR         0x00000002U
#define TRAINING_FAILED     0x00000003U
#define BITFIELD_EXCEEDED   0x00000004U

/* DDRC related */
#define DDRC_BASE_ADDR                   (u32)0x403C0000U
#define OFFSET_DDRC_SWCTL                (u32)0x320U
#define OFFSET_DDRC_DFIMISC              (u32)0x1b0U
#define OFFSET_DDRC_DFISTAT              (u32)0x1bcU
#define OFFSET_DDRC_PWRCTL               (u32)0x30U
#define OFFSET_DDRC_SWSTAT               (u32)0x324U
#define OFFSET_DDRC_STAT                 (u32)0x04U
#define OFFSET_DDRC_DBG1                 (u32)0x304U
#define OFFSET_DDRC_ECCCFG0              (u32)0x70U
#define OFFSET_DDRC_ECCCFG1              (u32)0x74U
#define OFFSET_DDRC_SBRCTL               (u32)0xf24U
#define OFFSET_DDRC_SBRSTAT              (u32)0xf28U
#define OFFSET_DDRC_SBRWDATA0            (u32)0xf2cU
#define OFFSET_DDRC_MRSTAT               (u32)0x18U
#define OFFSET_DDRC_MRCTRL0              (u32)0x10U
#define OFFSET_DDRC_MRCTRL1              (u32)0x14U

#ifdef CONFIG_SYS_ERRATUM_ERR050543
#define OFFSET_DDRC_DERATEEN             (u32)0x20U
#define OFFSET_DDRC_RFSHTMG              (u32)0x64U
#define OFFSET_DDRC_DRAMTMG0             (u32)0x100U
#define OFFSET_DDRC_DRAMTMG1             (u32)0x104U
#define OFFSET_DDRC_DRAMTMG4             (u32)0x110U
#endif

/* DDRC masks and values */
#define MSTR_LPDDR4_MASK	0x20U
#define MSTR_LPDDR4_VAL		0x20U
#define SWSTAT_SW_DONE		1U
#define SWSTAT_SW_NOT_DONE	0U
#define SWCTL_SWDONE_DONE	0x1
#define SWCTL_SWDONE_ENABLE	0x0
#define SWSTAT_SWDONE_ACK_MASK	0x1U

#ifdef CONFIG_SYS_ERRATUM_ERR050543
#define RFSHTMG_VAL_SHIFT           16
#define RFSHTMG_VAL                 (u32)0xfffU
#define RFSHTMG_MASK                (RFSHTMG_VAL << \
	RFSHTMG_VAL_SHIFT)
#define RFSHCTL3_UPDATE_SHIFT       1
#define RFSHCTL3_AUTO_REFRESH_VAL   0x1U
#define RFSHCTL3_MASK               (RFSHCTL3_AUTO_REFRESH_VAL \
	<< RFSHCTL3_UPDATE_SHIFT)
#define DERATEEN_ENABLE		0x1U
#define DRAMTMG4_TRCD_POS	24
#define DRAMTMG4_TRCD_MASK	0x1f
#define DRAMTMG4_TRRD_POS	8
#define DRAMTMG4_TRRD_MASK	0xf
#define DRAMTMG0_TRAS_POS	0
#define DRAMTMG0_TRAS_MASK	0x3f
#define DRAMTMG4_TRP_POS	0
#define DRAMTMG4_TRP_MASK	0x1f
#define DRAMTMG1_TRC_POS	0
#define DRAMTMG1_TRC_MASK	0x7f
#define SUCCESSIVE_READ		0x2U
#define	DERATEEN_MASK_DIS	0x1U

#define RFSHTMG_UPDATE_SHIFT		2
#define RFSHCTL3_UPDATE_LEVEL_TOGGLE	0x1U
#define DRAMTMG4_TRCD_DELTA_TIME	2
#define DRAMTMG4_TRRD_DELTA_TIME	2
#define DRAMTMG0_TRAS_DELTA_TIME	2
#define DRAMTMG4_TRP_DELTA_TIME		2
#define DRAMTMG1_TRC_DELTA_TIME		3
#define ERRATA_CHANGES_REVERTED		1
#define ERRATA_CHANGES_UNMODIFIED	0
#endif

#define CSS_SELSTAT_MASK		0x3f000000U
#define	CSS_SELSTAT_POS			24
#define	CSS_SWIP_POS			16
#define	CSS_SW_IN_PROGRESS		0x1U
#define	CSS_SW_COMPLETED		0x0U
#define	CSC_SELCTL_MASK			0xC0FFFFFFU
#define CSC_SELCTL_POS			24
#define	CSC_CLK_SWITCH_REQUEST		0x1U
#define	CSC_CLK_SWITCH_POS		2
#define	CSS_SW_AFTER_REQUEST_SUCCEDED	0x1U
#define	CSS_SW_TRIGGER_CAUSE_POS	17

#define DDR_SS_AXI_PARITY_ENABLE_MASK	0x00001FF0U
#define DDR_SS_AXI_PARITY_TYPE_MASK	0x01FF0000U
#define DDR_SS_DFI_1_ENABLED		0x1U
#define FORCED_RESET_ON_PERIPH		0x1U
#define PRST_0_PERIPH_3_RST_POS		3
#define DBG1_DISABLE_DE_QUEUEING	0x0U
#define RFSHCTL3_DISABLE_AUTO_REFRESH	0x1U
#define ENABLE_AXI_PORT			0x000000001

#define PWRCTL_POWER_DOWN_ENABLE_MASK		0x00000002U
#define PWRCTL_SELF_REFRESH_ENABLE_MASK		0x00000001U
#define PWRCTL_EN_DFI_DRAM_CLOCK_DIS_MASK	0x00000008U
#define DFIMISC_DFI_INIT_COMPLETE_EN_MASK	0x000000001U

#define MASTER0_CAL_ACTIVE		0x1U
#define MASTER0_CAL_DONE		0x0U
#define	DFIMISC_DFI_INIT_START_MASK	0x00000020U
#define	DFISTAT_DFI_INIT_DONE		0x1U
#define	DFISTAT_DFI_INIT_INCOMPLETE	0x0U
#define	PWRCTL_SELFREF_SW_MASK		0x00000020U
#define	STAT_OPERATING_MODE_MASK	0x7U
#define	STAT_OPERATING_MODE_INIT	0x0U
#define	RFSHCTL3_DIS_AUTO_REFRESH_MASK	0x00000001U
#define	ECCCFG0_ECC_MODE_MASK		0x7U
#define	ECCCFG0_ECC_DISABLED		0x0U
#define	TRAINING_OK_MSG			0x07U
#define	TRAINING_FAILED_MSG		0xFFU
#define	ECCCFG1_REGION_PARITY_LOCKED	(u32)0x1U
#define	ECCCFG1_REGION_PARITY_LOCK_POS	4
#define	SBRCTL_SCRUB_MODE_WRITE		(u32)0x1U
#define	SBRCTL_SCRUB_MODE_POS		2

#define	APBONLY_DCTWRITEPROT_ACK_EN              0
#define	APBONLY_DCTWRITEPROT_ACK_DIS             1
#define	SBRCTL_SCRUB_DURING_LOWPOWER_CONTINUED   (u32)0x1U
#define	SBRCTL_SCRUB_DURING_LOWPOWER_POS         1

#define	SBRCTL_SCRUB_INTERVAL_FIELD     0x1FFFU
#define	SBRCTL_SCRUB_INTERVAL_POS       8
#define	SBRCTL_SCRUB_EN	                0x1U
#define	SBRSTAT_SCRUB_DONE_MASK         0x2U
#define	SBRSTAT_SCRUBBER_NOT_DONE       0x0U
#define	SBRSTAT_SCRUBBER_BUSY_MASK      0x1U
#define	SBRSTAT_SCRUBBER_NOT_BUSY       0x0U
#define	SBRCTL_SCRUB_INTERVAL_VALUE_1   (u32)0x1U
#define	MRR_0_DDR_SEL_REG_MASK          0x1U

#define	MRSTAT_MR_BUSY                  0x1U
#define	MRSTAT_MR_NOT_BUSY              0x0U
#define	MRCTRL0_MR_TYPE_READ            0x1U
#define	MRCTRL0_RANK_ACCESS_POS         4
#define	MRCTRL0_RANK_ACCESS_FIELD       (u32)0xfU
#define	MRCTRL0_RANK_0                  0x1U
#define	MRCTRL1_MR_ADDRESS_FIELD        (u32)0xffU
#define	MRCTRL1_MR_ADDRESS_POS          8
#define	MRCTRL0_WR_ENGAGE               (u32)0x1U
#define	MRCTRL0_WR_ENGAGE_POS           31
#define	MRCTRL1_MR_DATA_ADDRESS_FIELD   (u32)0xffffU
#define	MRCTRL1_MR_DATA_ADDRESS_POS     16
#define STORE_CSR_DISABLED              0x0U
#define INIT_MEM_DISABLED               0x0U

/* Performance monitoring registers */
#define PERF_BASE_ADDR                   (u32)0x403E0000U
#define OFFSET_MRR_0_DATA_REG_ADDR       (u32)0x40U
#define OFFSET_MRR_1_DATA_REG_ADDR       (u32)0x44U

/* uMCTL2 Multi-Port Registers */
#define DDRC_UMCTL2_MP_BASE_ADDR         (u32)0x403C03F8U
#define OFFSET_DDRC_PCTRL_0              (u32)0x98U
#define OFFSET_DDRC_PCTRL_1              (u32)0x148U
#define OFFSET_DDRC_PCTRL_2              (u32)0x1f8U

/* PHY related */
#define DDR_PHYA_MASTER0_CALBUSY		0x4038165C
#define DDR_PHYA_APBONLY_UCTSHADOWREGS		0x40380404U
#define UCT_WRITE_PROT_SHADOW_MASK              0x1U
#define DDR_PHYA_DCTWRITEPROT			0x4038040C
#define DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW	0x40380410
#define OFFSET_DDRC_RFSHCTL3			(u32)0x60U
#define DDR_PHYA_UCCLKHCLKENABLES		0x40380BEC
#define UCT_WRITE_PROT_SHADOW_ACK		0x0U

#define SHIFT_BIT(nr)             (((u32)0x1U) << (nr))
#define UCCLKEN_MASK              SHIFT_BIT(0)
#define HCLKEN_MASK               SHIFT_BIT(1)
#define OFFSET_DDRC_INIT0         (u32)0xd0U

#define STORE_CSR_MASK            SHIFT_BIT(0)
#define INIT_MEM_MASK             SHIFT_BIT(1)
#define SCRUB_EN_MASK             SHIFT_BIT(0)
#define SCRUB_BUSY_MASK           SHIFT_BIT(0)
#define SELFREF_SW_MASK           SHIFT_BIT(5)
#define SELFREF_TYPE_MASK         (SHIFT_BIT(4) | SHIFT_BIT(5))
#define OPERATING_MODE_MASK       (SHIFT_BIT(0) | SHIFT_BIT(1) | SHIFT_BIT(2))
#define DFI_INIT_COMPLETE_MASK    SHIFT_BIT(0)
#define DFI_INIT_START_MASK       SHIFT_BIT(5)
#define SW_DONE_ACK_MASK          SHIFT_BIT(0)
#define SW_DONE_MASK              SHIFT_BIT(0)
#define SKIP_DRAM_INIT_MASK       (SHIFT_BIT(30) | SHIFT_BIT(31))

/* Standby SRAM */
#define STNDBY_RAM_BASE           0x24000000
#define RETENTION_ADDR            STNDBY_RAM_BASE

/* DDR Subsystem */
#define DDR_SS_REG                0x403D0000

/* Reset Generation Module */
#define MC_RGM_PRST_0             0x40078040
#ifndef MC_CGM5_BASE_ADDR
#define MC_CGM5_BASE_ADDR         (u32)0x40068000U
#endif
#define OFFSET_MUX_0_CSS          (u32)0x304U
#define OFFSET_MUX_0_CSC          (u32)0x300U
#define FIRC_CLK_SRC              0x0U
#define DDR_PHI0_PLL              0x24U

/* Default timeout for DDR PHY operations */
#define DEFAULT_TIMEOUT 1000000

/* Start addresses of IMEM and DMEM memory areas */
#define IMEM_START_ADDR 0x403A0000
#define DMEM_START_ADDR 0x403B0000

#ifdef CONFIG_SYS_ERRATUM_ERR050543
/* ERR050543 related defines */
#define MR4_IDX            4
#define MR4_MASK           0x7U
#define MR4_SHIFT          16
#define TUF_THRESHOLD      0x3U
#define REQUIRED_OK_CHECKS 0x3U
#endif

/* ERR050760 related defines */
#define REQUIRED_MRSTAT_READS 0x2U

#ifdef CONFIG_SYS_ERRATUM_ERR050543
extern u8 polling_needed;
#endif

/* Set default AXI parity. */
u32 set_axi_parity(void);

/*
 * Post PHY train setup - complementary settings
 * that needs to be performed after running the firmware.
 * @param options - various flags controlling post training actions
 * (whether to init memory with ECC scrubber / whether to store CSR)
 */
u32 post_train_setup(u8 options);

/* Wait until firmware finishes execution and return training result */
u32 wait_firmware_execution(void);

/* Read lpddr4 mode register.
 * @param mr_index - index of mode register to be read
 */
u32 read_lpddr4_mr(u8 mr_index);

/* Write lpddr4 mode register
 * @param mr_index - index of mode register to be read
 * @param mr_data - data to be written
 */
u32 write_lpddr4_mr(u8 mr_index, u8 mr_data);

#ifdef CONFIG_SYS_ERRATUM_ERR050543
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
u32 poll_derating_temp_errata(bool traffic_halted);

/* Modify bitfield value with delta, given bitfield position and mask */
bool update_bf(u32 *v, u8 pos, u32 mask, int32_t delta);
#endif

#endif /* DDR_UTILS_H_ */
