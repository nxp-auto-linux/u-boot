// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2022 NXP
 *
 * The SerDes module source file.
 */

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>
#include <stdio.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/printk.h>

#include <serdes_s32gen1_io.h>
#include <sgmii.h>

#define VPTR(a)		((void *)(uintptr_t)(a))

static struct {
	u32 start;
	u32 end;
} regs[] = {
	{0x1F0000, 0x1F0006},
	{0x1F000F, 0x1F000F},
	{0x1F0708, 0x1F070F},
	{0x1F0710, 0x1F0710},
	{0x1F0710, 0x1F0710},
	{0x1F8000, 0x1F8003},
	{0x1F8005, 0x1F8005},
	{0x1F800A, 0x1F800A},
	{0x1F8010, 0x1F8012},
	{0x1F8015, 0x1F8015},
	{0x1F8018, 0x1F8018},
	{0x1F8020, 0x1F8020},
	{0x1F8030, 0x1F8037},
	{0x1F803C, 0x1F803C},
	{0x1F8040, 0x1F8040},
	{0x1F8050, 0x1F8058},
	{0x1F805C, 0x1F805E},
	{0x1F8060, 0x1F8060},
	{0x1F8064, 0x1F8064},
	{0x1F806B, 0x1F806B},
	{0x1F8070, 0x1F8078},
	{0x1F8091, 0x1F8092},
	{0x1F8096, 0x1F8096},
	{0x1F8098, 0x1F8099},
	{0x1F80A0, 0x1F80A2},
	{0x1F80E1, 0x1F80E1},
};

static u16 serdes_xpcs_read_gen2(void __iomem *base, u32 reg)
{
	u32 ofsleft = (reg >> 8) & 0xffffU;
	u32 ofsright = (reg & 0xffU);

	writel(ofsleft, VPTR(base + 0x3fc));
	return readl(VPTR(base + 4 * ofsright)) & 0xffffU;
}

static void serdes_xpcs_write_gen2(void __iomem *base, u32 reg, u16 val)
{
	u32 ofsleft = (reg >> 8) & 0xffffU;
	u32 ofsright = (reg & 0xffU);

	writel(ofsleft, VPTR(base + 0x3fc));
	writel(val, VPTR(base + 4 * ofsright));
}

static void serdes_xpcs_clr_setb_gen2(void __iomem *base, u32 reg,
				      u16 clr_mask, u16 mask)
{
	u16 tmp_rd =  0;

	if (!base)
		return;

	if (mask || clr_mask)
		tmp_rd = serdes_xpcs_read_gen2(base, reg);

	serdes_xpcs_write_gen2(base, reg, (tmp_rd & ~clr_mask) | mask);
}

#define PCSW16(xpcs_base, reg, val) ({\
	serdes_xpcs_write_gen2(xpcs_base, reg, val);\
})

#define PCSR16(xpcs_base, reg) ({\
	serdes_xpcs_read_gen2(xpcs_base, reg);\
})

#define PCSBCLR(xpcs_base, reg, mask) \
	serdes_xpcs_clr_setb_gen2(xpcs_base, reg, mask, 0)

#define PCSBSET(xpcs_base, reg, mask) \
	serdes_xpcs_clr_setb_gen2(xpcs_base, reg, 0, mask)

#define PCSBCLRSET(xpcs_base, reg, clr_mask, mask) \
	serdes_xpcs_clr_setb_gen2(xpcs_base, reg, clr_mask, mask)

void serdes_pcs_dump_reg(void __iomem *base)
{
	u32 regidx;

	for (regidx = 0; regidx < ARRAY_SIZE(regs); regidx++) {
		u32 regcurr = regs[regidx].start;
		u32 regend = regs[regidx].end;

		do {
			printf("0x%08x => 0x%04x\n",
			       regcurr, PCSR16(base, regcurr));
			regcurr++;
		} while (regcurr <= regend);
	}
}

