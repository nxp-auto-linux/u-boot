// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2024 NXP
 * SERDES driver for S32CC SoCs
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <hwconfig.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/of_access.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>
#include <linux/stringify.h>
#include <linux/time.h>
#include <s32-cc/serdes_hwconfig.h>
#include <s32-cc/xpcs.h>
#include <dt-bindings/phy/phy.h>

#define UPTR(X)			((uintptr_t)(X))
#define SERDES_MAX_LANES 2U
#define SERDES_MAX_INSTANCES 2U

#define XPCS_LANE0		0
#define XPCS_LANE1		1

#define XPCS_DISABLED		-1
#define XPCS_ID_0		0
#define XPCS_ID_1		1

#define SERDES_LINE(TYPE, INSTANCE)\
	{ \
		.type = (TYPE), \
		.instance = (INSTANCE), \
	}

#define PCIE_LANE(N) SERDES_LINE(PHY_TYPE_PCIE, N)
#define XPCS_LANE(N) SERDES_LINE(PHY_TYPE_XPCS, N)

#define SERDES_CLK_MODE(EXT_CLK) \
			((EXT_CLK) ? "external" : "internal")
#define SERDES_CLK_FMHZ(clk_type) \
			((clk_type == MHZ_100) ? "100Mhz" : "125Mhz")

#define PCIE_PHY_GEN_CTRL	(0x0)
#define  REF_USE_PAD_MASK	BIT_32(17)
#define  RX_SRIS_MODE_MASK	BIT_32(9)
#define PCIE_PHY_MPLLA_CTRL	(0x10)
#define  MPLLA_STATE_MASK	BIT_32(31)
#define  MPLL_STATE_MASK	BIT_32(30)
#define PCIE_PHY_MPLLB_CTRL	(0x14U)
#define  MPLLB_SSC_EN_MASK	BIT_32(1)
#define PCIE_PHY_EXT_CTRL_SEL	(0x18U)
#define  EXT_PHY_CTRL_SEL	BIT_32(0)
#define PCIE_PHY_EXT_BS_CTRL	(0x1cU)
#define  EXT_BS_TX_LOWSWING	BIT_32(6)
#define  EXT_BS_RX_BIGSWING	BIT_32(5)
#define  EXT_BS_RX_LEVEL(x)	(((x) & 0x1fU) << 0)
#define PCIE_PHY_REF_CLK_CTRL	(0x20U)
#define  EXT_REF_RANGE(x)	(((x) & 0x7U) << 3)
#define  REF_CLK_DIV2_EN	BIT_32(2)
#define  REF_CLK_MPLLB_DIV2_EN	BIT_32(1)
#define PCIE_PHY_EXT_MPLLA_CTRL_1	(0x30U)
#define  EXT_MPLLA_BANDWIDTH(x)		(((x) & 0xffffU) << 0)
#define PCIE_PHY_EXT_MPLLB_CTRL_1	(0x40U)
#define  EXT_MPLLB_DIV_MULTIPLIER(x)	(((x) & 0xffU) << 24)
#define  EXT_MPLLB_DIV_CLK_EN	BIT_32(19)
#define  EXT_MPLLB_DIV8_CLK_EN	BIT_32(18)
#define  EXT_MPLLB_DIV10_CLK_EN	BIT_32(16)
#define  EXT_MPLLB_BANDWIDTH(x)	(((x) & 0xffffU) << 0)
#define PCIE_PHY_EXT_MPLLB_CTRL_2	(0x44U)
#define  EXT_MPLLB_FRACN_CTRL(x)	(((x) & 0x7ffU) << 12)
#define  MPLLB_MULTIPLIER(n)		(((n) & 0xffU) << 0)
#define PCIE_PHY_EXT_MPLLB_CTRL_3	(0x48U)
#define  EXT_MPLLB_WORD_DIV2_EN		BIT_32(31)
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
#define SS_RW_REG_0		(0xf0u)
#define  SUBMODE_MASK		(0x7u)
#define  CLKEN_MASK		BIT_32(23)
#define  PHY0_CR_PARA_SEL_MASK	BIT_32(9)

#define PHY_REG_ADDR		(0x0)
#define  PHY_REG_EN		BIT_32(31)
#define PHY_REG_DATA		(0x4)
#define PHY_RST_CTRL		(0x8)
#define  COLD_RST		BIT_32(0)

