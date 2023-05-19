// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2023 NXP
 * S32CC PCIe Host driver
 */

#define ENABLE_DEBUG (IS_ENABLED(CONFIG_PCI_S32CC_DEBUG))
#if ENABLE_DEBUG
#define DEBUG
#endif

#include <common.h>
#include <cmd_pci.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <hwconfig.h>
#include <malloc.h>
#include <misc.h>
#include <pci.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/uclass-internal.h>
#include <dm/uclass.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include <s32-cc/nvmem.h>
#include <s32-cc/pcie.h>
#include <s32-cc/serdes_hwconfig.h>
#include <dt-bindings/phy/phy.h>

#include "pci-s32cc-regs.h"
#include "pci_s32cc.h"

#define PCIE_DEFAULT_INTERNAL_CLK 0

#define PCIE_ALIGNMENT 2

#define PCIE_TABLE_HEADER \
"BusDevFun               VendorId   DeviceId   Device Class       Sub-Class\n" \
"__________________________________________________________________________\n"

#define PCI_MAX_BUS_NUM			256
#define PCI_UNROLL_OFF			0x200
#define PCI_UPPER_ADDR_SHIFT		32

#define PCIE_LINKUP_MASK	(PCIE_SS_SMLH_LINK_UP | PCIE_SS_RDLH_LINK_UP | \
			PCIE_SS_SMLH_LTSSM_STATE)
#define PCIE_LINKUP_EXPECT	(PCIE_SS_SMLH_LINK_UP | PCIE_SS_RDLH_LINK_UP | \
			PCIE_SS_SMLH_LTSSM_STATE_VALUE(LTSSM_STATE_L0))

/* Default timeout (ms) */
#define PCIE_CX_CPL_BASE_TIMER_VALUE	100

/* PHY link timeout */
#define PCIE_LINK_TIMEOUT_MS		1000
#define PCIE_LINK_TIMEOUT_US		(PCIE_LINK_TIMEOUT_MS * USEC_PER_MSEC)
#define PCIE_LINK_WAIT_US		100

enum pcie_dev_type_val {
	PCIE_EP_VAL = 0x0,
	PCIE_RC_VAL = 0x4
};

static inline
void s32cc_pcie_write(struct dw_pcie *pci,
		      void __iomem *base, u32 reg, size_t size, u32 val)
{
	int ret;
	struct s32cc_pcie *s32cc_pci = to_s32cc_from_dw_pcie(pci);

	if (IS_ENABLED(CONFIG_PCI_S32CC_DEBUG_WRITES)) {
		if ((uintptr_t)base == (uintptr_t)(s32cc_pci->ctrl_base))
			dev_dbg(pci->dev, "W%d(ctrl+0x%x, 0x%x)\n",
				(int)size * 8, (u32)(reg), (u32)(val));
		else if ((uintptr_t)base == (uintptr_t)(pci->atu_base))
			dev_dbg(pci->dev, "W%d(atu+0x%x, 0x%x)\n",
				(int)size * 8, (u32)(reg), (u32)(val));
		else if ((uintptr_t)base == (uintptr_t)(pci->dbi_base))
			dev_dbg(pci->dev, "W%d(dbi+0x%x, 0x%x)\n",
				(int)size * 8, (u32)(reg), (u32)(val));
		else if ((uintptr_t)base == (uintptr_t)(pci->dbi_base2))
			dev_dbg(pci->dev, "W%d(dbi2+0x%x, 0x%x)\n",
				(int)size * 8, (u32)(reg), (u32)(val));
		else
			dev_dbg(pci->dev, "W%d(%lx+0x%x, 0x%x)\n",
				(int)size * 8, (uintptr_t)(base), (u32)(reg),
				(u32)(val));
	}

	ret = dw_pcie_write(base + reg, size, val);
	if (ret)
		dev_err(pci->dev, "PCIe%d: Write to address 0x%lx failed\n",
			s32cc_pci->id, (uintptr_t)(base + reg));
}