static int serdes_pcs_wait_bits(void __iomem *base, u32 reg, u16 mask,
				u16 val, u16 us, u16 cnt)
{
	u32 tmp = cnt; /* Take care so this is not optimized out */

	while ((((serdes_xpcs_read_gen2(base, reg) & mask) != val) &&
		(tmp > 0))) {
		udelay(us);
		tmp--;
	}

	return ((tmp > 0)) ? (0) : (-ETIMEDOUT);
}

static void serdes_pma_high_freq_recovery(void __iomem *base)
{
	/* PCS signal protection, PLL railout recovery */
	PCSBSET(base, VR_MII_DBG_CTRL, SUPPRESS_LOS_DET | RX_DT_EN_CTL);
	PCSBSET(base, VR_MII_Gen5_12G_16G_MISC_CTRL0, PLL_CTRL);
}

void serdes_pcs_loopback_enable(void __iomem *base)
{
	PCSBSET(base, VR_MII_DIG_CTRL1, R2TLBE);
}

void serdes_pcs_loopback_disable(void __iomem *base)
{
	PCSBCLR(base, VR_MII_DIG_CTRL1, R2TLBE);
}

int serdes_pcs_wait_for_power_good(void __iomem *base)
{
	return serdes_pcs_wait_bits(base, VR_MII_DIG_STS,
				    0x7U << 2, 0x4U << 2, 1000U, 1000U);
}

void serdes_pcs_issue_vreset(void __iomem *base)
{
	PCSBSET(base, VR_MII_DIG_CTRL1, VR_RST);
}

int serdes_pcs_wait_for_vreset(void __iomem *base)
{
	return serdes_pcs_wait_bits(base, VR_MII_DIG_CTRL1,
				    VR_RST, 0, 1000U, 1000U);
}

static void serdes_pcs_set_2500M_mode(void __iomem *base)
{
	PCSBSET(base, VR_MII_DIG_CTRL1, EN_2_5G_MODE);
}

static void serdes_pcs_set_1000M_mode(void __iomem *base)
{
	PCSBCLR(base, VR_MII_DIG_CTRL1, EN_2_5G_MODE);
}

int serdes_pcs_speed_select(void __iomem *base, u32 div)
{
	u16 reg16 = 0;

	switch (div) {
	case 100:
		break;

	case 10:
		reg16 |= MII_CTRL_SS13;
		break;

	case 1:
		reg16 |= MII_CTRL_SS6;
		break;

	default:
		/*	Unsupported value */
		return -EINVAL;
	}

	PCSBCLRSET(base, SR_MII_CTRL,
		   MII_CTRL_SS13 | MII_CTRL_SS6,
		   reg16);

	return 0;
}

void serdes_pcs_set_fd(void __iomem *base)
{
	PCSBSET(base, SR_MII_CTRL, MII_CTRL_DUPLEX_MODE);
}

void serdes_pcs_set_hd(void __iomem *base)
{
	PCSBCLR(base, SR_MII_CTRL, MII_CTRL_DUPLEX_MODE);
}

/* Call in case MII bus is in all speeds 8bit */
void serdes_pcs_mii_bus_control_disable(void __iomem *base)
{
	PCSBSET(base, VR_MII_AN_CTRL, MII_AN_CTRL_MII_CTRL);
}

/* Call in case MII bus is in 1G 8bit and other speeds 4bit */
void serdes_pcs_mii_bus_control_enable(void __iomem *base)
{
	PCSBCLR(base, VR_MII_AN_CTRL, MII_AN_CTRL_MII_CTRL);
}

void serdes_pcs_an_enable(void __iomem *base)
{
	/* Select SGMII type AN, enable interrupt */
	PCSBCLRSET(base, VR_MII_AN_CTRL,
		   MII_AN_CTRL_PCS_MODE(0x3),
		   MII_AN_CTRL_PCS_MODE(PCS_MODE_SGMII) |
		   MII_AN_INTR_EN);
	/* Enable SGMII AN */
	PCSBSET(base, SR_MII_CTRL, MII_CTRL_AN_ENABLE);
}

void serdes_pcs_an_disable(void __iomem *base)
{
	PCSBCLR(base, SR_MII_CTRL, MII_CTRL_AN_ENABLE);
	/* Disable interrupt */
	PCSBCLR(base, VR_MII_AN_CTRL, MII_AN_INTR_EN);
}

