// SPDX-License-Identifier: GPL-2.0+
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

#include "ddr_utils.h"

static u32 enable_axi_ports(void);
static u32 get_mail(u32 *mail);
static u32 ack_mail(void);

u8 polling_needed = 2;

/* Modify bitfield value with delta, given bitfield position and mask */
bool update_bf(u32 *v, u8 pos, u32 mask, int32_t delta)
{
	if (mask >= (((*v >> pos) & mask) + delta)) {
		*v = (*v & ~(mask << pos)) | ((((*v >> pos)
			& mask) + delta) << pos);
		return true;
	} else {
		return false;
	}
}

/*
 * Set the ddr clock source, FIRC or DDR_PLL_PHI0.
 * @param clk_src - requested clock source
 * @return - true whether clock source has been changed, false otherwise
 */
bool sel_clk_src(u32 clk_src)
{
	u32 tmp32;

	/* Check if the clock source is already set to clk_src*/
	tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	if (((tmp32 & 0x3fffffffU) >> 24) == clk_src)
		return false;

	/* To wait till clock switching is completed */
	do {
		tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1U) != 0x0);

	/* Set DDR_CLK source on src_clk */
	tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);
	writel((0xc0ffffffU & tmp32) | (clk_src << 24),
	       MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);

	/* Request clock switch */
	tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);
	writel((0x1u << 2) | tmp32, MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSC);

	/* To wait till clock switching is completed */
	do {
		tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1U) != 0x0);

	/* To wait till Switch after request is succeeded */
	do {
		tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 17) & 0x1U) != 0x1);

	/* Make sure correct clock source is selected */
	do {
		tmp32 = readl(MC_CGM5_BASE_ADDR + OFFSET_MUX_0_CSS);
	} while (((tmp32 & 0x3fffffffU) >> 24) != clk_src);

	return true;
}

/* Sets default AXI parity. */
u32 set_axi_parity(void)
{
	u32 tmp32;
	bool switched_to_firc;

	/* Enable Parity For All AXI Interfaces */
	tmp32 = readl(DDR_SS_REG);
	writel(tmp32 | 0x1ff0U, DDR_SS_REG);

	/* Set AXI_PARITY_TYPE to 0x1ff;   0-even, 1-odd */
	tmp32 = readl(DDR_SS_REG);
	writel(tmp32 | 0x1ff0000U, DDR_SS_REG);

	/* For LPDDR4 Set DFI1_ENABLED to 0x1 */
	tmp32 = readl(DDRC_BASE_ADDR);
	if ((tmp32 & MSTR_LPDDR4_MASK) == MSTR_LPDDR4_VAL) {
		tmp32 = readl(DDR_SS_REG);
		writel(tmp32 | 0x1U, DDR_SS_REG);
	}

	/*
	 * Set ddr clock source on FIRC_CLK.
	 * If it's already set on FIRC_CLK, it returns false.
	 */
	switched_to_firc = sel_clk_src(FIRC_CLK_SRC);

	/* De-assert Reset To Controller and AXI Ports */
	tmp32 = readl(MC_RGM_PRST_0);
	writel(~(0x1U << 3) & tmp32, MC_RGM_PRST_0);

	/* Check if the initial clock source was not on FIRC */
	if (switched_to_firc)
		switched_to_firc = sel_clk_src(DDR_PHI0_PLL);

	/* Enable HIF, CAM Queueing */
	writel(0x0, DDRC_BASE_ADDR + OFFSET_DDRC_DBG1);

	/* Disable auto-refresh: RFSHCTL3.dis_auto_refresh = 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	writel((0x1U | tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

	/* Disable power down: PWRCTL.powerdown_en = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000002U) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Disable self-refresh: PWRCTL.selfref_en = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000001U) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/*
	 * Disable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 0
	 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000008U) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Enable Quasi-Dynamic Programming */
	writel(DDRC_SWCTL_SWDONE_ENABLE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);

	/* Confirm Register Programming Done Ack is Cleared */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 1);

	/* DFI_INIT_COMPLETE_EN set to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((~0x1U) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set SWCTL.sw_done to 1 */
	writel(DDRC_SWCTL_SWDONE_DONE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);

	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	return NO_ERR;
}

