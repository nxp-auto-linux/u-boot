// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Imagination Technologies Limited
 * Copyright 2019-2023 NXP
 *
 */

#include <common.h>
#include <command.h>
#include <phy.h>
#include <asm/io.h>
#include <asm/arch/clock.h>

#include <clk.h>
#include <s32gen1_clk_utils.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/string.h>
#include <s32-cc/serdes_hwconfig.h>

#include "pfeng.h"

static u32 emac_intf[PFENG_EMACS_COUNT] = {
#if CONFIG_IS_ENABLED(NXP_S32GEVB_BOARD)
	PHY_INTERFACE_MODE_SGMII, /* ARQ107 on PROC board */
	PHY_INTERFACE_MODE_SGMII, /* ARQ107 on PLAT board */
	PHY_INTERFACE_MODE_RGMII  /* SJA1105 on PLAT board */
#endif
#if CONFIG_IS_ENABLED(NXP_S32GRDB_BOARD)
	PHY_INTERFACE_MODE_SGMII, /* SJA1110A */
	PHY_INTERFACE_MODE_SGMII, /* ARQ107/ARQ113 */
	PHY_INTERFACE_MODE_RGMII  /* KSZ9031 */
#endif
#if CONFIG_IS_ENABLED(CONFIG_TARGET_S32G274ABLUEBOX3)
	PHY_INTERFACE_MODE_SGMII, /* PFE_MAC0: SGMII */
	PHY_INTERFACE_MODE_NONE, /* disabled */
	PHY_INTERFACE_MODE_NONE  /* disabled */
#endif
};

static u32 pfeng_mode = PFENG_MODE_DISABLE;

static u32 pfeng_intf_to_s32g(u32 intf);
static inline bool pfeng_emac_type_is_valid(u32 idx, u32 mode);
static void print_emacs_mode(char *label);

/* pfeng cfg api */

u32 pfeng_cfg_get_mode(void)
{
	return pfeng_mode;
}

u32 pfeng_cfg_emac_get_interface(u32 idx)
{
	if (idx >= ARRAY_SIZE(emac_intf)) {
		pr_err("invalid emac index %d", idx);
		return PHY_INTERFACE_MODE_NONE;
	}
	return emac_intf[idx];
}

static bool pfeng_cfg_emac_set_interface(u32 idx, u32 mode)
{
	if (idx >= ARRAY_SIZE(emac_intf)) {
		pr_err("invalid emac index %d", idx);
		return false;
	}

	if (emac_intf[idx] == mode)
		/* already in the same mode */
		return true;

	if (!pfeng_emac_type_is_valid(idx, mode)) {
		pr_err("invalid interface mode for emac%i 0x%x", idx, mode);
		return false;
	}

	emac_intf[idx] = mode;

	return true;
}

static void switch_pfe0_clock(int intf)
{
	u32 csel = 0;

	if (intf == PHY_INTERFACE_MODE_SGMII)
		csel = SGMII_CSEL;

	/* Extra switch driving TX_CLK for PFE_EMAC_0 */
	writel(csel, S32G_MAIN_GENCTRL1);
}

static void set_clock_freq(const char *tx, const char *rx,
			   ulong tx_freq, ulong rx_freq,
			   struct udevice *pfe_dev)
{
	ulong rate;

	rate = s32gen1_set_dev_clk_rate(tx, pfe_dev, tx_freq);
	if (rate != tx_freq)
		dev_err(pfe_dev, "Failed to set the frequency of %s\n", tx);

	rate = s32gen1_set_dev_clk_rate(rx, pfe_dev, rx_freq);
	if (rate != rx_freq)
		dev_err(pfe_dev, "Failed to set the frequency of %s\n", rx);
}

static void set_clocks_state(const char *tx, const char *rx,
			     struct udevice *pfe_dev, bool enable)
{
	int ret;
	const char *op = (enable ? "enable" : "disable");

