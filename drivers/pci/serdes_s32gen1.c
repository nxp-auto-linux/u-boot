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
#include <linux/ioport.h>
#include <linux/sizes.h>

#include "serdes_regs.h"
#include "serdes_s32gen1.h"
#include "serdes_xpcs_regs.h"
#include "sgmii.h"

#define SERDES_CLK_MODE(clk_type) \
			((clk_type == CLK_INT) ? "internal" : "external")
#define SERDES_CLK_FMHZ(clk_type) \
			((clk_type == CLK_100MHZ) ? "100Mhz" : "125Mhz")

int wait_read32(void __iomem *address, u32 expected,
		u32 mask, int read_attempts)
{
	__maybe_unused u32 tmp;

	while ((tmp = (s32_dbi_readl(UPTR(address)) & (mask))) != (expected)) {
		udelay(DELAY_QUANTUM); read_attempts--;
		if (read_attempts < 0) {
			debug_wr("WARNING: timeout read 0x%x from 0x%lx,",
				 tmp, UPTR(address));
			debug_wr(" expected 0x%x\n", expected);
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int s32_serdes_set_mode(void __iomem *dbi, enum serdes_mode mode)
{
	BSET32(UPTR(dbi) + SS_SS_RW_REG_0, BUILD_MASK_VALUE(SUBSYS_MODE, mode));

	/* small delay for stabilizing the signals */
	udelay(100);

	return 0;
}

static int s32_serdes_assert_reset(struct s32_serdes *serdes)
{
	__maybe_unused struct udevice *dev = serdes->bus;
	int ret;

	ret = reset_assert(serdes->pcie_rst);
	if (ret) {
		dev_err(dev, "Failed to assert SerDes reset: %d\n", ret);
		return ret;
	}

	ret = reset_assert(serdes->serdes_rst);
	if (ret) {
		dev_err(dev, "Failed to assert SerDes reset: %d\n", ret);
		return ret;
	}

	return 0;
}

static int s32_serdes_deassert_reset(struct s32_serdes *serdes)
{
	__maybe_unused struct udevice *dev = serdes->bus;
	int ret;

	ret = reset_deassert(serdes->pcie_rst);
	if (ret) {
		dev_err(dev, "Failed to deassert SerDes reset: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(serdes->serdes_rst);
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
static void s32_serdes_phy_reg_write(struct s32_serdes *pcie, u16 addr,
				     u16 wdata, u16 wmask)
{
	u32 temp_data = wdata & wmask;

	W32(UPTR(pcie->dbi) + SS_PHY_REG_ADDR,
	    BUILD_MASK_VALUE(PHY_REG_ADDR_FIELD, addr) | PHY_REG_EN);
	udelay(100);
	if (wmask == 0xFFFF)
		W32(UPTR(pcie->dbi) + SS_PHY_REG_DATA, temp_data);
	else
		RMW32(UPTR(pcie->dbi) + SS_PHY_REG_DATA, temp_data, wmask);

	udelay(100);
}

static void s32_serdes_phy_init(struct s32_serdes *pcie)
{
	/* Select the CR parallel interface */
	BSET32(UPTR(pcie->dbi) + SS_SS_RW_REG_0, PHY0_CR_PARA_SEL);

	/* Address erratum TKT0527889:
	 * PCIe Gen3 Receiver Long Channel Stressed Voltage Test Failing
	 */
	/* RX_EQ_DELTA_IQ_OVRD enable and override value for PCIe0 lane 0 */
	s32_serdes_phy_reg_write(pcie,
				 RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x03, 0xff);
	s32_serdes_phy_reg_write(pcie,
				 RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x13, 0xff);

	/* RX_EQ_DELTA_IQ_OVRD enable and override value for PCIe0 lane 1 */
	s32_serdes_phy_reg_write(pcie,
				 RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x03, 0xff);
	s32_serdes_phy_reg_write(pcie,
				 RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN,
				 0x13, 0xff);
}

static void s32_serdes_xpcs1_pma_config(struct s32_serdes *pcie)
{
	/* Configure TX_VBOOST_LVL and TX_TERM_CTRL */
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MISC_CTRL_2,
	      EXT_TX_VBOOST_LVL(0x3) | EXT_TX_TERM_CTRL(0x4),
	      EXT_TX_VBOOST_LVL(0x7) | EXT_TX_TERM_CTRL(0x7));
	/* Enable phy external control */
	BSET32(UPTR(pcie->dbi) + SS_PHY_EXT_CTRL_SEL, EXT_PHY_CTRL_SEL);
	/* Configure ref range, disable PLLB/ref div2 */
	RMW32(UPTR(pcie->dbi) + SS_PHY_REF_CLK_CTRL,
	      EXT_REF_RANGE(0x3),
	      REF_CLK_DIV2_EN | REF_CLK_MPLLB_DIV2_EN | EXT_REF_RANGE(0x7));
	/* Configure multiplier */
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MPLLB_CTRL_2,
	      MPLLB_MULTIPLIER(0x27U) | EXT_MPLLB_FRACN_CTRL(0x414),
	      MPLLB_MULTIPLIER(0xffU) | EXT_MPLLB_FRACN_CTRL(0x7ff) |
	      1 << 24U | 1 << 28U);

	BCLR32(UPTR(pcie->dbi) + SS_PHY_MPLLB_CTRL, 1 << 1);

	/* Configure tx lane division, disable word clock div2*/
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MPLLB_CTRL_3,
	      EXT_MPLLB_TX_CLK_DIV(0x5),
	      EXT_MPLLB_WORD_DIV2_EN | EXT_MPLLB_TX_CLK_DIV(0x7));

	/* Configure configure bandwidth for filtering and div10*/
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MPLLB_CTRL_1,
	      EXT_MPLLB_BANDWIDTH(0x5f) | EXT_MPLLB_DIV10_CLK_EN,
	      EXT_MPLLB_BANDWIDTH(0xffff) | EXT_MPLLB_DIV_CLK_EN |
	      EXT_MPLLB_DIV8_CLK_EN | EXT_MPLLB_DIV_MULTIPLIER(0xff));

	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MPLLA_CTRL_1,
	      EXT_MPLLA_BANDWIDTH(0xc5), EXT_MPLLA_BANDWIDTH(0xffff));

	/* Configure VCO */
	RMW32(UPTR(pcie->dbi) + SS_PHY_XPCS1_RX_OVRD_CTRL,
	      XPCS1_RX_VCO_LD_VAL(0x540U) | XPCS1_RX_REF_LD_VAL(0x2bU),
	      XPCS1_RX_VCO_LD_VAL(0x1fffU) | XPCS1_RX_REF_LD_VAL(0x3fU));

	/* Boundary scan control */
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_BS_CTRL,
	      EXT_BS_RX_LEVEL(0xb) | EXT_BS_RX_BIGSWING,
	      EXT_BS_RX_LEVEL(0x1f) | EXT_BS_TX_LOWSWING);

	/* Rx loss threshold */
	RMW32(UPTR(pcie->dbi) + SS_PHY_EXT_MISC_CTRL_1,
	      EXT_RX_LOS_THRESHOLD(0x3U) | EXT_RX_VREF_CTRL(0x11U),
	      EXT_RX_LOS_THRESHOLD(0x3fU) | EXT_RX_VREF_CTRL(0x1fU));
}

