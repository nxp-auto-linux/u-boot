// SPDX-License-Identifier:     GPL-2.0+
/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 * (C) Copyright 2017,2020 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/lpddr2.h>
#include <asm/arch/mmdc.h>
#include <hang.h>

volatile int mscr_offset_ck0;

void ddr_config_iomux(uint8_t module)
{
	int i;

	switch(module) {
		case DDR0:
			mscr_offset_ck0 = SIUL2_MSCRn(_DDR0_CKE0);
			writel(LPDDR2_CLK0_PAD, SIUL2_MSCRn(_DDR0_CLK0));

			writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR0_CKE0));
			writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR0_CKE1));

			writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR0_CS_B0));
			writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR0_CS_B1));

			for (i = _DDR0_DM0; i <= _DDR0_DM3; i++)
				writel(LPDDR2_DMn_PAD, SIUL2_MSCRn(i));

			for (i = _DDR0_DQS0; i <= _DDR0_DQS3; i++)
				writel(LPDDR2_DQSn_PAD, SIUL2_MSCRn(i));

			for (i = _DDR0_A0; i <= _DDR0_A9; i++)
				writel(LPDDR2_An_PAD, SIUL2_MSCRn(i));

			for (i = _DDR0_D0; i <= _DDR0_D31; i++)
				writel(LPDDR2_Dn_PAD, SIUL2_MSCRn(i));
			break;
		case DDR1:
			writel(LPDDR2_CLK0_PAD, SIUL2_MSCRn(_DDR1_CLK0));

			writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR1_CKE0));
			writel(LPDDR2_CKEn_PAD, SIUL2_MSCRn(_DDR1_CKE1));

			writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR1_CS_B0));
			writel(LPDDR2_CS_Bn_PAD, SIUL2_MSCRn(_DDR1_CS_B1));

			for (i = _DDR1_DM0; i <= _DDR1_DM3; i++)
				writel(LPDDR2_DMn_PAD, SIUL2_MSCRn(i));

			for (i = _DDR1_DQS0; i <= _DDR1_DQS3; i++)
				writel(LPDDR2_DQSn_PAD, SIUL2_MSCRn(i));

			for (i = _DDR1_A0; i <= _DDR1_A9; i++)
				writel(LPDDR2_An_PAD, SIUL2_MSCRn(i));

			for (i = _DDR1_D0; i <= _DDR1_D31; i++)
				writel(LPDDR2_Dn_PAD, SIUL2_MSCRn(i));
			break;
	}
}

void config_mmdc(uint8_t module)
{
	unsigned long mmdc_addr = (module)? MMDC1_BASE_ADDR : MMDC0_BASE_ADDR;
	const struct lpddr2_config *config = s32_get_lpddr2_config();

	if (!config) {
		pr_err("DDR: LPDDR2 config not found\n");
		hang();
	}

	writel(MMDC_MDSCR_CFG_VALUE, mmdc_addr + MMDC_MDSCR);

	writel(config->mdcfg0, mmdc_addr + MMDC_MDCFG0);
	writel(config->mdcfg1, mmdc_addr + MMDC_MDCFG1);
	writel(config->mdcfg2, mmdc_addr + MMDC_MDCFG2);
	writel(config->mdcfg3lp, mmdc_addr + MMDC_MDCFG3LP);
	writel(MMDC_MDOTC_VALUE, mmdc_addr + MMDC_MDOTC);
	writel(config->mdmisc, mmdc_addr + MMDC_MDMISC);
	writel(MMDC_MDOR_VALUE, mmdc_addr + MMDC_MDOR);
	writel(config->mdctl, mmdc_addr + MMDC_MDCTL);
	writel(MMDC_MPMUR0_VALUE, mmdc_addr + MMDC_MPMUR0);

	while (readl(mmdc_addr + MMDC_MPMUR0) & MMDC_MPMUR0_FRC_MSR) {}

	/* Perform ZQ calibration */
	writel(MMDC_MPZQLP2CTL_VALUE, mmdc_addr + MMDC_MPZQLP2CTL);
	writel(MMDC_MPZQHWCTRL_VALUE, mmdc_addr + MMDC_MPZQHWCTRL);
	while (readl(mmdc_addr + MMDC_MPZQHWCTRL) & MMDC_MPZQHWCTRL_ZQ_HW_FOR) {}

	/* Enable MMDC with CS0 */
	writel(config->mdctl + 0x80000000, mmdc_addr + MMDC_MDCTL);

	/* Precharge all command per JEDEC */
	/* Ensures robust DRAM initialization */
	writel(MMDC_MDSCR_CS0_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_CS1_VALUE, mmdc_addr + MMDC_MDSCR);

	/* Complete the initialization sequence as defined by JEDEC */
	writel(MMDC_MDSCR_RST_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR1_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(config->mdscr_mr2, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR3_VALUE, mmdc_addr + MMDC_MDSCR);
	writel(MMDC_MDSCR_MR10_VALUE, mmdc_addr + MMDC_MDSCR);

	/* Set the amount of DRAM */
	/* Set DQS settings based on board type */

	switch(module) {
		case MMDC0:
			writel(config->mdasp_module0, mmdc_addr + MMDC_MDASP);
		writel(config->mprddlctl_module0,
		       mmdc_addr + MMDC_MPRDDLCTL);
		writel(config->mpwrdlctl_module0,
		       mmdc_addr + MMDC_MPWRDLCTL);
		writel(MMDC_MPDGCTRL0_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL0);
		writel(MMDC_MPDGCTRL1_MODULE0_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL1);
			break;
		case MMDC1:
			writel(config->mdasp_module1, mmdc_addr + MMDC_MDASP);
		writel(config->mprddlctl_module1,
		       mmdc_addr + MMDC_MPRDDLCTL);
		writel(config->mpwrdlctl_module1,
		       mmdc_addr + MMDC_MPWRDLCTL);
		writel(MMDC_MPDGCTRL0_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL0);
		writel(MMDC_MPDGCTRL1_MODULE1_VALUE,
		       mmdc_addr + MMDC_MPDGCTRL1);
			break;
	}

	writel(MMDC_MDRWD_VALUE, mmdc_addr + MMDC_MDRWD);
	writel(MMDC_MDPDC_VALUE, mmdc_addr + MMDC_MDPDC);
	writel(MMDC_MDREF_VALUE, mmdc_addr + MMDC_MDREF);
	writel(MMDC_MPODTCTRL_VALUE, mmdc_addr + MMDC_MPODTCTRL);
	writel(MMDC_MDSCR_DEASSERT_VALUE, mmdc_addr + MMDC_MDSCR);

}