	ret = s32gen1_set_dev_clk_state(rx, pfe_dev, enable);
	if (ret)
		dev_err(pfe_dev, "Failed to %s %s clock\n", op, rx);

	ret = s32gen1_set_dev_clk_state(tx, pfe_dev, enable);
	if (ret)
		dev_err(pfe_dev, "Failed to %s %s clock\n", op, tx);
}

static ulong get_mac_mode_freq(u32 intf)
{
	ulong freq;

	switch (intf) {
	case PHY_INTERFACE_MODE_SGMII:
		freq = 125000000UL;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		freq = 125000000UL;
		break;
	case PHY_INTERFACE_MODE_RMII:
		freq = 25000000UL;
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_NONE:
	default:
		freq = 0;
		break;
	}

	return freq;
}

static void set_pfe_mac0_clk(u32 intf0, struct udevice *pfe_dev, bool enable)
{
	ulong freq;
	const char *rx, *tx;

	rx = NULL;
	tx = NULL;

	switch (intf0) {
	case PHY_INTERFACE_MODE_SGMII:
		switch_pfe0_clock(PHY_INTERFACE_MODE_SGMII);
		rx = "mac0_rx_sgmii";
		tx = "mac0_tx_sgmii";
		break;
	case PHY_INTERFACE_MODE_RGMII:
		rx = "mac0_rx_rgmii";
		tx = "mac0_tx_rgmii";
		break;
	case PHY_INTERFACE_MODE_RMII:
		rx = "mac0_rx_rmii";
		tx = "mac0_tx_rmii";
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_NONE:
	default:
		break;
	}

	if (!rx || !tx) {
		if (intf0 == PHY_INTERFACE_MODE_NONE)
			return;

		pr_err("pfe_mac0: Invalid operation mode: %s (%d)\n",
		       phy_string_for_interface(intf0), intf0);
		return;
	}

	freq = get_mac_mode_freq(intf0);
	if (enable)
		set_clock_freq(tx, rx, freq, freq, pfe_dev);
	set_clocks_state(tx, rx, pfe_dev, enable);
}

static void set_pfe_mac1_clk(u32 intf1, struct udevice *pfe_dev, bool enable)
{
	ulong freq;
	const char *rx, *tx;

	rx = NULL;
	tx = NULL;

	switch (intf1) {
	case PHY_INTERFACE_MODE_SGMII:
		rx = "mac1_rx_sgmii";
		tx = "mac1_tx_sgmii";
		break;
	case PHY_INTERFACE_MODE_RGMII:
		rx = "mac1_rx_rgmii";
		tx = "mac1_tx_rgmii";
		break;
	case PHY_INTERFACE_MODE_RMII:
		rx = "mac1_rx_rmii";
		tx = "mac1_tx_rmii";
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_NONE:
	default:
		break;
	}


	if (!rx || !tx) {
		if (intf1 == PHY_INTERFACE_MODE_NONE)
			return;

		pr_err("pfe_mac1: Invalid operation mode: %s (%d)\n",
		       phy_string_for_interface(intf1), intf1);
		return;
	}

	freq = get_mac_mode_freq(intf1);
	if (enable)
		set_clock_freq(tx, rx, freq, freq, pfe_dev);
	set_clocks_state(tx, rx, pfe_dev, enable);
}

static void set_pfe_mac2_clk(u32 intf2, struct udevice *pfe_dev, bool enable)
{
	ulong freq;
	const char *rx, *tx;

	rx = NULL;
	tx = NULL;

	switch (intf2) {
	case PHY_INTERFACE_MODE_SGMII:
		rx = "mac2_rx_sgmii";
		tx = "mac2_tx_sgmii";
		break;
	case PHY_INTERFACE_MODE_RGMII:
		rx = "mac2_rx_rgmii";
		tx = "mac2_tx_rgmii";
		break;
	case PHY_INTERFACE_MODE_RMII:
		rx = "mac2_rx_rmii";
		tx = "mac2_tx_rmii";
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_NONE:
	default:
		break;
	}


	if (!rx || !tx) {
		if (intf2 == PHY_INTERFACE_MODE_NONE)
			return;

		pr_err("pfe_mac2: Invalid operation mode: %s (%d)\n",
		       phy_string_for_interface(intf2), intf2);
		return;
	}

	freq = get_mac_mode_freq(intf2);
	if (enable)
		set_clock_freq(tx, rx, freq, freq, pfe_dev);
	set_clocks_state(tx, rx, pfe_dev, enable);
}

