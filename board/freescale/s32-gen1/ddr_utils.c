// SPDX-License-Identifier: GPL-2.0+
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

#include "ddr_utils.h"

static uint32_t enable_axi_ports(void);
static uint32_t get_mail(uint32_t *mail);
static uint32_t ack_mail(void);

/*
 * Do not deinitialize this variable.
 * We want this variable to be stored in .data section.
 * If left un-initialized, or if initialized with value 0,
 * it will be stored in .bss section and therefore not be
 * available after u-boot is relocated into DRAM, when its
 * value will be used.
 */
uint8_t polling_needed = 2;

/*
 * Set the ddr clock source, FIRC or DDR_PLL_PHI0.
 * @param clk_src - requested clock source
 * @return - true whether clock source has been changed, false otherwise
 */
bool sel_clk_src(uint32_t clk_src)
{
	uint32_t tmp32;

	/* Check if the clock source is already set to clk_src*/
	tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSS);
	if (((tmp32 & 0x3fffffff) >> 24) == clk_src)
		return false;

	/* To wait till clock switching is completed */
	do {
		tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1) != 0x0);

	/* Set DDR_CLK source on src_clk */
	tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSC);
	writel((0xc0ffffff & tmp32) | (clk_src << 24),
	       MC_CGM5 + OFFSET_MUX_0_CSC);

	/* Request clock switch */
	tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSC);
	writel((0x1 << 2) | tmp32, MC_CGM5 + OFFSET_MUX_0_CSC);

	/* To wait till clock switching is completed */
	do {
		tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 16) & 0x1) != 0x0);

	/* To wait till Switch after request is succeeded */
	do {
		tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSS);
	} while (((tmp32 >> 17) & 0x1) != 0x1);

	/* Make sure correct clock source is selected */
	do {
		tmp32 = readl(MC_CGM5 + OFFSET_MUX_0_CSS);
	} while (((tmp32 & 0x3fffffff) >> 24) != clk_src);

	return true;
}

/* Sets default AXI parity. */
uint32_t set_axi_parity(void)
{
	uint32_t ret = NO_ERR;
	uint32_t tmp32;
	bool switched_to_firc;

	/* Enable Parity For All AXI Interfaces */
	writel(readl(DDR_SS_REG) | 0x1ff0, DDR_SS_REG);

	/* Set AXI_PARITY_TYPE to 0x1ff;   0-even, 1-odd */
	writel(readl(DDR_SS_REG) | 0x1ff0000, DDR_SS_REG);

	/* Set DFI1_ENABLED to 0x1 */
	writel(readl(DDR_SS_REG) | 0x1, DDR_SS_REG);

	/*
	 * Set ddr clock source on FIRC_CLK.
	 * If it's already set on FIRC_CLK, it returns false.
	 */
	switched_to_firc = sel_clk_src(FIRC_CLK_SRC);

	/* De-assert Reset To Controller and AXI Ports */
	tmp32 = readl(MC_RGM_PRST_0);
	writel(~(0x1 << 3) & tmp32, MC_RGM_PRST_0);

	/* Check if the initial clock source was not on FIRC */
	if (switched_to_firc)
		sel_clk_src(DDR_PHI0_PLL);

	/* Enable HIF, CAM Queueing */
	writel(0x0, DDRC_BASE_ADDR + OFFSET_DDRC_DBG1);

	/* Disable auto-refresh: RFSHCTL3.dis_auto_refresh = 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	writel((1 | tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

	/* Disable power down: PWRCTL.powerdown_en = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000002) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Disable self-refresh: PWRCTL.selfref_en = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000001) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/*
	 * Disable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 0
	 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000008) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Enable Quasi-Dynamic Programming */
	writel(0x0, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);

	/* Confirm Register Programming Done Ack is Cleared */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & 0x1) == 1);

	/* DFI_INIT_COMPLETE_EN set to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((~0x1) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	} while ((tmp32 & 0x1) != 0);

	return ret;
}

/* Enables AXI port n. Programming Mode: Dynamic */
static uint32_t enable_axi_ports(void)
{
	uint32_t ret = NO_ERR;

	/* Port 0 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_0);
	/* Port 1 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_1);
	/* Port 2 Control Register */
	writel(0x00000001, DDRC_UMCTL2_MP_BASE_ADDR + OFFSET_DDRC_PCTRL_2);

	return ret;
}

/*
 * Post PHY training setup - complementary settings that need to be
 * performed after running the firmware.
 */