#define RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3019)
#define RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3119)

#define SERDES_LOCK_TIMEOUT_US	(10 * USEC_PER_MSEC)

#define EXTERNAL_CLK_NAME	"ext"
#define INTERNAL_CLK_NAME	"ref"

#define SERDES_PCIE_FREQ	100000000

#define XPCS_RX_STABLE_TIMEOUT	25 /* µs */

struct serdes_lane_conf {
	/* Phy type from : include/dt-bindings/phy/phy.h */
	u32 type;
	u8 instance; /* Instance ID (e.g PCIE0, XPCS1) */
};

struct serdes_conf {
	struct serdes_lane_conf lanes[SERDES_MAX_LANES];
};

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
	struct s32cc_xpcs *phys[2];
	const struct s32cc_xpcs_ops *ops;
	void __iomem *base0, *base1;
	bool powered_on[2];
	bool initialized_clks;
};

struct serdes {
	struct pcie_ctrl pcie;
	struct serdes_ctrl ctrl;
	struct xpcs_ctrl xpcs;
	struct udevice *dev;
	u32 phys_type[SERDES_MAX_LANES];
	u8 lanes_status;

	unsigned int id;
};

static unsigned long lane_id_to_xpcs_id(unsigned int mode,
					unsigned long lane_id);

static const char * const serdes_clk_names[] = {
	"axi", "aux", "apb", "ref", "ext"
};

static void mark_configured_lane(struct serdes *serdes, u32 lane)
{
	if (BIT_32(lane) < U8_MAX)
		serdes->lanes_status |= BIT_32(lane);
}

static bool is_lane_configured(struct serdes *serdes, u32 lane)
{
	return !!(serdes->lanes_status & BIT_32(lane));
}

static int serdes_phy_reset(struct phy *p)
{
	return 0;
}

static void phy_config_gen_ctrl(struct serdes_ctrl *sctrl,
				bool is_sris_phy_mode)
{
	u32 ctrl;

	ctrl = readl(UPTR(sctrl->ss_base) + PCIE_PHY_GEN_CTRL);

	if (is_sris_phy_mode)
		ctrl |= RX_SRIS_MODE_MASK;

	if (sctrl->ext_clk)
		ctrl |= REF_USE_PAD_MASK;
	else
		ctrl &= ~REF_USE_PAD_MASK;

	writel(ctrl, UPTR(sctrl->ss_base) + PCIE_PHY_GEN_CTRL);
}

