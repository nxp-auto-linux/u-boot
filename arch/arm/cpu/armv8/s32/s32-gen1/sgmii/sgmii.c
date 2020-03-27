// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2020 NXP
 *
 * The SerDes config code
 */
#include <common.h>
#include <stdio.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <stdlib.h>
#include <linux/printk.h>
#include <log.h>

#include <asm/arch-s32/mc_rgm_regs.h>

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>

#define S32G_MAIN_GPR_BASE			0x4007ca00U
#define S32G_MAIN_GPR_LEN			0x1000U
#define S32G_MC_RGM_BASE			0x40078000U
#define S32G_MC_RGM_LEN				0x200
#define S32G_RDC_BASE				0x40080000U
#define S32G_RDC_LEN				0x100U
#define GENCTRL0				0xe0U
#define GENCTRL1				0xe4U
#define PFE_EMACX_INTF_SEL			0x04U
#define PFE_PWR_CTRL				0x20U
#define SGMII_CSEL				(1U << 0)

#define S32G_SERDES_0_BASE			0x40400000U
#define S32G_SERDES_1_BASE			0x44100000U
#define S32G_SERDES_BASE_LEN			0x100000

/*
 * When this is not defined, both SerDes1 lanes are configured in 1G mode
 * and PFE_MAC0 and PFE_MAC1 can be configured to use SGMII.
 *
 * Define this if you want use PFE_EMAC0 in 2.5G mode. In that case the
 * SerDes Lane1 is unusable and PFE_EMAC1 must not use SGMII.
 */
#if CONFIG_IS_ENABLED(TARGET_S32G274ARDB)
#define USE_2500_EMAC0_MODE
#endif

/*
 * Demo feature
 *
 * When defined, the Serdes1.Lane1 is uses internal PLL clocking.
 * Tested only on S32G-VNP-PROC EVB.
 *
 * WARNING: can be enabled only for SGMII 1.25G mode
 *
 */
/* #define USE_INTERNAL_SERDES_CLOCK */

int s32_serdes1_wait_link(int id)
{
	void *serdes1_base = (void *)(phys_addr_t)S32G_SERDES_1_BASE;
	int ret;

	debug("Waiting for link (XPCS%i)...\n", id);
	ret = serdes_wait_for_link(serdes1_base,
				   id ? SERDES_XPCS_1_BASE : SERDES_XPCS_0_BASE,
				   1U);
	if (ret)
		printf("XPCS%i link timed-out\n", id);
	else
		debug("XPCS%i link is up\n", id);

	return ret;
}

int s32_serdes1_setup(int mode)
{
	int retval;
	void *serdes1_base = (void *)(phys_addr_t)S32G_SERDES_1_BASE;

	/* Configure SERDES
	 * Is SERDES already configured?
	 * TODO: Unify this with code in SerDes driver.
	 */
	if (!s32_get_serdes_mode_from_target(serdes1_base,
			SERDES_MODE_SGMII_SGMII)) {

		/* Configure SERDES */

		/*	Issue SERDES_1 reset */
		if (rgm_issue_reset(PRST_PCIE_1_SERDES)) {
			printf("PCIE reset failed\n");
			return -EXIT_FAILURE;
		}

		if (rgm_issue_reset(PRST_PCIE_1_FUNC)) {
			printf("PCIE reset failed\n");
			return -EXIT_FAILURE;
		}

		/*	Set pipeP_pclk */
		writel(EXT_PCLK_REQ, serdes1_base + SS_PHY_GEN_CTRL);

		/*	PFE_MAC0 = Lane0 = SGMII && PFE_MAC1 = Lane1 = SGMII */
		if (serdes_set_mode(serdes1_base, 1, SERDES_MODE_SGMII_SGMII)) {
			printf("SerDes1 PHY mode selection failed\n");
			return -EXIT_FAILURE;
		}

		udelay(50); /* At least 10us */

		/*	Release PCIE_1 reset */
		if (rgm_release_reset(PRST_PCIE_1_SERDES)) {
			printf("PCIE reset failed\n");
			return -EXIT_FAILURE;
		}

		if (rgm_release_reset(PRST_PCIE_1_FUNC)) {
			printf("PCIE reset failed\n");
			return -EXIT_FAILURE;
		}
	}

	/*	Set SerDes reference clock from external pads.
	 *	See HW connections for reference clock frequency.
	 *	TODO: Use 'hwconfig' for setting the clock.
	 */
	writel(PHY_GEN_CTRL_REF_USE_PAD, serdes1_base + SS_PHY_GEN_CTRL);

#ifdef USE_INTERNAL_SERDES_CLOCK
	/*	1Gbps */
	/*	Configure XPCS_0 (internal 100 MHz reference clock) */
	retval = serdes_xpcs_set_1000_mode(serdes1_base, SERDES_XPCS_0_BASE,
					   true, 100U);
#else
	/*	1Gbps */
	/*	Configure XPCS_0 (external 125 MHz reference clock) */
	retval = serdes_xpcs_set_1000_mode(serdes1_base, SERDES_XPCS_0_BASE,
					   true, 125U);
#endif
	if (retval) {
		printf("XPCS_0 init failed\n");
		return -EXIT_FAILURE;
	}
	debug("XPCS_0 in 1G mode\n");

	/*	Configure XPCS_1 (external reference clock) */
	retval = serdes_xpcs_set_1000_mode(serdes1_base, SERDES_XPCS_1_BASE,
					   true, 125U);
	if (retval) {
		printf("XPCS_1 init failed\n");
		return -EXIT_FAILURE;
	}
	debug("XPCS_1 in 1G mode\n");

	/*	Configure XPCS_0 speed (1000Mpbs, Full duplex) */
	retval = serdes_xpcs_set_sgmii_speed(serdes1_base, SERDES_XPCS_0_BASE,
					     1000U, true);
	if (retval)
		/*	Unable to set speed */
		return -EXIT_FAILURE;

	/*	Configure XPCS_1 speed (1000Mpbs, Full duplex) */
	retval = serdes_xpcs_set_sgmii_speed(serdes1_base, SERDES_XPCS_1_BASE,
					     1000U, true);
	if (retval)
		/*	Unable to set speed */
		return -EXIT_FAILURE;

#ifdef USE_2500_EMAC0_MODE
	/*	2.5Gbps */
	/*	Configure XPCS_0 (external reference clock) */
	retval = serdes_xpcs_set_2500_mode(serdes1_base, SERDES_XPCS_0_BASE,
					   true, 125U);
	if (retval) {
		printf("XPCS_0 init failed\n");
		return -EXIT_FAILURE;
	}
	debug("XPCS_0 in 2.5G mode\n");
#endif /* 0 */

	debug("SerDes Init Done.\n");

#ifdef SGMII_VERIFY_LINK_ON_STARTUP
	/* disabled by default */
	serdes_xpcs_set_loopback(serdes1_base, SERDES_XPCS_0_BASE, true);
	serdes_xpcs_set_loopback(serdes1_base, SERDES_XPCS_1_BASE, true);

	s32_serdes1_wait_link(0);
	s32_serdes1_wait_link(1);

	serdes_xpcs_set_loopback(serdes1_base, SERDES_XPCS_0_BASE, false);
	serdes_xpcs_set_loopback(serdes1_base, SERDES_XPCS_1_BASE, false);
#endif

	return 0;
}

