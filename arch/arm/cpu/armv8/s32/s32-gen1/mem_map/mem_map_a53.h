// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019 NXP
 */

/*****************************************************************
*
* A53_CLUSTER Registers
*
******************************************************************/

/* Instance A53_CLUSTER */

#define A53_CLUSTER_BASEADDRESS        0x4007C400          

/* Register definitions */

/* GPR00 Register */

#define A53_CLUSTER_GPR00              (A53_CLUSTER_BASEADDRESS+0x00000000) 

/* GPR01 Register */

#define A53_CLUSTER_GPR01              (A53_CLUSTER_BASEADDRESS+0x00000004) 

/* GPR06 Register */

#define A53_CLUSTER_GPR06              (A53_CLUSTER_BASEADDRESS+0x00000018) 

/* GPR07 Register */

#define A53_CLUSTER_GPR07              (A53_CLUSTER_BASEADDRESS+0x0000001C) 

/* GPR08 Register */

#define A53_CLUSTER_GPR08              (A53_CLUSTER_BASEADDRESS+0x00000020) 

/* GPR09 Register */

#define A53_CLUSTER_GPR09              (A53_CLUSTER_BASEADDRESS+0x00000024) 

/* GPR10 Register */

#define A53_CLUSTER_GPR10              (A53_CLUSTER_BASEADDRESS+0x00000028) 

/* GPR11 Register */

#define A53_CLUSTER_GPR11              (A53_CLUSTER_BASEADDRESS+0x0000002C) 

/* GPR12 Register */

#define A53_CLUSTER_GPR12              (A53_CLUSTER_BASEADDRESS+0x00000030) 

/* GPR13 Register */

#define A53_CLUSTER_GPR13              (A53_CLUSTER_BASEADDRESS+0x00000034) 

/* GPR14 register */

#define A53_CLUSTER_GPR14              (A53_CLUSTER_BASEADDRESS+0x00000038) 

/* GPR15 Register */

#define A53_CLUSTER_GPR15              (A53_CLUSTER_BASEADDRESS+0x0000003C) 

/* GPR16 Register */

#define A53_CLUSTER_GPR16              (A53_CLUSTER_BASEADDRESS+0x00000040) 

/* GPR17 Register */

#define A53_CLUSTER_GPR17              (A53_CLUSTER_BASEADDRESS+0x00000044) 

/* GPR18 Register */

#define A53_CLUSTER_GPR18              (A53_CLUSTER_BASEADDRESS+0x00000048) 

/* GPR19 Register */

#define A53_CLUSTER_GPR19              (A53_CLUSTER_BASEADDRESS+0x0000004C) 

/* GPR20 Register */

#define A53_CLUSTER_GPR20              (A53_CLUSTER_BASEADDRESS+0x00000050) 

/* GPR21 Register */

#define A53_CLUSTER_GPR21              (A53_CLUSTER_BASEADDRESS+0x00000054) 

/* Field definitions for GPR00 */

#define A53_CLUSTER_CA53_0_CORE0_AA64nAA32_VALUE(x) (((x)&0x00000001)<<0)  
#define A53_CLUSTER_CA53_0_CORE0_AA64nAA32_BIT (0)  
#define A53_CLUSTER_CA53_0_CORE0_AA64nAA32 ((1) << (A53_CLUSTER_CA53_0_CORE0_AA64nAA32_BIT)) 

#define A53_CLUSTER_CA53_0_CORE1_AA64nAA32_VALUE(x) (((x)&0x00000001)<<1)  
#define A53_CLUSTER_CA53_0_CORE1_AA64nAA32_BIT (1)  
#define A53_CLUSTER_CA53_0_CORE1_AA64nAA32 ((1) << (A53_CLUSTER_CA53_0_CORE1_AA64nAA32_BIT)) 

#define A53_CLUSTER_CA53_1_CORE0_AA64nAA32_VALUE(x) (((x)&0x00000001)<<2)  
#define A53_CLUSTER_CA53_1_CORE0_AA64nAA32_BIT (2)  
#define A53_CLUSTER_CA53_1_CORE0_AA64nAA32 ((1) << (A53_CLUSTER_CA53_1_CORE0_AA64nAA32_BIT)) 

#define A53_CLUSTER_CA53_1_CORE1_AA64nAA32_VALUE(x) (((x)&0x00000001)<<3)  
#define A53_CLUSTER_CA53_1_CORE1_AA64nAA32_BIT (3)  
#define A53_CLUSTER_CA53_1_CORE1_AA64nAA32 ((1) << (A53_CLUSTER_CA53_1_CORE1_AA64nAA32_BIT)) 

