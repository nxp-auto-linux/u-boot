// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2022 NXP
 * SERDES driver for S32CC SoCs
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <hwconfig.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/of_access.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>
#include <linux/time.h>

#include "serdes_regs.h"
#include "serdes_s32gen1_io.h"
#include "sgmii.h"

#define SERDES_CLK_MODE(EXT_CLK) \
			((EXT_CLK) ? "external" : "internal")
#define SERDES_CLK_FMHZ(clk_type) \
			((clk_type == MHZ_100) ? "100Mhz" : "125Mhz")

#define PCIE_PHY_GEN_CTRL	(0x0)
#define  REF_USE_PAD_MASK	BIT(17)
#define  RX_SRIS_MODE_MASK	BIT(9)
#define PCIE_PHY_MPLLA_CTRL	(0x10)
#define  MPLLA_STATE_MASK	BIT(31)
#define  MPLL_STATE_MASK	BIT(30)
#define PCIE_PHY_MPLLB_CTRL	(0x14U)
#define  MPLLB_SSC_EN_MASK	BIT(1)
#define PCIE_PHY_EXT_CTRL_SEL	(0x18U)
#define  EXT_PHY_CTRL_SEL	BIT(0)
#define PCIE_PHY_EXT_BS_CTRL	(0x1cU)
#define  EXT_BS_TX_LOWSWING	BIT(6)
#define  EXT_BS_RX_BIGSWING	BIT(5)
#define  EXT_BS_RX_LEVEL(x)	(((x) & 0x1fU) << 0)
#define PCIE_PHY_REF_CLK_CTRL	(0x20U)
#define  EXT_REF_RANGE(x)	(((x) & 0x7U) << 3)
#define  REF_CLK_DIV2_EN	BIT(2)
#define  REF_CLK_MPLLB_DIV2_EN	BIT(1)
#define PCIE_PHY_EXT_MPLLA_CTRL_1	(0x30U)
#define  EXT_MPLLA_BANDWIDTH(x)		(((x) & 0xffffU) << 0)
#define PCIE_PHY_EXT_MPLLB_CTRL_1	(0x40U)
#define  EXT_MPLLB_DIV_MULTIPLIER(x)	(((x) & 0xffU) << 24)
#define  EXT_MPLLB_DIV_CLK_EN	BIT(19)
#define  EXT_MPLLB_DIV8_CLK_EN	BIT(18)
#define  EXT_MPLLB_DIV10_CLK_EN	BIT(16)
#define  EXT_MPLLB_BANDWIDTH(x)	(((x) & 0xffffU) << 0)
#define PCIE_PHY_EXT_MPLLB_CTRL_2	(0x44U)
#define  EXT_MPLLB_FRACN_CTRL(x)	(((x) & 0x7ffU) << 12)
#define PCIE_PHY_EXT_MPLLB_CTRL_3	(0x48U)
#define  EXT_MPLLB_WORD_DIV2_EN		BIT(31)
#define  EXT_MPLLB_TX_CLK_DIV(x)	(((x) & 0x7U) << 28)
#define PCIE_PHY_EXT_MISC_CTRL_1	(0xa0U)
#define  EXT_RX_LOS_THRESHOLD(x)	(((x) & 0x3fU) << 1)
#define  EXT_RX_VREF_CTRL(x)		(((x) & 0x1fU) << 24)
#define PCIE_PHY_EXT_MISC_CTRL_2	(0xa4U)
#define  EXT_TX_VBOOST_LVL(x)		(((x) & 0x7U) << 16)
#define  EXT_TX_TERM_CTRL(x)		(((x) & 0x7U) << 24)
#define PCIE_PHY_XPCS1_RX_OVRD_CTRL	(0xd0U)
#define  XPCS1_RX_VCO_LD_VAL(x)		(((x) & 0x1fffU) << 16)
#define  XPCS1_RX_REF_LD_VAL(x)		(((x) & 0x3fU) << 8)
#define SS_RW_REG_0		(0xf0)
#define  SUBMODE_MASK		(0x7)
#define  CLKEN_MASK		BIT(23)
#define  PHY0_CR_PARA_SEL_MASK	BIT(9)

