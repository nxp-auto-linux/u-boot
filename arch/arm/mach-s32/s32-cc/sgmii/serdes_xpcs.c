// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2021 NXP
 *
 * The SerDes module source file.
 */

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>
#include <stdio.h>
#include <linux/delay.h>
#include <linux/printk.h>
#include <linux/ethtool.h>
#include <asm/io.h>

#define XPCS_BASE(xpcs) (((xpcs) == 0)  ? (SERDES_XPCS_0_ADDR2) : \
					  (SERDES_XPCS_1_ADDR2))

u16 serdes_xpcs_read_gen2(void *base, u32 xpcs, u32 reg)
{
	u32 ofsleft = (reg >> 8) & 0xffffU;
	u32 ofsright = (reg & 0xffU);
	u32 pcs_off = XPCS_BASE(xpcs);

	writel(ofsleft, base + pcs_off + 0x3fc);
	return readl(base + pcs_off + 4 * ofsright) & 0xffffU;
}

void serdes_xpcs_write_gen2(void *base, u32 xpcs, u32 reg, u16 val)
{
	u32 ofsleft = (reg >> 8) & 0xffffU;
	u32 ofsright = (reg & 0xffU);
	u32 pcs_off = XPCS_BASE(xpcs);

	writel(ofsleft, base + pcs_off + 0x3fc);
	writel(val, base + pcs_off + 4 * ofsright);
}

void serdes_xpcs_clr_setb_gen2(void *base, u32 xpcs, u32 reg,
			       u16 clr_mask, u16 mask)
{
	u16 tmp_rd =  0;

	if (!base)
		return;

	if (mask || clr_mask)
		tmp_rd = serdes_xpcs_read_gen2(base, xpcs, reg);

	serdes_xpcs_write_gen2(base, xpcs, reg, (tmp_rd & ~clr_mask) | mask);
}

#define PCSW16(serdes_base, pcs, reg, val) ({\
	serdes_xpcs_write_gen2(serdes_base, pcs, reg, val);\
})

#define PCSR16(serdes_base, pcs, reg) ({\
	serdes_xpcs_read_gen2(serdes_base, pcs, reg);\
})

#define PCSBCLR(serdes_base, pcs, reg, mask) \
	serdes_xpcs_clr_setb_gen2(serdes_base, pcs, reg, mask, 0)

#define PCSBSET(serdes_base, pcs, reg, mask) \
	serdes_xpcs_clr_setb_gen2(serdes_base, pcs, reg, 0, mask)

#define PCSBCLRSET(serdes_base, pcs, reg, clr_mask, mask) \
	serdes_xpcs_clr_setb_gen2(serdes_base, pcs, reg, clr_mask, mask)

static int serdes_pcs_wait_bits(void *base, u32 xpcs, u32 reg, u16 mask,
				u16 val, u16 us, u16 cnt)
{
	u32 tmp = cnt; /* Take care so this is not optimized out */

	while ((((serdes_xpcs_read_gen2(base, xpcs, reg) & mask) != val) &&
		(tmp > 0))) {
		udelay(us);
		tmp--;
	}

	return ((tmp > 0)) ? (0) : (-ETIMEDOUT);
}

void serdes_pcs_loopback_enable(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, R2TLBE);
}

void serdes_pcs_loopback_disable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, VR_MII_DIG_CTRL1, R2TLBE);
}

int serdes_pcs_wait_for_power_good(void *base, u32 xpcs)
{
	return serdes_pcs_wait_bits(base, xpcs, VR_MII_DIG_STS,
				    0x7U << 2, 0x4U << 2, 1000U, 1000U);
}

void serdes_pcs_issue_vreset(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, VR_RST);
}

int serdes_pcs_wait_for_vreset(void *base, u32 xpcs)
{
	return serdes_pcs_wait_bits(base, xpcs, VR_MII_DIG_CTRL1,
				    VR_RST, 0, 1000U, 1000U);
}

static void serdes_pcs_set_2500M_mode(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, EN_2_5G_MODE);
}

static void serdes_pcs_set_1000M_mode(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, VR_MII_DIG_CTRL1, EN_2_5G_MODE);
}

int serdes_pcs_speed_select(void *base, u32 xpcs, u32 div)
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

	PCSBCLRSET(base, xpcs, SR_MII_CTRL,
		   MII_CTRL_SS13 | MII_CTRL_SS6,
		   reg16);

	return 0;
}