void serdes_pcs_an_restart(void __iomem *base)
{
	PCSBSET(base, SR_MII_CTRL, MII_CTRL_RESTART_AN);
}

void serdes_pcs_an_auto_sw_enable(void __iomem *base)
{
	PCSBSET(base, VR_MII_DIG_CTRL1, MAC_AUTO_SW);
}

void serdes_pcs_an_auto_sw_disable(void __iomem *base)
{
	PCSBCLR(base, VR_MII_DIG_CTRL1, MAC_AUTO_SW);
}

void serdes_pcs_an_set_link_timer(void __iomem *base, u16 link_timer)
{
	PCSW16(base, VR_MII_LINK_TIMER_CTRL, link_timer);
	PCSBCLR(base, VR_MII_DIG_CTRL1, CL37_TMR_OVR_RIDE);
	PCSBSET(base, VR_MII_DIG_CTRL1, CL37_TMR_OVR_RIDE);
}

/* This is intended to be called from AN interrupt to resolve the AN result */
int serdes_pcs_an_decode(void __iomem *base, bool *link,
			 bool *fduplex, u16 *speed)
{
	u16 reg16 = PCSR16(base, VR_MII_AN_INTR_STS);

	if (reg16 & CL37_ANSGM_STS_LINK) {
		*link = true;
	} else {
		*link = false;
		/* Remote link is down Auto-negotiation didn't work*/
		pr_warn("Auto-negotiation wasn't successful\n");
		return -EINVAL;
	}
	*fduplex = false;
	if (reg16 & CL37_ANSGM_STS_FD)
		*fduplex = true;

	switch (CL37_ANSGM_STS_GET_SPEED(reg16)) {
	case CL37_ANSGM_STS_SPEED_10M:
		*speed = 10;
		break;
	case CL37_ANSGM_STS_SPEED_100M:
		*speed = 100;
		break;
	case CL37_ANSGM_STS_SPEED_1000M:
		*speed = 1000;
		break;
	default:
		*speed = 0;
		return -EINVAL;
	}

	return 0;
}

int serdes_pma_wait_link(void __iomem *base, u8 sec)
{
	return serdes_pcs_wait_bits(base, SR_MII_STS,
				    MII_STS_LINK_STS, MII_STS_LINK_STS,
				    1000U, 1000U * sec);
}

void serdes_pma_issue_rx_reset(void __iomem *base)
{
	PCSBSET(base, VR_MII_GEN5_12G_16G_RX_GENCTRL1, RX_RST_0);
	PCSBCLR(base, VR_MII_GEN5_12G_16G_RX_GENCTRL1, RX_RST_0);
}

void serdes_pma_lane_disable(void __iomem *base)
{
	PCSBSET(base, VR_MII_GEN5_12G_16G_TX_POWER_STATE_CTRL,
		TX_DISABLE_0);
	PCSBSET(base, VR_MII_GEN5_12G_16G_RX_POWER_STATE_CTRL,
		RX_DISABLE_0);
}

void serdes_pma_lane_enable(void __iomem *base)
{
	PCSBCLR(base, VR_MII_GEN5_12G_16G_TX_POWER_STATE_CTRL,
		TX_DISABLE_0);
	PCSBCLR(base, VR_MII_GEN5_12G_16G_RX_POWER_STATE_CTRL,
		RX_DISABLE_0);
}

void serdes_pma_loopback_enable(void __iomem *base)
{
	PCSBSET(base, SR_MII_CTRL, LBE);
}

void serdes_pma_loopback_disable(void __iomem *base)
{
	PCSBCLR(base, SR_MII_CTRL, LBE);
}

static void serdes_pma_configure_tx_eq_post(void __iomem *base)
{
	PCSBSET(base, VR_MII_GEN5_12G_16G_TX_EQ_CTRL1, 1u << 6U);
}

static void serdes_pma_configure_tx_ctr(void __iomem *base)
{
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_TX_EQ_CTRL0,
		   0x3fU << 8U,
		   0xCU << 8U);
	PCSBCLRSET(base, VR_MII_CONSUMER_10G_TX_TERM_CTRL,
		   0x7U,
		   0x4U);
}

