// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2019-2020 NXP
 */

/**
 * @addtogroup  dxgr_OAL_MM
 * @{
 *
 * @file		oal_mm_linux.c
 * @brief		The oal_mm module source file (UBOOT).
 * @details		This file contains UBOOT-specific memory management implementation.
 *
 */

#include <malloc.h>
#include <memalign.h>

#include "oal.h"
#include "oal_mm.h"

#define PTR_ALIGN(p, a)         ((typeof(p))ALIGN((unsigned long)(p), (a)))

#define OAL_CACHE_ALLIGN	64

/**
 *	Allocate physically contiguous buffer physically aligned to "align" bytes. If
 *	"align" is zero, no alignment will be performed.
 */
static void *__oal_mm_malloc_contig(const addr_t size, const uint32_t align, const bool_t cacheable)
{
	return memalign(align, size);
}

/**
 *	Allocate aligned, contiguous, non-cacheable memory region
 */
void *oal_mm_malloc_contig_aligned_nocache(const addr_t size, const uint32_t align)
{
	return __oal_mm_malloc_contig(size, align, FALSE);
}

/**
 *	Allocate aligned, contiguous, cacheable memory region
 */
void *oal_mm_malloc_contig_aligned_cache(const addr_t size, const uint32_t align)
{
	return __oal_mm_malloc_contig(size, align, TRUE);
}

/**
 *	Allocate aligned, contiguous, non-cacheable memory region from named pool
 */
void *oal_mm_malloc_contig_named_aligned_nocache(const char_t *pool, const addr_t size, const uint32_t align)
{
	return __oal_mm_malloc_contig(size, align, FALSE);
}

/**
 *	Release memory allocated by __hwb_malloc_contig()
 */
void oal_mm_free_contig(const void *vaddr)
{
	/* EMPTY */
}

/**
 * Standard memory allocation
 */
void *oal_mm_malloc(const addr_t size)
{
	return malloc_cache_aligned(size);
}

/**
 * Standard memory release
 */
void oal_mm_free(const void *vaddr)
{
	free((void *)vaddr);
}

void *oal_mm_virt_to_phys_contig(void *vaddr)
{
	/* no virtual, 1:1 map */
	return vaddr;
}

/**
 *	Try to find physical address associated with mapped virtual range
 */
void *oal_mm_virt_to_phys(void *vaddr)
{
	/* no virtual, 1:1 map */
	return vaddr;
}

/**
 * Get virtual address based on physical address. Function assumes that region containing the 'paddr'
 * is inside kernel address space.
 */
void *oal_mm_phys_to_virt(void *paddr)
{
	/* no virtual, 1:1 map */
	return paddr;
}

void *oal_mm_dev_map(void *paddr, const addr_t len)
{
	/* no remap, 1:1 map */
	return paddr;
}

void *oal_mm_dev_map_cache(void *paddr, const addr_t len)
{
	/* no remap, 1:1 map */
	return paddr;
}

errno_t oal_mm_dev_unmap(void *paddr, const addr_t len)
{
	return 0;
}

void oal_mm_cache_inval(const void *vad, const void *paddr, const addr_t len)
{
	unsigned long start = rounddown((unsigned long)vad, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)vad + (unsigned long)len,
				    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
	return;
}

void oal_mm_cache_flush(const void *vad, const void *paddr, const addr_t len)
{
	unsigned long start = rounddown((unsigned long)vad, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)vad + (unsigned long)len,
				    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
	return;
}

uint32_t oal_mm_cache_get_line_size(void)
{
	return OAL_CACHE_ALLIGN;
}

errno_t oal_mm_init(void *dev)
{
	return EOK;
}

void oal_mm_shutdown(void)
{
	return;
}

/** @}*/