uint32_t post_train_setup(void)
{
	uint32_t ret = NO_ERR;
	uint32_t tmp32;

	/*
	 * CalBusy.0 = 1, indicates the calibrator is actively calibrating.
	 * Wait Calibrating done.
	 */
	do {
		tmp32 = readl(DDR_PHYA_MASTER0_CALBUSY);
	} while ((tmp32 & 0x1) != 0);

	/* Set SWCTL.sw_done to 0 */
	writel(0x00000000, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & 0x1) != 0);

	/* Set DFIMISC.dfi_init_start to 1*/
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((0x00000020 | tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set SWCTL.sw_done to 1 */
	writel(0x00000001, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	/* Wait SWSTAT.sw_done_ack to 1*/
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & 0x1) == 0);

	/* Wait DFISTAT.dfi_init_complete to 1 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFISTAT);
	} while ((tmp32 & 0x1) == 0);

	/* Set SWCTL.sw_done to 0 */
	writel(0x00000000, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & 0x1));

	/* Set dfi_init_start to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel((~0x00000020) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set dfi_complete_en to 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	writel(0x00000001 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	/* Set PWRCTL.selfref_sw to 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(((~0x00000020) & tmp32), DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Set SWCTL.sw_done to 1 */
	writel(0x00000001, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
	} while ((tmp32 & 0x1) == 0);

	/* Wait for DWC_ddr_umctl2 to move to normal operating mode */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_STAT);
	} while ((tmp32 & 0x7) == 0);

	/* Enable auto-refresh: RFSHCTL3.dis_auto_refresh = 0 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
	writel((~0x00000001) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

	/*
	 * If ECC feature is enabled (ECCCFG0[ecc_mode] > 0)
	 * initialize memory with the ecc scrubber
	 */
	if ((readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG0) & 0x7) > 0) {
		ret = init_memory_ecc_scrubber();
		if (ret != NO_ERR)
			return ret;
	}

	/* Enable power down: PWRCTL.powerdown_en = 1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000002 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/* Enable self-refresh: PWRCTL.selfref_en = 1*/
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000001 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	/*
	 * Enable assertion of dfi_dram_clk_disable:
	 * PWRTL.en_dfi_dram_clk_disable = 1
	 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);
	writel(0x00000008 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_PWRCTL);

	enable_derating_temp_errata();

	/*
	 * Each platform has a different number of AXI ports so this
	 * method should be implemented in hardware specific source
	 */
	ret = enable_axi_ports();
	return ret;
}

/* Wait until firmware finishes execution and return training result */
uint32_t wait_firmware_execution(void)
{
	uint32_t mail = 0;

	while (mail == 0) {
		/* Obtain message from PHY (major message) */
		uint32_t ret = get_mail(&mail);

		if (ret != NO_ERR)
			return ret;

		/* 0x07 means OK, 0xFF means failure */
		if (mail == 0x07)
			return NO_ERR;
		if (mail == 0xff)
			/* Training stage failed */
			return TRAINING_FAILED;

		/* No error. Keep querying for mails */
		mail = 0;
	}
	return TIMEOUT_ERR;
}

/* Acknowledge received message */
static uint32_t ack_mail(void)
{
	uint32_t timeout = DEFAULT_TIMEOUT;
	/* ACK message */
	writel(0, DDR_PHYA_APBONLY_DCTWRITEPROT);

	/* Wait firmware to respond to ACK (UctWriteProtShadow to be set) */
	while (--timeout && !(readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS) &
		   UCT_WRITE_PROT_SHADOW_MASK))
		;

	if (!timeout)
		return TIMEOUT_ERR;

	writel(1, DDR_PHYA_APBONLY_DCTWRITEPROT);

	return NO_ERR;
}

/* Read available message from DDR PHY microcontroller */
static uint32_t get_mail(uint32_t *mail)
{
	uint32_t timeout = DEFAULT_TIMEOUT;

	while (--timeout && (readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS) &
		   UCT_WRITE_PROT_SHADOW_MASK))
		;

	if (!timeout)
		return TIMEOUT_ERR;

	*mail = readl(DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW);

	/* ACK */
	return ack_mail();
}

/* Initialize memory with the ecc scrubber */
uint32_t init_memory_ecc_scrubber(void)
{
	uint8_t region_lock;
	uint32_t tmp32, pattern = 0x00000000;

	/* Save previous ecc region parity locked state. */
	region_lock = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1) & (0x1 << 4);

	/* Enable ecc region lock. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	writel((0x1 << 4) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);

	/* Set SBRCTL.scrub_mode = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel((0x1 << 2) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_during_lowpower = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel((0x1 << 1) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_interval = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1fff << 8) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set the desired pattern through SBRWDATA0 register. */
	writel(pattern, DDRC_BASE_ADDR + OFFSET_DDRC_SBRWDATA0);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(0x1 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/*
	 * Poll until SBRSTAT.scrub_done = 1
	 * (all scrub writes commands have been sent).
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while ((tmp32 & 0x2) == 0);

	/*
	 * Poll until SBRSTAT.scrub_busy = 0
	 * (all scrub writes data have been sent).
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRSTAT);
	} while (tmp32 & 0x1);

	/* Disable SBR by programming SBRCTL.scrub_en = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Enter normal scrub operation (Reads): SBRCTL.scrub_mode = 0. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(~(0x1 << 2) & tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Set SBRCTL.scrub_interval = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	tmp32 = ~(0x1fff << 8) & tmp32;
	writel((0x1 << 8) | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Enable the SBR by programming SBRCTL.scrub_en = 1. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);
	writel(0x1 | tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_SBRCTL);

	/* Restore locked state of ecc region. */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);
	tmp32 = (tmp32 & ~(0x1 << 4)) | (region_lock << 4);
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG1);

	return NO_ERR;
}