#define PHY_REG_ADDR		(0x0)
#define  PHY_REG_EN		BIT(31)
#define PHY_REG_DATA		(0x4)

#define RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3019)
#define RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3119)

#define SERDES_LOCK_TIMEOUT_US	(10 * USEC_PER_MSEC)

#define EXTERNAL_CLK_NAME	"ext"
#define INTERNAL_CLK_NAME	"ref"

#define SERDES_PCIE_FREQ	100000000

struct pcie_ctrl {
	struct reset_ctl *rst;
	void __iomem *phy_base;
	bool powered_on[2];
	bool initialized_phy;
};

struct serdes_ctrl {
	struct reset_ctl *rst;
	void __iomem *ss_base;
	struct {
		struct clk clk;
		bool enabled;
	} clks[5];
	u32 ss_mode;
	enum pcie_phy_mode phy_mode;
	bool ext_clk;
};

struct xpcs_ctrl {
	void __iomem *base0, *base1;
};

struct serdes {
	struct pcie_ctrl pcie;
	struct serdes_ctrl ctrl;
	struct xpcs_ctrl xpcs;
	struct udevice *dev;

	int id;
	enum serdes_dev_type devtype;
	enum serdes_xpcs_mode xpcs_mode;
};

static const char * const serdes_clk_names[] = {
	"axi", "aux", "apb", "ref", "ext"
};

