/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2020 Imagination Technologies Limited
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @file		pfe_platform_cfg.h
 *
 */

#ifndef SRC_PFE_PLATFORM_CFG_H_
#define SRC_PFE_PLATFORM_CFG_H_

#define TMU_TYPE_TMU	  1U
#define TMU_TYPE_TMU_LITE 2U
/**
 * @brief	Number of entries of a HIF ring
 * @note	Must be power of 2
 */
#define PFE_HIF_RING_CFG_LENGTH 16U

/*
 * @brief TMU variant
 */
#define PFE_CFG_TMU_VARIANT TMU_TYPE_TMU_LITE

/*
 * @brief	Accessible memory space base (PA)
 * @details	This is PFE-accessible address space base
 * @warning	Address range given by this base and PFE_CFG_DDR_MASTER_LEN must
 *		be reserved to be exclusively accessible by PFE.
 */
#define PFE_CFG_DDR_MASTER_ADDR 0x00400000ULL /* S32G */

/*
 * @brief	Length of the PFE DDR memory
 */
#define PFE_CFG_DDR_MASTER_LEN 0x01000000ULL /* S32G: 16MB */

/*
 * @brief	The PFE HIF IRQ ID as seen by the host
 */
#define PFE_CFG_HIF_IRQ_ID 204 /* HIF (copy) IRQ */

/*
 * @brief	Maximum supported number of channels
 */
#define HIF_CFG_MAX_CHANNELS 4U

/**
 * @brief	Maximum number of logical interfaces
 */
#define PFE_CFG_MAX_LOG_IFS 8U

/**
 * @brief	The CLASS_PE_SYS_CLK_RATIO[csr_clmode]
 * @details	See the IMG-NPU Technical Reference Manual
 */
#define PFE_CFG_CLMODE 1U /* SYS/AXI = 250MHz, HFE = 500MHz */

/**
 * @brief	Maximum number of buffers - BMU1
 */
#define PFE_CFG_BMU1_BUF_COUNT 0x200U

/**
 * @brief	BMU1 buffer size
 * @details	Value = log2(size)
 */
#define PFE_CFG_BMU1_BUF_SIZE 0x8U /* 256 bytes */

/**
 * @brief	Maximum number of buffers - BMU2
 */
#define PFE_CFG_BMU2_BUF_COUNT 0x10U

/**
 * @brief	BMU2 buffer size
 * @details	Value = log2(size)
 */
#define PFE_CFG_BMU2_BUF_SIZE 0xbU /* 2048 bytes */

/**
 * @brief	DMEM base address as defined by .elf
 */
#define PFE_CFG_CLASS_ELF_DMEM_BASE 0x20000000UL

/**
 * @brief	Size of DMEM per CLASS PE
 */
#define PFE_CFG_CLASS_DMEM_SIZE 0x00004000UL /* 16k */

/**
 * @brief	IMEM base address as defined by .elf
 */
#define PFE_CFG_CLASS_ELF_IMEM_BASE 0x9fc00000UL

/**
 * @brief	Size of IMEM per CLASS PE
 */
#define PFE_CFG_CLASS_IMEM_SIZE 0x00008000UL /* 32kB */

/**
 * @brief	DMEM base address as defined by .elf
 */
#define PFE_CFG_TMU_ELF_DMEM_BASE 0x00000000UL

/**
 * @brief	Size of DMEM per TMU PE
 */
#define PFE_CFG_TMU_DMEM_SIZE 0x00000800UL /* 2kB */

/**
 * @brief	IMEM base address as defined by .elf
 */
#define PFE_CFG_TMU_ELF_IMEM_BASE 0x00010000UL

/**
 * @brief	Size of IMEM per TMU PE
 */
#define PFE_CFG_TMU_IMEM_SIZE 0x00002000UL /* 8kB */

/**
 * @brief	DMEM base address as defined by .elf
 */
