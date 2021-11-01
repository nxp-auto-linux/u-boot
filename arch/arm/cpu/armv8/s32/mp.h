/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2014, Freescale Semiconductor
 * Copyright 2017-2018, 2020-2021 NXP
 */

#ifndef _S32_MP_H
#define _S32_MP_H

#ifdef __ASSEMBLY__
#define PGTABLE_SIZE		(4096 * 4)
#else
extern u64 __spin_table[];
extern u64 __real_cntfrq;
extern u64 *secondary_boot_page;
extern size_t __secondary_boot_page_size;
int fsl_s32_wake_secondary_cores(void);
void *get_spin_tbl_addr(void);
phys_addr_t determine_mp_bootpg(void);
void secondary_boot_func(void);
u32 cpu_pos_mask(void);
u64 fdt_to_cpu_id(u64 fdt_id);
#endif

#endif /* _S32_MP_H */
