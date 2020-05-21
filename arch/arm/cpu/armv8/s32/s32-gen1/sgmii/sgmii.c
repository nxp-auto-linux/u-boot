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

#include <asm/arch-s32/siul.h>
#include <asm/arch-s32/mc_rgm_regs.h>

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>

#define S32G_SERDES_0_BASE			0x40400000U
#define S32G_SERDES_1_BASE			0x44100000U
#define S32G_SERDES_BASE_LEN			0x100000

/*
 * Auto-negotiation is currently not supported.
 *
 * Only limited number of configurations were tested (Only on SerDes1).
 * You should take care and check, if everything works in your configuration.
 *
 * Tested configurations (of hwconfig):
 * Board s32g274ardb:
 *	-Default - MAC0 2.5G to sja1110 switch
 *		pcie1:mode=sgmii,clock=ext,fmhz=125,xpcs_mode=2G5
 *	-MAC0 1G to the sja1110 switch/PCIeX1 on lane 0
 *		pcie1:mode=rc&sgmii,clock=int,fmhz=100,xpcs_mode=0
 *	-MAC1 1G to the sja1110 switch/PCIeX1 on lane 0
 *		pcie1:mode=rc&sgmii,clock=int,fmhz=100,xpcs_mode=1
 *
 * Board s32g274aevb:
 *	-Default - MAC0 1G to Aqauntia (CPU board)
 *		pcie1:mode=sgmii,clock=ext,fmhz=125,xpcs_mode=0 or
 *		pcie1:mode=sgmii,clock=ext,fmhz=100,xpcs_mode=0 or
 *		pcie1:mode=sgmii,clock=int,fmhz=100,xpcs_mode=0
 *
 * Warning: Currently only internal clocks are supported in rc&sgmii
 * and ep&sgmii modes.
 *
 */

#ifdef CONFIG_TARGET_S32G274AEVB
/* rev. 1.0.1*/
#define SGMII_MIN_SOC_REV_SUPPORTED 0x1
#endif /* CONFIG_TARGET_S32G274AEVB */

static inline void *s32_get_serdes_base(int id)
{
	if (id == 0)
		return (void *)(phys_addr_t)S32G_SERDES_0_BASE;
	else if (id == 1)
		return (void *)(phys_addr_t)S32G_SERDES_1_BASE;
	else
		return NULL;
}

int s32_serdes1_wait_link(int id)
{
	void *serdes1_base;
	int ret;

	serdes1_base = s32_get_serdes_base(id);
	if (!serdes1_base)
		return -EINVAL;

	debug("Waiting for link (XPCS%i)...\n", id);
	ret = serdes_wait_for_link(serdes1_base,
				   id ? SERDES_XPCS_1_BASE : SERDES_XPCS_0_BASE,
				   1U);
	if (ret)
		printf("XPCS%i link timed-out\n", id);
	else
		printf("XPCS%i link is up\n", id);

	return ret;
}

#ifdef SGMII_VERIFY_LINK_ON_STARTUP
static int s32_serdes_verify_link(int serdes, int xpcs)
{
	void *serdes_base;
	int ret = 0;
	u32 xpcs_base = xpcs ? SERDES_XPCS_1_BASE : SERDES_XPCS_0_BASE;

	serdes_base = s32_get_serdes_base(serdes);
	if (!serdes_base)
		return -EINVAL;

	xpcs_base = s32_get_xpcs_base(xpcs);
	if (!xpcs_base)
		return -EINVAL;

	if (serdes_xpcs_set_loopback(serdes_base, xpcs_base, true)) {
		ret = -EINVAL;
		goto link_error;
	}

	if (s32_sgmii_wait_link(serdes, xpcs))
		ret = -ETIMEDOUT;

link_error:
	serdes_xpcs_set_loopback(serdes_base, xpcs, false);
	return ret;
}
#endif /* SGMII_VERIFY_LINK_ON_STARTUP */

int s32_eth_xpcs_init(void __iomem *serdes_base, int id,
		      enum serdes_xpcs_mode xpcs_mode,
		      enum serdes_clock clktype,
		      enum serdes_clock_fmhz fmhz)
{
#ifdef SGMII_MIN_SOC_REV_SUPPORTED
	u32 raw_rev = 0;