/* Read lpddr4 mode register with given index */
uint32_t read_lpddr4_MR(uint16_t MR_index)
{
	uint32_t tmp32;

	/* Set MRR_DDR_SEL_REG to 0x1 to enable LPDDR4 mode */
	tmp32 = readl(PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);
	writel((tmp32 | 0x1), PERF_BASE_ADDR + OFFSET_MRR_0_DATA_REG_ADDR);

	/*
	 * Ensure no MR transaction is in progress:
	 * mr_wr_busy signal must be low
	 */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	/* Set MR_TYPE = 0x1 (Read) and MR_RANK = 0x1 (Rank 0) */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	tmp32 |= 0x1;
	tmp32 &= ~(0x1 << 4); // TODO - check bitfield size!
	writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Configure MR address: MRCTRL1[8:15] */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);
	writel(tmp32 | (MR_index << 8), DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL1);

	asm("DSB SY");

	/* Initiate MR transaction: MR_WR = 0x1 */
	tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);
	writel(tmp32 | (0x1u << 31), DDRC_BASE_ADDR + OFFSET_DDRC_MRCTRL0);

	/* Wait until MR transaction completed */
	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_MRSTAT);
	} while ((tmp32 & 0x1) != 0);

	return readl(PERF_BASE_ADDR + OFFSET_MRR_1_DATA_REG_ADDR);
}

/* Read Temperature Update Flag from lpddr4 MR4 register. */
uint8_t read_TUF(void)
{
	uint32_t MR4_val;
	uint8_t MR4_die_1, MR4_die_2;

	MR4_val = read_lpddr4_MR(MR4);
	MR4_die_1 = MR4_val & 0x7;
	MR4_die_2 = (MR4_val >> 16) & 0x7;

	return MR4_die_1 > MR4_die_2 ? MR4_die_1 : MR4_die_2;
}

/*
 * Enable ERR050543 errata workaround.
 * If the system is hot or cold prior to enabling derating, Temperature Update
 * Flag might not be set in MR4 register, causing incorrect refresh period and
 * derated timing parameters (tRCD, tRAS, rRP, tRRD being used.
 * Software workaround requires reading MR register and adjusting timing
 * parameters, if necessary.
 */