static void pcie_phy_write(struct serdes *serdes, u32 reg, u32 val)
{
	writel(PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	writel(reg | PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	udelay(100);
	writel(val, UPTR(serdes->pcie.phy_base) + PHY_REG_DATA);
	udelay(100);
}

static struct clk *get_serdes_clk(struct serdes *serdes, const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(serdes->ctrl.clks); i++) {
		if (!strcmp(serdes_clk_names[i], name) &&
		    serdes->ctrl.clks[i].enabled)
			return &serdes->ctrl.clks[i].clk;
	}

	return NULL;
}

static int get_clk_rate(struct serdes *serdes, unsigned long *rate)
{
	__maybe_unused struct udevice *dev = serdes->dev;
	struct clk *clk;

	if (serdes->ctrl.ext_clk)
		clk = get_serdes_clk(serdes, EXTERNAL_CLK_NAME);
	else
		clk = get_serdes_clk(serdes, INTERNAL_CLK_NAME);

	if (!clk) {
		dev_err(dev, "Failed to determine SerDes clock\n");
		return -EINVAL;
	}

	*rate = clk_get_rate(clk);
	return 0;
}

static int check_pcie_clk(struct serdes *serdes)
{
	__maybe_unused struct udevice *dev = serdes->dev;
	unsigned long rate;
	int ret;

	ret = get_clk_rate(serdes, &rate);
	if (ret)
		return ret;

	if (rate != SERDES_PCIE_FREQ) {
		dev_err(dev, "PCIe PHY cannot operate at %lu HZ\n", rate);
		return -EINVAL;
	}

	return 0;
}

static int pci_phy_power_on_common(struct serdes *serdes)
{
	struct serdes_ctrl *sctrl = &serdes->ctrl;
	struct pcie_ctrl *pcie = &serdes->pcie;
	u32 ctrl, reg0, val, mask;
	int ret;

	if (pcie->initialized_phy)
		return 0;

	ret = check_pcie_clk(serdes);
	if (ret)
		return ret;

	ctrl = readl(UPTR(sctrl->ss_base) + PCIE_PHY_GEN_CTRL);

	/* if PCIE PHY is in SRIS mode */
	if (sctrl->phy_mode == SRIS)
		ctrl |= RX_SRIS_MODE_MASK;

	if (sctrl->ext_clk)
		ctrl |= REF_USE_PAD_MASK;
	else
		ctrl &= ~REF_USE_PAD_MASK;

	/* Monitor Serdes MPLL state */
	writel(ctrl, UPTR(sctrl->ss_base) + PCIE_PHY_GEN_CTRL);

	mask = MPLL_STATE_MASK;
	ret = readl_poll_timeout(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_MPLLA_CTRL,
				 val, (val & mask) == mask,
				 SERDES_LOCK_TIMEOUT_US);
	if (ret) {
		dev_err(serdes->dev, "Failed to lock PCIe phy\n");
		return -ETIMEDOUT;
	}

	/* Set PHY register access to CR interface */
	reg0 = readl(UPTR(sctrl->ss_base) + SS_RW_REG_0);
	reg0 |=  PHY0_CR_PARA_SEL_MASK;
	writel(reg0, UPTR(sctrl->ss_base) + SS_RW_REG_0);

	pcie->initialized_phy = true;
	return 0;
}

static int pcie_phy_power_on(struct serdes *serdes, int id)
{
	struct pcie_ctrl *pcie = &serdes->pcie;
	u32 iq_ovrd_in;
	int ret;

	ret = pci_phy_power_on_common(serdes);
	if (ret)
		return ret;

	/* RX_EQ_DELTA_IQ_OVRD enable and override value for PCIe lanes */
	if (!id)
		iq_ovrd_in = RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN;
	else
		iq_ovrd_in = RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN;

	pcie_phy_write(serdes, iq_ovrd_in, 0x3);
	pcie_phy_write(serdes, iq_ovrd_in, 0x13);
	pcie->powered_on[id] = true;

	return 0;
}

static int s32_serdes_assert_reset(struct serdes *serdes)
{
	__maybe_unused struct udevice *dev = serdes->dev;
	int ret;

	ret = reset_assert(serdes->pcie.rst);
	if (ret) {
		dev_err(dev, "Failed to assert SerDes reset: %d\n", ret);
		return ret;
	}

	ret = reset_assert(serdes->ctrl.rst);
	if (ret) {
		dev_err(dev, "Failed to assert SerDes reset: %d\n", ret);
		return ret;
	}

	return 0;
}

static int deassert_reset(struct serdes *serdes)
{
	__maybe_unused struct udevice *dev = serdes->dev;
	int ret;

	ret = reset_deassert(serdes->pcie.rst);
	if (ret) {
		dev_err(dev, "Failed to deassert SerDes reset: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(serdes->ctrl.rst);
	if (ret) {
		dev_err(dev, "Failed to deassert SerDes reset: %d\n", ret);
		return ret;
	}

	return 0;
}

static int init_serdes(struct serdes *serdes)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	u32 reg0;
	int ret;

	ret = assert_reset(serdes);
	if (ret)
		return ret;

	reg0 = readl(UPTR(ctrl->ss_base) + SS_RW_REG_0);
	reg0 &= ~SUBMODE_MASK;
	reg0 |= ctrl->ss_mode;
	writel(reg0, UPTR(ctrl->ss_base) + SS_RW_REG_0);

	reg0 = readl(UPTR(ctrl->ss_base) + SS_RW_REG_0);
	if (ctrl->ext_clk)
		reg0 &= ~CLKEN_MASK;
	else
		reg0 |= CLKEN_MASK;

	writel(reg0, UPTR(ctrl->ss_base) + SS_RW_REG_0);

	udelay(100);

	ret = deassert_reset(serdes);
	if (ret)
		return ret;

	dev_info(serdes->dev, "Using mode %d for SerDes subsystem\n",
		 ctrl->ss_mode);

	return 0;
}

static void s32_serdes_xpcs1_pma_config(struct serdes *serdes)
{
	/* Configure TX_VBOOST_LVL and TX_TERM_CTRL */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MISC_CTRL_2,
	      EXT_TX_VBOOST_LVL(0x3) | EXT_TX_TERM_CTRL(0x4),
	      EXT_TX_VBOOST_LVL(0x7) | EXT_TX_TERM_CTRL(0x7));
	/* Enable phy external control */
	BSET32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_CTRL_SEL,
	       EXT_PHY_CTRL_SEL);
	/* Configure ref range, disable PLLB/ref div2 */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_REF_CLK_CTRL,
	      EXT_REF_RANGE(0x3),
	      REF_CLK_DIV2_EN | REF_CLK_MPLLB_DIV2_EN | EXT_REF_RANGE(0x7));
	/* Configure multiplier */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_2,
	      MPLLB_MULTIPLIER(0x27U) | EXT_MPLLB_FRACN_CTRL(0x414),
	      MPLLB_MULTIPLIER(0xffU) | EXT_MPLLB_FRACN_CTRL(0x7ff) |
	      1 << 24U | 1 << 28U);

	BCLR32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_MPLLB_CTRL,
	       MPLLB_SSC_EN_MASK);

	/* Configure tx lane division, disable word clock div2*/
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_3,
	      EXT_MPLLB_TX_CLK_DIV(0x5),
	      EXT_MPLLB_WORD_DIV2_EN | EXT_MPLLB_TX_CLK_DIV(0x7));

	/* Configure configure bandwidth for filtering and div10*/
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_1,
	      EXT_MPLLB_BANDWIDTH(0x5f) | EXT_MPLLB_DIV10_CLK_EN,
	      EXT_MPLLB_BANDWIDTH(0xffff) | EXT_MPLLB_DIV_CLK_EN |
	      EXT_MPLLB_DIV8_CLK_EN | EXT_MPLLB_DIV_MULTIPLIER(0xff));

	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLA_CTRL_1,
	      EXT_MPLLA_BANDWIDTH(0xc5), EXT_MPLLA_BANDWIDTH(0xffff));

	/* Configure VCO */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_XPCS1_RX_OVRD_CTRL,
	      XPCS1_RX_VCO_LD_VAL(0x540U) | XPCS1_RX_REF_LD_VAL(0x2bU),
	      XPCS1_RX_VCO_LD_VAL(0x1fffU) | XPCS1_RX_REF_LD_VAL(0x3fU));

	/* Boundary scan control */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_BS_CTRL,
	      EXT_BS_RX_LEVEL(0xb) | EXT_BS_RX_BIGSWING,
	      EXT_BS_RX_LEVEL(0x1f) | EXT_BS_TX_LOWSWING);

	/* Rx loss threshold */
	RMW32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MISC_CTRL_1,
	      EXT_RX_LOS_THRESHOLD(0x3U) | EXT_RX_VREF_CTRL(0x11U),
	      EXT_RX_LOS_THRESHOLD(0x3fU) | EXT_RX_VREF_CTRL(0x1fU));
}

