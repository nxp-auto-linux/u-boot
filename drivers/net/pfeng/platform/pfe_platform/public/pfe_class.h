/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup    dxgr_PFE_CLASS CLASS
 * @brief		The Classifier
 * @details     This is the software representation of the classifier block.
 *
 * @addtogroup  dxgr_PFE_CLASS
 * @{
 *
 * @file		pfe_class.h
 * @brief		The CLASS module header file.
 * @details		This file contains CLASS-related API.
 *
 */

#ifndef PFE_CLASS_H_
#define PFE_CLASS_H_

#include "pfe_ct.h" /* fw/host shared types */

typedef struct __pfe_classifier_tag pfe_class_t;

typedef struct {
	bool_t resume;		   /*	Resume flag */
	bool_t toe_mode;	   /*	TCP offload mode */
	u32 pe_sys_clk_ratio; /*	Clock mode ratio for sys_clk and pe_clk */
	u32 pkt_parse_offset; /*	Offset which says from which point packet needs to be parsed */
	void *route_table_base_pa; /*	Route table physical address */
	void *route_table_base_va; /*	Route table virtual address */
	u32 route_entry_size; /*	Route entry size */
	u32 route_hash_size;  /*	Route hash size (bits) */
	void *ddr_base_va;	   /*	DDR region base address (virtual) */
	void *ddr_base_pa;	   /*	DDR region base address (physical) */
	u32 ddr_size;	   /*	Size of the DDR region */
} pfe_class_cfg_t;

pfe_class_t *pfe_class_create(void *cbus_base_va, uint32_t pe_num,
			      pfe_class_cfg_t *cfg);
errno_t pfe_class_isr(pfe_class_t *class);
void pfe_class_irq_mask(pfe_class_t *class);
void pfe_class_irq_unmask(pfe_class_t *class);
void pfe_class_enable(pfe_class_t *class);
void pfe_class_reset(pfe_class_t *class);
void pfe_class_disable(pfe_class_t *class);
errno_t pfe_class_load_firmware(pfe_class_t *class, const void *elf);
errno_t pfe_class_get_mmap(pfe_class_t *class, int32_t pe_idx,
			   pfe_ct_pe_mmap_t *mmap);
errno_t pfe_class_write_dmem(pfe_class_t *class, int32_t pe_idx, void *dst,
			     void *src, uint32_t len);
errno_t pfe_class_read_dmem(pfe_class_t *class, uint32_t pe_idx, void *dst,
			    void *src, uint32_t len);
errno_t pfe_class_read_pmem(pfe_class_t *class, uint32_t pe_idx, void *dst,
			    void *src, uint32_t len);
errno_t pfe_class_set_rtable(pfe_class_t *class, void *rtable_pa,
			     u32 rtable_len, uint32_t entry_size);
errno_t pfe_class_set_default_vlan(pfe_class_t *class, uint16_t vlan);
uint32_t pfe_class_get_num_of_pes(pfe_class_t *class);
uint32_t pfe_class_get_text_statistics(pfe_class_t *class, char_t *buf,
				       u32 buf_len, uint8_t verb_level);
void pfe_class_destroy(pfe_class_t *class);
addr_t pfe_class_dmem_heap_alloc(pfe_class_t *class, uint32_t size);
void pfe_class_dmem_heap_free(pfe_class_t *class, addr_t addr);
errno_t pfe_class_set_flexible_filter(pfe_class_t *class,
				      const uint32_t dmem_addr);

#endif /* PFE_CLASS_H_ */

/** @}*/
/** @}*/