static void serdes_pma_1250Mhz_prepare(void __iomem *base,
				       unsigned long fmhz)
{
	u16 vco_cal_ld, vco_cal_ref;

	if (fmhz == MHZ_100) {
		vco_cal_ld = 1350U;
		vco_cal_ref = 27U;
	} else {
		vco_cal_ld = 1360U;
		vco_cal_ref = 17U;
	}
	/* RX VCO calibration value */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
		   0x1fff,
		   vco_cal_ld);

	/* VCO calibration reference */
	PCSBCLRSET(base, VR_MII_GEN5_12G_VCO_CAL_REF0,
		   0x3f,
		   vco_cal_ref);

	/* TX rate baud/4 (baud 1250Mhz) */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
		   0x7,
		   0x2U); /* b010 */

	/* Rx rate baud/8 (baud 1250Mhz) */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
		   0x3U,
		   0x3U); /* b11 */

	/* Clear low-frequency operating band */
	PCSBCLR(base, VR_MII_GEN5_12G_16G_CDR_CTRL, VCO_LOW_FREQ_0);
}

/* Call only with 125mhz ref clk */
static void serdes_pma_3125Mhz_prepare(void __iomem *base,
				       unsigned long fmhz)
{
	u16 vco_cal_ld, vco_cal_ref;

	if (fmhz == MHZ_100) {
		vco_cal_ld = 1344U;
		vco_cal_ref = 43U;
	} else {
		vco_cal_ld = 1350U;
		vco_cal_ref = 27U;
	}
	/* RX VCO calibration value */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
		   0x1fff,
		   vco_cal_ld);

	/* VCO calibration reference */
	PCSBCLRSET(base, VR_MII_GEN5_12G_VCO_CAL_REF0,
		   0x3f,
		   vco_cal_ref);

	/* TX rate baud  */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
		   0x7,
		   0x0U);

	/* Rx rate baud/2 */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
		   0x3U,
		   0x1U);

	/* Set low-frequency operating band */
	PCSBSET(base, VR_MII_GEN5_12G_16G_CDR_CTRL, VCO_LOW_FREQ_0);
}

static void serdes_pma_mplla_start_cal(void __iomem *base,
				       unsigned long fmhz)
{
	if (fmhz == MHZ_100) {
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U) | REF_CLK_DIV2 | REF_MPLLA_DIV2,
			   REF_RANGE(0x3U) | REF_CLK_EN);

		/* Clear multiplier and set it to 25 and enable PPL cal */
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			   MPLLA_MULTIPLIER_VALUE(0xff) |
			   MPLLA_CAL_DISABLE,
			   MPLLA_MULTIPLIER_VALUE(25U));

	} else {
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U),
			   REF_RANGE(0x2U) | REF_CLK_DIV2 |
			   REF_MPLLA_DIV2 | REF_CLK_EN);

		/* Clear multiplier and set it to 80 and enable PPL cal */
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			   MPLLA_MULTIPLIER_VALUE(0xff) | MPLLA_CAL_DISABLE,
			   MPLLA_MULTIPLIER_VALUE(80U));
	}

	PCSBCLR(base, VR_MII_GEN5_12G_MPLLA_CTRL1, 0xffe0U);

	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLA_CTRL2,
		   MPLLA_TX_CLK_DIV(0x7U),
		   MPLLA_TX_CLK_DIV(1U) | MPLLA_DIV10_CLK_EN);

	if (fmhz == MHZ_100)
		PCSW16(base, VR_MII_GEN5_12G_MPLLA_CTRL3, 357U);
	else
		PCSW16(base, VR_MII_GEN5_12G_MPLLA_CTRL3, 43U);
}

/* Configure PLLB and start calibration
 * Note: Enable this only with 125Mhz ref !!
 */