static void s32_serdes_start_mode5(struct serdes *serdes,
				   enum serdes_xpcs_mode_gen2 xpcs[2])
{
	if (!s32_serdes_has_mode5_enabled(serdes->id))
		return;

	printf("SerDes%d: Enabling serdes mode5\n", serdes->id);
	/* Initialize PMA */
	serdes_pma_mode5(serdes->xpcs.base1);
	/* Initialize PHY */
	s32_serdes_xpcs1_pma_config(serdes);
	/* Initialize PCS */
	serdes_pcs_mode5(serdes->xpcs.base1);
	/* mode5 representation */
	xpcs[0] = SGMII_XPCS_PCIE;
	xpcs[1] = SGMII_XPCS_2G5_OP;
}

__weak int s32_eth_xpcs_init(void __iomem *xpcs0, void __iomem *xpcs1,
			     int id, u32 ss_mode,
			     enum serdes_xpcs_mode xpcs_mode,
			     bool ext_clk,
			     unsigned long fmhz,
			     enum serdes_xpcs_mode_gen2 xpcs[2])
{
	/* Configure SereDes XPCS for PFE/GMAC*/
	printf("SerDes%d: XPCS is disabled\n", id);
	return -ENODEV;
}

static const char *s32_serdes_get_pcie_phy_mode(struct serdes *serdes)
{
	if (serdes->ctrl.phy_mode == CRSS)
		return "CRSS";
	else if (serdes->ctrl.phy_mode == SRIS)
		return "SRIS";

	/* Default PCIE PHY mode */
	return "CRNS";
}

