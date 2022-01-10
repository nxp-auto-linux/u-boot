// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2022 NXP
 *
 */

/*
 * s32cc:
 *    NXP S32G and S32R chips.
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
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <asm/io.h>

#include "board_common.h"
#include "serdes_regs.h"
#include "serdes_xpcs_regs.h"

#include <dm/platform_data/dwc_eth_qos_dm.h>

#include "dwc_eth_qos.h"
#include <s32gen1_clk_utils.h>
#include <s32gen1_gmac_utils.h>
#include <dm/device.h>

/* S32 SRC register for phyif selection */
#define PHY_INTF_SEL_SGMII	0x01
#define PHY_INTF_SEL_RGMII	0x02
#define PHY_INTF_SEL_RMII	0x08
#define PHY_INTF_SEL_MII	0x00
#define S32CC_GMAC_0_CTRL_STS		0x4007C004
#define S32CC_GMAC_1_CTRL_STS		0x4007CA00

struct s32cc_priv {
	phy_interface_t mac_intf;
	u32 gmac_mode;
};

static bool s32ccgmac_set_interface(struct udevice *dev,
				    phy_interface_t mode);

static struct s32cc_priv *s32ccgmac_get_priv(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	if (!eqos) {
		pr_err("Could not get private data for the device\n");
		return NULL;
	}

	return eqos->priv;
}

static struct udevice *s32ccgmac_get_dev_by_idx(int cardnum)
{
	char devname[20];
	struct udevice *dev = NULL;

	eqos_name(devname, cardnum);
	(void)uclass_get_device_by_name(UCLASS_ETH, devname, &dev);
	return dev;
}

static int s32ccgmac_alloc_priv(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	if (!eqos) {
		printf("Could not get eqos private structure\n");
		return -EINVAL;
	}

	eqos->priv = malloc(sizeof(struct s32cc_priv));
	if (!eqos->priv)
		return -ENOMEM;

	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return -EINVAL;

	s32cc->mac_intf = PHY_INTERFACE_MODE_NONE;
	s32cc->gmac_mode = S32CCGMAC_MODE_UNINITED;

	return 0;
}

static const char *env_var_mode_name(struct udevice *dev)
{
	static char *prefix = "s32cc_gmac";
	static char *postfix = "_mode";
	static char name[sizeof(prefix) + sizeof(postfix) + 4];

	int cardnum = eqos_num(dev);

	if (cardnum > 0)
		snprintf(name, ARRAY_SIZE(name), "%s%d%s",
			 prefix, cardnum, postfix);
	else
		snprintf(name, ARRAY_SIZE(name), "%s%s", prefix, postfix);

	return name;
}

u32 s32ccgmac_cfg_get_mode(int cardnum)
{
	struct s32cc_priv *s32cc;
	struct udevice *dev = s32ccgmac_get_dev_by_idx(cardnum);

	s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return -EINVAL;
	return s32cc->gmac_mode;
}

static int s32ccgmac_cfg_set_mode(struct udevice *dev, u32 mode)
{
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	const char *env_var = env_var_mode_name(dev);
	int ret;

	if (!s32cc)
		return -EINVAL;

	if (s32cc->gmac_mode == mode)
		/* already in the same mode */
		return 0;

	switch (mode) {
	case S32CCGMAC_MODE_DISABLE:
		/* TODO: GMAC IP: stop the driver, stop clocks and power down */
		env_set(env_var, "disable");
		ret = 0;
		break;
	case S32CCGMAC_MODE_ENABLE:
		if (s32ccgmac_set_interface(dev, s32cc->mac_intf))
			ret = 0;
		else
			ret = -EINVAL;
		if (ret)
			goto err;

		env_set(env_var, "enable");
		break;
	default:
		/* invalid mode */
		ret = -EINVAL;
		goto err;
	}

	s32cc->gmac_mode = mode;

err:
	return ret;
}

static bool s32ccgmac_cfg_set_interface(struct udevice *dev,
					phy_interface_t mode)
{
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return NULL;

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

	s32cc->mac_intf = mode;

	return true;
}

static const char *
s32ccgmac_cfg_get_interface_mode_str(struct s32cc_priv *s32cc)
{
	phy_interface_t mac_intf = s32cc->mac_intf;

	return strlen(phy_string_for_interface(mac_intf)) ?
		phy_string_for_interface(mac_intf) : "none";
}

static bool s32ccgmac_set_interface(struct udevice *dev, phy_interface_t mode)
{
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return false;

	setup_iomux_enet_gmac(dev, mode);
	setup_clocks_enet_gmac(mode, dev);
	s32cc->mac_intf = mode;

	return true;
}

phy_interface_t s32ccgmac_cfg_get_interface(int cardnum)
{
	struct udevice *dev = s32ccgmac_get_dev_by_idx(cardnum);
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return PHY_INTERFACE_MODE_NONE;

	return s32cc->mac_intf;
}

phy_interface_t eqos_get_interface_s32cc(struct udevice *dev)
{
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return PHY_INTERFACE_MODE_NONE;

	return s32cc->mac_intf;
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
	u32 mode;
	struct eqos_priv *eqos;
	int gmac_no;

	eqos = dev_get_priv(dev);
	if (!eqos)
		return -EINVAL;

	gmac_no = eqos_num(dev);
	if (gmac_no < 0)
		return gmac_no;

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

	if (gmac_no == 0)
		writel(mode, S32CC_GMAC_0_CTRL_STS);
	else
		writel(mode, S32CC_GMAC_1_CTRL_STS);

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
	enum serdes_xpcs_mode_gen2 mode, desired_mode1, desired_mode2;

#if defined(CONFIG_TARGET_S32G2XXAEVB) || \
	defined(CONFIG_TARGET_S32G3XXAEVB) || \
	defined(CONFIG_TARGET_S32G274ARDB) || \
	defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G399AEMU)

	desired_mode1 = SGMII_XPCS_1G_OP;
	desired_mode2 = SGMII_XPCS_1G_OP;

