/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "mmu.h"
#include "comp.h"

extern void enable_DDRWorkaround(void);

unsigned int L1PageTable[4096] __attribute__ ((aligned(0x4000))); ;

void init_mmu(void)
{
	int k;
	volatile unsigned long DACR  ;/*System Control Register */
	for (k=0; k<4096 ; k++) /* make 4096 sections @ 1MB*/
	{
		L1PageTable[k] =
			(( k  << MMU_SECTION_SECTION_BASE_ADDR_SHIFT  ) & MMU_SECTION_SECTION_BASE_ADDR_MASK  ) +
			(( 0  << MMU_SECTION_TABLE_ENTRY_TYPE_A_SHIFT ) & MMU_SECTION_TABLE_ENTRY_TYPE_A_MASK ) +
			/* Section entry */
			(( 0  << MMU_SECTION_NG_SHIFT                 ) & MMU_SECTION_NG_MASK                 ) +
			/* Page description is Global */
			(( 0  <<  MMU_SECTION_S_SHIFT                 ) &  MMU_SECTION_S_MASK                 ) +
			/*Page is Not-shared */
			(( 0  <<  MMU_SECTION_APX_SHIFT               ) &  MMU_SECTION_APX_MASK               ) +
			(( 3  <<  MMU_SECTION_AP_SHIFT                ) &  MMU_SECTION_AP_MASK                ) +
			/* Access Permissions are Full Access */
			(( 0  <<  MMU_SECTION_TEX_SHIFT               ) & MMU_SECTION_TEX_MASK                ) +
			/* Memory Attribute is Strongly Ordered */
			(( 15 <<  MMU_SECTION_DOMAIN_SHIFT            ) & MMU_SECTION_DOMAIN_MASK             ) +
			/* Page Domain is 15 */
			(( 0  <<  MMU_SECTION_XN_SHIFT                ) &  MMU_SECTION_XN_MASK                ) +
			/* Execute Never disabled - this would prevent erroneous execution of data if set */
			(( 0  <<  MMU_SECTION_C_SHIFT                 ) &   MMU_SECTION_C_MASK                ) +
			/* not cached */
			(( 0  <<  MMU_SECTION_B_SHIFT                 ) &   MMU_SECTION_B_MASK                ) +
			/* not buffered */
			(( 2  <<  MMU_SECTION_ENTRY_TYPE_SHIFT        ) &  MMU_SECTION_ENTRY_TYPE_MASK        );
			/* Section or SuperSection entry */
  }

	/* write TTBR0 to be L1PageTable[0]*/
	MCR(15, 0, (unsigned long) L1PageTable, 2, 0, 0);

	DACR = 0x55555555; /* set Client mode for all Domains */

	/* write modified DACR*/
	MCR(15, 0, DACR, 3, 0, 0);

}

 void enable_mmu(void){

	volatile unsigned long SCTLR  ;/*System Control Register */

	/* read SCTLR */
	MRC(15, 0, SCTLR, 1, 0, 0);

	SCTLR = SCTLR | 0x1; /* set MMU enable bit */

	/* write modified SCTLR*/
	MCR(15, 0, SCTLR, 1, 0, 0);

}
 void disable_mmu(void)
 {

	volatile unsigned long SCTLR  ;/*System Control Register */

	/* read SCTLR */
	MRC(15, 0, SCTLR, 1, 0, 0);
	SCTLR &=~ 0x1; /* clear MMU enable bit */
	/* write modified SCTLR*/
	MCR(15, 0, SCTLR, 1, 0, 0);
}

void invalidate_all_tlb(void)
{
	/*   MCR p15, 0, <Rd>, c8, c7, 0 -- Invalidate entire Unified TLB : TLBIALL*/
	MCR(15, 0, 0, 8, 7, 0);
}
void enable_DDRWorkaround(void)
{
	init_mmu();

	/* enable cache, but set DRAM as non- cacheable */
	L1PageTable[2048] = 0x80001DE2;
	L1PageTable[2049] = 0x80101DE2;
	L1PageTable[2050] = 0x80201DE2;
	L1PageTable[2051] = 0x80301DE2;
	L1PageTable[2052] = 0x80401DE2;
	L1PageTable[2053] = 0x80501DE2;
	L1PageTable[2054] = 0x80601DE2;
	L1PageTable[2055] = 0x80701DE2;
	L1PageTable[2056] = 0x80801DE2;
	L1PageTable[2057] = 0x80901DE2;
	L1PageTable[2058] = 0x80a01DE2;
	L1PageTable[2059] = 0x80b01DE2;
	L1PageTable[2060] = 0x80c01DE2;
	L1PageTable[2061] = 0x80d01DE2;
	L1PageTable[2062] = 0x80e01DE2;
	L1PageTable[2063] = 0x80f01DE2;

	enable_mmu();
}