static void pcie_phy_write(struct serdes *serdes, u32 reg, u32 val)
{
	writel(PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	writel(reg | PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	udelay(100);
	writel(val, UPTR(serdes->pcie.phy_base) + PHY_REG_DATA);
	udelay(100);
}

static void pcie_phy_cold_reset(struct serdes *serdes)
{
	u32 val;

	val = readl(UPTR(serdes->pcie.phy_base) + PHY_RST_CTRL);
	writel(val | COLD_RST, UPTR(serdes->pcie.phy_base) + PHY_RST_CTRL);
	udelay(1000);
	writel(val, UPTR(serdes->pcie.phy_base) + PHY_RST_CTRL);
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
	u32 reg0, val, mask;
	int ret;

	if (pcie->initialized_phy)
		return 0;

	ret = check_pcie_clk(serdes);
	if (ret)
		return ret;

	if (sctrl->phy_mode == SRIS)
		phy_config_gen_ctrl(sctrl, true);
	else
		phy_config_gen_ctrl(sctrl, false);

	/* Monitor Serdes MPLL state */
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

static int pcie_phy_power_on(struct serdes *serdes, unsigned long id)
{
	struct pcie_ctrl *pcie = &serdes->pcie;
	u32 iq_ovrd_in;
	int ret;

	if (id >= ARRAY_SIZE(pcie->powered_on))
		return -EINVAL;

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

static int xpcs_phy_init(struct serdes *serdes, unsigned long id)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	struct xpcs_ctrl *xpcs = &serdes->xpcs;
	struct udevice *dev = serdes->dev;
	enum pcie_xpcs_mode shared = NOT_SHARED;

	void __iomem *base;
	unsigned long rate;
	int ret;

	if (id >= ARRAY_SIZE(xpcs->phys))
		return -EINVAL;

	if (xpcs->phys[id])
		return 0;

	if (!id)
		base = xpcs->base0;
	else
		base = xpcs->base1;

	ret = get_clk_rate(serdes, &rate);
	if (ret)
		return ret;

	/* Mode 5 is virtual mode, a particularization of mode 2 */
	if (ctrl->ss_mode == 1 || ctrl->ss_mode == 2)
		shared = PCIE_XPCS_1G;
	else if (ctrl->ss_mode == 5)
		shared = PCIE_XPCS_2G5;

	return xpcs->ops->init(&xpcs->phys[id], dev, id, base,
			       ctrl->ext_clk, rate, shared);
}

static int xpcs_phy_power_on(struct serdes *serdes, unsigned long id)
{
	struct xpcs_ctrl *xpcs = &serdes->xpcs;
	int ret;

	if (id >= ARRAY_SIZE(xpcs->powered_on) || id >= ARRAY_SIZE(xpcs->phys))
		return -EINVAL;

	if (xpcs->powered_on[id])
		return 0;

	ret = xpcs->ops->power_on(xpcs->phys[id]);
	if (ret)
		dev_err(serdes->dev, "Failed to power on XPCS%lu\n", id);
	else
		xpcs->powered_on[id] = true;

	return ret;
}

static bool is_xpcs_rx_stable(struct serdes *serdes, int id)
{
	struct xpcs_ctrl *xpcs = &serdes->xpcs;

	if (id < 0 || id >= ARRAY_SIZE(xpcs->phys))
		return false;

	return xpcs->ops->has_valid_rx(xpcs->phys[id]);
}

static void prepare_pma_mode5(struct serdes *serdes)
{
	/* Configure TX_VBOOST_LVL and TX_TERM_CTRL */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MISC_CTRL_2,
		      EXT_TX_VBOOST_LVL(0x7) | EXT_TX_TERM_CTRL(0x7),
		      EXT_TX_VBOOST_LVL(0x3) | EXT_TX_TERM_CTRL(0x4));

	/* Enable phy external control */
	setbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_CTRL_SEL,
		   EXT_PHY_CTRL_SEL);

	/* Configure ref range, disable PLLB/ref div2 */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_REF_CLK_CTRL,
		      REF_CLK_DIV2_EN | REF_CLK_MPLLB_DIV2_EN |
		      EXT_REF_RANGE(0x7),
		      EXT_REF_RANGE(0x3));

	/* Configure multiplier */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_2,
		      MPLLB_MULTIPLIER(0xffU) | EXT_MPLLB_FRACN_CTRL(0x7ff) |
		      1 << 24U | 1 << 28U,
		      MPLLB_MULTIPLIER(0x27U) | EXT_MPLLB_FRACN_CTRL(0x414));

	clrbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_MPLLB_CTRL,
		   MPLLB_SSC_EN_MASK);

	/* Configure tx lane division, disable word clock div2*/
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_3,
		      EXT_MPLLB_WORD_DIV2_EN | EXT_MPLLB_TX_CLK_DIV(0x7),
		      EXT_MPLLB_TX_CLK_DIV(0x5));

	/* Configure configure bandwidth for filtering and div10*/
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLB_CTRL_1,
		      EXT_MPLLB_BANDWIDTH(0xffff) | EXT_MPLLB_DIV_CLK_EN |
		      EXT_MPLLB_DIV8_CLK_EN | EXT_MPLLB_DIV_MULTIPLIER(0xff),
		      EXT_MPLLB_BANDWIDTH(0x5f) | EXT_MPLLB_DIV10_CLK_EN);

	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MPLLA_CTRL_1,
		      EXT_MPLLA_BANDWIDTH(0xffff),
		      EXT_MPLLA_BANDWIDTH(0xc5));

	/* Configure VCO */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_XPCS1_RX_OVRD_CTRL,
		      XPCS1_RX_VCO_LD_VAL(0x1fffU) | XPCS1_RX_REF_LD_VAL(0x3fU),
		      XPCS1_RX_VCO_LD_VAL(0x540U) | XPCS1_RX_REF_LD_VAL(0x2bU));

	/* Boundary scan control */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_BS_CTRL,
		      EXT_BS_RX_LEVEL(0x1f) | EXT_BS_TX_LOWSWING,
		      EXT_BS_RX_LEVEL(0xb) | EXT_BS_RX_BIGSWING);

	/* Rx loss threshold */
	clrsetbits_32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_EXT_MISC_CTRL_1,
		      EXT_RX_LOS_THRESHOLD(0x3fU) | EXT_RX_VREF_CTRL(0x1fU),
		      EXT_RX_LOS_THRESHOLD(0x3U) | EXT_RX_VREF_CTRL(0x11U));
}