static void setup_pfe_clocks(u32 intf0, u32 intf1, u32 intf2,
			     struct udevice *pfe_dev, bool enable)
{
	const char *op = (enable ? "enable" : "disable");
	int ret;
	struct pfeng_priv *priv;

	if (!pfe_dev) {
		pr_err("%s: Invalid PFE device\n", __func__);
		return;
	}

	priv = dev_get_priv(pfe_dev);
	if (!priv) {
		pr_err("%s: Invalid PFE device data\n", __func__);
		return;
	}

	/* Apply clock setting to all EMAC ports for first time only.
	 * Otherwise setup clocks only on focused EMAC port
	 *
	 * Disable all clocks together when requested.
	 */
	if ((!priv->clocks_done && enable) || (priv->clocks_done && !enable)) {
		if (enable) {
			ret = s32gen1_set_dev_clk_state("pe", pfe_dev, enable);
			if (ret)
				dev_err(pfe_dev, "Failed to %s pfe_pe clock\n", op);
		}

		set_pfe_mac0_clk(intf0, pfe_dev, enable);
		set_pfe_mac1_clk(intf1, pfe_dev, enable);
		set_pfe_mac2_clk(intf2, pfe_dev, enable);

		ret = s32gen1_set_dev_clk_state("ts", pfe_dev, enable);
		if (ret)
			dev_err(pfe_dev, "Failed to %s ts clock\n", op);

		if (!enable) {
			ret = s32gen1_set_dev_clk_state("pe", pfe_dev, enable);
			if (ret)
				dev_err(pfe_dev, "Failed to %s pfe_pe clock\n", op);
		}

		/* Toggle the status */
		priv->clocks_done = !priv->clocks_done;
	} else if (priv->if_index == 0) {
		set_pfe_mac0_clk(intf0, pfe_dev, enable);
	} else if (priv->if_index == 1) {
		set_pfe_mac1_clk(intf1, pfe_dev, enable);
	} else if (priv->if_index == 2) {
		set_pfe_mac2_clk(intf2, pfe_dev, enable);
	}
}

unsigned long long get_pfe_axi_clk_f(struct udevice *pfe_dev)
{
	return s32gen1_get_dev_clk_rate("axi", pfe_dev);
}

static void setup_iomux_pfe(struct udevice *dev,
			    int intf0, int intf1, int intf2)
{
	/* EMAC 0 */

	switch (intf0) {
	case PHY_INTERFACE_MODE_SGMII:
		pinctrl_select_state(dev, "pfe0_sgmii");
		break;

	case PHY_INTERFACE_MODE_RGMII:
		pinctrl_select_state(dev, "pfe0_rgmii");
		break;

	case PHY_INTERFACE_MODE_RMII:
		pinctrl_select_state(dev, "pfe0_rmii");
		break;

	case PHY_INTERFACE_MODE_MII:
		/* TODO */
		break;

	default:
		break;
	}

	/* EMAC 1 */

	switch (intf1) {
	case PHY_INTERFACE_MODE_SGMII:
		pinctrl_select_state(dev, "pfe1_sgmii");
		break;

	case PHY_INTERFACE_MODE_RGMII:
		pinctrl_select_state(dev, "pfe1_rgmii");
		break;

	case PHY_INTERFACE_MODE_RMII:
		pinctrl_select_state(dev, "pfe1_rmii");
		break;

	case PHY_INTERFACE_MODE_MII:
		/* TODO */
		break;

	default:
		break;
	}

	/* EMAC 2 */

	switch (intf2) {
	case PHY_INTERFACE_MODE_SGMII:
		pinctrl_select_state(dev, "pfe2_sgmii");
		break;

	case PHY_INTERFACE_MODE_RGMII:
		pinctrl_select_state(dev, "pfe2_rgmii");
		break;

	case PHY_INTERFACE_MODE_RMII:
		pinctrl_select_state(dev, "pfe2_rmii");
		break;

	case PHY_INTERFACE_MODE_MII:
		/* TODO */
		break;

	default:
		break;
	}
}

