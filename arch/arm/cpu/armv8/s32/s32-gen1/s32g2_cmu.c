// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
 */

#include <asm/arch/siul.h>
#include <cmu.h>

static struct cmu cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC_CLK, FXOSC_FREQ),
	FXOSC_PERIPH_CMU_FM(1, FIRC_CLK, FIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC_CLK, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF_CLK, 40),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF_CLK, 40),
	FIRC_PERIPH_CMU_FC(0, FXOSC_CLK, FXOSC_FREQ),
	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(6, XBAR_CLK_M7_0, 400),
	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(8, XBAR_CLK_M7_1, 400),
	FIRC_PERIPH_CMU_FC(9, XBAR_CLK_M7_2, 400),
	FIRC_PERIPH_CMU_FC(10, PER_CLK, 80),
	FXOSC_PERIPH_CMU_FC(11, SERDES_REF_CLK, 125),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE_CLK, 80),
	FXOSC_PERIPH_CMU_FC(14, GMAC_0_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(15, GMAC_TS_CLK, 200),
	FXOSC_PERIPH_CMU_FC(16, LIN_CLK, 125),
	FXOSC_PERIPH_CMU_FC(17, QSPI_1X_CLK, 200),
	FXOSC_PERIPH_CMU_FC(18, SDHC_CLK, 400),
	FIRC_PERIPH_CMU_FC(20, DDR_CLK, 666.66),
	FXOSC_PERIPH_CMU_FC(21, GMAC_0_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(22, SPI_CLK, 100),
	FXOSC_PERIPH_CMU_FC(27, A53_CORE_CLK, 1000),
	FIRC_PERIPH_CMU_FC(28, A53_CORE_CLK, 1000),
	FXOSC_PERIPH_CMU_FC(39, PFE_SYS_CLK, 300),
	FXOSC_PERIPH_CMU_FC(46, PFE_MAC_0_TX_CLK, 312.5),
	FXOSC_PERIPH_CMU_FC(47, PFE_MAC_0_RX_CLK, 312.5),
	FXOSC_PERIPH_CMU_FC(48, PFE_MAC_1_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(49, PFE_MAC_1_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(50, PFE_MAC_2_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(51, PFE_MAC_2_RX_CLK, 125),

};

void cmu_fixup(void)
{
#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
	if (is_s32gen1_soc_rev1())
		cmu_blocks[17].mon_freq = 133.33;
#endif
}

struct cmu *get_cmu_block(int index)
{
	return &cmu_blocks[index];
}

int get_cmu_blocks_number(void)
{
	return ARRAY_SIZE(cmu_blocks);
}