static void xpcs_check_rx_stable(struct serdes *serdes, u32 xpcs_active)
{
	u32 xpcs_rx_stable = 0;
	u32 rx_stable_timeoutus, i;

	/*
	 * Wait for RX to become stable over all referenced XPCS
	 * after the PLLs have been set up.
	 * The max timeout really should be determined based on the
	 * worst case protocol that we actually have on the active
	 * XPCS's and XPCS<=>PHY behavior. It is known to be non-zero
	 * in time, but the max is not clearly defined. We chose 25µs
	 * and experience shows that it is in practice below 10µs.
	 *
	 * It is questionable if this check is doing anything useful for
	 * the user, especially as it would report failure needlessly
	 * for unused lanes.
	 */
	for (i = 0; i < SERDES_MAX_LANES; i++) {
		if (!(xpcs_active & BIT(i)))
			continue;

		for (rx_stable_timeoutus = 0;
			rx_stable_timeoutus < XPCS_RX_STABLE_TIMEOUT;
			rx_stable_timeoutus++) {
			if (xpcs_rx_stable & BIT((i)))
				break;

			if (is_xpcs_rx_stable(serdes, i)) {
				xpcs_rx_stable |= BIT(i);
				dev_info(serdes->dev,
					 "Stable RX detected on XPCS%u after %u µs\n", i,
					 rx_stable_timeoutus);
			}
			udelay(1);
		}
	}

	/*
	 * Final report of all unstable RX XPCS. We only know after
	 * the timeout expired.
	 */
	if (xpcs_rx_stable != xpcs_active) {
		for (i = 0; i < SERDES_MAX_LANES; i++) {
			if (!(xpcs_active & BIT(i)))
				continue;

			if (xpcs_rx_stable & BIT(i))
				continue;

			dev_warn(serdes->dev,
				 "Unstable RX detected on XPCS%u\n", i);
		}
	}
}

static int xpcs_init_clks(struct serdes *serdes)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	struct xpcs_ctrl *xpcs = &serdes->xpcs;
	int ret, order[2], i, xpcs_id;

	if (xpcs->initialized_clks)
		return 0;

	switch (ctrl->ss_mode) {
	case 0:
		return 0;
	case 1:
		order[0] = XPCS_ID_0;
		order[1] = XPCS_DISABLED;
		break;
	case 2:
	case 5:
		order[0] = XPCS_ID_1;
		order[1] = XPCS_DISABLED;
		break;
	case 3:
		order[0] = XPCS_ID_1;
		order[1] = XPCS_ID_0;
		break;
	case 4:
		order[0] = XPCS_ID_0;
		order[1] = XPCS_ID_1;
		break;
	default:
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(order); i++) {
		xpcs_id = order[i];

		if (xpcs_id == XPCS_DISABLED)
			continue;

		if (xpcs_id >= ARRAY_SIZE(xpcs->phys) || xpcs_id < 0)
			continue;

		ret = xpcs_phy_init(serdes, xpcs_id);
		if (ret)
			return ret;

		ret = xpcs_phy_power_on(serdes, xpcs_id);
		if (ret)
			return ret;

		ret = xpcs->ops->init_plls(xpcs->phys[xpcs_id]);
		if (ret)
			return ret;
	}

	if (ctrl->ss_mode == 5) {
		phy_config_gen_ctrl(ctrl, false);
		prepare_pma_mode5(serdes);

		xpcs->ops->pre_reset_pcie_2g5(xpcs->phys[1]);
		pcie_phy_cold_reset(serdes);
		ret = xpcs->ops->post_reset_pcie_2g5(xpcs->phys[1]);
		if (ret) {
			dev_err(serdes->dev,
				"Failed to configure SerDes for PCIE & XPCS @ 2G5 mode\n");
			return ret;
		}
	} else {
		for (i = 0; i < ARRAY_SIZE(order); i++) {
			xpcs_id = order[i];

			if (xpcs_id == XPCS_DISABLED)
				continue;

			if (xpcs_id < 0 || xpcs_id >= ARRAY_SIZE(xpcs->phys))
				continue;

			ret = xpcs->ops->vreset(xpcs->phys[xpcs_id]);
			if (ret)
				return ret;
		}
	}

	for (i = 0; i < ARRAY_SIZE(order); i++) {
		xpcs_id = order[i];

		if (xpcs_id == XPCS_DISABLED)
			continue;

		if (xpcs_id < 0 || xpcs_id >= ARRAY_SIZE(xpcs->phys))
			continue;

		ret = xpcs->ops->wait_vreset(xpcs->phys[xpcs_id]);
		if (ret)
			return ret;

		xpcs->ops->reset_rx(xpcs->phys[xpcs_id]);
	}

	xpcs->initialized_clks = true;
	return 0;
}