/* Enables AXI port n. Programming Mode: Dynamic */
static u32 enable_axi_ports(void)
{
	/* Port 0 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_0);
	/* Port 1 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_1);
	/* Port 2 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_2);

	return NO_ERR;
}

/*
 * Post PHY training setup - complementary settings that need to be
 * performed after running the firmware.
 * @param options - various flags controlling post training actions
 * (whether to init memory with ECC scrubber / whether to store CSR)
 */
u32 post_train_setup(u8 options)
{
	u32 ret = NO_ERR;
	u32 tmp32;

	/*
	 * CalBusy.0 = 1, indicates the calibrator is actively calibrating.
	 * Wait Calibrating done.
	 */
	do {
		tmp32 = readl(DDR_PHYA_MASTER0_CALBUSY);
	} while ((tmp32 & 0x1U) != 0);

	/* Set SWCTL.sw_done to 0 */
	writel(DDRC_SWCTL_SWDONE_ENABLE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) != 0);

	/* Set DFIMISC.dfi_init_start to 1*/
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((0x00000020U | tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set SWCTL.sw_done to 1 */
	writel(DDRC_SWCTL_SWDONE_DONE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	/* Wait SWSTAT.sw_done_ack to 1*/
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	/* Wait DFISTAT.dfi_init_complete to 1 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFISTAT);
	} while ((tmp32 & 0x1U) == 0);

	/* Set SWCTL.sw_done to 0 */
	writel(DDRC_SWCTL_SWDONE_ENABLE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) != 0);

	/* Set dfi_init_start to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((~0x00000020U) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set dfi_complete_en to 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel(0x00000001U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set PWRCTL.selfref_sw to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000020U) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Set SWCTL.sw_done to 1 */
	writel(DDRC_SWCTL_SWDONE_DONE, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

	/* Wait for DWC_ddr_umctl2 to move to normal operating mode */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & 0x7U) == 0);

	/* Enable auto-refresh: RFSHCTL3.dis_auto_refresh = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	writel((~0x00000001U) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

	/*
	 * If ECC feature is enabled (ECCCFG0[ecc_mode] > 0)
	 * initialize memory with the ecc scrubber
	 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG0);
	if (((tmp32 & 0x7U) > 0) && ((options & INIT_MEM_MASK) != 0)) {
		ret = init_memory_ecc_scrubber();
		if (ret != NO_ERR)
			return ret;
	}

	/* Enable power down: PWRCTL.powerdown_en = 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000002U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Enable self-refresh: PWRCTL.selfref_en = 1*/
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000001U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/*
	 * Enable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 1
	 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000008U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	ret |= enable_derating_temp_errata();

	/*
	 * Each platform has a different number of AXI ports so this
	 * method should be implemented in hardware specific source
	 */
	ret |= enable_axi_ports();
	return ret;
}

/* Wait until firmware finishes execution and return training result */
u32 wait_firmware_execution(void)
{
	u32 mail = 0;
	u32 ret;

	while (true) {
		/* Obtain message from PHY (major message) */
		ret = get_mail(&mail);

		if (ret != NO_ERR)
			break;

		/* 0x07 means OK, 0xFF means failure */
		if (mail == 0x07)
			break;

		if (mail == 0xff) {
			/* Training stage failed */
			ret = TRAINING_FAILED;
			break;
		}
	}

	return ret;
}

/* Acknowledge received message */
static u32 ack_mail(void)
{
	u32 timeout = DEFAULT_TIMEOUT;
	/* ACK message */
	writel(0, DDR_PHYA_APBONLY_DCTWRITEPROT);
	u32 tmp32 = readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS);

	/* Wait firmware to respond to ACK (UctWriteProtShadow to be set) */
	while ((--timeout != 0) &&
	       ((tmp32 & UCT_WRITE_PROT_SHADOW_MASK) == 0))
		tmp32 = readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS);

	if (timeout == 0)
		return TIMEOUT_ERR;

	writel(1, DDR_PHYA_APBONLY_DCTWRITEPROT);

	return NO_ERR;
}

/* Read available message from DDR PHY microcontroller */
static u32 get_mail(u32 *mail)
{
	u32 timeout = DEFAULT_TIMEOUT;
	u32 tmp32 = readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS);

	while ((--timeout != 0) &&
	       ((tmp32 & UCT_WRITE_PROT_SHADOW_MASK) != 0))
		tmp32 = readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS);

	if (timeout == 0)
		return TIMEOUT_ERR;

	*mail = readl(DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW);

	/* ACK */
	return ack_mail();
}