static void s32_serdes_start_mode5(struct s32_serdes *pcie,
				   enum serdes_xpcs_mode_gen2 xpcs[2])
{
	if (!s32_serdes_has_mode5_enabled(pcie->id))
		return;

	printf("SerDes%d: Enabling serdes mode5\n", pcie->id);
	/* Initialize PMA */
	serdes_pma_mode5((void *)UPTR(pcie->dbi), 1);
	/* Initialize PHY */
	s32_serdes_xpcs1_pma_config(pcie);
	/* Initialize PCS */
	serdes_pcs_mode5((void *)UPTR(pcie->dbi), 1);
	/* mode5 representation */
	xpcs[0] = SGMII_XPCS_PCIE;
	xpcs[1] = SGMII_XPCS_2G5_OP;
}

static bool s32_serdes_init(struct s32_serdes *pcie)
{
	int ret;

	pcie->ss_mode = s32_serdes_get_op_mode_from_hwconfig(pcie->id);
	if (pcie->ss_mode == SERDES_MODE_INVAL) {
		printf("ERROR: Invalid opmode config on PCIe%d\n",  pcie->id);
		return false;
	}

	/* Reset the Serdes module */
	ret = s32_serdes_assert_reset(pcie);
	if (ret)
		return false;

	if (s32_serdes_set_mode(pcie->dbi, pcie->ss_mode))
		return false;

	/* Set the clock for the Serdes module */
	if (pcie->clktype == CLK_INT) {
		debug("Set internal clock\n");
		BCLR32(UPTR(pcie->dbi) + SS_PHY_GEN_CTRL,
		       PHY_GEN_CTRL_REF_USE_PAD);
		BSET32(UPTR(pcie->dbi) + SS_SS_RW_REG_0, 1 << 23);
	} else {
		debug("Set external clock\n");
		BSET32(UPTR(pcie->dbi) + SS_PHY_GEN_CTRL,
		       PHY_GEN_CTRL_REF_USE_PAD);
		BCLR32(UPTR(pcie->dbi) + SS_SS_RW_REG_0, 1 << 23);
	}

	/* Deassert SerDes reset */
	ret = s32_serdes_deassert_reset(pcie);
	if (ret)
		return false;

	/* Enable PHY's SRIS mode in PCIe mode*/
	if (pcie->phy_mode == SRIS)
		BSET32(UPTR(pcie->dbi) + SS_PHY_GEN_CTRL,
		       PHY_GEN_CTRL_RX_SRIS_MODE_MASK);

	if (IS_SERDES_PCIE(pcie->devtype)) {

		/* Monitor Serdes MPLL state, which is 1 when
		 * either MPLLA is 1 (for Gen1 and 2) or
		 * MPLLB is 1 (for Gen3)
		 */
		if (wait_read32((void __iomem *)
				(UPTR(pcie->dbi) + SS_PHY_MPLLA_CTRL),
				MPLL_STATE,
				MPLL_STATE,
				PCIE_MPLL_LOCK_COUNT)) {
			printf("WARNING: Failed to lock PCIe%d MPLLs\n",
				pcie->id);
			return false;
		}

		/* Set PHY register access to CR interface */
		BSET32(UPTR(pcie->dbi) + SS_SS_RW_REG_0, 0x200);
		s32_serdes_phy_init(pcie);
	}

	return true;
}

