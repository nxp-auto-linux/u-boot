/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This file contains code taken from Linux
 * Synopsys DesignWare PCIe host controller header file
 * pcie-designware.h.
 *
 * Copyright (C) 2013 Samsung Electronics Co., Ltd.
 *		https://www.samsung.com
 *
 * Author: Jingoo Han <jg1.han@samsung.com>
 */

#ifndef _PCIE_DESIGNWARE_H
#define _PCIE_DESIGNWARE_H

#include "pcie_dw_common.h"

/* Linux and U-Boot don't use the same name for this struct */
#define dw_pcie pcie_dw

/* Parameters for the waiting for link up routine */
#define LINK_WAIT_MAX_RETRIES		        10
#define LINK_WAIT_USLEEP_MAX		        100000

/* Parameters for the waiting for DBI R/W enabled routine */
#define LINK_WAIT_MAX_DBI_RW_EN_RETRIES		1000
#define LINK_WAIT_DBI_RW_EN			10

/* More Synopsys-specific PCIe configuration registers */

#define PCIE_PORT_DEBUG0		        0x728U
#define PORT_LOGIC_LTSSM_STATE_MASK	        0x1f
#define PORT_LOGIC_LTSSM_STATE_L0	        0x11
#define PCIE_PORT_DEBUG1		        0x72CU
#define PCIE_PORT_DEBUG1_LINK_UP		BIT(4)
#define PCIE_PORT_DEBUG1_LINK_IN_TRAINING	BIT(29)

enum dw_pcie_device_mode {
	DW_PCIE_UNKNOWN_TYPE,
	DW_PCIE_EP_TYPE,
	DW_PCIE_LEG_EP_TYPE,
	DW_PCIE_RC_TYPE,
};

enum dw_pcie_as_type {
	DW_PCIE_AS_UNKNOWN,
	DW_PCIE_AS_MEM,
	DW_PCIE_AS_IO,
};

u8 dw_pcie_find_capability(struct dw_pcie *pci, u8 cap);

int dw_pcie_read(void __iomem *addr, int size, u32 *val);
int dw_pcie_write(void __iomem *addr, int size, u32 val);

u32 dw_pcie_read_dbi(struct dw_pcie *pci, u32 reg, int size);
void dw_pcie_write_dbi(struct dw_pcie *pci, u32 reg, int size, u32 val);
void dw_pcie_write_dbi2(struct dw_pcie *pci, u32 reg, int size, u32 val);

int dw_pcie_link_up(struct dw_pcie *pci);
int dw_pcie_wait_for_link(struct dw_pcie *pci);

void dw_pcie_setup_rc(struct dw_pcie *pci);

static inline void dw_pcie_writel_dbi(struct dw_pcie *pci, u32 reg, u32 val)
{
	dw_pcie_write_dbi(pci, reg, 0x4, val);
}

static inline void dw_pcie_writel_dbi2(struct dw_pcie *pci, u32 reg, u32 val)
{
	dw_pcie_write_dbi2(pci, reg, 0x4, val);
}

static inline u32 dw_pcie_readl_dbi(struct dw_pcie *pci, u32 reg)
{
	return dw_pcie_read_dbi(pci, reg, 0x4);
}

static inline void dw_pcie_writew_dbi(struct dw_pcie *pci, u32 reg, u16 val)
{
	dw_pcie_write_dbi(pci, reg, 0x2, val);
}

static inline u16 dw_pcie_readw_dbi(struct dw_pcie *pci, u32 reg)
{
	return dw_pcie_read_dbi(pci, reg, 0x2);
}

static inline void dw_pcie_writeb_dbi(struct dw_pcie *pci, u32 reg, u8 val)
{
	dw_pcie_write_dbi(pci, reg, 0x1, val);
}

static inline u8 dw_pcie_readb_dbi(struct dw_pcie *pci, u32 reg)
{
	return dw_pcie_read_dbi(pci, reg, 0x1);
}

u32 dw_pcie_readl_ib_unroll(struct dw_pcie *pci, u32 index, u32 reg);
void dw_pcie_writel_ib_unroll(struct dw_pcie *pci, u32 index, u32 reg,
			      u32 val);
int dw_pcie_prog_inbound_atu_unroll(struct dw_pcie *pci, u8 func_no,
				    int index, int bar, u64 cpu_addr,
				    enum dw_pcie_as_type as_type);

static inline void dw_pcie_dbi_ro_wr_en(struct dw_pcie *pci)
{
	u32 ret;

	/* Make sure DBI R/W is really enabled. */
	for (ret = 0; ret < LINK_WAIT_MAX_DBI_RW_EN_RETRIES; ret++) {
		u32 reg = PCIE_MISC_CONTROL_1_OFF;

		dw_pcie_dbi_write_enable(pci, true);
		u32 val2 = dw_pcie_readl_dbi(pci, reg);

		if (val2 & PCIE_DBI_RO_WR_EN)
			return;

		udelay(LINK_WAIT_DBI_RW_EN);
	}
	dev_err(pci->dev, "DBI R/W is not being enabled\n");
}

static inline void dw_pcie_dbi_ro_wr_dis(struct dw_pcie *pci)
{
	dw_pcie_dbi_write_enable(pci, false);
}

#endif /* _PCIE_DESIGNWARE_H */
