/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
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

#define W16(address, write_data) \
do { \
	debug_wr("%s: W16(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(write_data)); \
	out_le16(address, write_data); \
} while (0)


#define W32(address, write_data) \
do { \
	debug_wr("%s: W32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(write_data)); \
	out_le32(address, write_data); \
} while (0)

#define BCLR32(address, mask) \
do { \
	debug_wr("%s: BCLR32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(mask)); \
	clrbits_le32(address, mask); \
} while (0)

#define BSET32(address, mask) \
do { \
	debug_wr("%s: BSET32(0x%llx, 0x%x)\n", __func__, (uint64_t)(address), \
		(uint32_t)(mask)); \
	setbits_le32(address, mask); \
} while (0)

#define RMW32(address, write_data, mask) \
do { \
	debug_wr("%s: RMW32(0x%llx, 0x%x, mask 0x%x)\n", __func__, \
		(uint64_t)(address), (uint32_t)(write_data), \
		(uint32_t)(mask)); \
	clrsetbits_le32(address, mask, write_data); \
} while (0)

#endif /* SERDES_S32GEN1_IO_H */
