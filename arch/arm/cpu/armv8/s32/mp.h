/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2014, Freescale Semiconductor
 * Copyright 2017-2018, 2020 NXP
 */

#ifndef _S32_MP_H
#define _S32_MP_H

#define id_to_core(x)	((x & 3) | (x >> 8))

#if defined (CONFIG_S32_GEN1)
#define S32_A53_GPR_BASE_ADDR	0x4007c400ul
#define S32_A53_GP06_OFF	0x18
#endif

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
#endif

#endif /* _S32_MP_H */
