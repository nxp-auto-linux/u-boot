// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2020 NXP
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

/**
 * @brief	Variables for XPCS indirect access
 */
typedef struct __serdes_xpcs_access_vars_tag {
	u32 ofsleft;
	u32 ofsright;
	u32 addr1;
	u32 data1;
	u32 addr2;
	u32 data2;
} serdes_xpcs_access_vars_t;

/**
 * @brief		Get variables needed for indirect XPCS access
 */
static int serdes_get_xpcs_access_vars(u32 xpcs, u32 reg,
					   serdes_xpcs_access_vars_t *vars)
{
	vars->ofsleft = (reg >> 8) & 0xffffU;
	vars->ofsright = (reg & 0xffU);
	vars->data1 = vars->ofsleft;

	if (SERDES_XPCS_0_BASE == xpcs) {
		vars->addr1 = SERDES_XPCS_0_ADDR1;
		vars->addr2 = SERDES_XPCS_0_ADDR2 + (vars->ofsright * 4U);
	} else if (SERDES_XPCS_1_BASE == xpcs) {
		vars->addr1 = SERDES_XPCS_1_ADDR1;
		vars->addr2 = SERDES_XPCS_1_ADDR2 + (vars->ofsright * 4U);
	} else {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief			Read XPCS register
 * @param[in]		base SerDes base address
 * @param[in]		xpcs XPCS offset within SerDes memory space
 * @param[in]		reg XPCS register address
 * @param[in,out]	val The XPCS register value
 * @return		0 if success, error code otherwise
 */
static void serdes_xpcs_reg_read(void *base, u32 xpcs, u32 reg,
				 volatile u16 *val)
{
	int ret;
	serdes_xpcs_access_vars_t vars = {0U};

	ret = serdes_get_xpcs_access_vars(xpcs, reg, &vars);
	if (ret)
		pr_warn("Can't read XPCS register (0x%x)\n", reg);

	writel(vars.data1, (phys_addr_t)base + vars.addr1);
	*val = readl((phys_addr_t)base + vars.addr2) & 0xffffU;
}

/**
 * @brief	Write XPCS register
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	reg XPCS register address
 * @param[in]	val The XPCS register value
 * @return	0 if success, error code otherwise
 */
static void serdes_xpcs_reg_write(void *base, u32 xpcs, u32 reg, u16 val)
{
	int ret;
	serdes_xpcs_access_vars_t vars = {0U};

	ret = serdes_get_xpcs_access_vars(xpcs, reg, &vars);
	if (ret)
		pr_warn("Can't write XPCS register (0x%x)\n", reg);

	writel(vars.data1, (phys_addr_t)base + vars.addr1);
	writel(val, (phys_addr_t)base + vars.addr2);
}

/**
 * @brief	Wait until XPCS power-up sequence state "Power_Good"
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @return	0 Power-up sequence state is "Power_Good"
 */
int serdes_xpcs_wait_for_power_good(void *base, u32 xpcs)
{
	u16 reg16;
	u8 pseq;
	int timeout = 1000U;

	do {
		serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_STS, &reg16);
		pseq = (reg16 >> 2) & 0x7U;
		if (pseq == 0x4U)
			break;
		timeout--;
		udelay(1000U);
	} while (timeout > 0);

	if (timeout > 0U)
		return 0;

	return -ETIMEDOUT;
}

/**
 * @brief	Wait until XPCS reset is cleared
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @return	0 Power-up sequence state is "Power_Good"
 */
static int serdes_xpcs_wait_for_reset(void *base, u32 xpcs)
{
	volatile u16 reg16;
	u8 pseq;
	u32 timeout = 1000U;

	do {
		serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
		pseq = reg16 & VR_RST;
		if (pseq == 0x0U)
			break;
		timeout--;
		udelay(1000U);
	} while (timeout > 0U);

	if (timeout > 0U)
		return 0;

	return -ETIMEDOUT;
}

/**
 * @brief		Set SGMII speed
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	mbps Speed in [Mbps]
 * @param[in]	fduplex Full duplex = TRUE, Half duplex = FALSE
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_sgmii_speed(void *base, u32 xpcs, u32 mbps,
				bool fduplex)
{
	u16 reg16;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	/*	Update control register (+ disable AN) */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);
	reg16 &= ~(MII_CTRL_SS13 | MII_CTRL_SS6 | MII_CTRL_DUPLEX_MODE
		   | MII_CTRL_AN_ENABLE);

	switch (mbps) {
	case SPEED_10:
		break;

	case SPEED_100:
		reg16 |= MII_CTRL_SS13;
		break;

	case SPEED_1000:
		reg16 |= MII_CTRL_SS6;
		break;

	default:
		/*	Unsupported value */
		return -EINVAL;
	}

	if (fduplex)
		reg16 |= MII_CTRL_DUPLEX_MODE;

	/*	Write the control register */
	serdes_xpcs_reg_write(base, xpcs, SR_MII_CTRL, reg16);

	return 0;
}