/* setup all EMACs clocks */
void pfeng_apply_clocks(struct udevice *pfe_dev)
{
	setup_pfe_clocks(emac_intf[0], emac_intf[1],
			 emac_intf[2], pfe_dev, true);
}

/* disable power for EMACs */
void pfeng_cfg_emacs_disable_all(void)
{
	writel(GPR_PFE_EMACn_PWR_DWN(0) |
	       GPR_PFE_EMACn_PWR_DWN(1) |
	       GPR_PFE_EMACn_PWR_DWN(2),
	       S32G_PFE_PRW_CTRL);
}

/* enable power for EMACs */
void pfeng_cfg_emacs_enable_all(void)
{
	int i;

	pfeng_cfg_emacs_disable_all();

	writel((pfeng_intf_to_s32g(emac_intf[2]) << 8) |
		(pfeng_intf_to_s32g(emac_intf[1]) << 4) |
		(pfeng_intf_to_s32g(emac_intf[0])),
		(unsigned long)S32G_PFE_EMACS_INTF_SEL);
	udelay(100);
	writel(0, S32G_PFE_PRW_CTRL);

	/* reset all EMACs */
	for (i = 0; i < PFENG_EMACS_COUNT; i++) {
		writel(readl((unsigned long)S32G_PFE_EMACn_MODE(i))
		       | EMAC_MODE_SWR_MASK,
		       (unsigned long)S32G_PFE_EMACn_MODE(i));
		udelay(10);
		while (readl((unsigned long)S32G_PFE_EMACn_MODE(i))
		       & EMAC_MODE_SWR_MASK)
			udelay(10);
	}
}

static int pfeng_cfg_mode_disable(struct udevice *pfe_dev)
{
	/* disable all EMACs to allow interface change */
	pfeng_cfg_emacs_disable_all();

	if (pfe_dev)
		setup_pfe_clocks(emac_intf[0], emac_intf[1],
				 emac_intf[2], pfe_dev, false);

	return 0;
}

static int pfeng_cfg_mode_enable(struct udevice *pfe_dev)
{
	struct pfeng_priv *priv = pfe_dev ? dev_get_priv(pfe_dev) : NULL;

	if (!pfe_dev || !priv) {
		pr_err("%s: Invalid PFE device\n", __func__);
		return -EINVAL;
	}

	/* Setup PFE coherency for all HIFs */
	writel(PFE_COH_PORTS_MASK_HIF_0_3, S32G_PFE_COH_EN);

	/* Setup pins */
	setup_iomux_pfe(pfe_dev, emac_intf[0], emac_intf[1], emac_intf[2]);

	/* Setup clocks */
	priv->clocks_done = false;
	setup_pfe_clocks(emac_intf[0], emac_intf[1],
			 emac_intf[2], pfe_dev, true);

	return 0;
}

bool pfeng_cfg_set_mode(u32 mode, struct udevice *pfe_dev)
{
	int ret = EINVAL;

	if (pfeng_mode == mode)
		/* already in the same mode */
		return true;

	switch (mode) {
	case PFENG_MODE_DISABLE:
		ret = pfeng_cfg_mode_disable(pfe_dev);
		break;
	case PFENG_MODE_ENABLE:
		ret = pfeng_cfg_mode_enable(pfe_dev);
		break;
	}

	if (!ret) {
		pfeng_mode = mode;
		print_emacs_mode(" PFE: ");
	}

	return !ret;
}