#define A53_CLUSTER_CA53_0_BROADCASTCACHEMAINT_VALUE(x) (((x)&0x00000001)<<4)  
#define A53_CLUSTER_CA53_0_BROADCASTCACHEMAINT_BIT (4)  
#define A53_CLUSTER_CA53_0_BROADCASTCACHEMAINT ((1) << (A53_CLUSTER_CA53_0_BROADCASTCACHEMAINT_BIT)) 

#define A53_CLUSTER_CA53_0_BROADCASTINNER_VALUE(x) (((x)&0x00000001)<<5)  
#define A53_CLUSTER_CA53_0_BROADCASTINNER_BIT (5)  
#define A53_CLUSTER_CA53_0_BROADCASTINNER ((1) << (A53_CLUSTER_CA53_0_BROADCASTINNER_BIT)) 

#define A53_CLUSTER_CA53_0_BROADCASTOUTER_VALUE(x) (((x)&0x00000001)<<6)  
#define A53_CLUSTER_CA53_0_BROADCASTOUTER_BIT (6)  
#define A53_CLUSTER_CA53_0_BROADCASTOUTER ((1) << (A53_CLUSTER_CA53_0_BROADCASTOUTER_BIT)) 

#define A53_CLUSTER_CA53_1_BROADCASTCACHEMAINT_VALUE(x) (((x)&0x00000001)<<7)  
#define A53_CLUSTER_CA53_1_BROADCASTCACHEMAINT_BIT (7)  
#define A53_CLUSTER_CA53_1_BROADCASTCACHEMAINT ((1) << (A53_CLUSTER_CA53_1_BROADCASTCACHEMAINT_BIT)) 

#define A53_CLUSTER_CA53_1_BROADCASTINNER_VALUE(x) (((x)&0x00000001)<<8)  
#define A53_CLUSTER_CA53_1_BROADCASTINNER_BIT (8)  
#define A53_CLUSTER_CA53_1_BROADCASTINNER ((1) << (A53_CLUSTER_CA53_1_BROADCASTINNER_BIT)) 

#define A53_CLUSTER_CA53_1_BROADCASTOUTER_VALUE(x) (((x)&0x00000001)<<9)  
#define A53_CLUSTER_CA53_1_BROADCASTOUTER_BIT (9)  
#define A53_CLUSTER_CA53_1_BROADCASTOUTER ((1) << (A53_CLUSTER_CA53_1_BROADCASTOUTER_BIT)) 

#define A53_CLUSTER_CA53_0_CORE0_CFGEND_VALUE(x) (((x)&0x00000001)<<10)  
#define A53_CLUSTER_CA53_0_CORE0_CFGEND_BIT (10)  
#define A53_CLUSTER_CA53_0_CORE0_CFGEND ((1) << (A53_CLUSTER_CA53_0_CORE0_CFGEND_BIT)) 

#define A53_CLUSTER_CA53_0_CORE1_CFGEND_VALUE(x) (((x)&0x00000001)<<11)  
#define A53_CLUSTER_CA53_0_CORE1_CFGEND_BIT (11)  
#define A53_CLUSTER_CA53_0_CORE1_CFGEND ((1) << (A53_CLUSTER_CA53_0_CORE1_CFGEND_BIT)) 

#define A53_CLUSTER_CA53_0_CORE0_CFGTE_VALUE(x) (((x)&0x00000001)<<12)  
#define A53_CLUSTER_CA53_0_CORE0_CFGTE_BIT (12)  
#define A53_CLUSTER_CA53_0_CORE0_CFGTE ((1) << (A53_CLUSTER_CA53_0_CORE0_CFGTE_BIT)) 

#define A53_CLUSTER_CA53_0_CORE1_CFGTE_VALUE(x) (((x)&0x00000001)<<13)  
#define A53_CLUSTER_CA53_0_CORE1_CFGTE_BIT (13)  
#define A53_CLUSTER_CA53_0_CORE1_CFGTE ((1) << (A53_CLUSTER_CA53_0_CORE1_CFGTE_BIT)) 

#define A53_CLUSTER_CA53_1_CORE0_CFGEND_VALUE(x) (((x)&0x00000001)<<14)  
#define A53_CLUSTER_CA53_1_CORE0_CFGEND_BIT (14)  
#define A53_CLUSTER_CA53_1_CORE0_CFGEND ((1) << (A53_CLUSTER_CA53_1_CORE0_CFGEND_BIT)) 