/**
 * @brief	Get SGMII speed
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 *		SERDES_XPCS_0_BASE or SERDES_XPCS_1_BASE
 * @param[in]	mbps Speed in [Mbps]
 * @param[in]	duplex Full duplex = TRUE, Half duplex = FALSE
 * @param[in]	an Auto-neg enabled = TRUE, Auto-neg disabled = FALSE
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_get_sgmii_speed(void *base, u32 xpcs, int *mbps,
				bool *fduplex, bool *an)
{
	u16 reg16;

	if (xpcs != SERDES_XPCS_0_BASE && xpcs != SERDES_XPCS_1_BASE)
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);

	*mbps = SPEED_10;
	*fduplex = false;
	*an = false;

	if (reg16 & MII_CTRL_SS13)
		*mbps = SPEED_100;

	if (reg16 & MII_CTRL_SS6)
		*mbps = SPEED_1000;

	if ((reg16 & MII_CTRL_SS6) && (reg16 & MII_CTRL_SS13))
		return -EINVAL;

	if (reg16 & MII_CTRL_DUPLEX_MODE)
		*fduplex = true;

	if (reg16 & MII_CTRL_AN_ENABLE)
		*an = true;

	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);
	if (reg16 & EN_2_5G_MODE) {
		*mbps = SPEED_2500;
		/* Auto-neg not supported in 2.5G mode */
		*an = false;
	}

	return 0;
}

int serdes_xpcs_set_loopback(void *base, u32 xpcs, bool enable)
{
	u16 reg16;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_CTRL, &reg16);

	/*	Update control register (+ manage LBE) */
	if (enable)
		reg16 |= R2TLBE;
	else
		reg16 &= ~R2TLBE;

	/*	Write the control register */
	serdes_xpcs_reg_write(base, xpcs, SR_MII_CTRL, reg16);

	return 0;
}

/**
 * @brief	Wait for PCS link
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	timeout Timeout in [s]
 * @return	0 link is up, error code otherwise
 */
int serdes_wait_for_link(void *base, u32 xpcs, u8 timeout)
{
	/*	Number of 100ms periods */
	u32 tout = (1000U * timeout) / 100U;
	u16 reg16;

	do {
		serdes_xpcs_reg_read(base, xpcs, SR_MII_STS, &reg16);
		if (0U != (reg16 & MII_STS_LINK_STS))
			break;
		tout--;
		udelay(100000U);
	} while (tout > 0U);

	if (0U == tout)
		return -ETIMEDOUT;

	return 0;
}


/**
 * @brief	Configure XPCS to 1G mode with respect to reference clock
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	ext_ref If reference clock is taken via pads then this shall be
 *		TRUE. If internal reference clock is used then use FALSE.
 * @param[in]	ref_mhz Reference clock frequency in [MHz]. 100 or 125.
 * @param[in]	bypass If true bypass initialization checks in case of ext_ref
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_1000_mode(void *base, u32 xpcs,
			      enum serdes_clock clktype,
			      enum serdes_clock_fmhz fmhz,
			      bool bypass)
{
	int retval = 0;
	u16 reg16, use_pad = 0U;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	if (clktype == CLK_EXT) {
		/*	Using external clock reference */
		use_pad = REF_CLK_CTRL_REF_USE_PAD;
	}

	/* Set bypass flag in case of internal clocks */
	if (clktype == CLK_INT)
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
				      EN_VSMMD1 | BYP_PWRUP);

/* Currently this can't be enabled due to issue in bifurcation modes */
#ifdef S32G_XPCS_ENABLE_PRECHECKS
	if (!(bypass && clktype == CLK_EXT)) {
		/*	Wait for XPCS power up */
		retval = serdes_xpcs_wait_for_power_good(base,
							 xpcs);
		if (retval)
			/*	XPCS power-up failed */
			return retval;

		/*	Compatibility check */
		serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID1,
				     &reg16);
		if (reg16 != 0x7996U)
			/*	Unexpected XPCS ID */
			return -EINVAL;

		serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID2,
				     &reg16);
		if (reg16 != 0xced0U)
			/*	Unexpected XPCS ID */
			return -EINVAL;
	}