static int get_serdes_alias_id(struct udevice *dev, int *devnump)
{
	ofnode node = dev_ofnode(dev);
	const char *uc_name = "serdes";
	int ret;

	if (ofnode_is_np(node)) {
		ret = of_alias_get_id(ofnode_to_np(node), uc_name);
		if (ret >= 0) {
			*devnump = ret;
			ret = 0;
		}
	} else {
		ret = fdtdec_get_alias_seq(gd->fdt_blob, uc_name,
					   ofnode_to_offset(node), devnump);
	}

	return ret;
}

static int ss_dt_init(struct udevice *dev, struct serdes *serdes)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	struct resource res = {};
	int ret = 0;
	size_t i;

	ret = get_serdes_alias_id(dev, &serdes->id);
	if (ret < 0) {
		printf("Failed to get SerDes device id\n");
		return ret;
	}

	ret = dev_read_u32(dev, "fsl,sys-mode", &ctrl->ss_mode);
	if (ret) {
		dev_err(dev, "Failed to get SerDes subsystem mode\n");
		return -EINVAL;
	}

	ret = dev_read_resource_byname(dev, "ss_pcie", &res);
	if (ret) {
		dev_err(dev, "Missing 'ss_pcie' reg region.\n");
		return -EIO;
	}

	ctrl->ss_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (!ctrl->ss_base) {
		dev_err(dev, "Failed to map 'ss_pcie'\n");
		return -ENOMEM;
	}

	serdes->ctrl.rst = devm_reset_control_get(dev, "serdes");
	if (IS_ERR(serdes->ctrl.rst)) {
		dev_err(dev, "Failed to get 'serdes' reset control\n");
		return PTR_ERR(serdes->ctrl.rst);
	}

	for (i = 0; i < ARRAY_SIZE(serdes_clk_names); i++) {
		ret = clk_get_by_name(dev, serdes_clk_names[i],
				      &ctrl->clks[i].clk);
		/* Only a subset of the clock may be available */
		if (ret) {
			ctrl->clks[i].enabled = false;
			continue;
		}

		ret = clk_enable(&ctrl->clks[i].clk);
		if (ret) {
			dev_err(dev, "Failed to enable '%s' clock\n",
				serdes_clk_names[i]);
		}
		ctrl->clks[i].enabled = true;
	}

	if (get_serdes_clk(serdes, EXTERNAL_CLK_NAME))
		ctrl->ext_clk = true;

	return 0;
}

static int pcie_dt_init(struct udevice *dev, struct serdes *serdes)
{
	struct pcie_ctrl *pcie = &serdes->pcie;
	struct resource res = {};
	int ret;

	ret = dev_read_resource_byname(dev, "pcie_phy", &res);
	if (ret) {
		dev_err(dev, "Missing 'pcie_phy' reg region.\n");
		return ret;
	}

	pcie->phy_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (!pcie->phy_base) {
		dev_err(dev, "Failed to map 'ss_pcie'\n");
		return -ENOMEM;
	}

	pcie->rst = devm_reset_control_get(dev, "pcie");
	if (IS_ERR(pcie->rst)) {
		dev_err(dev, "Failed to get 'pcie' reset control\n");
		return PTR_ERR(pcie->rst);
	}

	return 0;
}

static int xpcs_dt_init(struct udevice *dev, struct serdes *serdes)
{
	struct xpcs_ctrl *xpcs = &serdes->xpcs;
	struct resource res = {};
	int ret;

	ret = dev_read_resource_byname(dev, "xpcs0", &res);
	if (ret) {
		dev_err(dev, "Missing 'xpcs0' reg region.\n");
		return -EIO;
	}

	xpcs->base0 = devm_ioremap(dev, res.start, resource_size(&res));
	if (!xpcs->base0) {
		dev_err(dev, "Failed to map 'xpcs0'\n");
		return -ENOMEM;
	}

	ret = dev_read_resource_byname(dev, "xpcs1", &res);
	if (ret) {
		dev_err(dev, "Missing 'xpcs1' reg region.\n");
		return -EIO;
	}

	xpcs->base1 = devm_ioremap(dev, res.start, resource_size(&res));
	if (!xpcs->base1) {
		dev_err(dev, "Failed to map 'xpcs1'\n");
		return -ENOMEM;
	}

	return 0;
}

