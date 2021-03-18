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
#define S32G_SERDES_XPCS_COUNT			2

/*
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
	void __iomem *base;
	enum serdes_xpcs_mode xpcs_mode;
	enum serdes_xpcs_mode_gen2 mode[2];
	enum serdes_mode ss_mode;
	enum serdes_clock clktype;
	enum serdes_clock_fmhz fmhz;
	bool is_init;
};

static struct s32_xpcs_cfg serdes_cfg[S32G_SERDES_COUNT] = { {.is_init = false},
							   {.is_init = false} };

static struct s32_xpcs_cfg *s32_get_serdes_priv(int platform_serdes_id)
{
	if (platform_serdes_id < S32G_SERDES_COUNT)
		return &serdes_cfg[platform_serdes_id];
	else
		return NULL;
}

int s32_sgmii_wait_link(int serdes_id, int xpcs)
{
	struct s32_xpcs_cfg *serdes = s32_get_serdes_priv(serdes_id);
	int ret;

	if (!serdes || xpcs >= S32G_SERDES_XPCS_COUNT)
		return -EINVAL;


	debug("Waiting for link (SerDes%d XPCS%i)...\n", serdes_id, xpcs);
	ret = serdes_pma_wait_link(serdes->base, xpcs, 1U);

	if (ret)
		printf("SerDes%d XPCS%i link timed-out\n", serdes_id, xpcs);
	else
		debug("SerDes%d XPCS%i link is up\n", serdes_id, xpcs);

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

enum serdes_xpcs_mode_gen2 s32_get_xpcs_mode(int serd, int xpcs)
{
	struct s32_xpcs_cfg *serdes = s32_get_serdes_priv(serd);

	if (!serdes || xpcs >= S32G_SERDES_XPCS_COUNT)
		return SGMII_XPCS_PCIE;

/* In case PCIe is disabled probe serdes drivers */
#if	!CONFIG_IS_ENABLED(PCI) && CONFIG_IS_ENABLED(SERDES_S32GEN1)
	if (!serdes->is_init)
		s32_serdes_no_pcie_init();
#elif !CONFIG_IS_ENABLED(SERDES_S32GEN1)
	printf("SGMII is not supported in this configuration");
#endif

	if (!serdes->is_init || serdes->xpcs_mode == SGMII_INAVALID) {
		printf("SerDes %d was not initialized\n", serd);
		return SGMII_XPCS_PCIE;
	}

	return serdes->mode[xpcs];
}

static void s32_serdes_issue_reset(struct s32_xpcs_cfg *serdes)
{
	switch (serdes->ss_mode) {
	case SERDES_MODE_PCIE_SGMII0:
		serdes_pcs_issue_vreset(serdes->base, 0);
		break;
	case SERDES_MODE_PCIE_SGMII1:
		serdes_pcs_issue_vreset(serdes->base, 1);
		break;
	case SERDES_MODE_SGMII_SGMII:
		serdes_pcs_issue_vreset(serdes->base, 1);
		serdes_pcs_issue_vreset(serdes->base, 0);
		break;
	case SERDES_MODE_SGMII_SGMII_ALT:
		serdes_pcs_issue_vreset(serdes->base, 0);
		serdes_pcs_issue_vreset(serdes->base, 1);
		break;
	default:
		break;
	}
}

static void s32_serdes_init_flags(struct s32_xpcs_cfg *serdes,
				  enum serdes_xpcs_mode_gen2 xpcs_mode,
				  u32 *init_flags, u32 f1g, u32 f25g, u32 fdis)
{
	if (xpcs_mode == SGMII_XPCS_2G5_OP)
		*init_flags |= PLLA_CAL_EN | PLLB_CAL_EN | f25g;

	if (xpcs_mode == SGMII_XPCS_1G_OP)
		*init_flags |= PLLA_CAL_EN | f1g;

	if (xpcs_mode == SGMII_XPCS_1G_OP)
		*init_flags |= PLLA_CAL_EN | f1g;

	if (xpcs_mode == SGMII_XPCS_DISABLED)
		*init_flags |= fdis;

	switch (serdes->ss_mode) {
	case SERDES_MODE_SGMII_SGMII:
		*init_flags |= PHY_CTRL_XPCS0_OWNED;
		break;
	case SERDES_MODE_SGMII_SGMII_ALT:
		*init_flags |= PHY_CTRL_XPCS1_OWNED;
		break;
	default:
		break;
	}
}