#elif defined(CONFIG_TARGET_S32R45EVB) || \
	defined(CONFIG_TARGET_S32R45SIM) || \
	defined(CONFIG_TARGET_S32R45EMU)

	desired_mode1 = SGMII_XPCS_1G_OP;
	desired_mode2 = SGMII_XPCS_2G5_OP;

#else
#error "Board not supported"
#endif

	mode = s32_get_xpcs_mode(serdes, 0);

	if (mode != desired_mode1 && mode != desired_mode2) {
		pr_err("Invalid SGMII configuration for GMAC%d\n", gmac_no);
		pr_err("Check hwconfig env. var.\n");
		return -EINVAL;
	}

	s32_sgmii_wait_link(serdes, xpcs);

	return 0;
}

static int eqos_probe_resources_s32cc(struct udevice *dev)
{
	struct eqos_pdata *pdata = dev_get_platdata(dev);
	const char *env_var = env_var_mode_name(dev);
	char *env_mode = env_get(env_var);
	int gmac_no = eqos_num(dev);
	struct s32cc_priv *s32cc = s32ccgmac_get_priv(dev);

	if (!s32cc || !pdata)
		return -EINVAL;

	s32cc->mac_intf = pdata->eth.phy_interface;

	if (s32cc->mac_intf == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	if (!s32ccgmac_cfg_set_interface(dev, s32cc->mac_intf)) {
		pr_err("HW setting error\n");
		return -EINVAL;
	}

	if (env_mode && !strcmp(env_mode, "disable")) {
		return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_DISABLE);
	}

	if (eqos_get_interface_s32cc(dev) == PHY_INTERFACE_MODE_SGMII) {
		int ret = check_sgmii_cfg(gmac_no);

		if (ret)
			return ret;
	}

	return s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_ENABLE);
}

static int eqos_remove_resources_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_pre_init_s32cc(struct udevice *dev)
{
	const char *env_var = env_var_mode_name(dev);
	char *env_mode = env_get(env_var);
	int ret;

	ret = s32ccgmac_alloc_priv(dev);
	if (ret)
		return ret;

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

/* command line */
static int do_s32ccgmac_cmd(cmd_tbl_t *cmdtp, int flag,
			    int argc, char * const argv[])
{
	unsigned long devnum;
	struct udevice *dev;
	struct s32cc_priv *s32cc;
	int coffs;
	/* check if device index was entered */
	devnum = simple_strtoul(argv[1], NULL, 10);
	if (strict_strtoul(argv[1], 10, &devnum)) {
		devnum = 0;
		coffs = 0;
	} else {
		coffs = 1;
	}

	dev = s32ccgmac_get_dev_by_idx(devnum);
	if (!dev) {
		pr_err("ERROR: device instance %lu does't exist\n", devnum);
		return CMD_RET_FAILURE;
	}

	s32cc = s32ccgmac_get_priv(dev);
	if (!s32cc)
		return CMD_RET_FAILURE;

	/* process command */
	if (!strcmp(argv[1 + coffs], "info")) {
		printf("GMAC%lu mode: %s\n", devnum,
		       s32ccgmac_cfg_get_mode(devnum) == S32CCGMAC_MODE_DISABLE
		       ? "disabled" : "enabled");
		printf("interface: %s\n",
		       s32ccgmac_cfg_get_interface_mode_str(s32cc));
		return 0;
	} else if (!strcmp(argv[1 + coffs], "enable")) {
		s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_ENABLE);
	} else if (!strcmp(argv[1 + coffs], "disable")) {
		s32ccgmac_cfg_set_mode(dev, S32CCGMAC_MODE_DISABLE);
		return 0;
	} else if (!strcmp(argv[1 + coffs], "emac")) {
		if (argc < 3) {
			printf("interface: %s\n",
				   s32ccgmac_cfg_get_interface_mode_str(s32cc));
		} else {
			phy_interface_t new_intf;

			if (!strcmp(argv[2 + coffs], "sgmii")) {
				new_intf = PHY_INTERFACE_MODE_SGMII;
			} else if (!strcmp(argv[2 + coffs], "rgmii")) {
				new_intf = PHY_INTERFACE_MODE_RGMII;
			} else if (!strcmp(argv[2 + coffs], "rmii")) {
				new_intf = PHY_INTERFACE_MODE_RMII;
			} else if (!strcmp(argv[2 + coffs], "mii")) {
				new_intf = PHY_INTERFACE_MODE_MII;
			} else if (!strcmp(argv[2 + coffs], "none")) {
				new_intf = PHY_INTERFACE_MODE_NONE;
			} else {
				pr_err("Invalid PHY interface type '%s'\n",
				       argv[2 + coffs]);
				return CMD_RET_USAGE;
			}
			s32ccgmac_cfg_set_interface(dev, new_intf);
		}
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	   s32ccgmac, 3, 0, do_s32ccgmac_cmd,
	   "NXP S32cc GMAC controller info",
	   /*      */"[idx] info			  - important hw info\n"
	   "s32ccgmac [idx] [disable|enable]  - disable/enable gmac/eqos subsystem\n"
	   "s32ccgmac [idx] emac [<inf-mode>] - read or set MAC interface mode\n"
	   "s32ccgmac help					  - additional gmac commands"
);
