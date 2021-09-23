// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2021 NXP
 *
 */

/*
 * s32cc:
 *    NXP S32G/S32R/S32V chips.
 *    Based on Synopsys DW EQOS MAC 5.10a
 *
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <asm/io.h>

#include "serdes_regs.h"
#include "serdes_xpcs_regs.h"

#include <dm/platform_data/dwc_eth_qos_dm.h>

#include "dwc_eth_qos.h"
#include <s32gen1_clk_utils.h>
#include <s32gen1_gmac_utils.h>
#include <dm/device.h>

#define S32CCGMAC_ENV_VAR_MODE_NAME "s32cc_gmac_mode"

/* S32 SRC register for phyif selection */
#define PHY_INTF_SEL_SGMII	0x01
#define PHY_INTF_SEL_RGMII	0x02
#define PHY_INTF_SEL_RMII	0x08
#define PHY_INTF_SEL_MII	0x00
#define S32CC_GMAC_0_CTRL_STS		0x4007C004

static u32 mac_intf = PHY_INTERFACE_MODE_NONE;
static u32 s32ccgmac_mode = S32CCGMAC_MODE_UNINITED;

static bool s32ccgmac_set_interface(struct udevice *dev, u32 mode);

u32 s32ccgmac_cfg_get_mode(void)
{
	return s32ccgmac_mode;
}

static int s32ccgmac_cfg_set_mode(struct udevice *dev, u32 mode)
{
	int ret;

	if (s32ccgmac_mode == mode)
		/* already in the same mode */
		return 0;

	switch (mode) {
	case S32CCGMAC_MODE_DISABLE:
		/* TODO: GMAC IP: stop the driver, stop clocks and power down */
		env_set(S32CCGMAC_ENV_VAR_MODE_NAME, "disable");
		ret = 0;
		break;
	case S32CCGMAC_MODE_ENABLE:
		ret = s32ccgmac_set_interface(dev, mac_intf) ? 0 : -EINVAL;
		if (ret)
			goto err;

		env_set(S32CCGMAC_ENV_VAR_MODE_NAME, "enable");
		break;
	default:
		/* invalid mode */
		ret = -EINVAL;
		goto err;
	}

	s32ccgmac_mode = mode;

err:
	return ret;
}

static bool s32ccgmac_cfg_set_interface(struct udevice *dev, u32 mode)
{
	if (mode != PHY_INTERFACE_MODE_NONE &&
	    mode != PHY_INTERFACE_MODE_SGMII &&
	    mode != PHY_INTERFACE_MODE_RGMII &&
	    mode != PHY_INTERFACE_MODE_RMII &&
	    mode != PHY_INTERFACE_MODE_MII) {
		pr_err("invalid interface mode 0x%x\n", mode);
		return false;
	}

	if (!s32ccgmac_set_interface(dev, mode))
		return false;

	mac_intf = mode;

	return true;
}

static const char *s32ccgmac_cfg_get_interface_mode_str(void)
{
	return strlen(phy_string_for_interface(mac_intf)) ?
		phy_string_for_interface(mac_intf) : "none";
}

static bool s32ccgmac_set_interface(struct udevice *dev, u32 mode)
{
	setup_iomux_enet_gmac(dev, mode);
	setup_clocks_enet_gmac(mode, dev);

	mac_intf = mode;

	return true;
}

phy_interface_t s32ccgmac_cfg_get_interface(void)
{
	return mac_intf;
}

phy_interface_t eqos_get_interface_s32cc(struct udevice *dev)
{
	return s32ccgmac_cfg_get_interface();
}

static int eqos_start_clks_s32cc(struct udevice *dev)
{
	u32 mode = eqos_get_interface_s32cc(dev);

	setup_iomux_enet_gmac(dev, mode);
	setup_clocks_enet_gmac(mode, dev);

	return 0;
}

static void eqos_stop_clks_s32cc(struct udevice *dev)
{
	/* empty */
}