static void serdes_pma_mpllb_start_cal(void __iomem *base,
				       unsigned long fmhz)
{
	if (fmhz == MHZ_100) {
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U) | REF_CLK_DIV2 | REF_MPLLB_DIV2,
			   REF_RANGE(0x3U) | REF_CLK_EN);

		/* Clear multiplier and set it to 25 and enable PPL cal */
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			   MPLLB_MULTIPLIER(0xffU) | MPPLB_CAL_DISABLE,
			   MPLLB_MULTIPLIER(0x27U));

	} else {
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U),
			   REF_RANGE(0x2U) | REF_MPLLB_DIV2 |
			   REF_CLK_DIV2 | REF_CLK_EN);

		/* Clear multiplier and set it to 125 and enable PPL cal */
		PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			   MPLLB_MULTIPLIER(0xffU) | MPPLB_CAL_DISABLE,
			   MPLLB_MULTIPLIER(125U));
	}

	/* Clear the fraction divider */
	PCSBCLR(base, VR_MII_GEN5_12G_MPLLB_CTRL1, 0xffe0U);

	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_MPLLB_CTRL2,
		   MPLLB_TX_CLK_DIV(0x7U),
		   MPLLB_TX_CLK_DIV(0x5U) | MPLLB_DIV10_CLK_EN);

	if (fmhz == MHZ_100) {
		/* Set fraction divider */
		PCSBSET(base, VR_MII_GEN5_12G_MPLLB_CTRL1, 0x414U << 5U);

		/* PLL bandwidth */
		PCSW16(base, VR_MII_GEN5_12G_MPLLB_CTRL3, 0x66U);
	} else {
		/* PLL bandwidth */
		PCSW16(base, VR_MII_GEN5_12G_MPLLB_CTRL3, 68U);
	}
}

static void serdes_pma_mplla_stop_cal(void __iomem *base)
{
	/* Disable PLLB calibration */
	PCSBSET(base, VR_MII_GEN5_12G_16G_MPLLA_CTRL0, MPLLA_CAL_DISABLE);
}

static void serdes_pma_mpllb_stop_cal(void __iomem *base)
{
	/* Disable PLLB calibration */
	PCSBSET(base, VR_MII_GEN5_12G_16G_MPLLB_CTRL0, MPPLB_CAL_DISABLE);
}

static void serdes_pma_select_plla_ref(void __iomem *base)
{
	/* Select PLLA */
	PCSBCLR(base, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLLB_SEL_0);
	/* Enable PLL */
	PCSBSET(base, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLL_EN_0);
}

static void serdes_pma_select_pllb_ref(void __iomem *base)
{
	/* Select PLLB */
	PCSBSET(base, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLLB_SEL_0);
	/* Enable PLL */
	PCSBSET(base, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLL_EN_0);
}

static int serdes_bifurcation_pll_transit(void __iomem *base, bool plla)
{
	int ret = 0;

	/* Signal that clock are not available */
	PCSBCLR(base, VR_MII_GEN5_12G_16G_TX_GENCTRL1, TX_CLK_RDY_0);

	if (plla) {
		/* Request PLLA */
		serdes_pma_select_plla_ref(base);
	} else {
		/* Request PLLB */
		serdes_pma_select_pllb_ref(base);
	}

	/* Initiate transmitter TX reconfiguration request */
	PCSBSET(base, VR_MII_GEN5_12G_16G_TX_GENCTRL2, TX_REQ_0);

	/* Wait for transmitter to reconfigure */
	ret = serdes_pcs_wait_bits(base, VR_MII_GEN5_12G_16G_TX_GENCTRL2,
				   TX_REQ_0, 0,
				   100U, 100U);
	if (ret)
		pr_err("TX_REQ_0 failed\n");

	/* Initiate transmitter RX reconfiguration request */
	PCSBSET(base, VR_MII_GEN5_12G_16G_RX_GENCTRL2, RX_REQ_0);

	/* Wait for transmitter to reconfigure */
	ret = serdes_pcs_wait_bits(base, VR_MII_GEN5_12G_16G_RX_GENCTRL2,
				   RX_REQ_0, 0,
				   100U, 100U);
	if (ret)
		pr_err("RX_REQ_0 failed\n");

	/* Signal that clock are available */
	PCSBSET(base, VR_MII_GEN5_12G_16G_TX_GENCTRL1, TX_CLK_RDY_0);

	/* Flush internal logic */
	PCSBSET(base, VR_MII_DIG_CTRL1, INIT);

	/* Wait for init */
	ret = serdes_pcs_wait_bits(base, VR_MII_DIG_CTRL1,
				   INIT, 0,
				   100U, 100U);
	if (ret)
		pr_err("INIT failed\n");

	return ret;
}