#define A53_CLUSTER_CA53_1_CORE1_CFGEND_VALUE(x) (((x)&0x00000001)<<15)  
#define A53_CLUSTER_CA53_1_CORE1_CFGEND_BIT (15)  
#define A53_CLUSTER_CA53_1_CORE1_CFGEND ((1) << (A53_CLUSTER_CA53_1_CORE1_CFGEND_BIT)) 

#define A53_CLUSTER_CA53_1_CORE0_CFGTE_VALUE(x) (((x)&0x00000001)<<16)  
#define A53_CLUSTER_CA53_1_CORE0_CFGTE_BIT (16)  
#define A53_CLUSTER_CA53_1_CORE0_CFGTE ((1) << (A53_CLUSTER_CA53_1_CORE0_CFGTE_BIT)) 

#define A53_CLUSTER_CA53_1_CORE1_CFGTE_VALUE(x) (((x)&0x00000001)<<17)  
#define A53_CLUSTER_CA53_1_CORE1_CFGTE_BIT (17)  
#define A53_CLUSTER_CA53_1_CORE1_CFGTE ((1) << (A53_CLUSTER_CA53_1_CORE1_CFGTE_BIT)) 

#define A53_CLUSTER_CA53_0_CORE0_VINITHI_VALUE(x) (((x)&0x00000001)<<18)  
#define A53_CLUSTER_CA53_0_CORE0_VINITHI_BIT (18)  
#define A53_CLUSTER_CA53_0_CORE0_VINITHI ((1) << (A53_CLUSTER_CA53_0_CORE0_VINITHI_BIT)) 

#define A53_CLUSTER_CA53_0_CORE1_VINITHI_VALUE(x) (((x)&0x00000001)<<19)  
#define A53_CLUSTER_CA53_0_CORE1_VINITHI_BIT (19)  
#define A53_CLUSTER_CA53_0_CORE1_VINITHI ((1) << (A53_CLUSTER_CA53_0_CORE1_VINITHI_BIT)) 

#define A53_CLUSTER_CA53_1_CORE0_VINITHI_VALUE(x) (((x)&0x00000001)<<20)  
#define A53_CLUSTER_CA53_1_CORE0_VINITHI_BIT (20)  
#define A53_CLUSTER_CA53_1_CORE0_VINITHI ((1) << (A53_CLUSTER_CA53_1_CORE0_VINITHI_BIT)) 

#define A53_CLUSTER_CA53_1_CORE1_VINITHI_VALUE(x) (((x)&0x00000001)<<21)  
#define A53_CLUSTER_CA53_1_CORE1_VINITHI_BIT (21)  
#define A53_CLUSTER_CA53_1_CORE1_VINITHI ((1) << (A53_CLUSTER_CA53_1_CORE1_VINITHI_BIT)) 

#define CA53_GPR00_CLK_DIV_VAL_SHIFT	(24)
#define CA53_GPR00_CLK_DIV_VAL_MASK	(0x7)

/* Field definitions for GPR01 */

#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER0_VALUE(x) (((x)&0x00000001)<<0)  
#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER0_BIT (0)  
#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER0 ((1) << (A53_CLUSTER_WFE_EVT_CA53_CLUSTER0_BIT)) 

#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER1_VALUE(x) (((x)&0x00000001)<<1)  
#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER1_BIT (1)  
#define A53_CLUSTER_WFE_EVT_CA53_CLUSTER1 ((1) << (A53_CLUSTER_WFE_EVT_CA53_CLUSTER1_BIT)) 

#define A53_CLUSTER_CLUSTER0_CG_EN_VALUE(x) (((x)&0x00000001)<<8)  
#define A53_CLUSTER_CLUSTER0_CG_EN_BIT (8)  
#define A53_CLUSTER_CLUSTER0_CG_EN     ((1) << (A53_CLUSTER_CLUSTER0_CG_EN_BIT)) 

#define A53_CLUSTER_CLUSTER1_CG_EN_VALUE(x) (((x)&0x00000001)<<9)  
#define A53_CLUSTER_CLUSTER1_CG_EN_BIT (9)  
#define A53_CLUSTER_CLUSTER1_CG_EN     ((1) << (A53_CLUSTER_CLUSTER1_CG_EN_BIT)) 

