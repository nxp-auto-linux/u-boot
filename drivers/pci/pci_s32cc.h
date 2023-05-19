/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2023 NXP
 * S32CC PCIe driver
 */

#ifndef PCIE_S32CC_H
#define PCIE_S32CC_H

#include <generic-phy.h>
#include <pci.h>
#include <pci_ep.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/ioport.h>
#include <linux/stringify.h>
#include <linux/types.h>

#include "pcie-designware.h"

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
	struct pcie_dw	pcie;

	void __iomem *ctrl_base;

	enum dw_pcie_device_mode mode;
	int id;
	enum pcie_phy_mode phy_mode;
	enum pcie_link_speed linkspeed;

	struct phy phy0, phy1;

	/* Used only if CONFIG_PCI_S32CC_USE_DW_CFG_IATU_SETUP not defined */
	void __iomem *cfg0;
	int cfg0_seq;
	void __iomem *cfg1;

	int atu_out_num;
	int atu_in_num;
};

struct s32cc_pcie_ep {
	struct s32cc_pcie common;

	void __iomem *shared_base;
	void __iomem *addr_space_base;
	phys_size_t shared_size;
	phys_size_t addr_space_size;

	bool auto_config_bars;

	struct pci_bar ep_bars[BAR_5 + 1];
};

/**
 * struct pci_epc_features - features supported by an EP controller device.
 * Adapted from Linux pci-epc.h, with interrupts and notifiers stripped down.
 * @reserved_bar: bitmap to indicate reserved BAR unavailable to driver
 * @bar_fixed_64bit: bitmap to indicate fixed 64bit BARs
 * @bar_fixed_size: Array specifying the size supported by each BAR
 * @align: alignment size required for BAR buffer allocation
 */
struct pci_epc_features {
	u8	reserved_bar;
	u8	bar_fixed_64bit;
	u64	bar_fixed_size[BAR_5 + 1];
	size_t	align;
};

static inline
struct s32cc_pcie *to_s32cc_from_dw_pcie(struct pcie_dw *x)
{
	return container_of(x, struct s32cc_pcie, pcie);
}

static inline
struct s32cc_pcie_ep *to_s32cc_ep_from_s32cc_pcie(struct s32cc_pcie *x)
{
	return container_of(x, struct s32cc_pcie_ep, common);
}

static inline
bool is_s32cc_pcie_rc(enum dw_pcie_device_mode mode)
{
	return mode == DW_PCIE_RC_TYPE;
}

static inline
bool is_s32cc_pcie_ep(enum dw_pcie_device_mode mode)
{
	return mode == DW_PCIE_EP_TYPE;
}

static inline
const char *s32cc_pcie_ep_rc_mode_str(enum dw_pcie_device_mode mode)
{
	return is_s32cc_pcie_rc(mode) ? "RootComplex" : "EndPoint";
}

void dw_pcie_writel_ctrl(struct s32cc_pcie *pci, u32 reg, u32 val);
u32 dw_pcie_readl_ctrl(struct s32cc_pcie *pci, u32 reg);

void s32cc_pcie_dump_atu(struct s32cc_pcie *s32cc_pp);

int s32cc_check_serdes(struct udevice *dev);
void s32cc_pcie_set_device_id(struct s32cc_pcie *s32cc_pp);
int s32cc_pcie_dt_init_common(struct s32cc_pcie *s32cc_pp);
int s32cc_pcie_dt_init_host(struct udevice *dev);
int s32cc_pcie_init_controller(struct s32cc_pcie *s32cc_pp);

void pci_header_show_brief(struct udevice *dev);

#endif /* PCIE_S32CC_H */