static int eqos_start_resets_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	u32 mode;

	/* set the interface mode */
	switch (eqos_get_interface_s32cc(dev)) {
	case PHY_INTERFACE_MODE_SGMII:
		pr_info("Interface mode set to SGMII\n");
		mode = PHY_INTF_SEL_SGMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
		pr_info("Interface mode set to RGMII\n");
		mode = PHY_INTF_SEL_RGMII;
		break;

	case PHY_INTERFACE_MODE_RMII:
		pr_info("Interface mode set to RMII\n");
		mode = PHY_INTF_SEL_RMII;
		break;

	case PHY_INTERFACE_MODE_MII:
		pr_info("Interface mode set to MII\n");
		mode = PHY_INTF_SEL_MII;
		break;

	case PHY_INTERFACE_MODE_NONE:
		pr_err("Interface mode not changed\n");
		return 0;

	default:
		pr_err("Invalid interface: 0x%x\n",
		       eqos_get_interface_s32cc(dev));
		return -1;
	}

	writel(mode, S32CC_GMAC_0_CTRL_STS);

	/* reset DMA to reread phyif config */
	writel(EQOS_DMA_MODE_SWR, &eqos->dma_regs->mode);
	udelay(10);
	while (readl(&eqos->dma_regs->mode) & EQOS_DMA_MODE_SWR)
		;

	return 0;
}

static int eqos_stop_resets_s32cc(struct udevice *dev)
{
	return 0;
}

static ulong eqos_get_tick_clk_rate_s32cc(struct udevice *dev)
{
	return s32gen1_get_dev_clk_rate("axi", dev);
}

static int eqos_calibrate_pads_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_disable_calibration_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_set_tx_clk_speed_s32cc(struct udevice *dev)
{
	u32 speed;
	struct eqos_priv *eqos = dev_get_priv(dev);

	if (!eqos || !eqos->phy || !eqos->mac_regs)
		return -ENODEV;

	if (eqos->phy->phy_id != PHY_FIXED_ID) {
		/*Auto neg.*/
		u32 idx = (eqos->mac_regs->phy_if_ctrl_status
			   >> EQOS_MAC_PHYIF_CTRL_STAT_LNKSPEED_SHIFT)
			  & EQOS_MAC_PHYIF_CTRL_STAT_LNKSPEED_MASK;

		switch (idx) {
		case 0:
			speed = SPEED_10;
			break;
		case 1:
			speed = SPEED_100;
			break;
		default:
		case 2:
			speed = SPEED_1000;
			break;
		}
	} else
		/*No auto neg.*/
		speed = eqos->phy->speed;

	return set_tx_clk_enet_gmac(dev, speed);
}

static int check_sgmii_cfg(int gmac_no)
{
	int serdes = gmac_no;
	int xpcs = 0;
	enum serdes_xpcs_mode mode, desired_mode1, desired_mode2;

#if defined(CONFIG_TARGET_S32G2XXAEVB) || \
	defined(CONFIG_TARGET_S32G3XXAEVB) || \
	defined(CONFIG_TARGET_S32G274ARDB) || \
	defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G398AEMU)

	desired_mode1 = SGMII_XPCS0;
	desired_mode2 = SGMII_XPCS0_XPCS1;

#elif defined(CONFIG_TARGET_S32R45EVB) || \
	defined(CONFIG_TARGET_S32R45SIM) || \
	defined(CONFIG_TARGET_S32R45EMU)

	desired_mode1 = SGMII_XPCS0;
	desired_mode2 = SGMII_XPCS0_2G5;

#else
#error "Board not supported"
#endif

	mode = s32_get_xpcs_mode(serdes);

	if (mode != desired_mode1 && mode != desired_mode2) {
		printf("Invalid SGMII configuration for GMAC%d", gmac_no);
		printf("Check hwconfig env. var.\n");
		return -EINVAL;
	}

	s32_sgmii_wait_link(serdes, xpcs);

	return 0;
}

static int eqos_probe_resources_s32cc(struct udevice *dev)
{
	struct eqos_pdata *pdata = dev_get_platdata(dev);
	char *env_mode = env_get(S32CCGMAC_ENV_VAR_MODE_NAME);

	debug("%s(dev=%p):\n", __func__, dev);

	mac_intf = pdata->eth.phy_interface;

	if (mac_intf == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	if (!s32ccgmac_cfg_set_interface(dev, mac_intf)) {
		pr_err("HW setting error\n");
		return -EINVAL;
	}

	if (env_mode && !strcmp(env_mode, "disable")) {
		return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_DISABLE);
	} else {
		if (eqos_get_interface_s32cc(dev) == PHY_INTERFACE_MODE_SGMII) {
			int ret = check_sgmii_cfg(0);

			if (ret)
				return ret;
		}

		return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_ENABLE);
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_remove_resources_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_pre_init_s32cc(struct udevice *dev)
{
	char *env_mode = env_get(S32CCGMAC_ENV_VAR_MODE_NAME);

	if (env_mode && !strcmp(env_mode, "disable"))
		return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_DISABLE);
	else
		return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_ENABLE);
}

