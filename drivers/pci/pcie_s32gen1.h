/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2022 NXP
 * S32Gen1 PCIe driver
 */

#ifndef PCIE_S32GEN1_H
#define PCIE_S32GEN1_H
#include <dm.h>
#include <generic-phy.h>
#include <pci.h>
#include <asm/io.h>
#include <linux/ioport.h>

#ifdef CONFIG_PCIE_DEBUG_WRITES
#define debug_wr debug
#else
#define debug_wr(...)
#endif

#define UPTR(a)		((uintptr_t)(a))

#define s32_dbi_readb(addr) \
	readb_relaxed((addr))

#define s32_dbi_readw(addr) \
	readw_relaxed((addr))

#define s32_dbi_readl(addr) \
	readl_relaxed((addr))

#define s32_dbi_writeb(addr, val) \
	writeb_relaxed((val), (addr))

#define s32_dbi_writew(addr, val) \
	writew_relaxed((val), (addr))

#define s32_dbi_writel(addr, val) \
	writel_relaxed((val), (addr))

#define W16(address, write_data) \
do { \
	debug_wr("%s: W16(0x%llx, 0x%x) ... ", __func__, (u64)(address), \
		(uint32_t)(write_data)); \
	s32_dbi_writew((address), (write_data)); \
	debug_wr("done\n"); \
} while (0)

#define W32(address, write_data) \
do { \
	debug_wr("%s: W32(0x%llx, 0x%x) ... ", __func__, (u64)(address), \
		(uint32_t)(write_data)); \
	s32_dbi_writel((address), (write_data)); \
	debug_wr("done\n"); \
} while (0)

/* BCLR32 - SPARSE compliant version of `clrbits_le32`, with debug support. */
#define BCLR32(address, mask_clear) \
do { \
	uintptr_t bclr_address = (address); \
	u32 bclr_mask_clear = (mask_clear); \
	debug_wr("%s: BCLR32(0x%lx, 0x%x) ... ", __func__, \
		bclr_address, bclr_mask_clear); \
	s32_dbi_writel(bclr_address, \
		       s32_dbi_readl(bclr_address) & ~(bclr_mask_clear)); \
	debug_wr("done\n"); \
} while (0)

/* BSET32 - SPARSE compliant version of `setbits_le32`, with debug support. */
#define BSET32(address, mask_set) \
do { \
	uintptr_t bset_address = (address); \
	u32 bset_mask_set = (mask_set); \
	debug_wr("%s: BSET32(0x%lx, 0x%x) ... ", __func__, \
		bset_address, bset_mask_set); \
	s32_dbi_writel(bset_address, \
		       s32_dbi_readl(bset_address) | (bset_mask_set)); \
	debug_wr("done\n"); \
} while (0)

/* RMW32 - SPARSE compliant version of `clrsetbits_le32`, with debug support.
 * Please pay attention to the arguments order.
 */
#define RMW32(address, mask_set, mask_clear) \
do { \
	uintptr_t rmw_address = (address); \
	u32 rmw_mask_set = (mask_set), rmw_mask_clear = (mask_clear); \
	debug_wr("%s: RMW32(0x%lx, set 0x%x, clear 0x%x) ... ", __func__, \
		rmw_address, rmw_mask_set, rmw_mask_clear); \
	s32_dbi_writel(rmw_address, \
		       (s32_dbi_readl(rmw_address) & ~(rmw_mask_clear)) | \
		       (rmw_mask_set)); \
	debug_wr("done\n"); \
} while (0)

#define SERDES_SS_BASE		0x80000

/* PCIe Controller 0 General Control 1-4 */
#define SS_PE0_GEN_CTRL_1	(SERDES_SS_BASE + 0x1050U)
#define SS_PE0_GEN_CTRL_2	(SERDES_SS_BASE + 0x1054U)
#define SS_PE0_GEN_CTRL_3	(SERDES_SS_BASE + 0x1058U)
#define SS_PE0_GEN_CTRL_4	(SERDES_SS_BASE + 0x105cU)

