// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019-2020 NXP */

#include <hang.h>
#include <asm/arch/s32-gen1/ddrss.h>
#include <linux/kernel.h>
#include <linux/printk.h>

static u16 mailbox_read_mail(void)
{
	u16 mail;

	while (readw(UCTSHADOWREGS) & UCTWRITEPROTSHADOW_MASK)
		;
	mail = readw(UCTWRITEONLYSHADOW);
	writew(0, DCTWRITEPROT);
	while (!(readw(UCTSHADOWREGS) & UCTWRITEPROTSHADOW_MASK))
		;
	writew(DCTWRITEPROT_MASK, DCTWRITEPROT);

	return mail;
}

static int run_firmware(void)
{
	writeb(HDTCTRL_FIRMWARE_COMPLETION, HDTCTRL);
	writew(MICROCONTMUXSEL_MASK, MICROCONTMUXSEL);
	writew(RESETTOMICRO_MASK | STALLTOMICRO_MASK, MICRORESET);
	writew(STALLTOMICRO_MASK, MICRORESET);
	writew(0, MICRORESET);

	return (mailbox_read_mail() == MAIL_TRAINING_SUCCESS);
}

void ddrss_init(struct ddrss_conf *ddrss_conf,
		struct ddrss_firmware *ddrss_firmware)
{
	populate_ddrss_conf(ddrss_conf);

	write_regconf_32(ddrss_conf->ddrc_conf, ddrss_conf->ddrc_conf_length);
	write_regconf_32(ddrss_conf->dq_bswap, ddrss_conf->dq_bswap_length);

	writel(readl(REG_GRP0) | AXI_PARITY_EN_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | AXI_PARITY_TYPE_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | DFI1_ENABLED_MASK, REG_GRP0);
	deassert_ddr_reset();
	writel(0, DBG1);
	writel(0, SWCTL);
	while (readl(SWSTAT) & SW_DONE_ACK_MASK)
		;

	writel(CTL_IDLE_EN_MASK | DIS_DYN_ADR_TRI_MASK, DFIMISC);
	while (readl(DFIMISC) & DFI_INIT_COMPLETE_EN_MASK)
		;

	write_regconf_16(ddrss_conf->ddrphy_conf,
			 ddrss_conf->ddrphy_conf_length);
	write_regconf_16(ddrss_firmware->imem_1d,
			 ddrss_firmware->imem_1d_length);
	write_regconf_16(ddrss_firmware->dmem_1d,
			 ddrss_firmware->dmem_1d_length);

	writel(MICROCONTMUXSEL_MASK, MICROCONTMUXSEL);
	writel((0x1 << PLLCPINTCTRL_OFFSET) |
	       (0x1 << PLLCPPROPCTRL_OFFSET),
	       PLLCTRL1_P0);
	writel(0x24, PLLTESTMODE_P0);
	writel((0x1f << PLLCPINTGSCTRL_OFFSET) |
	       (0x5 << PLLCPPROPGSCTRL_OFFSET),
	       PLLCTRL4_P0);

	writew(DCTWRITEPROT_MASK, DCTWRITEPROT);
	writew(UCTWRITEPROT_MASK, UCTWRITEPROT);

	if (!run_firmware()) {
		pr_err("ddrss: 1D firmware execution failed!\n");
		hang();
	}
	writew(STALLTOMICRO_MASK, MICRORESET);
	write_regconf_16(ddrss_conf->pie, ddrss_conf->pie_length);
	while (readw(CALBUSY) & CALBUSY_MASK)
		;

	writel(0, SWCTL);
	writel(readl(DFIMISC) | DFI_INIT_START_MASK, DFIMISC);
	writel(SW_DONE_MASK, SWCTL);
	while (readl(SWSTAT) != SW_DONE_ACK_MASK)
		;

	writel(readl(DFIMISC) | DFI_INIT_START_MASK, DFIMISC);
	writel(0, SWCTL);
	writel(readl(DFIMISC) & (~DFI_INIT_START_MASK), DFIMISC);
	writel(readl(DFIMISC) | DFI_INIT_COMPLETE_EN_MASK, DFIMISC);
	writel(readl(PWRCTL) & (~SELFREF_SW_MASK), PWRCTL);
	writel(SW_DONE_MASK, SWCTL);
	while (readl(SWSTAT) != SW_DONE_ACK_MASK)
		;
	while ((readl(STAT) & OPERATING_MODE_MASK) != OPERATING_MODE_NORMAL)
		;
	while (readl(DFISTAT) != DFI_INIT_COMPLETE_MASK)
		;

	writel(readl(RFSHCTL3) & (~DIS_AUTO_REFRESH_MASK) , RFSHCTL3);
	writel(readl(PWRCTL) | SELFREF_EN_MASK, PWRCTL);
	writel(readl(PWRCTL) | POWERDOWN_EN_MASK, PWRCTL);
	writel(readl(PWRCTL) | EN_DFI_DRAM_CLK_DISABLE_MASK, PWRCTL);
	writel(readl(PCTRL_0) | PORT_EN_MASK, PCTRL_0);
	writel(readl(PCTRL_1) | PORT_EN_MASK, PCTRL_1);
	writel(readl(PCTRL_2) | PORT_EN_MASK, PCTRL_2);
}
