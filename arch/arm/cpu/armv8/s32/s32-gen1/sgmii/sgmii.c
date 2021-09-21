// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2021 NXP
 *
 * The SerDes config code
 */
#include <common.h>
#include <stdio.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/ethtool.h>
#include <asm/io.h>
#include <stdlib.h>
#include <linux/printk.h>
#include <log.h>
#include <dm.h>

#include <asm/arch-s32/siul.h>
#include <asm/arch-s32/mc_rgm_regs.h>

#include <serdes_regs.h>
#include <serdes_xpcs_regs.h>

#define S32G_SERDES_0_BASE			0x40400000U
#define S32G_SERDES_1_BASE			0x44100000U
#define S32G_SERDES_BASE_LEN			0x100000
#define S32G_SERDES_COUNT			2

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
 */

struct s32_xpcs_cfg {
	enum serdes_xpcs_mode xpcs_mode;
	bool is_init;
};

static struct s32_xpcs_cfg xpcs_cfg[S32G_SERDES_COUNT] = { {.is_init = false},
							   {.is_init = false} };

static inline void *s32_get_serdes_base(int id)
{
	if (id == 0)
		return (void *)(phys_addr_t)S32G_SERDES_0_BASE;
	else if (id == 1)
		return (void *)(phys_addr_t)S32G_SERDES_1_BASE;
	else
		return NULL;
}

static inline u32 s32_get_xpcs_base(int id)
{
	if (id == 0)
		return SERDES_XPCS_0_BASE;
	else if (id == 1)
		return SERDES_XPCS_1_BASE;
	else
		return 0;
}

int s32_sgmii_wait_link(int serdes, int xpcs)
{
	void *serdes_base;
	u32 xpcs_base;
	int ret;

	serdes_base = s32_get_serdes_base(serdes);
	if (!serdes_base)
		return -EINVAL;

	xpcs_base = s32_get_xpcs_base(xpcs);
	if (!xpcs_base)
		return -EINVAL;

	debug("Waiting for link (SerDes%d XPCS%i)...\n", serdes, xpcs);
	ret = serdes_wait_for_link(serdes_base, xpcs_base, 1U);

	if (ret)
		printf("SerDes%d XPCS%i link timed-out\n", serdes, xpcs);
	else
		debug("SerDes%d XPCS%i link is up\n", serdes, xpcs);

	return ret;
}

