// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2022 NXP
 * S32Gen1 PCIe driver
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

#include "serdes_regs.h"
#include "serdes_s32gen1_io.h"
#include "sgmii.h"
#include "ss_pcie_regs.h"

#define PCIE_MPLL_LOCK_COUNT 10
#define DELAY_QUANTUM 1000

#define SERDES_CLK_MODE(clk_type) \
			((clk_type == CLK_INT) ? "internal" : "external")
#define SERDES_CLK_FMHZ(clk_type) \
			((clk_type == CLK_100MHZ) ? "100Mhz" : "125Mhz")

#define PCIE_PHY_GEN_CTRL	(0x0)
#define  REF_USE_PAD_MASK	BIT(17)
#define  RX_SRIS_MODE_MASK	BIT(9)
#define PCIE_PHY_MPLLA_CTRL	(0x10)
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
#define  PHY0_CR_PARA_SEL_MASK	BIT(9)

#define PHY_REG_ADDR		(0x0)
#define  PHY_REG_EN		BIT(31)
#define PHY_REG_DATA		(0x4)

#define RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3019)
#define RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3119)

struct pcie_ctrl {
	struct reset_ctl *rst;
	void __iomem *phy_base;
};

struct serdes_ctrl {
	struct reset_ctl *rst;
	void __iomem *ss_base;
	struct {
		struct clk clk;
		bool enabled;
	} clks[5];
};

struct xpcs_ctrl {
	void __iomem *base0, *base1;
};

struct serdes {
	struct pcie_ctrl pcie;
	struct serdes_ctrl ctrl;
	struct xpcs_ctrl xpcs;
	struct udevice *dev;
	enum serdes_mode ss_mode;

	int id;
	enum serdes_dev_type devtype;
	enum serdes_xpcs_mode xpcs_mode;
	enum serdes_clock clktype;
	enum serdes_clock_fmhz fmhz;
	enum serdes_phy_mode phy_mode;
};

static const char * const serdes_clk_names[] = {
	"axi", "aux", "apb", "ref", "ext"
};

static int wait_read32(void __iomem *address, u32 expected,
		       u32 mask, unsigned long read_attempts)
{
	unsigned long maxtime = read_attempts * DELAY_QUANTUM;
	int ret;
	u32 val;

	ret = readl_poll_timeout(UPTR(address), val, (val & mask) != expected,
				 maxtime);
	if (ret) {
		debug_wr("WARNING: timeout read 0x%x from 0x%lx,",
			 val, UPTR(address));
		debug_wr(" expected 0x%x\n", expected);
		return -ETIMEDOUT;
	}

	return 0;
}

