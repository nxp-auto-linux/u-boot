/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2020 NXP
 * S32Gen1 PCIe driver
 */

#ifndef PCIE_S32GEN1_H
#define PCIE_S32GEN1_H
#include <pci.h>
#include <dm.h>
#include <asm/io.h>

#include "serdes_regs.h"
#include "ss_pcie_regs.h"
#include "mc_rgm_regs.h"

#ifdef CONFIG_PCIE_DEBUG_WRITES
#define debug_wr debug
#else
#define debug_wr(...)
#endif

#define W16(address, write_data) \
do { \
	debug_wr("%s: W16(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(write_data)); \
	out_le16(address, write_data); \
} while(0)


#define W32(address, write_data) \
do { \
	debug_wr("%s: W32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(write_data)); \
	out_le32(address, write_data); \
} while(0)

#define BCLR32(address, mask) \
do { \
	debug_wr("%s: BCLR32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(mask)); \
	clrbits_le32(address, mask); \
} while(0)

#define BSET32(address, mask) \
do { \
	debug_wr("%s: BSET32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(mask)); \
	setbits_le32(address, mask); \
} while(0)

#define RMW32(address, write_data, mask) \
do { \
	debug_wr("%s: RMW32(0x%llx, 0x%x, mask 0x%x)\n", __func__, \
		(uint64_t)(address), (uint32_t)(write_data), \
		(uint32_t)(mask)); \
	clrsetbits_le32(address, mask, write_data); \
} while(0)

#ifndef CONFIG_SYS_PCI_MEMORY_BUS
#define CONFIG_SYS_PCI_MEMORY_BUS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_PHYS
#define CONFIG_SYS_PCI_MEMORY_PHYS CONFIG_SYS_SDRAM_BASE
#endif

#ifndef CONFIG_SYS_PCI_MEMORY_SIZE
#define CONFIG_SYS_PCI_MEMORY_SIZE (128 * 1024 * 1024UL) /* 128MB */
#endif

/* Used for setting iATU windows for EP mode */
#ifndef CONFIG_SYS_PCI_EP_MEMORY_BASE
#define CONFIG_SYS_PCI_EP_MEMORY_BASE \
			DDR_BASE_ADDR + (CONFIG_SYS_DDR_SIZE / 2)
#endif

#define PCIE_BIT_VALUE(reg_val, bit_name) \
			((reg_val & bit_name ## _MASK) >> bit_name ## _OFF)
#define PCIE_BIT_MASK(bit_name) \
			((bit_name ## _MASK) << (bit_name ## _OFF))

/* iATU register offsets and fields */

#define PCIE_ATU_TYPE_MEM		(0x0)
#define PCIE_ATU_TYPE_IO		(0x2)
#define PCIE_ATU_TYPE_CFG0		(0x4)
#define PCIE_ATU_TYPE_CFG1		(0x5)

#define PCIE_ATU_BUS(x)			(((x) & 0xff) << 24)
#define PCIE_ATU_DEV(x)			(((x) & 0x1f) << 19)
#define PCIE_ATU_FUNC(x)		(((x) & 0x7) << 16)

/*
 * DBI register offsets and fields
 */

/* Upper half of the LINK_CTRL_STATUS register, accessible by half-word reads */
#define PCIE_LINK_STATUS(dbi_base)	((uint64_t)(dbi_base) + 0x82)
#define PCIE_LINK_SPEED_MASK		0xf
#define PCIE_LINK_SPEED_OFF			0
#define PCIE_LINK_WIDTH_MASK		0x3f0
#define PCIE_LINK_WIDTH_OFF			4

/* Debug Register 0 (PL_DEBUG0_OFF) */
#define PCIE_PL_DEBUG0(dbi_base)	((uint64_t)(dbi_base) + 0x728)
#define LTSSM_STATE_MASK	0x3f
#define LTSSM_STATE_OFF		0

#define LTSSM_STATE_L0		0x11 /* L0 state */

/* Debug Register 1 (PL_DEBUG1_OFF) */
#define PCIE_PL_DEBUG1(dbi_base)	((uint64_t)(dbi_base) + 0x72C)

#define PCIE_ATU_NR_REGIONS			6

/* The following defines are used for EP mode only */
#define PCIE_ATU_MSI_REGION_NR			3
/* PCI_BASE_ADDRESS_MEM_PREFETCH is defined in pci.h, but not
 * PCI_BASE_ADDRESS_MEM_NON_PREFETCH
 */
#define PCI_BASE_ADDRESS_MEM_NON_PREFETCH	0x00	/* non-prefetchable */
#define PCIE_EP_BAR0_SIZE			SZ_1M
#define PCIE_EP_BAR1_SIZE			0
#define PCIE_EP_BAR2_SIZE			SZ_1M
#define PCIE_EP_BAR3_SIZE			0
#define PCIE_EP_BAR4_SIZE			0
#define PCIE_EP_BAR5_SIZE			0
#define PCIE_EP_ROM_SIZE			0
#define PCIE_EP_BAR0_EN_DIS		1
#define PCIE_EP_BAR1_EN_DIS		0
#define PCIE_EP_BAR2_EN_DIS		1
#define PCIE_EP_BAR3_EN_DIS		1
#define PCIE_EP_BAR4_EN_DIS		1
#define PCIE_EP_BAR5_EN_DIS		0
#define PCIE_EP_ROM_EN_DIS		0
#define PCIE_EP_BAR0_INIT	(PCI_BASE_ADDRESS_SPACE_MEMORY | \
			PCI_BASE_ADDRESS_MEM_TYPE_32 | \
			PCI_BASE_ADDRESS_MEM_NON_PREFETCH)
#define PCIE_EP_BAR1_INIT	(PCI_BASE_ADDRESS_SPACE_MEMORY | \
			PCI_BASE_ADDRESS_MEM_TYPE_32 | \
			PCI_BASE_ADDRESS_MEM_NON_PREFETCH)
#define PCIE_EP_BAR2_INIT	(PCI_BASE_ADDRESS_SPACE_MEMORY | \
			PCI_BASE_ADDRESS_MEM_TYPE_32 | \
			PCI_BASE_ADDRESS_MEM_NON_PREFETCH)
#define PCIE_EP_BAR3_INIT	(PCI_BASE_ADDRESS_SPACE_MEMORY | \
			PCI_BASE_ADDRESS_MEM_TYPE_32 | \
			PCI_BASE_ADDRESS_MEM_NON_PREFETCH)
#define PCIE_EP_BAR4_INIT	(PCI_BASE_ADDRESS_SPACE_MEMORY | \
			PCI_BASE_ADDRESS_MEM_TYPE_32 | \
			PCI_BASE_ADDRESS_MEM_NON_PREFETCH)
#define PCIE_EP_BAR5_INIT	0
#define PCIE_EP_ROM_INIT	0
/* End EP specific defines */

/* Shadow registers (dbi_cs2) */

#define PCIE_CS2_OFFSET		0x20000

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

enum pcie_dev_type {
	PCIE_EP = 0x0,
	PCIE_RC = 0x4
};

enum pcie_link_speed {
	GEN1 = 0x1,
	GEN2 = 0x2,
	GEN3 = 0x3
};

enum pcie_link_width {
	X1 = 0x1,
	X2 = 0x2
};

struct s32_pcie {
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	struct fdt_resource ctrl_res;
	struct fdt_resource cfg_res;
	void __iomem *dbi;
	void __iomem *cfg0;
#ifdef PCIE_USE_CFG1
	void __iomem *cfg1;
#endif
	int id;
	bool enabled;
	bool ep_mode;
	bool clk_int;
	enum pcie_link_width linkwidth;
	enum pcie_link_speed linkspeed;
	int atu_out_num;
	int atu_in_num;
};

extern struct list_head s32_pcie_list;

void pci_header_show_brief(struct udevice *dev);
int uclass_find_first_device(enum uclass_id id, struct udevice **devp);
int uclass_find_next_device(struct udevice **devp);

#endif /* PCIE_S32GEN1_H */