static bool parse_interface_name(char *modestr, int *intf)
{
	if (!strcmp(modestr, "mii"))
		*intf = PHY_INTERFACE_MODE_MII;
	else if (!strcmp(modestr, "rmii"))
		*intf = PHY_INTERFACE_MODE_RMII;
	else if (!strcmp(modestr, "rgmii"))
		*intf = PHY_INTERFACE_MODE_RGMII;
	else if (!strcmp(modestr, "sgmii"))
		*intf = PHY_INTERFACE_MODE_SGMII;
	else if (!strcmp(modestr, "none"))
		*intf = PHY_INTERFACE_MODE_NONE;
	else
		return false;
	return true;
}

int pfeng_set_emacs_from_env(char *env_mode)
{
	char *tok, *loc_mode;
	int i, intf[PFENG_EMACS_COUNT] = { -1, -1, -1 };

	loc_mode = strdup(env_mode);
	tok = strtok(loc_mode, ",");
	for (i = 0; i < PFENG_EMACS_COUNT; i++) {
		if (!tok)
			break;
		if (!parse_interface_name(tok, &intf[i])) {
			pr_err("invalid interface name for emac%d", i);
			return CMD_RET_USAGE;
		}
		tok = strtok(NULL, ",");
	}

	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		if (intf[i] > -1)
			pfeng_cfg_emac_set_interface(i, intf[i]);

	/* set INTF_SEL */
	writel((pfeng_intf_to_s32g(emac_intf[2]) << 8) |
		(pfeng_intf_to_s32g(emac_intf[1]) << 4) |
		(pfeng_intf_to_s32g(emac_intf[0])),
		S32G_PFE_EMACS_INTF_SEL);

	return 0;
}

/* command interface */

static inline bool pfeng_emac_type_is_valid(u32 idx, u32 mode)
{
	return (mode == PHY_INTERFACE_MODE_NONE ||
		mode == PHY_INTERFACE_MODE_SGMII ||
		mode == PHY_INTERFACE_MODE_RGMII ||
		mode == PHY_INTERFACE_MODE_RMII ||
		mode == PHY_INTERFACE_MODE_MII);
}

static u32 pfeng_intf_to_s32g(u32 intf)
{
	switch (intf) {
	case PHY_INTERFACE_MODE_MII:
		return GPR_PFE_EMAC_IF_MII;
	case PHY_INTERFACE_MODE_RMII:
		return GPR_PFE_EMAC_IF_RMII;
	case PHY_INTERFACE_MODE_RGMII:
		return GPR_PFE_EMAC_IF_RGMII;
	default:
		return GPR_PFE_EMAC_IF_SGMII; /* SGMII mode by default */
	}
}

static const char *pfeng_emac_get_interface_type_str(u32 idx)
{
	if (idx >= ARRAY_SIZE(emac_intf)) {
		pr_err("invalid emac index %d", idx);
		return "<invalid>";
	}

	return strlen(phy_string_for_interface(emac_intf[idx])) ?
	       phy_string_for_interface(emac_intf[idx]) : "none";
}

static void print_emacs_mode(char *label)
{
	printf("%semac0: %s emac1: %s emac2: %s\n", label,
	       pfeng_emac_get_interface_type_str(0),
	       pfeng_emac_get_interface_type_str(1),
	       pfeng_emac_get_interface_type_str(2));
}

const char *pfeng_cfg_get_mode_str(void)
{
	static const char *modes[3] = {
		"disable",
		"enable",
		"enable/run"
	};

	return modes[pfeng_cfg_get_mode()];
}