/* Initialize memory with the ecc scrubber */
u32 init_memory_ecc_scrubber(void)
{
	u8 region_lock;
	u32 tmp32, pattern = 0x00000000U;

	/* Save previous ecc region parity locked state. */
	region_lock = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1) &
				  (0x1UL << 4);

	/* Enable ecc region lock. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	writel((0x01U << 4) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);

	/* Set SBRCTL.scrub_mode = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel((0x1U << 2) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_during_lowpower = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel((0x1U << 1) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_interval = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1fffU << 8) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set the desired pattern through SBRWDATA0 register. */
	writel(pattern, DDRC_BASE_ADDR + OFFSET_DDRC_SBRWDATA0);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(0x1U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/*
	 * Poll until SBRSTAT.scrub_done = 1
	 * (all scrub writes commands have been sent).
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while ((tmp32 & 0x2U) == 0);

	/*
	 * Poll until SBRSTAT.scrub_busy = 0
	 * (all scrub writes data have been sent).
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while ((tmp32 & 0x1U) != 0);

	/* Disable SBR by programming SBRCTL.scrub_en = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1U) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Enter normal scrub operation (Reads): SBRCTL.scrub_mode = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1U << 2) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_interval = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	tmp32 = ~(0x1fffU << 8) & tmp32;
	writel((0x1UL << 8) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(0x1U | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Restore locked state of ecc region. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	tmp32 = (tmp32 & ~(0x1UL << 4)) | (region_lock << 4);
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);

	return NO_ERR;
}

/* Read lpddr4 mode register with given index */
u32 read_lpddr4_mr(u8 mr_index)
{
	u32 tmp32;
	u8 succesive_reads = 0;

	/* Set MRR_DDR_SEL_REG to 0x1 to enable LPDDR4 mode */
	tmp32 = readl(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);
	writel((tmp32 | 0x1U), PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);

	/*
	 * Ensure no MR transaction is in progress:
	 * mr_wr_busy signal must be low
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
		if ((tmp32 & 0x1U) == 0)
			succesive_reads++;
		else
			succesive_reads = 0;
	} while (succesive_reads != REQUIRED_MRSTAT_READS);

	/* Set MR_TYPE = 0x1 (Read) and MR_RANK = 0x1 (Rank 0) */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	tmp32 |= 0x1U;
	tmp32 = (tmp32 & ~(0xfUL << 4)) | (0x1UL << 4);
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Configure MR address: MRCTRL1[8:15] */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);
	tmp32 = (tmp32 & ~(0xffUL << 8)) | ((u16)mr_index << 8);
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);

	dsb();

	/* Initiate MR transaction: MR_WR = 0x1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	writel(tmp32 | (0x1UL << 31), DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Wait until MR transaction completed */
	succesive_reads = 0;
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
		if ((tmp32 & 0x1U) == 0)
			succesive_reads++;
		else
			succesive_reads = 0;
	} while (succesive_reads != REQUIRED_MRSTAT_READS);

	return readl(PERF_BASE_ADDR + OFFSET_MRR_1_DATA_REG_ADDR);
}

/* Write data in lpddr4 mode register with given index */
u32 write_lpddr4_mr(u8 mr_index, u8 mr_data)
{
	u32 tmp32;
	u8 succesive_reads = 0;

	/* Set MRR_DDR_SEL_REG to 0x1 to enable LPDDR4 mode */
	tmp32 = readl(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);
	writel(tmp32 | 0x1U, PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);

	/*
	 * Ensure no MR transaction is in progress:
	 * mr_wr_busy signal must be low
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
		if ((tmp32 & 0x1U) == 0)
			succesive_reads++;
		else
			succesive_reads = 0;
	} while (succesive_reads != REQUIRED_MRSTAT_READS);

	/* Set MR_TYPE = 0x0 (Write) and MR_RANK = 0x1 (Rank 0) */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	tmp32 &= ~(0x1U);
	tmp32 = (tmp32 & ~(0xfUL << 4)) | (0x1UL << 4);
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Configure MR address: MRCTRL1[8:15] and MR data: MRCTRL1[0:7]*/
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);
	tmp32 = (tmp32 & (0xffffUL << 16)) | ((u16)mr_index << 8) |
		mr_data;
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);

	dsb();

	/* Initiate MR transaction: MR_WR = 0x1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	writel(tmp32 | (0x1UL << 31), DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Wait until MR transaction completed */
	succesive_reads = 0;
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
		if ((tmp32 & 0x1U) == 0)
			succesive_reads++;
		else
			succesive_reads = 0;
	} while (succesive_reads != REQUIRED_MRSTAT_READS);

	return NO_ERR;
}