static struct eqos_ops eqos_s32cc_ops = {
	.eqos_pre_init = eqos_pre_init_s32cc,
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_s32cc,
	.eqos_remove_resources = eqos_remove_resources_s32cc,
	.eqos_stop_resets = eqos_stop_resets_s32cc,
	.eqos_start_resets = eqos_start_resets_s32cc,
	.eqos_stop_clks = eqos_stop_clks_s32cc,
	.eqos_start_clks = eqos_start_clks_s32cc,
	.eqos_calibrate_pads = eqos_calibrate_pads_s32cc,
	.eqos_disable_calibration = eqos_disable_calibration_s32cc,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_s32cc,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_s32cc
};

struct eqos_config eqos_s32cc_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 50,
	.swr_wait = 50,
	.tx_fifo_size = 20480,
	.rx_fifo_size = 20480,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_500_800,
	.interface = eqos_get_interface_s32cc,
	.ops = &eqos_s32cc_ops
};

static struct udevice *get_fake_gmac_dev(void)
{
	static struct udevice gmac_dev;
	ofnode node;

	if (gmac_dev.node.np)
		return &gmac_dev;

	node = ofnode_by_compatible(ofnode_null(), "fsl,s32cc-dwmac");
	if (!node.np) {
		pr_err("Could not find 'fsl,s32cc-dwmac' node\n");
		return NULL;
	}

	gmac_dev.node = node;
	return &gmac_dev;
}

/* command line */
static int do_s32ccgmac_cmd(cmd_tbl_t *cmdtp, int flag,
			    int argc, char * const argv[])
{
	struct udevice *gmac_dev = get_fake_gmac_dev();

	/* process command */
	if (!strcmp(argv[1], "info")) {
		printf("GMAC mode: %s\n",
		       s32ccgmac_cfg_get_mode() == S32CCGMAC_MODE_DISABLE
		       ? "disabled" : "enabled");
		printf("interface: %s\n",
		       s32ccgmac_cfg_get_interface_mode_str());
		return 0;
	} else if (!strcmp(argv[1], "disable")) {
		s32ccgmac_cfg_set_mode(gmac_dev, S32CCGMAC_MODE_DISABLE);
		return 0;
	} else if (!strcmp(argv[1], "enable")) {
		s32ccgmac_cfg_set_mode(gmac_dev, S32CCGMAC_MODE_ENABLE);
		return 0;
	} else if (!strcmp(argv[1], "emac")) {
		if (argc < 3) {
			printf("interface: %s%s\n",
			       s32ccgmac_cfg_get_interface_mode_str(),
			       s32ccgmac_cfg_get_mode() == S32CCGMAC_MODE_DISABLE
			       ? "/disabled" : "");
		} else {
			int new_intf = -1;

			if (!strcmp(argv[2], "sgmii")) {
				new_intf = PHY_INTERFACE_MODE_SGMII;
			} else if (!strcmp(argv[2], "rgmii")) {
				new_intf = PHY_INTERFACE_MODE_RGMII;
			} else if (!strcmp(argv[2], "rmii")) {
				new_intf = PHY_INTERFACE_MODE_RMII;
			} else if (!strcmp(argv[2], "mii")) {
				new_intf = PHY_INTERFACE_MODE_MII;
			} else if (!strcmp(argv[2], "none")) {
				new_intf = PHY_INTERFACE_MODE_NONE;
			} else {
				pr_err("Invalid PHY interface type '%s'\n",
				       argv[2]);
				return CMD_RET_USAGE;
			}
			if (new_intf > -1)
				s32ccgmac_cfg_set_interface(gmac_dev, new_intf);
		}
		return 0;
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	   s32ccgmac, 3, 0, do_s32ccgmac_cmd,
	   "NXP S32cc GMAC controller info",
	   /*      */"info               - important hw info\n"
	   "s32ccgmac [disable|enable]   - disable/enable gmac/eqos subsystem\n"
	   "s32ccgmac emac [<inf-mode>]  - read or set MAC interface mode\n"
	   "eqos help                    - additional gmac commands"
);