static int do_pfeng_cmd(cmd_tbl_t *cmdtp, int flag,
		       int argc, char * const argv[])
{
	char *env_mode = env_get(PFENG_ENV_VAR_MODE_NAME);
	struct udevice *pfe_dev = eth_get_dev_by_name("eth_pfeng");

	if (!env_mode) {
		/* set the default mode */
		env_set(PFENG_ENV_VAR_MODE_NAME,
			PFENG_MODE_DEFAULT == PFENG_MODE_ENABLE ?
			"enable" : "disable");
	}

	/* process command */
	if (!strcmp(argv[1], "info")) {
		char *env_fw = env_get(PFENG_ENV_VAR_FW_SOURCE);

		printf("PFE mode: %s\n", pfeng_cfg_get_mode_str());
		print_emacs_mode("  ");
		if (env_fw)
			printf("  fw: '%s' (from env)\n", env_fw);
		else
#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
			printf("  fw: '%s' on mmc@%s\n",
			       CONFIG_FSL_PFENG_FW_NAME,
			       CONFIG_FSL_PFENG_FW_PART);
#else
			printf("  fw: on qspi@%s\n",
			       CONFIG_FSL_PFENG_FW_PART);
#endif
		return 0;
	} else if (!strcmp(argv[1], "disable")) {
		pfeng_cfg_set_mode(PFENG_MODE_DISABLE, pfe_dev);
		return 0;
	} else if (!strcmp(argv[1], "enable")) {
		pfeng_cfg_set_mode(PFENG_MODE_ENABLE, pfe_dev);
		return 0;
	} else if (!strcmp(argv[1], "stop")) {
		if (pfeng_cfg_get_mode() > PFENG_MODE_DISABLE) {
			/* we emulate STOP by DISABLE/ENABLE */
			pfeng_cfg_set_mode(PFENG_MODE_DISABLE, pfe_dev);
			pfeng_cfg_set_mode(PFENG_MODE_ENABLE, pfe_dev);
		}
		return 0;
	} else if (!strcmp(argv[1], "emacs")) {
		if (argc < 3) {
			print_emacs_mode("  ");
			return 0;
		} else if (!strcmp(argv[2], "reapply-clocks")) {
			setup_iomux_pfe(pfe_dev, emac_intf[0], emac_intf[1],
					emac_intf[2]);
			pfeng_apply_clocks(pfe_dev);
			printf("PFE reapply clocks\n");
			print_emacs_mode("  ");
			return 0;
		} else {
			/* parse argv[2] for "rgmii,none,mii" */
			if (pfeng_set_emacs_from_env(argv[2]))
				return CMD_RET_USAGE;
			return 0;
		}
	} else if (!strcmp(argv[1], "reg")) {
		u32 reg, offs = 0;

		if (argc != 3)
			return CMD_RET_USAGE;

		offs = simple_strtoul(argv[2], NULL, 16);
		reg = readl(((void *)(S32G_PFE_REGS_BASE)) + offs);
		printf("reg 0x%x at 0x%p: %08x\n", offs,
		       ((void *)(S32G_PFE_REGS_BASE)) + offs,
		       reg);
		return 0;
	/* for development only */
	} else if (!strcmp(argv[1], "debug")) {
		pfeng_debug();
		return 0;
	} else if (!strcmp(argv[1], "help")) {
	}

	return CMD_RET_USAGE;
}

U_BOOT_CMD(
	   pfeng, 6, 0, do_pfeng_cmd,
	   "PFE controller info",
	   /*  */"info                              - important hw info\n"
	   "pfeng [disable|enable]                  - disable/enable full PFE/EMACs subsystem\n"
	   "pfeng stop                              - stop the driver but don't disable PFE/EMACs\n"
	   "pfeng emacs [<inf0-mode>,<intf1-mode>,<intf2-mode>] - read or set EMAC0-2 interface mode\n"
	   "pfeng emacs reapply-clocks              - reapply clock setting for all EMACs\n"
	   "pfeng reg <offset>                      - read register"
);
