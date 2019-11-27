// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019 NXP */
#include <asm/arch/s32-gen1/ddrss.h>
#include <linux/kernel.h>
#include <linux/printk.h>

void ddrss_init(struct ddrss_conf *ddrss_conf,
		struct ddrss_firmware *ddrss_firmware)
{
	u32 dfmisc_conf = CTL_IDLE_EN_MASK | DIS_DYN_ADR_TRI_MASK;
	write_regconf_32(ddrss_conf->ddrc_conf, ddrss_conf->ddrc_conf_length);

	writel(readl(REG_GRP0) | AXI_PARITY_EN_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | AXI_PARITY_TYPE_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | DFI1_ENABLED_MASK, REG_GRP0);

	deassert_ddr_reset();
	writel(0, PWRCTL);
	writel(0, PWRCTL);
	writel(0, SWCTL);

	while (readl(SWSTAT) & SW_DONE_ACK_MASK)
		;

	writel(dfmisc_conf, DFIMISC);
	writel(PBA_MODE | MR_ADDR_MR6 | MR_RANK_01 | SW_INIT_INT, MRCTRL0);
	writel(dfmisc_conf, DFIMISC);
	writel(MR_RANK_01 | SW_INIT_INT, MRCTRL0);
	writel(0xb05, MRCTRL1);
	writel(MR_RANK_01 | SW_INIT_INT, MRCTRL0);

	write_regconf_16(ddrss_conf->ddrphy_conf,
			 ddrss_conf->ddrphy_conf_length);

	writel(DFI_INIT_START_MASK | DIS_DYN_ADR_TRI_MASK, DFIMISC);

	while ((readl(DFISTAT) & DFI_INIT_COMPLETE_MASK) !=
	       DFI_INIT_COMPLETE_MASK)
		;

	writel(dfmisc_conf, DFIMISC);
	writel(dfmisc_conf | DFI_INIT_COMPLETE_EN_MASK, DFIMISC);
	writel(dfmisc_conf | DFI_INIT_COMPLETE_EN_MASK, DFIMISC);

	while (readl(MRSTAT) & MR_WR_BUSY)
		;

	writel(MR_RANK_01, MRCTRL0);
	writel(SW_DONE_MASK, SWCTL);

	while ((readl(SWSTAT) & SW_DONE_ACK_MASK) != SW_DONE_ACK_MASK)
		;

	while ((readl(STAT) & OPERATING_MODE_NORMAL) != OPERATING_MODE_NORMAL)
		;

	writel(0, PWRCTL);
	writel(0, PWRCTL);
	writel(readl(PCTRL_0) | PORT_EN_MASK, PCTRL_0);
	writel(readl(PCTRL_1) | PORT_EN_MASK, PCTRL_1);
	writel(readl(PCTRL_2) | PORT_EN_MASK, PCTRL_2);
}