__weak int s32_eth_xpcs_init(void __iomem *dbi, int id,
			     enum serdes_mode ss_mode,
			     enum serdes_xpcs_mode xpcs_mode,
			     enum serdes_clock clktype,
			     enum serdes_clock_fmhz fmhz,
			     enum serdes_xpcs_mode_gen2 xpcs[2])
{
	/* Configure SereDes XPCS for PFE/GMAC*/
	printf("PCIe%d disabled\n", id);
	return -ENODEV;
}

static const char *s32_serdes_get_pcie_phy_mode(struct s32_serdes *pcie)
{
	if (pcie->phy_mode == CRSS)
		return "CRSS";
	else if (pcie->phy_mode == SRIS)
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

static int s32_serdes_get_config_from_device_tree(struct s32_serdes *pcie)
{
	struct resource res;
	struct udevice *dev = pcie->bus;
	int ret = 0;

	ret = get_serdes_alias_id(dev, &pcie->id);
	if (ret < 0) {
		printf("Failed to get SerDes device id\n");
		return ret;
	}

	ret = dev_read_resource_byname(dev, "dbi", &res);
	if (ret) {
		printf("s32-serdes: resource \"dbi\" not found\n");
		return ret;
	}

	pcie->dbi = devm_ioremap(dev, res.start, resource_size(&res));
	if (!pcie->dbi) {
		printf("PCIe%d: Failed to map 'dbi' resource\n", pcie->id);
		return -ENOMEM;
	}

	pcie->pcie_rst = devm_reset_control_get(dev, "pcie");
	if (IS_ERR(pcie->pcie_rst)) {
		dev_err(dev, "Failed to get 'pcie' reset control\n");
		return PTR_ERR(pcie->pcie_rst);
	}

	pcie->serdes_rst = devm_reset_control_get(dev, "serdes");
	if (IS_ERR(pcie->serdes_rst)) {
		dev_err(dev, "Failed to get 'serdes' reset control\n");
		return PTR_ERR(pcie->serdes_rst);
	}

	debug("%s: dbi: 0x%lx (0x%p)\n", __func__, (uintptr_t)res.start,
	      pcie->dbi);

	return 0;
}

static int enable_serdes_clocks(struct udevice *dev)
{
	struct clk_bulk bulk;
	int ret;

	ret = clk_get_bulk(dev, &bulk);
	if (ret == -ENOSYS)
		return 0;
	if (ret)
		return ret;

	ret = clk_enable_bulk(&bulk);
	if (ret) {
		clk_release_bulk(&bulk);
		return ret;
	}
	return 0;
}

static int s32_serdes_probe(struct udevice *dev)
{
	struct s32_serdes *pcie = dev_get_priv(dev);
	const char *pcie_phy_mode;
	int ret = 0;

	debug("%s: probing %s\n", __func__, dev->name);
	if (!pcie) {
		printf("s32-serdes: invalid internal data\n");
		return -EINVAL;
	}

	pcie->bus = dev;

	ret = s32_serdes_get_config_from_device_tree(pcie);
	if (ret)
		return ret;

	ret = enable_serdes_clocks(dev);
	if (ret) {
		dev_err(dev, "Failed to enable SERDES clocks\n");
		return ret;
	}

	pcie->devtype = s32_serdes_get_mode_from_hwconfig(pcie->id);
	pcie->clktype = s32_serdes_get_clock_from_hwconfig(pcie->id);
	/* Get XPCS configuration */
	pcie->xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(pcie->id);
	pcie->fmhz = s32_serdes_get_clock_fmhz_from_hwconfig(pcie->id);

	pcie->phy_mode = s32_serdes_get_phy_mode_from_hwconfig(pcie->id);

	if (pcie->clktype == CLK_INT) {
		if (pcie->phy_mode == CRSS || pcie->phy_mode == SRIS) {
			printf("CRSS or SRIS for PCIe%d PHY mode cannot be used with internal clock\n",
			       pcie->id);
			return -EINVAL;
		}
	}

	pcie_phy_mode = s32_serdes_get_pcie_phy_mode(pcie);
	printf("Using %s clock for PCIe%d, %s\n",
	       SERDES_CLK_MODE(pcie->clktype),
	       pcie->id, pcie_phy_mode);
	if (IS_SERDES_SGMII(pcie->devtype) &&
	    pcie->xpcs_mode != SGMII_INAVALID)
		printf("Frequency %s configured for PCIe%d\n",
		       SERDES_CLK_FMHZ(pcie->fmhz),
		       pcie->id);

	/* Apply the base SerDes/PHY settings */
	if (!s32_serdes_init(pcie))
		return ret;

	if (IS_SERDES_SGMII(pcie->devtype) &&
	    pcie->xpcs_mode != SGMII_INAVALID) {
		enum serdes_xpcs_mode_gen2 xpcs[2] = {SGMII_XPCS_PCIE};

		/* Check, if mode5 demo is requested */
		s32_serdes_start_mode5(pcie, xpcs);

		ret = s32_eth_xpcs_init(pcie->dbi, pcie->id,
					pcie->ss_mode,
					pcie->xpcs_mode,
					pcie->clktype,
					pcie->fmhz,
					xpcs);
		if (ret) {
			printf("Error during configuration of SGMII on");
			printf(" PCIe%d\n", pcie->id);
		}
	}

	return ret;
}

static const struct udevice_id s32_serdes_ids[] = {
	{ .compatible = "nxp,s32cc-serdes" },
	{ }
};

U_BOOT_DRIVER(serdes_s32gen1) = {
	.name = "serdes_s32gen1",
	.id = UCLASS_PCI_GENERIC,
	.of_match = s32_serdes_ids,
	.probe	= s32_serdes_probe,
	.priv_auto_alloc_size = sizeof(struct s32_serdes),
};