#define A53_CLUSTER_CA53_0_CORE0_CP15SDISABLE_VALUE(x) (((x)&0x00000001)<<24)  
#define A53_CLUSTER_CA53_0_CORE0_CP15SDISABLE_BIT (24)  
#define A53_CLUSTER_CA53_0_CORE0_CP15SDISABLE ((1) << (A53_CLUSTER_CA53_0_CORE0_CP15SDISABLE_BIT)) 

#define A53_CLUSTER_CA53_0_CORE1_CP15SDISABLE_VALUE(x) (((x)&0x00000001)<<25)  
#define A53_CLUSTER_CA53_0_CORE1_CP15SDISABLE_BIT (25)  
#define A53_CLUSTER_CA53_0_CORE1_CP15SDISABLE ((1) << (A53_CLUSTER_CA53_0_CORE1_CP15SDISABLE_BIT)) 

#define A53_CLUSTER_CA53_1_CORE0_CP15SDISABLE_VALUE(x) (((x)&0x00000001)<<26)  
#define A53_CLUSTER_CA53_1_CORE0_CP15SDISABLE_BIT (26)  
#define A53_CLUSTER_CA53_1_CORE0_CP15SDISABLE ((1) << (A53_CLUSTER_CA53_1_CORE0_CP15SDISABLE_BIT)) 

#define A53_CLUSTER_CA53_1_CORE1_CP15SDISABLE_VALUE(x) (((x)&0x00000001)<<27)  
#define A53_CLUSTER_CA53_1_CORE1_CP15SDISABLE_BIT (27)  
#define A53_CLUSTER_CA53_1_CORE1_CP15SDISABLE ((1) << (A53_CLUSTER_CA53_1_CORE1_CP15SDISABLE_BIT)) 

/* Field definitions for GPR06 */

#define A53_CLUSTER_CA53_LOCKSTEP_EN_VALUE(x) (((x)&0x00000001)<<0)  
#define A53_CLUSTER_CA53_LOCKSTEP_EN_BIT (0)  
#define A53_CLUSTER_CA53_LOCKSTEP_EN   ((1) << (A53_CLUSTER_CA53_LOCKSTEP_EN_BIT)) 

#define A53_CLUSTER_GIC500_LOCKSTEP_EN_VALUE(x) (((x)&0x00000003)<<1)  
#define A53_CLUSTER_GIC500_LOCKSTEP_EN_MSB (2)
#define A53_CLUSTER_GIC500_LOCKSTEP_EN_LSB (1)
#define A53_CLUSTER_GIC500_LOCKSTEP_EN_MASK (0x00000003) 
#define A53_CLUSTER_GIC500_LOCKSTEP_EN ((A53_CLUSTER_GIC500_LOCKSTEP_EN_MASK) << (A53_CLUSTER_GIC500_LOCKSTEP_EN_LSB)) 

/* Field definitions for GPR07 */

#define A53_CLUSTER_CA53_0_CLUSTERIDAFF1_VALUE(x) (((x)&0x000000FF)<<0)  
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF1_MSB (7)
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF1_LSB (0)
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF1_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF1 ((A53_CLUSTER_CA53_0_CLUSTERIDAFF1_MASK) << (A53_CLUSTER_CA53_0_CLUSTERIDAFF1_LSB)) 

#define A53_CLUSTER_CA53_0_CLUSTERIDAFF2_VALUE(x) (((x)&0x000000FF)<<8)  
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF2_MSB (15)
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF2_LSB (8)
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF2_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_0_CLUSTERIDAFF2 ((A53_CLUSTER_CA53_0_CLUSTERIDAFF2_MASK) << (A53_CLUSTER_CA53_0_CLUSTERIDAFF2_LSB)) 

#define A53_CLUSTER_CA53_1_CLUSTERIDAFF1_VALUE(x) (((x)&0x000000FF)<<16)  
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF1_MSB (23)
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF1_LSB (16)
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF1_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF1 ((A53_CLUSTER_CA53_1_CLUSTERIDAFF1_MASK) << (A53_CLUSTER_CA53_1_CLUSTERIDAFF1_LSB)) 

#define A53_CLUSTER_CA53_1_CLUSTERIDAFF2_VALUE(x) (((x)&0x000000FF)<<24)  
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF2_MSB (31)
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF2_LSB (24)
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF2_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_1_CLUSTERIDAFF2 ((A53_CLUSTER_CA53_1_CLUSTERIDAFF2_MASK) << (A53_CLUSTER_CA53_1_CLUSTERIDAFF2_LSB)) 

/* Field definitions for GPR08 */