#ifdef SGMII_VERIFY_LINK_ON_STARTUP
static int s32_serdes_verify_link(int serdes, int xpcs)
{
	void *serdes_base;
	u32 xpcs_base;
	int ret = 0;

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

/* WARNING: This function is only experimental and it is not tested*/
/* Get SGMII speed/duplex/auto-neg */
int s32_sgmii_get_speed(int serdes, int xpcs, int *mbps, bool *fd, bool *an)
{
	void *serdes_base;
	u32 xpcs_base;
	int ret;

	serdes_base = s32_get_serdes_base(serdes);
	if (!serdes_base)
		return -EINVAL;

	xpcs_base = s32_get_xpcs_base(xpcs);
	if (!xpcs_base)
		return -EINVAL;

	ret = serdes_xpcs_get_sgmii_speed(serdes_base, xpcs_base,
					  mbps, fd, an);
	debug("SerDes%d XPCS%d is configured to %dMbs (AN %s, %s)",
	      serdes, xpcs, *mbps,
	      an ? "ON" : "OFF",
	      fd ? "FD" : "HD");

	return ret;
}

/* WARNING: This function is only experimental and it is not tested*/
/* Set SGMII speed/duplex/auto-neg */
int s32_sgmii_set_speed(int serdes, int xpcs, int mbps, bool fd, bool an)
{
	void *serdes_base;
	u32 xpcs_base;
	int ret;

	/* SGMII auto-negotiation is currently not supported  */
	(void)an;

	serdes_base = s32_get_serdes_base(serdes);
	if (!serdes_base)
		return -EINVAL;

	xpcs_base = s32_get_xpcs_base(xpcs);
	if (!xpcs_base)
		return -EINVAL;

	ret = serdes_xpcs_set_sgmii_speed(serdes_base, xpcs_base,
					  mbps, fd);
	debug("SerDes%d XPCS%d config was updated to %dMbs (AN %s, %s)",
	      serdes, xpcs, mbps,
	      an ? "ON" : "OFF",
	      fd ? "FD" : "HD");

	return ret;
}

/* Function used to probe the SerDes in case the PCIe is disabled */
#if	!CONFIG_IS_ENABLED(PCI) && CONFIG_IS_ENABLED(SERDES_S32GEN1)
static void s32_serdes_no_pcie_init(void)
{
	struct udevice *bus;

	debug("%s\n", __func__);

	/*
	 * Enumerate all known UCLASS_PCI_GENERIC devices. This will
	 * also probe them, so the SerDes devices will be enumerated too.
	 */
	for (uclass_first_device(UCLASS_PCI_GENERIC, &bus);
	     bus;
	     uclass_next_device(&bus)) {
		;
	}
}
#endif

enum serdes_xpcs_mode s32_get_xpcs_mode(int serdes)
{
	if (serdes > S32G_SERDES_COUNT) {
		printf("Invalid SerDes ID %d\n", serdes);
		return SGMII_INAVALID;
	}

/* In case PCIe is disabled probe serdes drivers */
#if	!CONFIG_IS_ENABLED(PCI) && CONFIG_IS_ENABLED(SERDES_S32GEN1)
	if (!xpcs_cfg[serdes].is_init)
		s32_serdes_no_pcie_init();
#elif !CONFIG_IS_ENABLED(SERDES_S32GEN1)
	printf("SGMII is not supported in this configuration");
#endif

	if (!xpcs_cfg[serdes].is_init ||
	    xpcs_cfg[serdes].xpcs_mode == SGMII_INAVALID) {
		printf("SerDes %d was not initialized\n", serdes);
		return SGMII_INAVALID;
	}
	return xpcs_cfg[serdes].xpcs_mode;
}

int s32_eth_xpcs_init(void __iomem *serdes_base, int id,
		      bool combo,
		      enum serdes_xpcs_mode xpcs_mode,
		      enum serdes_clock clktype,
		      enum serdes_clock_fmhz fmhz)
{
	int retval = 0;
	u32 xpcs0_base;
	u32 xpcs1_base;
	bool xpcs0 = false;
	bool xpcs1 = false;

	xpcs0_base = s32_get_xpcs_base(0);
	if (!xpcs0_base)
		return -EINVAL;

	xpcs1_base = s32_get_xpcs_base(1);
	if (!xpcs1_base)
		return -EINVAL;

	if (id > S32G_SERDES_COUNT) {
		printf("Invalid XPCS ID %d\n", id);
		return false;
	}

	xpcs_cfg[id].xpcs_mode = SGMII_INAVALID;

	/* Decode XPCS */
	if (xpcs_mode == SGMII_XPCS0 || xpcs_mode == SGMII_XPCS0_2G5) {
		xpcs0 = true;
		xpcs1 = false;
	} else if (xpcs_mode == SGMII_XPCS1) {
#if CONFIG_TARGET_S32R45EVB
		printf("Configuration not supported");
		printf(" on this platform for SerDes %d\n", id);
		return -EINVAL;
#endif
		xpcs0 = false;
		xpcs1 = true;
	} else if (xpcs_mode == SGMII_XPCS0_XPCS1) {
#if CONFIG_TARGET_S32R45EVB
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

	if (xpcs1) {
		debug("SerDes %d XPCS_1 init to 1G mode started\n", id);
		retval = serdes_xpcs_set_1000_mode(serdes_base,
						   xpcs1_base,
						   clktype, fmhz, combo);
		if (retval) {
			printf("SerDes %d XPCS_1 init failed\n", id);
			return retval;
		}
		debug("SerDes %d XPCS_1 init to 1G mode successful\n", id);
	}

	if (xpcs0) {
		debug("SerDes %d XPCS_0 init to 1G mode started\n", id);
		retval = serdes_xpcs_set_1000_mode(serdes_base,
						   xpcs0_base,
						   clktype, fmhz, combo);
		if (retval) {
			printf("SerDes %d XPCS_0 init failed\n", id);
			return retval;
		}
		debug("SerDes %d XPCS_0 init to 1G mode successful\n", id);
	}

	if (xpcs0) {
		/*	Configure XPCS_0 speed (1000Mpbs, Full duplex) */
		retval = serdes_xpcs_set_sgmii_speed(serdes_base,
						     xpcs0_base,
						     SPEED_1000, true);
		if (retval)
			return retval;
	}

	if (xpcs1) {
		/*	Configure XPCS_1 speed (1000Mpbs, Full duplex) */
		retval = serdes_xpcs_set_sgmii_speed(serdes_base,
						     xpcs1_base,
						     SPEED_1000, true);
		if (retval)
			return retval;
	}

	if (xpcs_mode == SGMII_XPCS0_2G5) {
		retval = serdes_xpcs_set_2500_mode(serdes_base,
						   xpcs0_base,
						   clktype, fmhz);
		if (retval) {
			printf("XPCS_0 init failed\n");
			return retval;
		}
		debug("XPCS_0 in 2.5G mode\n");
	}

	debug("SerDes Init Done.\n");

	xpcs_cfg[id].xpcs_mode = xpcs_mode;
	xpcs_cfg[id].is_init = true;

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

/* Provide UCLASS DRV so SerDes driver can bind to it*/
#if	!CONFIG_IS_ENABLED(PCI) && CONFIG_IS_ENABLED(SERDES_S32GEN1)
UCLASS_DRIVER(pci_uc_gen) = {
	.id		= UCLASS_PCI_GENERIC,
	.name		= "sgmii_s32gen1",
};
#endif