static int serdes_phy_init(struct phy *p)
{
	struct serdes *serdes = dev_get_priv(p->dev);
	unsigned long id = p->id;
	unsigned long xpcs_id;
	int ret;

	if (!serdes)
		return -EINVAL;

	if (id >= ARRAY_SIZE(serdes->phys_type))
		return -EINVAL;

	if (serdes->phys_type[id] == PHY_TYPE_PCIE)
		return 0;

	if (serdes->phys_type[id] == PHY_TYPE_XPCS) {
		xpcs_id = lane_id_to_xpcs_id(serdes->ctrl.ss_mode, id);
		ret = xpcs_phy_init(serdes, xpcs_id);
		if (ret)
			return ret;

		/*
		 * Below function could check all lanes concurrently, but
		 * we only check the current one as we have no global
		 * visibility of all lanes here.
		 */
		xpcs_check_rx_stable(serdes, BIT(xpcs_id));

		return ret;
	}

	return -EINVAL;
}

static int serdes_phy_set_mode_ext(struct phy *p, int mode, int submode)
{
	struct serdes *serdes = dev_get_priv(p->dev);
	unsigned long id = p->id;

	if (!serdes)
		return -EINVAL;

	if (id >= ARRAY_SIZE(serdes->phys_type))
		return -EINVAL;

	if (serdes->phys_type[id] != PHY_TYPE_PCIE)
		return -EINVAL;

	/* Check if same PCIE PHY mode is set on both lanes */
	if (id == 1ul)
		if (submode != serdes->ctrl.phy_mode)
			return -EINVAL;

	if (mode == PHY_TYPE_PCIE) {
		/* Do not configure SRIS or CRSS PHY MODE in conjunction
		 * with any SGMII mode on the same SerDes subsystem
		 */
		if (submode == CRSS || submode == SRIS) {
			if (serdes->ctrl.ss_mode != 0)
				return -EINVAL;
		}

		/* CRSS or SRIS PCIE PHY mode cannot be used
		 * with internal clock
		 */
		if (!serdes->ctrl.ext_clk)
			if (submode == CRSS || submode == SRIS)
				return -EINVAL;

		serdes->ctrl.phy_mode = (enum pcie_phy_mode)submode;

		return 0;
	}

	return -EINVAL;
}

static int serdes_phy_power_on(struct phy *p)
{
	struct serdes *serdes = dev_get_priv(p->dev);
	unsigned long id = p->id;
	unsigned long xpcs_id;

	if (!serdes)
		return -EINVAL;

	if (id >= ARRAY_SIZE(serdes->phys_type))
		return -EINVAL;

	if (serdes->phys_type[id] == PHY_TYPE_PCIE)
		return pcie_phy_power_on(serdes, p->id);

	if (serdes->phys_type[id] == PHY_TYPE_XPCS){
		xpcs_id = lane_id_to_xpcs_id(serdes->ctrl.ss_mode, id);
		return xpcs_phy_power_on(serdes, xpcs_id);
	}

	return 0;
}

static int serdes_phy_power_off(struct phy *p)
{
	return 0;
}