#endif

	/*	(Switch to 1G mode: #1) */
	if (clktype == CLK_INT)
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
				      EN_VSMMD1 | BYP_PWRUP);
	else
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
				      EN_VSMMD1);
	/*	(Switch to 1G mode: #2) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DBG_CTRL, 0U);
	/*	(Switch to 1G mode: #3) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL,
			      MPLL_CMN_CTRL_MPLL_EN_0);

	if (fmhz == CLK_100MHZ) {
		/*	RefClk = 100MHz */
		/*	(Switch to 1G mode: #4) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
				      REF_CLK_CTRL_REF_RANGE(3U)
				      | use_pad
				      | REF_CLK_CTRL_REF_CLK_EN);
		/*	(Switch to 1G mode: #5) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
				      MPLLA_MULTIPLIER_VALUE(25U));
	} else {
		/*	RefClk = 125MHz */
		/*	(Switch to 1G mode: #4) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
				      REF_CLK_CTRL_REF_RANGE(2U)
				      | use_pad
				      | REF_CLK_CTRL_REF_MPLLA_DIV2
				      | REF_CLK_CTRL_REF_CLK_DIV2
				      | REF_CLK_CTRL_REF_CLK_EN);
		/*	(Switch to 1G mode: #5) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
				      MPLLA_MULTIPLIER_VALUE(80U));
	}

	/*	(Switch to 1G mode: #6) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL1, 0U);

	/*	(Switch to 1G mode: #7) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL2,
			      MPLLA_TX_CLK_DIV(1U) | MPLLA_DIV10_CLK_EN);

	if (fmhz == CLK_100MHZ) {
		/*	RefClk = 100MHz */
		/*	(Switch to 1G mode: #8) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3,
				      357U);
		/*	(Switch to 1G mode: #9) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_VCO_CAL_LD0, 1350U);
		/*	(Switch to 1G mode: #10) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
				      27U);
	} else {
		/*	RefClk = 125MHz */
		/*	(Switch to 1G mode: #8) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLA_CTRL3,
				      43U);
		/*	(Switch to 1G mode: #9) */
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_GEN5_12G_16G_VCO_CAL_LD0, 1360U);
		/*	(Switch to 1G mode: #10) */
		serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0,
				      17U);
	}

	/*	(Switch to 1G mode: #11) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL,
			      0x2U); /* b010 */
	/*	(Switch to 1G mode: #12) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
			      0x3U); /* b11 */
	/*	(Switch to 1G mode: #13) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL,
			      0x1U); /* VCO_LOW_FREQ_0=0 + CDR_TRACK_ENA=1 */
	/*	(Switch to 1G mode: #14) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			      MPPLB_CAL_DISABLE|0x7dU);
			      /* CAL_DISABLE=1, MPPLB_MULTIPLIER=default */

	/*	(Switch to 1G mode: #15) */
	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);

	/* Clear bypass flag in case of internal clocks */
	if (clktype == CLK_INT) {
		reg16 &= ~BYP_PWRUP;
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16);
	}
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16 | VR_RST);

	/* Issue reset */
	/*	(Switch to 1G mode: #16) */
	if (serdes_xpcs_wait_for_reset(base, xpcs))
		pr_err("XPCS pre power-up soft reset failed\n");

	/*	Wait for XPCS power up */
	pr_debug("Waiting for XPCS power-up\n");
	if (serdes_xpcs_wait_for_power_good(base, xpcs)) {
		pr_err("XPCS power-up failed\n");
		return -EXIT_FAILURE;
	}

	return retval;
}

/**
 * @brief	Configure XPCS to 2.5G mode with respect to reference clock
 * @param[in]	base SerDes base address
 * @param[in]	xpcs XPCS offset within SerDes memory space
 * @param[in]	clktype If reference clock external/internal
 * @param[in]	fmhz Reference clock frequency
 * @return	0 if success, error code otherwise
 */
int serdes_xpcs_set_2500_mode(void *base, u32 xpcs,
			      enum serdes_clock clktype,
			      enum serdes_clock_fmhz fmhz)
{
	int retval;
	u16 reg16, use_pad = 0U;