void dw_pcie_writel_ctrl(struct s32cc_pcie *pci, u32 reg, u32 val)
{
	s32cc_pcie_write(&pci->pcie, pci->ctrl_base, reg, 0x4, val);
}

u32 dw_pcie_readl_ctrl(struct s32cc_pcie *pci, u32 reg)
{
	u32 val = 0;

	if (dw_pcie_read(pci->ctrl_base + reg, 0x4, &val))
		dev_err(pci->pcie.dev, "Read ctrl address failed\n");

	return val;
}

int s32cc_check_serdes(struct udevice *dev)
{
	struct nvmem_cell c;
	int ret;
	u32 serdes_presence = 0;

	ret = nvmem_cell_get(dev, "serdes_presence", &c);
	if (ret) {
		printf("Failed to get 'serdes_presence' cell\n");
		return ret;
	}

	ret = nvmem_cell_read(&c, &serdes_presence, sizeof(serdes_presence));
	if (ret) {
		printf("%s: Failed to read cell 'serdes_presence' (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	if (!serdes_presence) {
		printf("SerDes Subsystem not present, skipping PCIe config\n");
		return -ENODEV;
	}

	return 0;
}

static u32 s32cc_pcie_get_dev_id_variant(struct udevice *dev)
{
	struct nvmem_cell c;
	int ret;
	u32 variant_bits = 0;

	ret = nvmem_cell_get(dev, "pcie_variant", &c);
	if (ret) {
		printf("Failed to get 'pcie_variant' cell\n");
		return ret;
	}

	ret = nvmem_cell_read(&c, &variant_bits, sizeof(variant_bits));
	if (ret) {
		printf("%s: Failed to read cell 'pcie_variant' (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	return variant_bits;
}

static void s32cc_pcie_disable_ltssm(struct s32cc_pcie *pci)
{
	u32 gen_ctrl_3 = dw_pcie_readl_ctrl(pci, PE0_GEN_CTRL_3)
			& ~(LTSSM_EN_MASK);

	dw_pcie_dbi_ro_wr_en(&pci->pcie);
	dw_pcie_writel_ctrl(pci, PE0_GEN_CTRL_3, gen_ctrl_3);
	dw_pcie_dbi_ro_wr_dis(&pci->pcie);
}

static void s32cc_pcie_enable_ltssm(struct s32cc_pcie *pci)
{
	u32 gen_ctrl_3 = dw_pcie_readl_ctrl(pci, PE0_GEN_CTRL_3) |
				LTSSM_EN_MASK;

	dw_pcie_dbi_ro_wr_en(&pci->pcie);
	dw_pcie_writel_ctrl(pci, PE0_GEN_CTRL_3, gen_ctrl_3);
	dw_pcie_dbi_ro_wr_dis(&pci->pcie);
}

static bool is_s32cc_pcie_ltssm_enabled(struct s32cc_pcie *pci)
{
	return (dw_pcie_readl_ctrl(pci, PE0_GEN_CTRL_3) & LTSSM_EN_MASK);
}

static bool has_data_phy_link(struct s32cc_pcie *s32cc_pp)
{
	u32 val = dw_pcie_readl_ctrl(s32cc_pp, PCIE_SS_PE0_LINK_DBG_2);

	return (val & PCIE_LINKUP_MASK) == PCIE_LINKUP_EXPECT;
}

static int s32cc_pcie_link_is_up(struct dw_pcie *pcie)
{
	struct s32cc_pcie *s32cc_pp = to_s32cc_from_dw_pcie(pcie);

	if (!is_s32cc_pcie_ltssm_enabled(s32cc_pp))
		return 0;

	return has_data_phy_link(s32cc_pp);
}

static int wait_phy_data_link(struct s32cc_pcie *s32cc_pp)
{
	bool has_link;
	int ret = read_poll_timeout(has_data_phy_link, s32cc_pp, has_link, has_link,
			PCIE_LINK_WAIT_US, PCIE_LINK_TIMEOUT_US);

	if (ret)
		dev_dbg(s32cc_pp->pcie.dev, "Failed to stabilize PHY link\n");

	return ret;
}

static bool speed_change_completed(struct dw_pcie *pcie)
{
	u32 ctrl = dw_pcie_readl_dbi(pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);

	return ctrl & PORT_LOGIC_SPEED_CHANGE;
}

static int s32cc_pcie_get_link_speed(struct s32cc_pcie *s32cc_pp)
{
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	u32 cap_offset = dw_pcie_find_capability(pcie, PCI_CAP_ID_EXP);
	u32 link_sta = dw_pcie_readw_dbi(pcie, cap_offset + PCI_EXP_LNKSTA);

	dev_dbg(pcie->dev, "PCIe%d: Speed Gen%d\n", s32cc_pp->id,
		link_sta & PCI_EXP_LNKSTA_CLS);

	/* return link speed based on negotiated link status */
	return link_sta & PCI_EXP_LNKSTA_CLS;
}

static int s32cc_pcie_start_link(struct dw_pcie *pcie)
{
	struct s32cc_pcie *s32cc_pp = to_s32cc_from_dw_pcie(pcie);
	u32 tmp, cap_offset;
	bool speed_set;
	int ret = 0;

	/* Don't do anything for End Point */
	if (!is_s32cc_pcie_rc(s32cc_pp->mode))
		return 0;

	/* Try to (re)establish the link, starting with Gen1 */
	s32cc_pcie_disable_ltssm(s32cc_pp);

	dw_pcie_dbi_ro_wr_en(pcie);
	cap_offset = dw_pcie_find_capability(pcie, PCI_CAP_ID_EXP);
	tmp = (dw_pcie_readl_dbi(pcie, cap_offset + PCI_EXP_LNKCAP) &
			~(PCI_EXP_LNKCAP_SLS)) | PCI_EXP_LNKCAP_SLS_2_5GB;
	dw_pcie_writel_dbi(pcie, cap_offset + PCI_EXP_LNKCAP, tmp);
	dw_pcie_dbi_ro_wr_dis(pcie);

	/* Start LTSSM. */
	s32cc_pcie_enable_ltssm(s32cc_pp);

	dw_pcie_dbi_ro_wr_en(pcie);
	/* Allow Gen2 or Gen3 mode after the link is up.
	 * s32cc_pcie.linkspeed is one of the speeds defined in pci_regs.h:
	 * PCI_EXP_LNKCAP_SLS_2_5GB for Gen1
	 * PCI_EXP_LNKCAP_SLS_5_0GB for Gen2
	 * PCI_EXP_LNKCAP_SLS_8_0GB for Gen3
	 */
	tmp = (dw_pcie_readl_dbi(pcie, cap_offset + PCI_EXP_LNKCAP) &
			~(PCI_EXP_LNKCAP_SLS)) | s32cc_pp->linkspeed;
	dw_pcie_writel_dbi(pcie, cap_offset + PCI_EXP_LNKCAP, tmp);

	/*
	 * Start Directed Speed Change so the best possible speed both link
	 * partners support can be negotiated.
	 * The manual says:
	 * When you set the default of the Directed Speed Change field of the
	 * Link Width and Speed Change Control register
	 * (GEN2_CTRL_OFF.DIRECT_SPEED_CHANGE) using the
	 * DEFAULT_GEN2_SPEED_CHANGE configuration parameter to 1, then
	 * the speed change is initiated automatically after link up, and the
	 * controller clears the contents of GEN2_CTRL_OFF.DIRECT_SPEED_CHANGE.
	 */
	tmp = dw_pcie_readl_dbi(pcie, PCIE_LINK_WIDTH_SPEED_CONTROL) |
			PORT_LOGIC_SPEED_CHANGE;
	dw_pcie_writel_dbi(pcie, PCIE_LINK_WIDTH_SPEED_CONTROL, tmp);
	dw_pcie_dbi_ro_wr_dis(pcie);

	ret = read_poll_timeout(speed_change_completed, pcie, speed_set,
				speed_set, PCIE_LINK_WAIT_US,
				PCIE_LINK_TIMEOUT_US);

	/* Make sure link training is finished as well! */
	if (!ret) {
		ret = wait_phy_data_link(s32cc_pp);
	} else {
		dev_err(pcie->dev, "Speed change timeout\n");
		ret = -EINVAL;
	}

	if (!ret)
		dev_info(pcie->dev, "Link up, Gen=%d\n",
			 s32cc_pcie_get_link_speed(s32cc_pp));

	return ret;
}

static int s32cc_pcie_dt_init_common(struct udevice *dev)
{
	struct s32cc_pcie *s32cc_pp = dev_get_priv(dev);
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	const char *pcie_phy_mode;
	u32 pcie_vendor_id = PCI_VENDOR_ID_FREESCALE, pcie_variant_bits = 0;
	int ret = 0;

	ret = dev_read_alias_seq(dev, &s32cc_pp->id);
	if (ret < 0) {
		dev_dbg(dev, "Failed to get PCIe sequence id\n");
		s32cc_pp->id = dev_read_s32_default(dev, "device_id", (-1));
		if (s32cc_pp->id == (-1)) {
			dev_err(dev, "Failed to get PCIe id\n");
			return -EINVAL;
		}
	}

	ret = generic_phy_get_by_name(dev, "serdes_lane0", &s32cc_pp->phy0);
	if (ret) {
		dev_err(dev, "Failed to get PHY 'serdes_lane0'\n");
		return ret;
	}
	/* PHY on lane 1 is optional */
	ret = generic_phy_get_by_name(dev, "serdes_lane1", &s32cc_pp->phy1);
	if (ret)
		dev_dbg(dev, "PHY 'serdes_lane1' not available\n");

	pcie_phy_mode = dev_read_string(dev, "nxp,phy-mode");
	if (!pcie_phy_mode) {
		dev_info(dev, "Missing 'nxp,phy-mode' property, using default CRNS\n");
		s32cc_pp->phy_mode = CRNS;
	} else if (!strcmp(pcie_phy_mode, "crns")) {
		s32cc_pp->phy_mode = CRNS;
	} else if (!strcmp(pcie_phy_mode, "crss")) {
		s32cc_pp->phy_mode = CRSS;
	} else if (!strcmp(pcie_phy_mode, "sris")) {
		s32cc_pp->phy_mode = SRIS;
	} else {
		dev_info(dev, "Unsupported 'nxp,phy-mode' specified, using default CRNS\n");
		s32cc_pp->phy_mode = CRNS;
	}

	pcie->dbi_base = (void *)dev_read_addr_name(dev, "dbi");
	if ((fdt_addr_t)pcie->dbi_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "PCIe%d: resource 'dbi' not found\n",
			s32cc_pp->id);
		return -EINVAL;
	}
	dev_dbg(dev, "Dbi base: 0x%lx\n", (uintptr_t)pcie->dbi_base);

	pcie->dbi_base2 = (void *)dev_read_addr_name(dev, "dbi2");
	if ((fdt_addr_t)pcie->dbi_base2 == FDT_ADDR_T_NONE) {
		dev_err(dev, "PCIe%d: resource 'dbi2' not found\n",
			s32cc_pp->id);
		return -EINVAL;
	}
	dev_dbg(dev, "Dbi2 base: 0x%lx\n", (uintptr_t)pcie->dbi_base2);

	pcie->atu_base = (void *)dev_read_addr_name(dev, "atu");
	if ((fdt_addr_t)pcie->atu_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "PCIe%d: resource 'atu' not found\n",
			s32cc_pp->id);
		return -EINVAL;
	}
	dev_dbg(dev, "Atu base: 0x%lx\n", (uintptr_t)pcie->atu_base);

	pcie->cfg_base = (void *)dev_read_addr_size_name(dev, "config",
							 &pcie->cfg_size);
	if ((fdt_addr_t)pcie->cfg_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "PCIe%d: resource 'config' not found\n",
			s32cc_pp->id);
		return -EINVAL;
	}
	dev_dbg(dev, "Config base: 0x%lx\n", (uintptr_t)pcie->cfg_base);

	s32cc_pp->ctrl_base  = (void *)dev_read_addr_name(dev, "ctrl");
	if ((fdt_addr_t)s32cc_pp->ctrl_base == FDT_ADDR_T_NONE) {
		dev_err(dev, "PCIe%d: resource 'ctrl' not found\n",
			s32cc_pp->id);
		return -EINVAL;
	}
	dev_dbg(dev, "Ctrl base: 0x%lx\n", (uintptr_t)s32cc_pp->ctrl_base);

	/* get supported speed (Gen1/Gen2/Gen3) from device tree */
	s32cc_pp->linkspeed = (enum pcie_link_speed)
		dev_read_u32_default(dev, "max-link-speed", GEN1);
	if (s32cc_pp->linkspeed  < GEN1 || s32cc_pp->linkspeed > GEN3) {
		dev_info(dev, "PCIe%d: Invalid speed\n", s32cc_pp->id);
		s32cc_pp->linkspeed  = GEN1;
	}

	pcie_variant_bits = dev_read_u32_default(dev, "pcie_device_id", 0);
	if (!pcie_variant_bits)
		pcie_variant_bits = s32cc_pcie_get_dev_id_variant(dev);
	if (!pcie_variant_bits) {
		dev_info(dev, "Could not set DEVICE ID\n");
		return 0;
	}

	/* Write PCI Vendor and Device ID. */
	pcie_vendor_id |= pcie_variant_bits << PCI_DEVICE_ID_SHIFT;
	dev_dbg(dev, "Setting PCI Device and Vendor IDs to 0x%x:0x%x\n",
		(u32)(pcie_vendor_id >> PCI_DEVICE_ID_SHIFT),
		(u32)(pcie_vendor_id & GENMASK(15, 0)));
	dw_pcie_dbi_ro_wr_en(pcie);
	dw_pcie_writel_dbi(pcie, PCI_VENDOR_ID, pcie_vendor_id);

	if (pcie_vendor_id != dw_pcie_readl_dbi(pcie, PCI_VENDOR_ID))
		dev_info(dev, "PCI Device and Vendor IDs could not be set\n");

	dw_pcie_dbi_ro_wr_dis(pcie);

	return 0;
}