static void s32_serdes_post_init(struct s32_xpcs_cfg *serdes, u32 xpcs)
{
	if (serdes_pcs_wait_for_vreset(serdes->base, xpcs))
		pr_err("XPCS%d pre power-up soft reset failed\n", xpcs);

	if (serdes_pcs_wait_for_power_good(serdes->base, xpcs))
		pr_err("XPCS%d power-up failed\n", xpcs);

	serdes_pma_issue_rx_reset(serdes->base, xpcs);

	/* Disable automatic MII width change */
	serdes_pcs_mii_bus_control_disable(serdes->base, xpcs);
	/* Disable AN */
	serdes_pcs_an_disable(serdes->base, xpcs);
	/* Full duplex */
	serdes_pcs_set_fd(serdes->base, xpcs);
	/* Speed select */
	serdes_pcs_speed_select(serdes->base, xpcs, 1);
}

int s32_eth_xpcs_init(void __iomem *serdes_base, int platform_serdes_id,
		      enum serdes_mode ss_mode,
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

	struct s32_xpcs_cfg *serdes = s32_get_serdes_priv(platform_serdes_id);
	int ret = 0;
	u32 init_flags = 0;
	bool shared = false;

	if (!serdes) {
		printf("Invalid Serdes ID %d\n", platform_serdes_id);
		return -EINVAL;
	}

	serdes->base = serdes_base;
	serdes->xpcs_mode = SGMII_INAVALID;
	serdes->clktype = clktype;
	serdes->ss_mode = ss_mode;
	serdes->fmhz = fmhz;

	if (serdes->ss_mode == SERDES_MODE_PCIE_SGMII0 ||
	    serdes->ss_mode == SERDES_MODE_PCIE_SGMII1)
		shared = true;

	/* Note: this is temporary until upper layers are reworked */
	switch (xpcs_mode) {
	case SGMII_XPCS0_2G5:
		serdes->mode[0] = SGMII_XPCS_2G5_OP;
		serdes->mode[1] = SGMII_XPCS_1G_OP;
		break;
	case SGMII_XPCS0_XPCS1:
		serdes->mode[0] = SGMII_XPCS_1G_OP;
		serdes->mode[1] = SGMII_XPCS_1G_OP;
		break;
	case SGMII_XPCS0:
		serdes->mode[0] = SGMII_XPCS_1G_OP;
		if (shared)
			serdes->mode[1] = SGMII_XPCS_PCIE;
		else
			serdes->mode[1] = SGMII_XPCS_DISABLED;
		break;
	case SGMII_XPCS1:
		serdes->mode[1] = SGMII_XPCS_1G_OP;
		if (shared)
			serdes->mode[0] = SGMII_XPCS_PCIE;
		else
			serdes->mode[0] = SGMII_XPCS_DISABLED;
		break;
	default:
		serdes->mode[0] = SGMII_XPCS_PCIE;
		serdes->mode[1] = SGMII_XPCS_PCIE;
	}

	if (serdes->mode[0] != SGMII_XPCS_PCIE) {
		/* Bypass power up in case of pcie combo or internal clock*/
		if (serdes->clktype != CLK_INT && shared != true) {
			ret = serdes_pcs_wait_for_power_good(serdes->base, 0);
			if (!ret)
				pr_info("XPCS0 power-up good success\n");
			else
				pr_err("XPCS0 power-up good failed\n");
		}

		s32_serdes_init_flags(serdes, serdes->mode[0], &init_flags,
				      XPCS0_1000M, XPCS0_2500M, XPCS0_DIS);
	}

	if (serdes->mode[1] != SGMII_XPCS_PCIE) {
		/* Bypass power up in case of pcie combo or internal clock*/
		if (serdes->clktype != CLK_INT && shared != true) {
			ret = serdes_pcs_wait_for_power_good(serdes->base, 1);
			if (!ret)
				pr_info("XPCS1 power-up good success\n");
			else
				pr_err("XPCS1 power-up good failed\n");
		}

		s32_serdes_init_flags(serdes, serdes->mode[1], &init_flags,
				      XPCS1_1000M, XPCS1_2500M, XPCS1_DIS);
	}

	/* Check, if we should init something */
	if (!init_flags)
		return 0;

	if (serdes->clktype == CLK_INT)
		init_flags |= PHY_CLK_INT;

	serdes_pcs_pma_init_gen2(serdes->base, fmhz,  init_flags);

	/* Issue ss mode dependent reset */
	s32_serdes_issue_reset(serdes);

	/* Wait reset + Post init */
	if (((init_flags & (XPCS0_OWNED)) != 0))
		s32_serdes_post_init(serdes, 0);

	if (((init_flags & (XPCS1_OWNED)) != 0))
		s32_serdes_post_init(serdes, 1);

	debug("SerDes Init Done.\n");

	serdes->xpcs_mode = xpcs_mode;
	serdes->is_init = true;

	return 0;
}