	if ((SERDES_XPCS_0_BASE != xpcs) && (SERDES_XPCS_1_BASE != xpcs))
		return -EINVAL;

	if (fmhz != CLK_125MHZ)
		return -EINVAL;

	if (clktype == CLK_EXT) {
		/*	Using external clock reference */
		use_pad = REF_CLK_CTRL_REF_USE_PAD;
	}

	/* Set bypass flag in case of internal clocks */
	if (clktype == CLK_INT)
		serdes_xpcs_reg_write(base, xpcs,
				      VR_MII_DIG_CTRL1, EN_VSMMD1 | BYP_PWRUP);

	/*	Wait for XPCS power up */
	retval = serdes_xpcs_wait_for_power_good(base, xpcs);
	if (retval)
		/*	XPCS power-up failed */
		return retval;

	/*	Compatibility check */
	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID1, &reg16);
	if (0x7996U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	serdes_xpcs_reg_read(base, xpcs, SR_MII_DEV_ID2, &reg16);
	if (0xced0U != reg16)
		/*	Unexpected XPCS ID */
		return -EINVAL;

	/*	(Switch to 2.5G mode: #1) */
	if (clktype == CLK_INT)
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
				      EN_VSMMD1 | EN_2_5G_MODE | BYP_PWRUP);
	else
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1,
				      EN_VSMMD1 | EN_2_5G_MODE);
	/*	(Switch to 2.5G mode: #2) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DBG_CTRL, 0U);
	/*	(Switch to 2.5G mode: #3) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL,
			      MPLL_CMN_CTRL_MPLL_EN_0
			      | MPLL_CMN_CTRL_MPLLB_SEL_0);
	/*	(Switch to 2.5G mode: #4) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_REF_CLK_CTRL,
			      REF_CLK_CTRL_REF_MPLLB_DIV2
			      | use_pad
			      | REF_CLK_CTRL_REF_RANGE(2U)
			      | REF_CLK_CTRL_REF_CLK_DIV2);
	/*	(Switch to 2.5G mode: #5) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL0,
			      MPLLB_MULTIPLIER(125U));
	/*	(Switch to 2.5G mode: #6) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL1, 0U);
	/*	(Switch to 2.5G mode: #7) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLB_CTRL2,
			      MPLLB_CTRL2_MPLLB_TX_CLK_DIV(5U)
			      | MPLLB_CTRL2_MPLLB_DIV10_CLK_EN);
	/*	(Switch to 2.5G mode: #8) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_MPLLB_CTRL3, 68U);
	/*	(Switch to 2.5G mode: #9) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_VCO_CAL_LD0,
			      1350U);
	/*	(Switch to 2.5G mode: #10) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_VCO_CAL_REF0, 27U);
	/*	(Switch to 2.5G mode: #11) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_TX_RATE_CTRL, 0U);
	/*	(Switch to 2.5G mode: #12) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_RX_RATE_CTRL,
			      0x1U); /* b01 */
	/*	(Switch to 2.5G mode: #13) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_CDR_CTRL,
			      CDR_CTRL_VCO_LOW_FREQ_0 | 0x1U);
			      /* +CDR_TRACKING_ENABLE=1 */
	/*	(Switch to 2.5G mode: #14) */
	serdes_xpcs_reg_write(base, xpcs, VR_MII_GEN5_12G_16G_MPLLA_CTRL0,
			      MPLLA_CAL_DISABLE | 0x50U);
			      /* MPPLA_MULTIPLIER=default */
	/*	(Switch to 2.5G mode: #15) */
	serdes_xpcs_reg_read(base, xpcs, VR_MII_DIG_CTRL1, &reg16);

	/* Clear bypass flag in case of internal clocks */
	if (clktype == CLK_INT) {
		reg16 &= ~BYP_PWRUP;
		serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16);
	}
	serdes_xpcs_reg_write(base, xpcs, VR_MII_DIG_CTRL1, reg16 | VR_RST);
		/* Issue reset */

	/*	(Switch to 2.5G mode: #16) */
	if (serdes_xpcs_wait_for_reset(base, xpcs))
		pr_err("XPCS pre power-up soft reset failed\n");

	/*	Wait for XPCS power up */
	pr_debug("Waiting for XPCS power-up\n");
	if (serdes_xpcs_wait_for_power_good(base, xpcs)) {
		pr_err("XPCS power-up failed\n");
		return -EXIT_FAILURE;
	}

	return 0;
}