static int xpcs_phy_configure(struct phy *phy, struct phylink_link_state *state)
{
	struct serdes *serdes = dev_get_priv(phy->dev);
	struct xpcs_ctrl *xpcs;
	__maybe_unused struct udevice *dev;
	unsigned long xpcs_id;
	int ret;

	if (!serdes)
		return -EINVAL;

	xpcs = &serdes->xpcs;
	dev = serdes->dev;
	xpcs_id = lane_id_to_xpcs_id(serdes->ctrl.ss_mode, phy->id);
	ret = xpcs->ops->config(xpcs->phys[xpcs_id], NULL);
	if (ret) {
		dev_err(dev, "Failed to configure XPCS\n");
		return ret;
	}

	return ret;
}

static int serdes_phy_configure(struct phy *phy, void *params)
{
	struct serdes *serdes = dev_get_priv(phy->dev);
	unsigned long id = phy->id;
	int ret = -EINVAL;

	if (!serdes)
		return -EINVAL;

	if (id >= ARRAY_SIZE(serdes->phys_type))
		return -EINVAL;

	if (serdes->phys_type[id] == PHY_TYPE_XPCS)
		ret = xpcs_phy_configure(phy,
					 (struct phylink_link_state *)params);

	return ret;
}

static const struct serdes_conf serdes_mux_table[] = {
	/* Mode 0 */
	{ .lanes = { [0] = PCIE_LANE(0), [1] = PCIE_LANE(1) }, },
	/* Mode 1 */
	{ .lanes = { [0] = PCIE_LANE(0), [1] = XPCS_LANE(0), }, },
	/* Mode 2 */
	{ .lanes = { [0] = PCIE_LANE(0), [1] = XPCS_LANE(1), }, },
	/* Mode 3 */
	{ .lanes = { [0] = XPCS_LANE(0), [1] = XPCS_LANE(1), }, },
	/* Mode 4 */
	{ .lanes = { [0] = XPCS_LANE(0), [1] = XPCS_LANE(1), }, },
	/* Demo mode 5 (Mode 2, where XPCS runs @2G5) */
	{ .lanes = { [0] = PCIE_LANE(0), [1] = XPCS_LANE(1), }, },
};

static unsigned long lane_id_to_xpcs_id(unsigned int mode,
					unsigned long lane_id)
{
	int xpcs_id = serdes_mux_table[mode].lanes[lane_id].instance;

	return xpcs_id;
}

struct s32cc_xpcs *s32cc_phy2xpcs(struct phy *phy)
{
	struct serdes *serdes = dev_get_priv(phy->dev);
	const struct serdes_lane_conf *lane_conf;
	unsigned long lane = phy->id;
	u32 ss_mode;
	struct xpcs_ctrl *xpcs;

	if (!serdes)
		return NULL;

	xpcs = &serdes->xpcs;
	ss_mode = serdes->ctrl.ss_mode;
	lane_conf = &serdes_mux_table[ss_mode].lanes[lane];

	if (lane_conf->type != PHY_TYPE_XPCS)
		return NULL;

	return xpcs->phys[lane_conf->instance];
}

static int mode_to_pcs_lane(u32 mode, u32 pcs_instance)
{
	const struct serdes_lane_conf *l0, *l1;

	if (mode >= ARRAY_SIZE(serdes_mux_table))
		return -EINVAL;

	l0 = &serdes_mux_table[mode].lanes[0];
	l1 = &serdes_mux_table[mode].lanes[1];

	if (l0->type == PHY_TYPE_XPCS && l0->instance == pcs_instance)
		return XPCS_LANE0;

	if (l1->type == PHY_TYPE_XPCS && l1->instance == pcs_instance)
		return XPCS_LANE1;

	return XPCS_DISABLED;
}

