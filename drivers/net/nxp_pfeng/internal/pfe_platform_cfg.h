/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2024 NXP
 */

#ifndef PFE_PLATFORM_CFG_H_
#define PFE_PLATFORM_CFG_H_

#include <linux/bitops.h>

/* Maximum number of buffers - BMU1 */
#define PFE_CFG_BMU1_BUF_COUNT 0x200U

/* BMU1 buffer size
 * Value = log2(size)
 */
#define PFE_CFG_BMU1_BUF_SIZE 0x8U /* 256 bytes */

/* Maximum number of buffers - BMU2 */
#define PFE_CFG_BMU2_BUF_COUNT 0x10U

/* BMU2 buffer size
 * Value = log2(size)
 */
#define PFE_CFG_BMU2_BUF_SIZE 0xbU /* 2048 bytes */

/* DMEM base address as defined by .elf */
#define PFE_CFG_CLASS_ELF_DMEM_BASE 0x20000000UL

/* Size of DMEM per CLASS PE */
#define PFE_CFG_CLASS_DMEM_SIZE 0x00004000UL /* 16k */

/* IMEM base address as defined by .elf */
#define PFE_CFG_CLASS_ELF_IMEM_BASE 0x9fc00000UL

/* Size of IMEM per CLASS PE */
#define PFE_CFG_CLASS_IMEM_SIZE 0x00008000UL /* 32kB */

/* Physical CBUS base address as seen by PFE */
#define PFE_CFG_CBUS_PHYS_BASE_ADDR 0xc0000000U

/* Physical CBUS base address as seen by CPUs */
#define PFE_CFG_CBUS_PHYS_BASE_ADDR_CPU 0x46000000U

/* CBUS length */
#define PFE_CFG_CBUS_LENGTH 0x01000000U

/* Offset in LMEM where BMU1 buffers area starts */
#define PFE_CFG_BMU1_LMEM_BASEADDR 0U

/* Size of BMU1 buffers area in number of bytes */
#define PFE_CFG_BMU1_LMEM_SIZE \
	((1UL << PFE_CFG_BMU1_BUF_SIZE) * PFE_CFG_BMU1_BUF_COUNT)

/* LMEM defines */
#define PFE_CFG_LMEM_HDR_SIZE_G2  0x0070U
#define PFE_CFG_LMEM_HDR_SIZE_G3  0x0030U
#define PFE_CFG_LMEM_BUF_SIZE_LN2 0x8U /* 256 */
#define PFE_CFG_LMEM_BUF_SIZE	  BIT_32(PFE_CFG_LMEM_BUF_SIZE_LN2)

/* DDR defines */
#define PFE_CFG_DDR_HDR_SIZE	 0x0200U
#define PFE_CFG_DDR_BUF_SIZE_LN2 0xbU /* 2048 */
#define PFE_CFG_DDR_BUF_SIZE	 BIT_32(PFE_CFG_DDR_BUF_SIZE_LN2)

/* RO defines */
#define PFE_CFG_RO_HDR_SIZE_G2	0x0100U
#define PFE_CFG_RO_HDR_SIZE_G3	0x0200U

#endif /* PFE_PLATFORM_CFG_H_ */