/* Read Temperature Update Flag from lpddr4 MR4 register. */
u8 read_tuf(void)
{
	u32 mr4_val;
	u8 mr4_die_1, mr4_die_2;

	mr4_val = read_lpddr4_mr(MR4_IDX);
	mr4_die_1 = mr4_val & 0x7U;
	mr4_die_2 = (mr4_val >> 16) & 0x7U;

	return (mr4_die_1 > mr4_die_2) ? mr4_die_1 : mr4_die_2;
}

/*
 * Enable ERR050543 errata workaround.
 * If the system is hot or cold prior to enabling derating, Temperature Update
 * Flag might not be set in MR4 register, causing incorrect refresh period and
 * derated timing parameters (tRCD, tRAS, rRP, tRRD being used.
 * Software workaround requires reading MR register and adjusting timing
 * parameters, if necessary.
 */
u32 enable_derating_temp_errata(void)
{
	u32 tmp32, bf_val;

	if (read_tuf() > TUF_THRESHOLD) {
		/* Disable timing parameter derating: DERATEEN.DERATE_EN = 0 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		writel(tmp32 & ~DDRC_DERATEEN_MASK_DISABLE,
		       DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);

		/*
		 * Update average time interval between refreshes per rank:
		 * RFSHTMG.T_RFC_NOM_X1_X32 = RFSHTMG.T_RFC_NOM_X1_X32 / 4
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);
		bf_val = (tmp32 >> DDRC_RFSHTMG_VAL_SHIFT) & DDRC_RFSHTMG_VAL;
		bf_val = bf_val >> 2;
		tmp32 = (tmp32 & ~DDRC_RFSHTMG_MASK) |
			(bf_val << DDRC_RFSHTMG_VAL_SHIFT);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);

		/*
		 * Toggle RFSHCTL3.REFRESH_UPDATE_LEVEL to indicate that
		 * refresh registers have been updated
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
		bf_val = (tmp32 >> DDRC_RFSHCTL3_UPDATE_SHIFT) &
			 DDRC_RFSHCTL3_AUTO_REFRESH_VAL;
		bf_val = bf_val ^ 0x1U;
		tmp32 = (tmp32 & ~DDRC_RFSHCTL3_MASK) |
			(bf_val << DDRC_RFSHCTL3_UPDATE_SHIFT);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

		/* Set SWCTL.sw_done to 0 */
		writel(DDRC_SWCTL_SWDONE_ENABLE, DDRC_BASE_ADDR +
		       OFFSET_DDRC_SWCTL);
		do {
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) != 0);

		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		/*
		 * Set minimum time from activate to read/write command to same
		 * bank: DRAMTMG4.T_RCD += 2
		 */
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRCD_POS,
			       DDRC_DRAMTMG5_TRCD_MASK, 2))
			return BITFIELD_EXCEEDED;

		/*
		 * Set minimum time between activates from bank "a" to bank "b"
		 * DRAMTMG4.T_RRD += 2
		 */
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRRD_POS,
			       DDRC_DRAMTMG5_TRRD_MASK, 2))
			return BITFIELD_EXCEEDED;

		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

		/*
		 * Set minimum time between activate and precharge to same bank
		 * DRAMTMG0.T_RAS_MIN += 2
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);
		if (!update_bf(&tmp32, DDRC_DRAMTMG0_TRAS_POS,
			       DDRC_DRAMTMG0_TRAS_MASK, 2))
			return BITFIELD_EXCEEDED;

		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);

		/*
		 * Set minimum time from single-bank precharge to activate of
		 * same bank: DRAMTMG4.T_RP += 2
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		if (!update_bf(&tmp32, DDRC_DRAMTMG4_TRP_POS,
			       DDRC_DRAMTMG4_TRP_MASK, 2))
			return BITFIELD_EXCEEDED;

		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

		/*
		 * Set minimum time between activates to same bank:
		 * DRAMTMG1.T_RC += 3
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);
		if (!update_bf(&tmp32, DDRC_DRAMTMG1_TRC_POS,
			       DDRC_DRAMTMG1_TRC_MASK, 3))
			return BITFIELD_EXCEEDED;

		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);

		/* Set SWCTL.sw_done to 1 */
		writel(DDRC_SWCTL_SWDONE_DONE, DDRC_BASE_ADDR +
		       OFFSET_DDRC_SWCTL);
		do {
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & DDRC_SWSTAT_SWDONE_ACK_MASK) == 0);

		polling_needed = 1;

	} else {
		/* Enable timing parameter derating: DERATEEN.DERATE_EN = 1 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		writel(tmp32 | DDRC_DERATEEN_ENABLE,
		       DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);

		polling_needed = 0;
	}
	return NO_ERR;
}