static int s32_serdes_set_mode(void __iomem *ss_base, enum serdes_mode mode)
{
	BSET32(UPTR(ss_base) + SS_RW_REG_0, SUBMODE_MASK & mode);

	/* small delay for stabilizing the signals */
	udelay(100);

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

static int s32_serdes_deassert_reset(struct serdes *serdes)
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

/**
 * @brief	Indirect write PHY register.
 * @param[in]	addr	Indirect PHY address (16bit).
 * @param[in]	wdata	Indirect PHY data to be written (16 bit).
 */
static void s32_serdes_phy_reg_write(struct serdes *serdes, u16 addr,
				     u16 wdata, u16 wmask)
{
	u32 temp_data = wdata & wmask;

	writel(PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	writel(addr | PHY_REG_EN, UPTR(serdes->pcie.phy_base) + PHY_REG_ADDR);
	udelay(100);
	if (wmask == 0xFFFF)
		W32(UPTR(serdes->pcie.phy_base) + PHY_REG_DATA, temp_data);
	else
		RMW32(UPTR(serdes->pcie.phy_base) + PHY_REG_DATA, temp_data, wmask);

	udelay(100);
}

static void s32_serdes_phy_init(struct serdes *serdes)
{
	/* Select the CR parallel interface */
	BSET32(UPTR(serdes->ctrl.ss_base) + SS_RW_REG_0, PHY0_CR_PARA_SEL_MASK);

	/* Address erratum TKT0527889:
	 * PCIe Gen3 Receiver Long Channel Stressed Voltage Test Failing
	 */
	/* RX_EQ_DELTA_IQ_OVRD enable and override value for PCIe0 lane 0 */
	s32_serdes_phy_reg_write(serdes,
				 RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x03, 0xff);
	s32_serdes_phy_reg_write(serdes,
				 RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x13, 0xff);

	/* RX_EQ_DELTA_IQ_OVRD enable and override value for PCIe0 lane 1 */
	s32_serdes_phy_reg_write(serdes,
				 RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x03, 0xff);
	s32_serdes_phy_reg_write(serdes,
				 RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x13, 0xff);
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

static bool s32_serdes_init(struct serdes *serdes)
{
	int ret;

	serdes->ss_mode = s32_serdes_get_op_mode_from_hwconfig(serdes->id);
	if (serdes->ss_mode == SERDES_MODE_INVAL) {
		printf("ERROR: Invalid opmode config on PCIe%d\n",  serdes->id);
		return false;
	}

	/* Reset the Serdes module */
	ret = s32_serdes_assert_reset(serdes);
	if (ret)
		return false;

	if (s32_serdes_set_mode(serdes->ctrl.ss_base, serdes->ss_mode))
		return false;

	/* Set the clock for the Serdes module */
	if (serdes->clktype == CLK_INT) {
		debug("Set internal clock\n");
		BCLR32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_GEN_CTRL,
		       REF_USE_PAD_MASK);
		BSET32(UPTR(serdes->ctrl.ss_base) + SS_RW_REG_0, 1 << 23);
	} else {
		debug("Set external clock\n");
		BSET32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_GEN_CTRL,
		       REF_USE_PAD_MASK);
		BCLR32(UPTR(serdes->ctrl.ss_base) + SS_RW_REG_0, 1 << 23);
	}

	/* Deassert SerDes reset */
	ret = s32_serdes_deassert_reset(serdes);
	if (ret)
		return false;

	/* Enable PHY's SRIS mode in PCIe mode*/
	if (serdes->phy_mode == SRIS)
		BSET32(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_GEN_CTRL,
		       RX_SRIS_MODE_MASK);

	if (IS_SERDES_PCIE(serdes->devtype)) {

		/* Monitor Serdes MPLL state, which is 1 when
		 * either MPLLA is 1 (for Gen1 and 2) or
		 * MPLLB is 1 (for Gen3)
		 */
		if (wait_read32((void __iomem *)
				(UPTR(serdes->ctrl.ss_base) + PCIE_PHY_MPLLA_CTRL),
				MPLL_STATE_MASK,
				MPLL_STATE_MASK,
				PCIE_MPLL_LOCK_COUNT)) {
			printf("WARNING: Failed to lock PCIe%d MPLLs\n",
				serdes->id);
			return false;
		}

		/* Set PHY register access to CR interface */
		BSET32(UPTR(serdes->ctrl.ss_base) + SS_RW_REG_0, 0x200);
		s32_serdes_phy_init(serdes);
	}

	return true;
}

__weak int s32_eth_xpcs_init(void __iomem *xpcs0, void __iomem *xpcs1,
			     int id, enum serdes_mode ss_mode,
			     enum serdes_xpcs_mode xpcs_mode,
			     enum serdes_clock clktype,
			     enum serdes_clock_fmhz fmhz,
			     enum serdes_xpcs_mode_gen2 xpcs[2])
{
	/* Configure SereDes XPCS for PFE/GMAC*/
	printf("SerDes%d: XPCS is disabled\n", id);
	return -ENODEV;
}

static const char *s32_serdes_get_pcie_phy_mode(struct serdes *serdes)
{
	if (serdes->phy_mode == CRSS)
		return "CRSS";
	else if (serdes->phy_mode == SRIS)
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

	ret = ofnode_read_resource_byname(node, "ss_pcie", &res);
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

	ret = pcie_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	ret = xpcs_dt_init(dev, serdes);
	if (ret)
		goto disable_clks;

	serdes->devtype = s32_serdes_get_mode_from_hwconfig(serdes->id);
	serdes->clktype = s32_serdes_get_clock_from_hwconfig(serdes->id);
	/* Get XPCS configuration */
	serdes->xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(serdes->id);
	serdes->fmhz = s32_serdes_get_clock_fmhz_from_hwconfig(serdes->id);

	serdes->phy_mode = s32_serdes_get_phy_mode_from_hwconfig(serdes->id);

	pcie_phy_mode = s32_serdes_get_pcie_phy_mode(serdes);
	printf("Using %s clock for PCIe%d, %s\n",
	       SERDES_CLK_MODE(serdes->clktype),
	       serdes->id, pcie_phy_mode);
	if (IS_SERDES_SGMII(serdes->devtype) &&
	    serdes->xpcs_mode != SGMII_INAVALID)
		printf("Frequency %s configured for PCIe%d\n",
		       SERDES_CLK_FMHZ(serdes->fmhz),
		       serdes->id);

	/* Apply the base SerDes/PHY settings */
	if (!s32_serdes_init(serdes))
		goto disable_clks;

	if (IS_SERDES_SGMII(serdes->devtype) &&
	    serdes->xpcs_mode != SGMII_INAVALID) {
		enum serdes_xpcs_mode_gen2 xpcs[2] = {SGMII_XPCS_PCIE};

		/* Check, if mode5 demo is requested */
		s32_serdes_start_mode5(serdes, xpcs);

		ret = s32_eth_xpcs_init(serdes->xpcs.base0, serdes->xpcs.base1,
					serdes->id,
					serdes->ss_mode,
					serdes->xpcs_mode,
					serdes->clktype,
					serdes->fmhz,
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

U_BOOT_DRIVER(serdes_s32gen1) = {
	.name = "serdes_s32gen1",
	.id = UCLASS_PCI_GENERIC,
	.of_match = serdes_match,
	.probe	= serdes_probe,
	.priv_auto_alloc_size = sizeof(struct serdes),
};