#define PFE_CFG_UTIL_ELF_DMEM_BASE PFE_CFG_CLASS_ELF_DMEM_BASE

/**
 * @brief	Size of DMEM per UTIL PE
 */
#define PFE_CFG_UTIL_DMEM_SIZE PFE_CFG_CLASS_DMEM_SIZE

/**
 * @brief	IMEM base address as defined by .elf
 */
#define PFE_CFG_UTIL_ELF_IMEM_BASE PFE_CFG_CLASS_ELF_IMEM_BASE

/**
 * @brief	Size of IMEM per UTIL PE
 */
#define PFE_CFG_UTIL_IMEM_SIZE PFE_CFG_CLASS_IMEM_SIZE

/**
 * @brief	Physical CBUS base address as seen by PFE
 */
#define PFE_CFG_CBUS_PHYS_BASE_ADDR 0xc0000000U

/**
 * @brief	Physical CBUS base address as seen by CPUs
 */
#define PFE_CFG_CBUS_PHYS_BASE_ADDR_CPU 0x46000000U

/**
 * @brief	CBUS length
 */
#define PFE_CFG_CBUS_LENGTH 0x01000000U

/**
 * @brief	Offset in LMEM where BMU1 buffers area starts
 */
#define PFE_CFG_BMU1_LMEM_BASEADDR 0U

/**
 * @brief	Size of BMU1 buffers area in number of bytes
 */
#define PFE_CFG_BMU1_LMEM_SIZE \
	((1UL << PFE_CFG_BMU1_BUF_SIZE) * PFE_CFG_BMU1_BUF_COUNT)

/**
 * @brief	Offset in LMEM, where PE memory area starts
 */
#define PFE_CFG_PE_LMEM_BASE \
	(PFE_CFG_BMU1_LMEM_BASEADDR + PFE_CFG_BMU1_LMEM_SIZE)

/**
 * @brief	Size of PE memory area in number of bytes
 */
#define PFE_CFG_PE_LMEM_SIZE (CBUS_LMEM_SIZE - PFE_CFG_BMU1_LMEM_SIZE)

/**
 * @brief Translates from host CPU physical address space to PFE address space
 */
#define PFE_CFG_MEMORY_PHYS_TO_PFE(p) (p)

/**
 * @brief Translates from PFE address space to host CPU physical address space
 */
#define PFE_CFG_MEMORY_PFE_TO_PHYS(p) (p)

/**
 * @brief	Local physical interface identifier
 * @details	In multi-instance environment, where multiple platform drivers
 *		can be deployed, this identifier represents the physical
 *		interface (usually HIF channel) associated with the current
 *		driver instance.
 */
#ifdef PFE_CFG_PFE_MASTER
#define PFE_CFG_LOCAL_PHY_IF_ID PFE_PHY_IF_ID_HIF0
#endif /* PFE_CFG_PFE_MASTER */

#ifdef PFE_CFG_PFE_SLAVE
#define PFE_CFG_LOCAL_PHY_IF_ID PFE_PHY_IF_ID_HIF1
#endif /* PFE_CFG_PFE_SLAVE */

/* LMEM defines */
#define PFE_CFG_LMEM_HDR_SIZE	  0x0070U
#define PFE_CFG_LMEM_BUF_SIZE_LN2 0x8U /* 256 */
#define PFE_CFG_LMEM_BUF_SIZE	  BIT(PFE_CFG_LMEM_BUF_SIZE_LN2)

/* DDR defines */
#define PFE_CFG_DDR_HDR_SIZE	 0x0200U
#define PFE_CFG_DDR_BUF_SIZE_LN2 0xbU /* 2048 */
#define PFE_CFG_DDR_BUF_SIZE	 BIT(PFE_CFG_DDR_BUF_SIZE_LN2)

/* RO defines */
#define PFE_CFG_RO_HDR_SIZE 0x0010U

#endif /* SRC_PFE_PLATFORM_CFG_H_ */

/** @}*/