void enable_derating_temp_errata(void)
{
	uint32_t tmp32, bf_val;

	if (read_TUF() > TUF_THRESHOLD) {
		/* Disable timing parameter derating: DERATEEN.DERATE_EN = 0 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		writel(tmp32 & ~(0x1), DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);

		/*
		 * Update average time interval between refreshes per rank:
		 * RFSHTMG.T_RFC_NOM_X1_X32 = RFSHTMG.T_RFC_NOM_X1_X32 / 4
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);
		bf_val = (tmp32 >> 16) & 0xfff;
		bf_val = bf_val >> 2;
		tmp32 = (tmp32 & ~(0xfff << 16)) | (bf_val << 16);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);

		/*
		 * Toggle RFSHCTL3.REFRESH_UPDATE_LEVEL to indicate that
		 * refresh registers have been updated
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
		bf_val = (tmp32 >> 1) & 0x1;
		bf_val = bf_val ^ 1;
		writel((tmp32 & ~(0x1 << 1)) | (bf_val << 1),
		       DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

		/* Set SWCTL.sw_done to 0 */
		writel(0x00000000, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
		do {
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & 0x1) != 0);

		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		/*
		 * Set minimum time from activate to read/write command to same
		 * bank: DRAMTMG4.T_RCD += 2
		 */
		tmp32 = update_bf(tmp32, 24, 0x1f, 2);
		/*
		 * Set minimum time between activates from bank "a" to bank "b"
		 * DRAMTMG4.T_RRD += 2
		 */
		tmp32 = update_bf(tmp32, 8, 0xf, 2);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

		/*
		 * Set minimum time between activate and precharge to same bank
		 * DRAMTMG0.T_RAS_MIN += 2
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);
		tmp32 = update_bf(tmp32, 0, 0x3f, 2);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);

		/*
		 * Set minimum time from single-bank precharge to activate of
		 * same bank: DRAMTMG4.T_RP += 2
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
		tmp32 = update_bf(tmp32, 0, 0x1f, 2);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

		/*
		 * Set minimum time between activates to same bank:
		 * DRAMTMG1.T_RC += 3
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);
		tmp32 = update_bf(tmp32, 0, 0x7f, 3);
		bf_val = tmp32 & 0x7f;
		bf_val += 6;
		tmp32 = (tmp32 & ~(0x7f)) | bf_val;
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);

		/* Set SWCTL.sw_done to 1 */
		writel(0x00000001, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
		do {
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_SWSTAT);
		} while ((tmp32 & 0x1) == 0);

		polling_needed = 1;

	} else {
		/* Enable timing parameter derating: DERATEEN.DERATE_EN = 1 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		writel(tmp32 | 0x1, DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);

		polling_needed = 0;
	}
}

/*
 * Periodically read Temperature Update Flag in MR4 and undo changes made by
 * ERR050543 workaround if no longer needed. Refresh rate is updated and auto
 * derating is turned on.
 * @param traffic_halted - if ddr traffic was halted, restore also timing
 * parameters
 */
void poll_derating_temp_errata(bool traffic_halted)
{
	int nominal_temp_flag = 0, val_1, val_2;
	uint32_t tmp32, bf_val;

	if (polling_needed) {
		if (read_TUF() <= TUF_THRESHOLD) {
			nominal_temp_flag++;

			val_1 = read_TUF();
			val_2 = read_TUF();

			if (val_1 <= TUF_THRESHOLD && val_2 <= TUF_THRESHOLD)
				nominal_temp_flag += 2;
		}
	}

	if (nominal_temp_flag == REQUIRED_OK_CHECKS) {
		/*
		 * Update average time interval between refreshes per rank:
		 * RFSHTMG.T_RFC_NOM_X1_X32 = RFSHTMG.T_RFC_NOM_X1_X32 * 4
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);
		bf_val = (tmp32 >> 16) & 0xfff;
		bf_val = bf_val << 2;
		tmp32 = (tmp32 & ~(0xfff << 16)) | (bf_val << 16);
		writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_RFSHTMG);

		/*
		 * Toggle RFSHCTL3.REFRESH_UPDATE_LEVEL to indicate that
		 * refresh registers have been updated
		 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);
		bf_val = (tmp32 >> 1) & 0x1;
		bf_val = bf_val ^ 1;
		writel((tmp32 & ~(0x1 << 1)) | (bf_val << 1),
		       DDRC_BASE_ADDR + OFFSET_DDRC_RFSHCTL3);

		/* If DDR trafic was halted, restore timing parameters */
		if (traffic_halted) {
			/* Set SWCTL.sw_done to 0 */
			writel(0x00000000, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
			do {
				tmp32 = readl(DDRC_BASE_ADDR +
					      OFFSET_DDRC_SWSTAT);
			} while ((tmp32 & 0x1) != 0);

			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
			/*
			 * Set minimum time from activate to read/write command
			 * to same bank: DRAMTMG4.T_RCD -= 2
			 */
			tmp32 = update_bf(tmp32, 24, 0x1f, -2LL);

			/*
			 * Set minimum time between activates from bank "a" to
			 * bank "b": DRAMTMG4.T_RRD -= 4
			 */
			tmp32 = update_bf(tmp32, 8, 0xf, -2LL);
			writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

			/*
			 * Set minimum time between activate and precharge to
			 * same bank: DRAMTMG0.T_RAS_MIN -= 2
			 */
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);
			tmp32 = update_bf(tmp32, 0, 0x3f, -2LL);
			writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG0);

			/*
			 * Set minimum time from single-bank precharge to
			 * activate of same bank: DRAMTMG4.T_RP -= 2
			 */
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);
			tmp32 = update_bf(tmp32, 0, 0x1f, -2LL);
			writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG4);

			/*
			 * Set minimum time between activates to same bank:
			 * DRAMTMG1.T_RC -= 3
			 */
			tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);
			tmp32 = update_bf(tmp32, 0, 0x7f, -3LL);
			writel(tmp32, DDRC_BASE_ADDR + OFFSET_DDRC_DRAMTMG1);

			/* Set SWCTL.sw_done to 1 */
			writel(0x00000001, DDRC_BASE_ADDR + OFFSET_DDRC_SWCTL);
			do {
				tmp32 = readl(DDRC_BASE_ADDR +
					      OFFSET_DDRC_SWSTAT);
			} while ((tmp32 & 0x1) == 0);
		}

		/* Enable timing parameter derating: DERATEEN.DERATE_EN = 1 */
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);
		writel(tmp32 | 0x1, DDRC_BASE_ADDR + OFFSET_DDRC_DERATEEN);

		polling_needed = 0;
	}
}