void serdes_pcs_set_fd(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, SR_MII_CTRL, MII_CTRL_DUPLEX_MODE);
}

void serdes_pcs_set_hd(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, SR_MII_CTRL, MII_CTRL_DUPLEX_MODE);
}

/* Call in case MII bus is in all speeds 8bit */
void serdes_pcs_mii_bus_control_disable(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_AN_CTRL, MII_AN_CTRL_MII_CTRL);
}

/* Call in case MII bus is in 1G 8bit and other speeds 4bit */
void serdes_pcs_mii_bus_control_enable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, VR_MII_AN_CTRL, MII_AN_CTRL_MII_CTRL);
}

void serdes_pcs_an_enable(void *base, u32 xpcs)
{
	/* Select SGMII type AN, enable interrupt */
	PCSBCLRSET(base, xpcs, VR_MII_AN_CTRL,
		   MII_AN_CTRL_PCS_MODE(0x3),
		   MII_AN_CTRL_PCS_MODE(PCS_MODE_SGMII) |
		   MII_AN_INTR_EN);
	/* Enable SGMII AN */
	PCSBSET(base, xpcs, SR_MII_CTRL, MII_CTRL_AN_ENABLE);
}

void serdes_pcs_an_disable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, SR_MII_CTRL, MII_CTRL_AN_ENABLE);
	/* Disable interrupt */
	PCSBCLR(base, xpcs, VR_MII_AN_CTRL, MII_AN_INTR_EN);
}

void serdes_pcs_an_restart(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, SR_MII_CTRL, MII_CTRL_RESTART_AN);
}

void serdes_pcs_an_auto_sw_enable(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, MAC_AUTO_SW);
}

void serdes_pcs_an_auto_sw_disable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, VR_MII_DIG_CTRL1, MAC_AUTO_SW);
}

void serdes_pcs_an_set_link_timer(void *base, u32 xpcs, u16 link_timer)
{
	PCSW16(base, xpcs, VR_MII_LINK_TIMER_CTRL, link_timer);
	PCSBCLR(base, xpcs, VR_MII_DIG_CTRL1, CL37_TMR_OVR_RIDE);
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, CL37_TMR_OVR_RIDE);
}

/* This is intended to be called from AN interrupt to resolve the AN result */
int serdes_pcs_an_decode(void *base, u32 xpcs, bool *link,
			 bool *fduplex, u16 *speed)
{
	u16 reg16 = PCSR16(base, xpcs, VR_MII_AN_INTR_STS);

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

int serdes_pma_wait_link(void *base, u32 xpcs, u8 sec)
{
	return serdes_pcs_wait_bits(base, xpcs, SR_MII_STS,
				    MII_STS_LINK_STS, MII_STS_LINK_STS,
				    1000U, 1000U * sec);
}

void serdes_pma_issue_rx_reset(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_RX_GENCTRL1, RX_RST_0);
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_RX_GENCTRL1, RX_RST_0);
}

void serdes_pma_lane_disable(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_POWER_STATE_CTRL,
		TX_DISABLE_0);
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_RX_POWER_STATE_CTRL,
		RX_DISABLE_0);
}

void serdes_pma_lane_enable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_TX_POWER_STATE_CTRL,
		TX_DISABLE_0);
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_RX_POWER_STATE_CTRL,
		RX_DISABLE_0);
}

void serdes_pma_loopback_enable(void *base, u32 xpcs)
{
	PCSBSET(base, xpcs, SR_MII_CTRL, LBE);
}

void serdes_pma_loopback_disable(void *base, u32 xpcs)
{
	PCSBCLR(base, xpcs, SR_MII_CTRL, LBE);
}

static void serdes_pma_configure_tx_ctr(void *base, u32 xpcs)
{
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_EQ_CTRL0,
		   0x3fU << 8U,
		   0xCU << 8U);
	PCSBCLRSET(base, xpcs, VR_MII_CONSUMER_10G_TX_TERM_CTRL,
		   0x7U,
		   0x4U);
}

static void serdes_pma_1250Mhz_prepare(void *base, u32 xpcs,
				       enum serdes_clock_fmhz fmhz)
{
	u16 vco_cal_ld, vco_cal_ref;

