/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 * S32Gen1 PCIe driver
 */

#ifndef SERDES_S32GEN1_H
#define SERDES_S32GEN1_H
#include <pci.h>
#include <dm.h>
#include <asm/io.h>

#include "serdes_regs.h"
#include "ss_pcie_regs.h"
#include "mc_rgm_regs.h"
#include "serdes_s32gen1_io.h"

#define PCIE_LINK_UP_COUNT 50
#define PCIE_MPLL_LOCK_COUNT 10
#define DELAY_QUANTUM 1000

#define SERDES_LINKUP_MASK	(SMLH_LINK_UP | RDLH_LINK_UP | \
			SMLH_LTSSM_STATE)
#define SERDES_LINKUP_EXPECT	(SMLH_LINK_UP | RDLH_LINK_UP | \
			SMLH_LTSSM_STATE_VALUE(LTSSM_STATE_L0))

/* Configuration Request Retry Status (CRS) Enable. Active high. */
/* Defer incoming configuration requests. */
#define CRS_EN					0x2
/* LTSSM Enable. Active high. Set it low to hold the LTSSM in Detect state. */
#define LTSSM_EN				0x1


/* Register SS_RW_REG_0 */

#define CLKEN_MASK	0x1
#define CLKEN_OFF	23

/* Serdes Subsystem mode :
 * 000b - PCIe Gen3x2 mode
 * 001b - PCIe Gen3x1 and SGMII 1G bifurcation mode
 * 010b - PCIe Gen3x1 and SGMII 1G bifurcation mode
 * 011b - Two SGMII 1G/2.5G bifurcation mode
 *
 * TODO: not sure of the difference between SUBSYS_MODE values 1 and 2, since
 * S32G RM, chapter 55.1.2, says:
 *
 * SerDes_0 working modes
 * Mode         PHY lane 0      PHY lane 1              PHY Ref Clock (MHz)
 * PCIe only    PCIe0_X2        PCIe0_X2                100
 * PCIe/SGMII   PCIe0_X1        SGMII 1.25Gbps          100
 *                              (GMAC0 or PFE_MAC2)
 * SGMII only   SGMII 1.25Gbps  SGMII 1.25Gbps          100/125
 *              (GMAC0)         (PFE_MAC2)
 *
 * SerDes_1 working modes
 * Mode         PHY lane 0      PHY lane 1              PHY Ref Clock (MHz)
 * PCIe only    PCIe1_X2        PCIe1_X2                100
 * PCIe/SGMII   PCIe1_X1        SGMII 1.25Gbps          100
 *                              (PFE_MAC0 or PFE_MAC1)
 * SGMII only   SGMII 1.25Gbps  SGMII 1.25Gbps          100/125
 *              (PFE_MAC0)      (PFE_MAC1)
 * SGMII only   SGMII 3.125Gbps SGMII 3.125Gbps         125
 *              (PFE_MAC0)      (PFE_MAC0)
 */
#define SUBSYS_MODE_MASK	0x7
#define SUBSYS_MODE_OFF		0

#define SUBSYS_MODE_PCIE_ONLY	0
#define SUBSYS_MODE_PCIE_SGMII0	1
#define SUBSYS_MODE_PCIE_SGMII1	2
#define SUBSYS_MODE_SGMII		3

#define RST_CTRL		0x3010
#define WARM_RST		0x2
#define COLD_RST		0x1

enum serdes_dev_type {
	SERDES_INVALID = -1,
	PCIE_EP = 0x1, /* EP mode is 0x0, use 0x1 to allow us to use masks */
	PCIE_RC = 0x4,
	SGMII = 0x10 /* outside range of PE0_GEN_CTRL_1:DEVICE_TYPE */
};
/* use a mask to fix DEVICE_TYPE for EP */
#define SERDES_PCIE_MODE(mode) (mode & 0xe)

enum serdes_link_width {
	X1 = 0x1,
	X2 = 0x2
};

struct s32_serdes {
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	void __iomem *dbi;

	int id;
	enum serdes_dev_type devtype;
	bool clk_int;
	enum serdes_link_width linkwidth;
};

bool wait_read32(void *address, uint32_t expect_data,
		uint32_t mask, int read_attempts);

#endif /* PCIE_S32GEN1_H */
