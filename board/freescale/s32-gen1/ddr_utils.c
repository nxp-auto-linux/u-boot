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

static u32 enable_axi_ports(void);
static u32 get_mail(u32 *mail);
static u32 ack_mail(void);

/* Sets default AXI parity. */
u32 set_axi_parity(void)
{
	u32 ret = NO_ERR;
	u32 tmp32;

	/* Enable Parity For All AXI Interfaces */
	writel(readl(DDR_SS_REG) | 0x1ff0, DDR_SS_REG);

	/* Set AXI_PARITY_TYPE to 0x1ff;   0-even, 1-odd */
	writel(readl(DDR_SS_REG) | 0x1ff0000, DDR_SS_REG);

	/* Set DFI1_ENABLED to 0x1 */
	writel(readl(DDR_SS_REG) | 0x1, DDR_SS_REG);

	/* De-assert Reset To Controller and AXI Ports */
	writel(0xfffffff6, MC_RGM_PRST_0);

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
	writel(0x50, DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);

	do {
		tmp32 = readl(DDRC_BASE_ADDR + OFFSET_DDRC_DFIMISC);
	} while ((tmp32 & 0x1) != 0);

	return ret;
}

/* Enables AXI port n. Programming Mode: Dynamic */
static u32 enable_axi_ports(void)
{
	u32 ret = NO_ERR;

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
u32 post_train_setup(void)
{
	u32 ret = NO_ERR;
	u32 tmp32;

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

	/*
	 * Each platform has a different number of AXI ports so this
	 * method should be implemented in hardware specific source
	 */
	ret = enable_axi_ports();
	return ret;
}

/* Wait until firmware finishes execution and return training result */
u32 wait_firmware_execution(void)
{
	u32 mail = 0;

	while (mail == 0) {
		/* Obtain message from PHY (major message) */
		u32 ret = get_mail(&mail);

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
static u32 ack_mail(void)
{
	u32 timeout = DEFAULT_TIMEOUT;
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
static u32 get_mail(u32 *mail)
{
	u32 timeout = DEFAULT_TIMEOUT;

	while (--timeout && (readl(DDR_PHYA_APBONLY_UCTSHSADOWREGS) &
				UCT_WRITE_PROT_SHADOW_MASK))
		;

	if (!timeout)
		return TIMEOUT_ERR;

	*mail = readl(DDR_PHYA_APBONLY_UCTWRITEONLYSHADOW);

	/* ACK */
	return ack_mail();
}