	if (fmhz == CLK_100MHZ) {
		vco_cal_ld = 1350U;
		vco_cal_ref = 27U;
	} else {
		vco_cal_ld = 1360U;
		vco_cal_ref = 17U;
	}
	/* RX VCO calibration value */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
		   0x1fff,
		   vco_cal_ld);

	/* VCO calibration reference */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
		   0x3f,
		   vco_cal_ref);

	/* TX rate baud/4 (baud 1250Mhz) */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
		   0x7,
		   0x2U); /* b010 */

	/* Rx rate baud/8 (baud 1250Mhz) */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
		   0x3U,
		   0x3U); /* b11 */

	/* Clear low-frequency operating band */
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL, VCO_LOW_FREQ_0);
}

/* Call only with 125mhz ref clk */
static void serdes_pma_3125Mhz_prepare(void *base, u32 xpcs,
				       enum serdes_clock_fmhz fmhz)
{
	u16 vco_cal_ld, vco_cal_ref;

	if (fmhz == CLK_100MHZ) {
		vco_cal_ld = 1344U;
		vco_cal_ref = 43U;
	} else {
		vco_cal_ld = 1350U;
		vco_cal_ref = 27U;
	}
	/* RX VCO calibration value */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
		   0x1fff,
		   vco_cal_ld);

	/* VCO calibration reference */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
		   0x3f,
		   vco_cal_ref);

	/* TX rate baud  */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
		   0x7,
		   0x0U);

	/* Rx rate baud/2 */
	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
		   0x3U,
		   0x1U);

	/* Set low-frequency operating band */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL, VCO_LOW_FREQ_0);
}

static void serdes_pma_mplla_start_cal(void *base, u32 xpcs,
				       enum serdes_clock_fmhz fmhz)
{
	if (fmhz == CLK_100MHZ) {
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U) | REF_CLK_DIV2 | REF_MPLLA_DIV2,
			   REF_RANGE(0x3U) | REF_CLK_EN);

		/* Clear multiplier and set it to 25 and enable PPL cal */
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			   MPLLA_MULTIPLIER_VALUE(0xff) |
			   MPLLA_CAL_DISABLE,
			   MPLLA_MULTIPLIER_VALUE(25U));

	} else {
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U),
			   REF_RANGE(0x2U) | REF_CLK_DIV2 |
			   REF_MPLLA_DIV2 | REF_CLK_EN);

		/* Clear multiplier and set it to 80 and enable PPL cal */
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			   MPLLA_MULTIPLIER_VALUE(0xff) | MPLLA_CAL_DISABLE,
			   MPLLA_MULTIPLIER_VALUE(80U));
	}

	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL1, 0xffe0U);

	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL2,
		   MPLLA_TX_CLK_DIV(0x7U),
		   MPLLA_TX_CLK_DIV(1U) | MPLLA_DIV10_CLK_EN);

	if (fmhz == CLK_100MHZ)
		PCSW16(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3, 357U);
	else
		PCSW16(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3, 43U);
}

/* Configure PLLB and start calibration
 * Note: Enable this only with 125Mhz ref !!
 */
static void serdes_pma_mpllb_start_cal(void *base, u32 xpcs,
				       enum serdes_clock_fmhz fmhz)
{
	if (fmhz == CLK_100MHZ) {
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U) | REF_CLK_DIV2 | REF_MPLLB_DIV2,
			   REF_RANGE(0x3U) | REF_CLK_EN);

		/* Clear multiplier and set it to 25 and enable PPL cal */
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			   MPLLB_MULTIPLIER(0xffU) | MPPLB_CAL_DISABLE,
			   MPLLB_MULTIPLIER(0x27U));

	} else {
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			   REF_RANGE(0x7U),
			   REF_RANGE(0x2U) | REF_MPLLB_DIV2 |
			   REF_CLK_DIV2 | REF_CLK_EN);

		/* Clear multiplier and set it to 125 and enable PPL cal */
		PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			   MPLLB_MULTIPLIER(0xffU) | MPPLB_CAL_DISABLE,
			   MPLLB_MULTIPLIER(125U));
	}

	/* Clear the fraction divider */
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL1, 0xffe0U);

	PCSBCLRSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL2,
		   MPLLB_TX_CLK_DIV(0x7U),
		   MPLLB_TX_CLK_DIV(0x5U) | MPLLB_DIV10_CLK_EN);

	if (fmhz == CLK_100MHZ) {
		/* Set fraction divider */
		PCSBSET(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL1, 0x414U << 5U);

		/* PLL bandwidth */
		PCSW16(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL3, 0x66U);
	} else {
		/* PLL bandwidth */
		PCSW16(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL3, 68U);
	}
}