enum xpcs_cmd {
	S32_XPCS_INVALID,
	S32_XPCS_TRANSIT_TO_1000M,
	S32_XPCS_TRANSIT_TO_2500M,
	S32_XPCS_AN_AUTO_SW_ENABLE,
	S32_XPCS_AN_ENABLE,
	S32_XPCS_AN_DISABLE
};

static int do_xpcs_cmd(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	struct s32_xpcs_cfg *serdes;
	u8 serdes_id, pcs_id;
	enum xpcs_cmd cmd = S32_XPCS_INVALID;
	u8 cmd_ss = 0;

	if (argc < 5)
		return CMD_RET_USAGE;

	serdes_id = simple_strtoul(argv[1], NULL, 10);
	pcs_id = simple_strtoul(argv[2], NULL, 10);
	serdes = s32_get_serdes_priv(serdes_id);

	if (!serdes || !serdes->is_init ||
	    pcs_id >= S32G_SERDES_XPCS_COUNT)
		return CMD_RET_USAGE;

	if (!strcmp(argv[3], "transit")) {
		if (!strcmp(argv[4], "1000M"))
			cmd = S32_XPCS_TRANSIT_TO_1000M;
		else if (!strcmp(argv[4], "2500M"))
			cmd = S32_XPCS_TRANSIT_TO_2500M;
	} else if (!strcmp(argv[3], "ss")) {
		if (!strcmp(argv[4], "10M")) {
			cmd = S32_XPCS_TRANSIT_TO_1000M;
			cmd_ss = 100;
		} else if (!strcmp(argv[4], "100M")) {
			cmd = S32_XPCS_TRANSIT_TO_1000M;
			cmd_ss = 10;
		} else if (!strcmp(argv[4], "1000M")) {
			cmd_ss = 1;
			cmd = S32_XPCS_TRANSIT_TO_1000M;
		} else if (!strcmp(argv[4], "2500M")) {
			cmd = S32_XPCS_TRANSIT_TO_2500M;
		}
	} else if (!strcmp(argv[3], "an") || !strcmp(argv[3], "an_auto")) {
		if (!strcmp(argv[4], "enable") && !strcmp(argv[3], "an_auto"))
			cmd = S32_XPCS_AN_AUTO_SW_ENABLE;
		else if (!strcmp(argv[4], "enable"))
			cmd = S32_XPCS_AN_ENABLE;
		else if (!strcmp(argv[4], "disable"))
			cmd = S32_XPCS_AN_DISABLE;
	} else {
		return CMD_RET_USAGE;
	}

	switch (cmd) {
	case S32_XPCS_TRANSIT_TO_1000M:
		serdes_bifurcation_pll_transit_to_1250Mhz(serdes->base,
							  pcs_id, serdes->fmhz);
		if (cmd_ss != 0)
			serdes_pcs_speed_select(serdes->base, pcs_id, cmd_ss);
		break;
	case S32_XPCS_TRANSIT_TO_2500M:
		serdes_bifurcation_pll_transit_to_3125Mhz(serdes->base,
							  pcs_id, serdes->fmhz);
		serdes_pcs_speed_select(serdes->base, pcs_id, 1);
		break;
	case S32_XPCS_AN_AUTO_SW_ENABLE:
		serdes_pcs_an_auto_sw_enable(serdes->base, pcs_id);
		/*fall through*/
	case S32_XPCS_AN_ENABLE:
		serdes_pcs_an_set_link_timer(serdes->base, pcs_id, 0x2faf);
		serdes_pcs_an_enable(serdes->base, pcs_id);
		break;
	case S32_XPCS_AN_DISABLE:
		serdes_pcs_an_auto_sw_disable(serdes->base, pcs_id);
		serdes_pcs_an_disable(serdes->base, pcs_id);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(xpcs, 6, 0, do_xpcs_cmd,
	   "Utility command for SGMMI control",
	   "xpcs <serdes> <xpcs> transit <1000M|2500M> - change serdes mode\n"
	   "xpcs <serdes> <xpcs> ss <10M|100M|1000M|2500M> - change speed and serdes mode when required\n"
	   "xpcs <serdes> <xpcs> an <enable|disable> - auto-negotiation control\n"
	   "xpcs <serdes> <xpcs> an_auto <enable|disable> - auto-negotiation control with automatic speed change"
);

/* Provide UCLASS DRV so SerDes driver can bind to it*/
#if	!CONFIG_IS_ENABLED(PCI) && CONFIG_IS_ENABLED(SERDES_S32GEN1)
UCLASS_DRIVER(pci_uc_gen) = {
	.id		= UCLASS_PCI_GENERIC,
	.name		= "sgmii_s32gen1",
};
#endif