#define A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_VALUE(x) (((x)&0x0000000F)<<0)  
#define A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_MSB (3)
#define A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_LSB (0)
#define A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_MASK (0x0000000F) 
#define A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS ((A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_MASK) << (A53_CLUSTER_CA53_CORE_WARM_RESET_STATUS_LSB)) 

#define A53_CLUSTER_CLUSTER0_STANDBYWFIL2_STATUS_VALUE(x) (((x)&0x00000001)<<8)  
#define A53_CLUSTER_CLUSTER0_STANDBYWFIL2_STATUS_BIT (8)  
#define A53_CLUSTER_CLUSTER0_STANDBYWFIL2_STATUS ((1) << (A53_CLUSTER_CLUSTER0_STANDBYWFIL2_STATUS_BIT)) 

#define A53_CLUSTER_CLUSTER1_STANDBYWFIL2_STATUS_VALUE(x) (((x)&0x00000001)<<9)  
#define A53_CLUSTER_CLUSTER1_STANDBYWFIL2_STATUS_BIT (9)  
#define A53_CLUSTER_CLUSTER1_STANDBYWFIL2_STATUS ((1) << (A53_CLUSTER_CLUSTER1_STANDBYWFIL2_STATUS_BIT)) 

#define A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_VALUE(x) (((x)&0x0000000F)<<16)  
#define A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_MSB (19)
#define A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_LSB (16)
#define A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_MASK (0x0000000F) 
#define A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS ((A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_MASK) << (A53_CLUSTER_CA53_CORE_STANDBYWFE_STATUS_LSB)) 

#define A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_VALUE(x) (((x)&0x0000000F)<<24)  
#define A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_MSB (27)
#define A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_LSB (24)
#define A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_MASK (0x0000000F) 
#define A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS ((A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_MASK) << (A53_CLUSTER_CA53_CORE_STANDBYWFI_STATUS_LSB)) 

/* Field definitions for GPR09 */

#define A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_VALUE(x) (((x)&0x000000FF)<<0)  
#define A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_MSB (7)
#define A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_LSB (0)
#define A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32 ((A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_MASK) << (A53_CLUSTER_CA53_0_CORE0_RVBARADDR_39_32_LSB)) 

#define A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_VALUE(x) (((x)&0x000000FF)<<8)  
#define A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_MSB (15)
#define A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_LSB (8)
#define A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32 ((A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_MASK) << (A53_CLUSTER_CA53_0_CORE1_RVBARADDR_39_32_LSB)) 

#define A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_VALUE(x) (((x)&0x000000FF)<<16)  
#define A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_MSB (23)
#define A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_LSB (16)
#define A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32 ((A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_MASK) << (A53_CLUSTER_CA53_1_CORE0_RVBARADDR_39_32_LSB)) 

#define A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_VALUE(x) (((x)&0x000000FF)<<24)  
#define A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_MSB (31)
#define A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_LSB (24)
#define A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_MASK (0x000000FF) 
#define A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32 ((A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_MASK) << (A53_CLUSTER_CA53_1_CORE1_RVBARADDR_39_32_LSB)) 

/* Field definitions for GPR10 */

#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS ((A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR11 */

#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS ((A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_0_CORE0_CPUMERRSR_LOW_BITS_LSB)) 

/* Field definitions for GPR12 */

#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS ((A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR13 */

#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS ((A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_0_CORE1_CPUMERRSR_LOW_BITS_LSB)) 

/* Field definitions for GPR14 */

#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS ((A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR15 */

#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS ((A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_1_CORE0_CPUMERRSR_LOW_BITS_LSB)) 

/* Field definitions for GPR16 */

#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS ((A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR17 */

#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS ((A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_1_CORE1_CPUMERRSR_LOW_BITS_LSB)) 

/* Field definitions for GPR18 */

#define A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS ((A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_0_L2MERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR19 */

#define A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS ((A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_0_L2MERRSR_LOW_BITS_LSB)) 

/* Field definitions for GPR20 */

#define A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS ((A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_MASK) << (A53_CLUSTER_CA53_1_L2MERRSR_HIGH_BITS_LSB)) 

/* Field definitions for GPR21 */

#define A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_VALUE(x) (((x)&0xFFFFFFFF)<<0)  
#define A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_MSB (31)
#define A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_LSB (0)
#define A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_MASK (0xFFFFFFFF) 
#define A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS ((A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_MASK) << (A53_CLUSTER_CA53_1_L2MERRSR_LOW_BITS_LSB)) 