static int disable_serdes_clocks(struct serdes *serdes)
{
	int ret;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(serdes->ctrl.clks); i++) {
		if (!serdes->ctrl.clks[i].enabled)
			continue;

		ret = clk_disable(&serdes->ctrl.clks[i].clk);
		if (ret)
			return ret;
	}

	return 0;
}

static int serdes_probe(struct udevice *dev)
{
	struct serdes *serdes = dev_get_priv(dev);
	unsigned long rate;
	const char *pcie_phy_mode;
	int ret = 0;

	debug("%s: probing %s\n", __func__, dev->name);
	if (!serdes) {
		printf("s32-serdes: invalid internal data\n");
		return -EINVAL;
	}

	serdes->dev = dev;

	ret = ss_dt_init(dev, serdes);
	if (ret)
		return ret;

	ret = get_clk_rate(serdes, &rate);
	if (ret)
		goto disable_clks;

	ret = pcie_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	ret = xpcs_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	serdes->devtype = s32_serdes_get_mode_from_hwconfig(serdes->id);
	/* Get XPCS configuration */
	serdes->xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(serdes->id);

	serdes->ctrl.phy_mode = s32_serdes_get_phy_mode_from_hwconfig(serdes->id);

	pcie_phy_mode = s32_serdes_get_pcie_phy_mode(serdes);
	printf("Using %s clock for PCIe%d, %s\n",
	       SERDES_CLK_MODE(serdes->ctrl.ext_clk),
	       serdes->id, pcie_phy_mode);
	if (IS_SERDES_SGMII(serdes->devtype) &&
	    serdes->xpcs_mode != SGMII_INAVALID)
		printf("Frequency %s configured for PCIe%d\n",
		       SERDES_CLK_FMHZ(rate),
		       serdes->id);

	ret = init_serdes(serdes);
	if (ret)
		goto disable_clks;

	if (IS_SERDES_PCIE(serdes->devtype)) {
		ret = pcie_phy_power_on(serdes, 0);
		if (ret) {
			dev_err(dev, "Failed to initialize PCIe line 0\n");
			goto disable_clks;
		}
		if (serdes->ctrl.ss_mode == 0) {
			ret = pcie_phy_power_on(serdes, 1);
			if (ret) {
				dev_err(dev, "Failed to initialize PCIe line 1\n");
				goto disable_clks;
			}
		}
	}

	if (IS_SERDES_SGMII(serdes->devtype) &&
	    serdes->xpcs_mode != SGMII_INAVALID) {
		enum serdes_xpcs_mode_gen2 xpcs[2] = {SGMII_XPCS_PCIE};

		/* Check, if mode5 demo is requested */
		s32_serdes_start_mode5(serdes, xpcs);

		ret = s32_eth_xpcs_init(serdes->xpcs.base0, serdes->xpcs.base1,
					serdes->id,
					serdes->ctrl.ss_mode,
					serdes->xpcs_mode,
					serdes->ctrl.ext_clk,
					rate,
					xpcs);
		if (ret) {
			printf("Error during configuration of SGMII on");
			printf(" PCIe%d\n", serdes->id);
		}
	}

disable_clks:
	if (ret)
		disable_serdes_clocks(serdes);

	return ret;
}

static const struct udevice_id serdes_match[] = {
	{ .compatible = "nxp,s32cc-serdes" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32cc_serdes) = {
	.name = "s32cc_serdes_phy",
	.id = UCLASS_PHY,
	.of_match = serdes_match,
	.probe	= serdes_probe,
	.priv_auto_alloc_size = sizeof(struct serdes),
};