static void serdes_pma_mplla_stop_cal(void *base, u32 xpcs)
{
	/* Disable PLLB calibration */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL0, MPLLA_CAL_DISABLE);
}

static void serdes_pma_mpllb_stop_cal(void *base, u32 xpcs)
{
	/* Disable PLLB calibration */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0, MPPLB_CAL_DISABLE);
}

void serdes_pma_select_plla_ref(void *base, u32 xpcs)
{
	/* Select PLLA */
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLLB_SEL_0);
	/* Enable PLL */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLL_EN_0);
}

void serdes_pma_select_pllb_ref(void *base, u32 xpcs)
{
	/* Select PLLB */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLLB_SEL_0);
	/* Enable PLL */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL, MPLL_EN_0);
}

int serdes_bifurcation_pll_transit(void *base, u32 xpcs, bool plla)
{
	int ret = 0;

	/* Signal that clock are not available */
	PCSBCLR(base, xpcs, VR_MII_GEN5_12G_16G_TX_GENCTRL1, TX_CLK_RDY_0);

	if (plla) {
		/* Request PLLA */
		serdes_pma_select_plla_ref(base, xpcs);
	} else {
		/* Request PLLB */
		serdes_pma_select_pllb_ref(base, xpcs);
	}

	/* Initiate transmitter TX reconfiguration request */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_GENCTRL2, TX_REQ_0);

	/* Wait for transmitter to reconfigure */
	ret = serdes_pcs_wait_bits(base, xpcs, VR_MII_GEN5_12G_16G_TX_GENCTRL2,
				   TX_REQ_0, 0,
				   100U, 100U);
	if (ret)
		pr_err("TX_REQ_0 failed\n");

	/* Initiate transmitter RX reconfiguration request */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_RX_GENCTRL2, RX_REQ_0);

	/* Wait for transmitter to reconfigure */
	ret = serdes_pcs_wait_bits(base, xpcs, VR_MII_GEN5_12G_16G_RX_GENCTRL2,
				   RX_REQ_0, 0,
				   100U, 100U);
	if (ret)
		pr_err("RX_REQ_0 failed\n");

	/* Signal that clock are available */
	PCSBSET(base, xpcs, VR_MII_GEN5_12G_16G_TX_GENCTRL1, TX_CLK_RDY_0);

	/* Flush internal logic */
	PCSBSET(base, xpcs, VR_MII_DIG_CTRL1, INIT);

	/* Wait for init */
	ret = serdes_pcs_wait_bits(base, xpcs, VR_MII_DIG_CTRL1,
				   INIT, 0,
				   100U, 100U);
	if (ret)
		pr_err("INIT failed\n");

	return ret;
}

/* Transit to PLLB */
int serdes_bifurcation_pll_transit_to_3125Mhz(void *base, u32 xpcs,
					      enum serdes_clock_fmhz fmhz)
{
	/* Switch PCS logic to 2.5G */
	serdes_pcs_set_2500M_mode(base, xpcs);

	/* Switch PMA logic to 3.125Ghz */
	serdes_pma_3125Mhz_prepare(base, xpcs, fmhz);

	/* Do the transit to PLLB */
	return serdes_bifurcation_pll_transit(base, xpcs, false);
}

/* Transit to PLLA */
int serdes_bifurcation_pll_transit_to_1250Mhz(void *base, u32 xpcs,
					      enum serdes_clock_fmhz fmhz)
{
	/* Switch PCS logic to 1G */
	serdes_pcs_set_1000M_mode(base, xpcs);

	/* Switch PMA logic to 1.250Ghz */
	serdes_pma_1250Mhz_prepare(base, xpcs, fmhz);

	/* Do the transit PLLA */
	return serdes_bifurcation_pll_transit(base, xpcs, true);
}