/* Transit to PLLB */
int serdes_bifurcation_pll_transit_to_3125mhz(void __iomem *base,
					      unsigned long fmhz)
{
	/* Switch PCS logic to 2.5G */
	serdes_pcs_set_2500M_mode(base);

	/* Switch PMA logic to 3.125Ghz */
	serdes_pma_3125Mhz_prepare(base, fmhz);

	/* Do the transit to PLLB */
	return serdes_bifurcation_pll_transit(base, false);
}

/* Transit to PLLA */
int serdes_bifurcation_pll_transit_to_1250mhz(void __iomem *base,
					      unsigned long fmhz)
{
	/* Switch PCS logic to 1G */
	serdes_pcs_set_1000M_mode(base);

	/* Switch PMA logic to 1.250Ghz */
	serdes_pma_1250Mhz_prepare(base, fmhz);

	/* Do the transit PLLA */
	return serdes_bifurcation_pll_transit(base, true);
}

void serdes_pma_mode5(void __iomem *base)
{
	if (serdes_pcs_wait_for_power_good(base))
		pr_err("XPCS (%p) power-up failed\n", base);
	/* Configure equlaization */
	serdes_pma_configure_tx_eq_post(base);
	/* Configure transmit eq and termination */
	serdes_pma_configure_tx_ctr(base);
	/* Reconfigure PCS to 2.5Gbps */
	serdes_pcs_set_2500M_mode(base);
	/* Enable receiver recover */
	serdes_pma_high_freq_recovery(base);
}

void serdes_pcs_mode5(void __iomem *base)
{
	/* Enable volatge boost */
	PCSBSET(base, VR_MII_GEN5_12G_16G_TX_GENCTRL1, VBOOST_EN_0);

	/* TX rate baud  */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
		   0x7,
		   0x0U);

	/* Rx rate baud/2 */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
		   0x3U,
		   0x1U);

	/* Set low-frequency operating band */
	PCSBCLRSET(base, VR_MII_GEN5_12G_16G_CDR_CTRL,
		   CDR_SSC_EN_0, VCO_LOW_FREQ_0);

	/* Reconfigure PHY */
	serdes_bifurcation_pll_transit(base, false);

	/* Now do cold reset */
	/* Issue PCIe cold reset to restart PHY and
	 * commit new parameters to PHY
	 */
	BSET32(UPTR(base) + SS_RST_CTRL, COLD_RST);
	udelay(1000U);
	BCLR32(UPTR(base) + SS_RST_CTRL, COLD_RST);

	/* Wait for vendor specific reset */
	serdes_pcs_wait_for_vreset(base);
}

