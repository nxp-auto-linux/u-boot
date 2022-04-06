/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020,2022 NXP
 * S32Gen1 SerDes I/O macros
 */

#ifndef SERDES_S32GEN1_IO_H
#define SERDES_S32GEN1_IO_H
#include <asm/io.h>

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

#endif /* SERDES_S32GEN1_IO_H */