void serdes_pcs_pma_init_gen2(void *base, enum serdes_clock_fmhz fmhz,
			      u32 init_flags)
{
	u32 xpcs_phy_ctr = 0;

	if ((init_flags & PHY_CTRL_XPCS_OWNED) != 0) {
		if ((init_flags & PHY_CTRL_XPCS0_OWNED) != 0)
			xpcs_phy_ctr = 0;
		else if ((init_flags & PHY_CTRL_XPCS1_OWNED) != 0)
			xpcs_phy_ctr = 1;
	}

	/* Set bypass flag in case of internal clocks */
	if (((init_flags & PHY_CLK_INT) != 0) &&
	    ((init_flags & (XPCS0_OWNED)) != 0)) {
		PCSBSET(base, 0, VR_MII_DIG_CTRL1, EN_VSMMD1 | BYP_PWRUP);
	} else if ((init_flags & (XPCS0_OWNED)) != 0) {
		PCSBCLRSET(base, 0, VR_MII_DIG_CTRL1, BYP_PWRUP, EN_VSMMD1);
	}

	if (((init_flags & PHY_CLK_INT) != 0) &&
	    ((init_flags & (XPCS1_OWNED)) != 0)) {
		PCSBSET(base, 1, VR_MII_DIG_CTRL1, EN_VSMMD1 | BYP_PWRUP);
	} else if ((init_flags & (XPCS1_OWNED)) != 0) {
		PCSBCLRSET(base, 1, VR_MII_DIG_CTRL1, BYP_PWRUP, EN_VSMMD1);
	}

	if ((init_flags & XPCS0_2500M) != 0) {
		serdes_pma_configure_tx_ctr(base, 0U);
		serdes_pcs_set_2500M_mode(base, 0);
		serdes_pma_select_pllb_ref(base, 0);
	} else if ((init_flags & XPCS0_1000M) != 0) {
		serdes_pma_configure_tx_ctr(base, 0U);
		serdes_pcs_set_1000M_mode(base, 0);
		serdes_pma_select_plla_ref(base, 0);
	}

	if ((init_flags & XPCS1_2500M) != 0) {
		serdes_pma_configure_tx_ctr(base, 1);
		serdes_pcs_set_2500M_mode(base, 1);
		serdes_pma_select_pllb_ref(base, 1);
	} else if ((init_flags & XPCS1_1000M) != 0) {
		serdes_pma_configure_tx_ctr(base, 1);
		serdes_pcs_set_1000M_mode(base, 1);
		serdes_pma_select_plla_ref(base, 1);
	}

	/* Using external clock reference */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PHY_CLK_INT) == 0)
		PCSBSET(base, xpcs_phy_ctr, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			REF_USE_PAD);
	else if ((init_flags & PHY_CTRL_XPCS_OWNED) != 0)
		PCSBCLR(base, xpcs_phy_ctr, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			REF_USE_PAD);

	/* Start PLLA cal */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLA_CAL_EN) != 0) {
		/* Configure PLLA and start calibration */
		serdes_pma_mplla_start_cal(base, xpcs_phy_ctr, fmhz);
	}

	/* Start PLLB cal */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLB_CAL_EN) != 0) {
		serdes_pma_mpllb_start_cal(base, xpcs_phy_ctr, fmhz);
	}

	/* Disable PLLA, if requested */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLA_CAL_DIS) != 0) {
		serdes_pma_mplla_stop_cal(base, xpcs_phy_ctr);
	}

	/* Disable PLLB, if requested */
	if (((init_flags & PHY_CTRL_XPCS_OWNED) != 0) &&
	    (init_flags & PLLB_CAL_DIS) != 0) {
		serdes_pma_mpllb_stop_cal(base, xpcs_phy_ctr);
	}

	if ((init_flags & XPCS0_2500M) != 0)
		serdes_pma_3125Mhz_prepare(base, 0, fmhz);
	else if ((init_flags & XPCS0_1000M) != 0)
		serdes_pma_1250Mhz_prepare(base, 0, fmhz);

	if ((init_flags & XPCS1_2500M) != 0)
		serdes_pma_3125Mhz_prepare(base, 1, fmhz);
	else if ((init_flags & XPCS1_1000M) != 0)
		serdes_pma_1250Mhz_prepare(base, 1, fmhz);

	if ((init_flags & XPCS0_DIS) != 0)
		serdes_pma_lane_disable(base, 0);

	if ((init_flags & XPCS1_DIS) != 0)
		serdes_pma_lane_disable(base, 1);

	/* Clear bypass flag in case of internal clocks */
	if (((init_flags & PHY_CLK_INT) != 0U) &&
	    ((init_flags & (XPCS0_OWNED)) != 0U) &&
	    ((init_flags & XPCS0_DIS) == 0U)) {
		PCSBCLR(base, 0U, VR_MII_DIG_CTRL1, BYP_PWRUP);
	}

	if (((init_flags & PHY_CLK_INT) != 0U) &&
	    ((init_flags & (XPCS1_OWNED)) != 0U) &&
	    ((init_flags & XPCS1_DIS) == 0U)) {
		PCSBCLR(base, 1U, VR_MII_DIG_CTRL1, BYP_PWRUP);
	}
}