static void disable_equalization(struct dw_pcie *pcie)
{
	u32 val;

	dw_pcie_dbi_ro_wr_en(pcie);

	val = dw_pcie_readl_dbi(pcie, PORT_LOGIC_GEN3_EQ_CONTROL);
	val &= ~(PCIE_GEN3_EQ_FB_MODE | PCIE_GEN3_EQ_PSET_REQ_VEC);
	val |= BUILD_MASK_VALUE(PCIE_GEN3_EQ_FB_MODE, 1) |
		 BUILD_MASK_VALUE(PCIE_GEN3_EQ_PSET_REQ_VEC, 0x84);
	dw_pcie_writel_dbi(pcie, PORT_LOGIC_GEN3_EQ_CONTROL, val);

	dw_pcie_dbi_ro_wr_dis(pcie);

	/* Test value */
	dev_dbg(pcie->dev, "PCIE_PORT_LOGIC_GEN3_EQ_CONTROL: 0x%08x\n",
		dw_pcie_readl_dbi(pcie, PORT_LOGIC_GEN3_EQ_CONTROL));
}

static int init_pcie(struct s32cc_pcie *s32cc_pp)
{
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	struct udevice *dev = pcie->dev;
	u32 val;

	if (is_s32cc_pcie_rc(s32cc_pp->mode)) {
		val = dw_pcie_readl_ctrl(s32cc_pp, PE0_GEN_CTRL_1) |
				BUILD_MASK_VALUE(DEVICE_TYPE, PCIE_RC_VAL);
		dw_pcie_writel_ctrl(s32cc_pp, PE0_GEN_CTRL_1, val);
	}

	if (s32cc_pp->phy_mode == SRIS) {
		val = dw_pcie_readl_ctrl(s32cc_pp, PE0_GEN_CTRL_1) |
				SRIS_MODE_MASK;
		dw_pcie_writel_ctrl(s32cc_pp, PE0_GEN_CTRL_1, val);
	}

	/* Enable writing dbi registers */
	dw_pcie_dbi_ro_wr_en(pcie);

	/* Enable direct speed change */
	val = dw_pcie_readl_dbi(pcie, PCIE_LINK_WIDTH_SPEED_CONTROL);
	val |= PORT_LOGIC_SPEED_CHANGE;
	dw_pcie_writel_dbi(pcie, PCIE_LINK_WIDTH_SPEED_CONTROL, val);
	dw_pcie_dbi_ro_wr_dis(pcie);

	/* Disable phase 2,3 equalization */
	disable_equalization(pcie);

	/* PCIE_COHERENCY_CONTROL_<n> registers provide defaults that configure
	 * the transactions as Outer Shareable, Write-Back cacheable; we won't
	 * change those defaults.
	 */

	/* Make sure DBI registers are R/W */
	dw_pcie_dbi_ro_wr_en(pcie);

	val = dw_pcie_readl_dbi(pcie, PORT_LOGIC_PORT_FORCE_OFF);
	val |= PCIE_DO_DESKEW_FOR_SRIS;
	dw_pcie_writel_dbi(pcie, PORT_LOGIC_PORT_FORCE_OFF, val);

	if (is_s32cc_pcie_rc(s32cc_pp->mode)) {
		/* Set max payload supported, 256 bytes and
		 * relaxed ordering.
		 */
		val = dw_pcie_readl_dbi(pcie, CAP_DEVICE_CONTROL_DEVICE_STATUS);
		val &= ~(CAP_EN_REL_ORDER | CAP_MAX_PAYLOAD_SIZE_CS |
			 CAP_MAX_READ_REQ_SIZE);
		val |= CAP_EN_REL_ORDER |
			BUILD_MASK_VALUE(CAP_MAX_PAYLOAD_SIZE_CS, 1) |
			BUILD_MASK_VALUE(CAP_MAX_READ_REQ_SIZE, 1),
		dw_pcie_writel_dbi(pcie, CAP_DEVICE_CONTROL_DEVICE_STATUS, val);

		/* Enable the IO space, Memory space, Bus master,
		 * Parity error, Serr and disable INTx generation
		 */
		dw_pcie_writel_dbi(pcie, PCIE_CTRL_TYPE1_STATUS_COMMAND_REG,
				   PCIE_SERREN | PCIE_PERREN | PCIE_INT_EN |
				   PCIE_IO_EN | PCIE_MSE | PCIE_BME);
		/* Test value */
		dev_dbg(dev, "PCIE_CTRL_TYPE1_STATUS_COMMAND_REG reg: 0x%08x\n",
			dw_pcie_readl_dbi(pcie,
					  PCIE_CTRL_TYPE1_STATUS_COMMAND_REG));

		/* Enable errors */
		val = dw_pcie_readl_dbi(pcie, CAP_DEVICE_CONTROL_DEVICE_STATUS);
		val |=  CAP_CORR_ERR_REPORT_EN |
			CAP_NON_FATAL_ERR_REPORT_EN |
			CAP_FATAL_ERR_REPORT_EN |
			CAP_UNSUPPORT_REQ_REP_EN;
		dw_pcie_writel_dbi(pcie, CAP_DEVICE_CONTROL_DEVICE_STATUS, val);
	}

	val = dw_pcie_readl_dbi(pcie, PORT_GEN3_RELATED_OFF);
	val |= PCIE_EQ_PHASE_2_3;
	dw_pcie_writel_dbi(pcie, PORT_GEN3_RELATED_OFF, val);

	/* Disable writing dbi registers */
	dw_pcie_dbi_ro_wr_dis(pcie);

	s32cc_pcie_enable_ltssm(s32cc_pp);

	return 0;
}

