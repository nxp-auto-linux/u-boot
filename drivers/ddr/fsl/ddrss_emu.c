// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2019 NXP */

#include <asm/arch/s32-gen1/ddrss.h>
#include <asm/arch/s32-gen1/mc_rgm_regs.h>

#include <common.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <asm/io.h>

static void deassert_ddr_reset(void)
{
	u32 rgm_prst_0;

	rgm_prst_0 = readl(RGM_PRST(0));
	rgm_prst_0 &= ~(BIT(3) | BIT(0));
	writel(rgm_prst_0, RGM_PRST(0));
	while (readl(RGM_PSTAT(0)) != rgm_prst_0)
		;
}

static u16 mailbox_read_mail(void)
{
	u16 mail;

	while (readw(UCTSHADOWREGS) & UCTWRITEPROTSHADOW_MASK)
		;
	mail = readw(UCTWRITEONLYSHADOW);
	writew(0, DCTWRITEPROT);
	while (!(readw(UCTSHADOWREGS) & UCTWRITEPROTSHADOW_MASK))
		;
	writew(1, DCTWRITEPROT);

	return mail;
}

static int run_firmware(void)
{
	writeb(HDTCTRL_FIRMWARE_COMPLETION, HDTCTRL);
	writew(SEQUENCECTRL_RUN_DEVINIT_MASK, SEQUENCECTRL);
	writew(MICROCONTMUXSEL_MASK, MICROCONTMUXSEL);
	writew(RESETTOMICRO_MASK | STALLTOMICRO_MASK, MICRORESET);
	writew(STALLTOMICRO_MASK, MICRORESET);
	writew(0, MICRORESET);

	return (mailbox_read_mail() == MAIL_TRAINING_SUCCESS);
}

static void write_regconf_16(struct regconf *rc, size_t length)
{
	size_t i;

	for (i = 0; i < length; i++)
		writew(rc[i].data, (uintptr_t)(DDRSS_BASE_ADDR + rc[i].addr));
}

static void write_regconf_32(struct regconf *rc, size_t length)
{
	size_t i;

	for (i = 0; i < length; i++)
		writel(rc[i].data, (uintptr_t)(DDRSS_BASE_ADDR + rc[i].addr));
}

void ddrss_init(struct ddrss_conf *ddrss_conf,
		struct ddrss_firmware *ddrss_firmware)
{
	write_regconf_32(ddrss_conf->ddrc_conf, ddrss_conf->ddrc_conf_length);

	writel((0xf << ADDRMAP_ROW_B16_OFFSET), ADDRMAP7);
	writel(readl(INIT0) & ~SKIP_DRAM_INIT_MASK, INIT0);
	writel((0x15 << ADDRMAP_CS_BIT0_OFFSET), ADDRMAP0);
	writel((0x7 << ADDRMAP_ROW_B12_OFFSET) |
	       (0x7 << ADDRMAP_ROW_B13_OFFSET) |
	       (0xf << ADDRMAP_ROW_B14_OFFSET) |
	       (0xf << ADDRMAP_ROW_B15_OFFSET),
	       ADDRMAP6);
	writel(readl(REG_GRP0) | AXI_PARITY_EN_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | AXI_PARITY_TYPE_MASK, REG_GRP0);
	writel(readl(REG_GRP0) | DFI1_ENABLED_MASK, REG_GRP0);
	deassert_ddr_reset();
	writel(0, DBG1);
	writel(0, SWCTL);
	while (readl(SWSTAT) & SW_DONE_ACK_MASK)
		;

	writel(readl(DFIMISC) & ~DFI_INIT_COMPLETE_EN_MASK, DFIMISC);
	while (readl(DFIMISC) & DFI_INIT_COMPLETE_EN_MASK)
		;

	write_regconf_16(ddrss_conf->ddrphy_conf,
			 ddrss_conf->ddrphy_conf_length);
	write_regconf_16(ddrss_conf->skiptrain,
			 ddrss_conf->skiptrain_length);
	writew(readw(PHYINLP3) | PHYINLP3_MASK, PHYINLP3);

	write_regconf_16(ddrss_firmware->imem_1d,
			 ddrss_firmware->imem_1d_length);
	write_regconf_16(ddrss_firmware->dmem_1d,
			 ddrss_firmware->dmem_1d_length);

	writew(0, MICROCONTMUXSEL);
	write_regconf_16(ddrss_conf->message_block_1d,
			 ddrss_conf->message_block_1d_length);

	writew(DCTWRITEPROT_MASK, DCTWRITEPROT);
	writew(UCTWRITEPROT_MASK, UCTWRITEPROT);

	if (!run_firmware()) {
		pr_err("ddrss: 1D firmware execution failed!\n");
		hang();
	}
	writew(STALLTOMICRO_MASK, MICRORESET);
	write_regconf_16(ddrss_conf->pie, ddrss_conf->pie_length);

	writew(0, MICROCONTMUXSEL);
	writew(0xffff, SEQ0BDISABLEFLAG6);
	writew(0, PPTTRAINSETUP_P0);
	writew(0, PPTTRAINSETUP2_P0);
	while (readw(CALBUSY) & CALBUSY_MASK)
		;

	writel(readl(DFIMISC) | DFI_INIT_START_MASK, DFIMISC);
	while (!(readl(DFISTAT) & DFI_INIT_COMPLETE_MASK))
		;

	writel(readl(DFIMISC) & ~DFI_INIT_START_MASK, DFIMISC);
	writel(readl(DFIMISC) | DFI_INIT_COMPLETE_EN_MASK, DFIMISC);
	writel(SW_DONE_MASK, SWCTL);
	while (!(readl(SWSTAT) & SW_DONE_ACK_MASK))
		;
	while ((readl(STAT) & OPERATING_MODE_MASK) != OPERATING_MODE_MASK)
		;

	writel(0, PWRCTL);
	writel(0, PWRCTL);
	writel(readl(PCTRL_0) | PORT_EN_MASK, PCTRL_0);
	writel(readl(PCTRL_1) | PORT_EN_MASK, PCTRL_1);
	writel(readl(PCTRL_2) | PORT_EN_MASK, PCTRL_2);
	writew(MICROCONTMUXSEL_MASK, MICROCONTMUXSEL);
}
