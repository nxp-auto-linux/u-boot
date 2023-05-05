/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2022 NXP
 * S32CC PCIe driver
 */

#ifndef PCIE_S32CC_H
#define PCIE_S32CC_H

#include <generic-phy.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/ioport.h>
#include <linux/stringify.h>
#include <linux/types.h>
#include <pci.h>

#include "pcie-designware.h"

#define BUILD_BIT_VALUE(field, x) (((x) & (1)) << field##_BIT)
#define BUILD_MASK_VALUE(field, x) (((x) & (field##_MASK)) << field##_LSB)

/* PCIe MSI capabilities register */
#define PCI_MSI_CAP		0x50U
/* MSI Enable bit */
#define MSI_EN			0x10000U

/* PCIe MSI-X capabilities register */
#define PCI_MSIX_CAP		0xB0U
/* MSI-X Enable bit */
#define MSIX_EN			BIT_32(31)

/* PCIe controller 0 general control 1 (PE0_GEN_CTRL_1) */
#define PE0_GEN_CTRL_1		0x50U
#define   DEVICE_TYPE_LSB	(0)
#define   DEVICE_TYPE_MASK	(0x0000000F)
#define   DEVICE_TYPE		((DEVICE_TYPE_MASK) << (DEVICE_TYPE_LSB))
#define   SRIS_MODE_BIT		(8)
#define   SRIS_MODE_MASK	BIT_32(SRIS_MODE_BIT)

#define PCI_EXP_CAP_ID_OFFSET	0x70U

/* PCIe controller 0 general control 3 (PE0_GEN_CTRL_3) */
#define PE0_GEN_CTRL_3		0x58U
/* LTSSM Enable. Active high. Set it low to hold the LTSSM in Detect state. */
#define LTSSM_EN_MASK		0x1U

#define LTSSM_STATE_L0		0x11U /* L0 state */

#define LINK_INT_CTRL_STS	0x40U
#define LINK_REQ_RST_NOT_INT_EN	BIT_32(1)
#define LINK_REQ_RST_NOT_CLR	BIT_32(2)

#define PE0_INT_STS		0xE8U
#define HP_INT_STS		BIT_32(6)

#define PCI_BASE_CLASS_OFF	24U
#define PCI_SUBCLASS_OTHER	(0x80)
#define PCI_SUBCLASS_OFF	16U

#define PCI_DEVICE_ID_SHIFT	16

#define SERDES_CELL_SIZE	4

struct s32cc_pcie;

enum pcie_link_speed;

struct s32cc_pcie {
	/* Must be the first member of the struct, as the
	 * U-Boot version of the DesignWare API relies on
	 * the (undocumented) restriction that
	 * both s32cc_pcie and pcie_dw objects be at the
	 * same address, so that the object returned by
	 * dev_get_priv(bus) could be cast to any of them
	 */
	struct pcie_dw	pcie;

	enum dw_pcie_device_mode mode;
	void __iomem *ctrl_base;

	int id;
	enum pcie_phy_mode phy_mode;
	enum pcie_link_speed linkspeed;

	struct phy phy0, phy1;
};

static inline
struct s32cc_pcie *to_s32cc_from_dw_pcie(struct pcie_dw *x)
{
	return container_of(x, struct s32cc_pcie, pcie);
}

static inline
bool is_s32cc_pcie_rc(enum dw_pcie_device_mode mode)
{
	return mode == DW_PCIE_RC_TYPE;
}

static inline
const char *s32cc_pcie_ep_rc_mode_str(enum dw_pcie_device_mode mode)
{
	return is_s32cc_pcie_rc(mode) ? "RootComplex" : "EndPoint";
}

void dw_pcie_writel_ctrl(struct s32cc_pcie *pci, u32 reg, u32 val);
u32 dw_pcie_readl_ctrl(struct s32cc_pcie *pci, u32 reg);

int s32cc_check_serdes(struct udevice *dev);

#endif /* PCIE_S32CC_H */