static int init_pcie_phy(struct s32cc_pcie *s32cc_pp)
{
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	struct udevice *dev = pcie->dev;
	int ret = 0;

	if (!generic_phy_valid(&s32cc_pp->phy0))
		return -ENODEV;

	generic_phy_reset(&s32cc_pp->phy0);
	ret = generic_phy_init(&s32cc_pp->phy0);
	if (ret) {
		dev_err(dev, "Failed to init PHY 'serdes_lane0'\n");
		return ret;
	}

	ret = generic_phy_set_mode_ext(&s32cc_pp->phy0, PHY_TYPE_PCIE,
				       s32cc_pp->phy_mode);
	if (ret) {
		dev_err(dev, "Failed to set mode on PHY 'serdes_lane0'\n");
		return ret;
	}

	ret = generic_phy_power_on(&s32cc_pp->phy0);
	if (ret) {
		dev_err(dev, "Failed to power on PHY 'serdes_lane0'\n");
		return ret;
	}

	if (!generic_phy_valid(&s32cc_pp->phy1))
		return ret;

	generic_phy_reset(&s32cc_pp->phy1);
	ret = generic_phy_init(&s32cc_pp->phy1);
	if (ret) {
		dev_err(dev, "Failed to init PHY 'serdes_lane1'\n");
		return ret;
	}

	ret = generic_phy_set_mode_ext(&s32cc_pp->phy1, PHY_TYPE_PCIE,
				       s32cc_pp->phy_mode);
	if (ret) {
		dev_err(dev, "Failed to set mode on PHY 'serdes_lane1'\n");
		return ret;
	}

	ret = generic_phy_power_on(&s32cc_pp->phy1);
	if (ret) {
		dev_err(dev, "Failed to power on PHY 'serdes_lane1'\n");
		return ret;
	}

	return 0;
}