	/* construct a revision number based on major, minor and subminor,
	 * each part using one hex digit
	 */
	raw_rev = (get_siul2_midr1_major() << 8) |
		  (get_siul2_midr1_minor() << 4) |
		  (get_siul2_midr2_subminor());

	if (raw_rev < SGMII_MIN_SOC_REV_SUPPORTED) {
		printf("SGMII not supported on rev.");
		printf("%d.%d.%d\n", get_siul2_midr1_major() + 1,
		       get_siul2_midr1_minor(),
		       get_siul2_midr2_subminor());
		return -ENXIO;
	}
#endif /* SGMII_MIN_SOC_REV_SUPPORTED */
	int retval = 0;
	bool xpcs0 = false;
	bool xpcs1 = false;

	/* Decode XPCS */
	if (xpcs_mode == SGMII_XPCS0 || xpcs_mode == SGMII_XPCS0_2G5) {
		xpcs0 = true;
		xpcs1 = false;
	} else if (xpcs_mode == SGMII_XPCS1) {
#if CONFIG_TARGET_S32R45XEVB
		printf("Configuration not supported");
		printf(" on this platform for SerDes %d\n", id);
		return -EINVAL;
#endif
		xpcs0 = false;
		xpcs1 = true;
	} else if (xpcs_mode == SGMII_XPCS0_XPCS1) {
#if CONFIG_TARGET_S32R45XEVB
		printf("Configuration not supported");
		printf(" on this platform for SerDes %d\n", id);
		return -EINVAL;
#endif
		xpcs0 = true;
		xpcs1 = true;
	} else {
		printf("Invalid XPCS mode on SerDes %d\n", id);
		return -EINVAL;
	}

	if (xpcs0) {
		debug("SerDes %d XPCS_0 init to 1G mode started\n", id);
		retval = serdes_xpcs_set_1000_mode(serdes_base,
						   SERDES_XPCS_0_BASE,
						   clktype, fmhz);
		if (retval) {
			printf("SerDes %d XPCS_0 init failed\n", id);
			return retval;
		}
		debug("SerDes %d XPCS_0 init to 1G mode successful\n", id);
	}

	if (xpcs1) {
		debug("SerDes %d XPCS_1 init to 1G mode started\n", id);
		retval = serdes_xpcs_set_1000_mode(serdes_base,
						   SERDES_XPCS_1_BASE,
						   clktype, fmhz);
		if (retval) {
			printf("SerDes %d XPCS_1 init failed\n", id);
			return retval;
		}
		debug("SerDes %d XPCS_1 init to 1G mode successful\n", id);
	}

	if (xpcs0) {
		/*	Configure XPCS_0 speed (1000Mpbs, Full duplex) */
		retval = serdes_xpcs_set_sgmii_speed(serdes_base,
						     SERDES_XPCS_0_BASE,
						     1000U, true);
		if (retval)
			return retval;
	}

	if (xpcs1) {
		/*	Configure XPCS_1 speed (1000Mpbs, Full duplex) */
		retval = serdes_xpcs_set_sgmii_speed(serdes_base,
						     SERDES_XPCS_1_BASE,
						     1000U, true);
		if (retval)
			return retval;
	}

	if (xpcs_mode == SGMII_XPCS0_2G5) {
		retval = serdes_xpcs_set_2500_mode(serdes_base,
						   SERDES_XPCS_0_BASE,
						   clktype, fmhz);
		if (retval) {
			printf("XPCS_0 init failed\n");
			return retval;
		}
		debug("XPCS_0 in 2.5G mode\n");
	}

	debug("SerDes Init Done.\n");

#ifdef SGMII_VERIFY_LINK_ON_STARTUP
	/* disabled by default */
	if (xpcs0)
		if (s32_serdes_verify_link(id, 0))
			printf("SerDes%d XPCS_%d link up failed\n", id, 0);
	if (xpcs1)
		if (s32_serdes_verify_link(id, 1))
			printf("SerDes%d XPCS_%d link up failed\n", id, 1);
#endif

	return 0;
}