/* PCIe Controller 0 Link Debug 2 */
#define SS_PE0_LINK_DBG_2	(SERDES_SS_BASE + 0x10b4U)

/* PCIe Controller 0 Link Debug 1 */
#define SS_PE0_LINK_DBG_1	(SERDES_SS_BASE + 0x10b0U)

/* Field definitions for PE0_GEN_CTRL_1 */
#define DEVICE_TYPE_OVERRIDE	0x10
#define DEVICE_TYPE_EP		0x0
#define DEVICE_TYPE_RC		0x4

#define DEVICE_TYPE_LSB		(0)
#define DEVICE_TYPE_MASK	(0x0000000F)
#define DEVICE_TYPE		((DEVICE_TYPE_MASK) << \
				 (DEVICE_TYPE_LSB))

/* Field definitions for PE0_LINK_DBG_2 */
#define SMLH_LTSSM_STATE_LSB	(0)
#define SMLH_LTSSM_STATE_MASK	(0x0000003F)
#define SMLH_LTSSM_STATE	((SMLH_LTSSM_STATE_MASK) << \
				 (SMLH_LTSSM_STATE_LSB))

#define SMLH_LINK_UP_BIT	(6)
#define SMLH_LINK_UP		BIT(SMLH_LINK_UP_BIT)

#define RDLH_LINK_UP_BIT	(7)
#define RDLH_LINK_UP		BIT(RDLH_LINK_UP_BIT)

#define SERDES_LINKUP_MASK	(SMLH_LINK_UP | RDLH_LINK_UP | \
		SMLH_LTSSM_STATE)
#define SERDES_LINKUP_EXPECT	(SMLH_LINK_UP | RDLH_LINK_UP | \
		BUILD_MASK_VALUE(SMLH_LTSSM_STATE, LTSSM_STATE_L0))

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
#define CONFIG_SYS_PCI_EP_MEMORY_BASE (PHYS_SDRAM_1 + \
				       (PHYS_SDRAM_1_SIZE / 2))
#endif

#define PCIE_BIT_VALUE(reg_val, bit_name) \
			((reg_val & bit_name ## _MASK) >> bit_name ## _LSB)
#define PCIE_BIT_MASK(bit_name) \
			((bit_name ## _MASK) << (bit_name ## _LSB))

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
#define PCIE_LINK_STATUS			(0x82)
#define PCIE_LINK_CRT_SPEED_MASK		0xf
#define PCIE_LINK_CRT_SPEED_LSB			0
#define PCIE_LINK_CRT_WIDTH_MASK		0x3f0
#define PCIE_LINK_CRT_WIDTH_LSB			4

/* Debug Register 0 (PL_DEBUG0_OFF) */
#define PCIE_PL_DEBUG0		(0x728)
#define LTSSM_STATE_MASK	0x3f
#define LTSSM_STATE_LSB		0

/* Debug Register 1 (PL_DEBUG1_OFF) */
#define PCIE_PL_DEBUG1		(0x72C)

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
#define LTSSM_STATE_L0		0x11 /* L0 state */

/* Register SS_RW_REG_0 */
#define CLKEN_MASK	0x1
#define CLKEN_LSB	23

struct s32_pcie {
	struct list_head list;
	struct phy phy0, phy1;
	struct udevice *bus;
	struct resource cfg_res;
	void __iomem *dbi;
	void __iomem *cfg0;
#ifdef PCIE_USE_CFG1
	void __iomem *cfg1;
#endif
	int id;
	bool enabled;
	bool ep_mode;
	u8 has_phy1;
	enum pcie_link_width linkwidth;
	enum pcie_link_speed linkspeed;
	enum pcie_phy_mode phy_mode;
	int atu_out_num;
	int atu_in_num;
};

extern struct list_head s32_pcie_list;

void pci_header_show_brief(struct udevice *dev);
int uclass_find_first_device(enum uclass_id id, struct udevice **devp);
int uclass_find_next_device(struct udevice **devp);

#endif /* PCIE_S32GEN1_H */