static int s32cc_pcie_init_controller(struct s32cc_pcie *s32cc_pp)
{
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	int ret = 0;

	s32cc_pcie_disable_ltssm(s32cc_pp);

	ret = init_pcie_phy(s32cc_pp);
	if (ret)
		return ret;

	ret = init_pcie(s32cc_pp);
	if (ret)
		return ret;

	/* Only wait for link if RC.
	 * With or witout link, go ahead and configure the controller.
	 */
	if (is_s32cc_pcie_rc(s32cc_pp->mode))
		wait_phy_data_link(s32cc_pp);

	dev_info(pcie->dev, "Configuring as %s\n",
		 s32cc_pcie_ep_rc_mode_str(s32cc_pp->mode));

	return 0;
}

static int s32cc_pcie_config_host(struct s32cc_pcie *s32cc_pp)
{
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	int ret = 0;

	ret = s32cc_pcie_init_controller(s32cc_pp);
	if (ret)
		return ret;

	pcie_dw_setup_host(pcie);

	ret = s32cc_pcie_start_link(pcie);
	if (ret) {
		dev_info(pcie->dev, "Failed to get link up\n");
		return 0;
	}

	pcie_dw_prog_outbound_atu_unroll(pcie, PCIE_ATU_REGION_INDEX0,
					 PCIE_ATU_TYPE_MEM,
					 pcie->mem.phys_start,
					 pcie->mem.bus_start,
					 pcie->mem.size);

	return 0;
}