void serdes_pcs_pma_init_gen2(void __iomem *xpcs0, void __iomem *xpcs1,
			      unsigned long fmhz,
			      u32 init_flags)
{
	void __iomem *selected_xpcs = xpcs0;

	if ((init_flags & PHY_CTRL_XPCS_OWNED) != 0) {
		if ((init_flags & PHY_CTRL_XPCS0_OWNED) != 0)
			selected_xpcs = xpcs0;
		else if ((init_flags & PHY_CTRL_XPCS1_OWNED) != 0)
			selected_xpcs = xpcs1;
	}

	/* Set bypass flag in case of internal clocks */
	if (((init_flags & PHY_CLK_INT) != 0) &&
	    ((init_flags & (XPCS0_OWNED)) != 0)) {
		PCSBSET(xpcs0, VR_MII_DIG_CTRL1, EN_VSMMD1 | BYP_PWRUP);
	} else if ((init_flags & (XPCS0_OWNED)) != 0) {
		PCSBCLRSET(xpcs0, VR_MII_DIG_CTRL1, BYP_PWRUP, EN_VSMMD1);
	}

	if (((init_flags & PHY_CLK_INT) != 0) &&
	    ((init_flags & (XPCS1_OWNED)) != 0)) {
		PCSBSET(xpcs1, VR_MII_DIG_CTRL1, EN_VSMMD1 | BYP_PWRUP);
	} else if ((init_flags & (XPCS1_OWNED)) != 0) {
		PCSBCLRSET(xpcs1, VR_MII_DIG_CTRL1, BYP_PWRUP, EN_VSMMD1);
	}

	if ((init_flags & XPCS0_2500M) != 0) {
		serdes_pma_configure_tx_ctr(xpcs0);
		serdes_pcs_set_2500M_mode(xpcs0);
		serdes_pma_select_pllb_ref(xpcs0);
	} else if ((init_flags & XPCS0_1000M) != 0) {
		serdes_pma_configure_tx_ctr(xpcs0);
		serdes_pcs_set_1000M_mode(xpcs0);
		serdes_pma_select_plla_ref(xpcs0);
	}

	if ((init_flags & XPCS1_2500M) != 0) {
		serdes_pma_configure_tx_ctr(xpcs1);
		serdes_pcs_set_2500M_mode(xpcs1);
		serdes_pma_select_pllb_ref(xpcs1);
	} else if ((init_flags & XPCS1_1000M) != 0) {
		serdes_pma_configure_tx_ctr(xpcs1);
		serdes_pcs_set_1000M_mode(xpcs1);
		serdes_pma_select_plla_ref(xpcs1);
	}

	/* Using external clock reference */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PHY_CLK_INT) == 0)
		PCSBSET(selected_xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			REF_USE_PAD);
	else if ((init_flags & PHY_CTRL_XPCS_OWNED) != 0)
		PCSBCLR(selected_xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			REF_USE_PAD);

	/* Start PLLA cal */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLA_CAL_EN) != 0) {
		/* Configure PLLA and start calibration */
		serdes_pma_mplla_start_cal(selected_xpcs, fmhz);
	}

	/* Start PLLB cal */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLB_CAL_EN) != 0) {
		serdes_pma_mpllb_start_cal(selected_xpcs, fmhz);
	}

	/* Disable PLLA, if requested */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLA_CAL_DIS) != 0) {
		serdes_pma_mplla_stop_cal(selected_xpcs);
	}

	/* Disable PLLB, if requested */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLB_CAL_DIS) != 0) {
		serdes_pma_mpllb_stop_cal(selected_xpcs);
	}

	if ((init_flags & XPCS0_2500M) != 0)
		serdes_pma_3125Mhz_prepare(xpcs0, fmhz);
	else if ((init_flags & XPCS0_1000M) != 0)
		serdes_pma_1250Mhz_prepare(xpcs0, fmhz);

	if ((init_flags & XPCS1_2500M) != 0)
		serdes_pma_3125Mhz_prepare(xpcs1, fmhz);
	else if ((init_flags & XPCS1_1000M) != 0)
		serdes_pma_1250Mhz_prepare(xpcs1, fmhz);

	if ((init_flags & XPCS0_DIS) != 0)
		serdes_pma_lane_disable(xpcs0);

	if ((init_flags & XPCS1_DIS) != 0)
		serdes_pma_lane_disable(xpcs1);

	/* Clear bypass flag in case of internal clocks */
	if (((init_flags & PHY_CLK_INT) != 0U) &&
	    ((init_flags & (XPCS0_OWNED)) != 0U) &&
	    ((init_flags & XPCS0_DIS) == 0U)) {
		PCSBCLR(xpcs0, VR_MII_DIG_CTRL1, BYP_PWRUP);
	}

	if (((init_flags & PHY_CLK_INT) != 0U) &&
	    ((init_flags & (XPCS1_OWNED)) != 0U) &&
	    ((init_flags & XPCS1_DIS) == 0U)) {
		PCSBCLR(xpcs1, VR_MII_DIG_CTRL1, BYP_PWRUP);
	}
}