static int check_lane_selection(struct serdes *serdes,
				u32 phy_type, u32 instance,
				u32 *lane_id)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	const struct serdes_conf *conf = &serdes_mux_table[ctrl->ss_mode];
	const struct serdes_lane_conf *lane_conf;
	__maybe_unused struct udevice *dev = serdes->dev;
	const char *phy_name;
	int pcs_lane_id;

	if (instance >= SERDES_MAX_INSTANCES) {
		dev_err(dev, "Invalid instance : %u\n", instance);
		return -EINVAL;
	}

	if (phy_type == PHY_TYPE_XPCS) {
		pcs_lane_id = mode_to_pcs_lane(ctrl->ss_mode, instance);
		if (pcs_lane_id >= 0) {
			*lane_id = pcs_lane_id;
		} else {
			dev_err(dev, "Couldn't translate XPCS to lane\n");
			return -EINVAL;
		}
	}

	if (*lane_id >= SERDES_MAX_LANES) {
		dev_err(dev, "Invalid lane : %u\n", *lane_id);
		return -EINVAL;
	}

	switch (phy_type) {
	case PHY_TYPE_PCIE:
		phy_name = __stringify_1(PHY_TYPE_PCIE);
		break;
	case PHY_TYPE_XPCS:
		phy_name = __stringify_1(PHY_TYPE_XPCS);
		break;
	default:
		dev_err(dev, "Invalid PHY type : %u\n", phy_type);
		return -EINVAL;
	}

	if (is_lane_configured(serdes, *lane_id) && phy_type != PHY_TYPE_XPCS) {
		dev_err(dev, "Lane %u is already configured\n", *lane_id);
		return -EINVAL;
	}

	lane_conf = &conf->lanes[*lane_id];

	if (lane_conf->type != phy_type) {
		dev_err(dev, "Invalid %u type applied on SerDes lane %d. Expected type %u\n",
			phy_type, *lane_id, lane_conf->type);
		return -EINVAL;
	}

	if (lane_conf->type != PHY_TYPE_PCIE &&
	    lane_conf->instance != instance) {
		dev_err(dev, "PHY %s instance %u cannot be applied on lane %u using SerDes mode %u)\n",
			phy_name, instance, *lane_id, serdes->ctrl.ss_mode);
		return -EINVAL;
	}

	mark_configured_lane(serdes, *lane_id);
	return 0;
}

static int serdes_phy_xlate(struct phy *phy, struct ofnode_phandle_args *args)
{
	struct serdes *serdes = dev_get_priv(phy->dev);
	u32 phy_type, instance, lane_id;
	int ret;

	if (!serdes)
		return -EINVAL;

	if (args->args_count < 3)
		return -EINVAL;

	phy_type = args->args[0];
	instance = args->args[1];
	lane_id = args->args[2];

	ret = check_lane_selection(serdes, phy_type, instance, &lane_id);
	if (ret)
		return ret;

	phy->id = lane_id;
	serdes->phys_type[lane_id] = phy_type;

	return 0;
}

static const struct phy_ops serdes_ops = {
	.of_xlate	= serdes_phy_xlate,
	.reset		= serdes_phy_reset,
	.init		= serdes_phy_init,
	.set_mode	= serdes_phy_set_mode_ext,
	.power_on	= serdes_phy_power_on,
	.power_off	= serdes_phy_power_off,
	.configure	= serdes_phy_configure,
};

static int assert_reset(struct serdes *serdes)
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
	if (ctrl->ss_mode == 5)
		reg0 |= 2;
	else
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

	ret = xpcs_init_clks(serdes);
	if (ret)
		dev_err(serdes->dev, "XPCS init failed\n");

	return ret;
}

static int ss_dt_init(struct udevice *dev, struct serdes *serdes)
{
	struct serdes_ctrl *ctrl = &serdes->ctrl;
	struct resource res = {};
	int ret = 0;
	size_t i;

	ret = s32_serdes_get_alias_id(dev, &serdes->id);
	if (ret < 0) {
		dev_err(dev, "Failed to get SerDes device id\n");
		return ret;
	}

	ret = dev_read_u32(dev, "nxp,sys-mode", &ctrl->ss_mode);
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
		dev_err(dev, "Failed to map 'pcie_phy'\n");
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

	xpcs->ops = s32cc_xpcs_get_ops();

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
	int ret = 0;

	debug("%s: probing %s\n", __func__, dev->name);
	if (!serdes) {
		dev_err(dev, "Invalid internal data\n");
		return -EINVAL;
	}

	serdes->dev = dev;

	ret = ss_dt_init(dev, serdes);
	if (ret)
		return ret;

	ret = pcie_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	ret = xpcs_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	ret = init_serdes(serdes);
	if (ret)
		goto disable_clks;

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
	.ops	= &serdes_ops,
	.probe	= serdes_probe,
	.priv_auto = sizeof(struct serdes),
};