struct dw_pcie_ops s32cc_dw_pcie_ops = {
	.link_up = s32cc_pcie_link_is_up,
	.start_link = s32cc_pcie_start_link,
	.write_dbi = s32cc_pcie_write,
};

static int s32cc_pcie_probe(struct udevice *dev)
{
	struct s32cc_pcie *s32cc_pp = dev_get_priv(dev);
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	int ret = 0;

	s32cc_pp->mode = DW_PCIE_UNKNOWN_TYPE;
	ret = s32cc_check_serdes(dev);
	if (ret)
		return ret;

	pcie->first_busno = dev_seq(dev);
	pcie->dev = dev;
	pcie->ops = &s32cc_dw_pcie_ops;

	s32cc_pp->mode =
		(enum dw_pcie_device_mode)dev_get_driver_data(dev);

	if (is_s32cc_pcie_rc(s32cc_pp->mode)) {
		ret = s32cc_pcie_config_host(s32cc_pp);
	} else {
		dev_err(dev, "Invalid PCIe host operating mode\n");
		ret = -EINVAL;
	}

	if (ret) {
		dev_err(dev, "Failed to set PCIe host settings\n");
		s32cc_pp->mode = DW_PCIE_UNKNOWN_TYPE;
	}

	dw_pcie_dbi_ro_wr_dis(pcie);
	return ret;
}

static
void show_pcie_devices_aligned(struct udevice *bus, struct udevice *dev,
			       int depth, int last_flag, bool *parsed_bus)
{
	int i, is_last;
	struct udevice *child;
	struct pci_child_plat *pplat;

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	pplat = dev_get_parent_plat(dev);
	printf("%02x:%02x.%02x", dev_seq(bus),
	       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));
	parsed_bus[dev_seq(bus)] = true;

	for (i = (PCIE_ALIGNMENT - depth + 1); i > 0; i--)
		printf("    ");
	pci_header_show_brief(dev);

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_pcie_devices_aligned(dev, child, depth + 1,
					  (last_flag << 1) | is_last,
					  parsed_bus);
	}
}

static int pci_get_depth(struct udevice *dev)
{
	if (!dev)
		return 0;

	return (1 + pci_get_depth(dev->parent));
}

int show_pcie_devices(void)
{
	struct udevice *bus;
	bool show_header = true;
	bool parsed_bus[PCI_MAX_BUS_NUM];

	memset(parsed_bus, false, sizeof(bool) * PCI_MAX_BUS_NUM);

	for (uclass_find_first_device(UCLASS_PCI, &bus);
		     bus;
		     uclass_find_next_device(&bus)) {
		struct udevice *dev;
		struct s32cc_pcie *pcie = dev_get_priv(bus);

		if (parsed_bus[dev_seq(bus)])
			continue;

		if (pcie && pcie->mode != DW_PCIE_UNKNOWN_TYPE) {
			if (show_header) {
				printf(PCIE_TABLE_HEADER);
				show_header = false;
			}
			printf("%s %s\n", bus->name,
			       s32cc_pcie_ep_rc_mode_str(pcie->mode));
		}
		for (device_find_first_child(bus, &dev);
			    dev;
			    device_find_next_child(&dev)) {
			int depth = pci_get_depth(dev);
			int is_last = list_is_last(&dev->sibling_node,
					&bus->child_head);
			if (dev_seq(dev) < 0)
				continue;
			show_pcie_devices_aligned(bus, dev, depth - 3,
						  is_last, parsed_bus);
		}
	}

	return 0;
}

static const struct dm_pci_ops s32cc_dm_pcie_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id s32cc_pcie_of_match[] = {
	{ .compatible = "nxp,s32cc-pcie", .data = DW_PCIE_RC_TYPE },
	{ }
};

U_BOOT_DRIVER(pci_s32cc) = {
	.name = "pci_s32cc",
	.id = UCLASS_PCI,
	.of_match = s32cc_pcie_of_match,
	.ops = &s32cc_dm_pcie_ops,
	.of_to_plat	= s32cc_pcie_dt_init_common,
	.probe	= s32cc_pcie_probe,
	.priv_auto = sizeof(struct s32cc_pcie),
};
