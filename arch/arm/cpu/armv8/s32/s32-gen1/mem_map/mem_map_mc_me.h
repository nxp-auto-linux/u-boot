// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019 NXP
 */

/*****************************************************************
*
* MC_ME Registers
*
******************************************************************/

/* Instance MC_ME */

#define MC_ME_BASEADDRESS              0x40088000          

/* Register definitions */

/* Control Key Register */

#undef MC_ME_CTL_KEY
#define MC_ME_CTL_KEY                  (MC_ME_BASEADDRESS+0x00000000) 
#define MC_ME_X_CTL_KEY(x)             ((MC_ME_BASEADDRESS + 0x00000000 + ((x)*0x4000)))

/* Mode Configuration Register */

#undef MC_ME_MODE_CONF
#define MC_ME_MODE_CONF                (MC_ME_BASEADDRESS+0x00000004) 
#define MC_ME_X_MODE_CONF(x)           ((MC_ME_BASEADDRESS + 0x00000004 + ((x)*0x4000)))

/* Mode Update Register */

#undef MC_ME_MODE_UPD
#define MC_ME_MODE_UPD                 (MC_ME_BASEADDRESS+0x00000008) 
#define MC_ME_X_MODE_UPD(x)            ((MC_ME_BASEADDRESS + 0x00000008 + ((x)*0x4000)))

/* Mode Status Register */

#undef MC_ME_MODE_STAT
#define MC_ME_MODE_STAT                (MC_ME_BASEADDRESS+0x0000000C) 
#define MC_ME_X_MODE_STAT(x)           ((MC_ME_BASEADDRESS + 0x0000000C + ((x)*0x4000)))

/* Main Core ID Register */

#define MC_ME_MAIN_COREID              (MC_ME_BASEADDRESS+0x00000010) 
#define MC_ME_X_MAIN_COREID(x)         ((MC_ME_BASEADDRESS + 0x00000010 + ((x)*0x4000)))

/* Partition 0 Process Configuration Register */

#define MC_ME_PRTN0_PCONF              (MC_ME_BASEADDRESS+0x00000100) 
#define MC_ME_X_PRTN0_PCONF(x)         ((MC_ME_BASEADDRESS + 0x00000100 + ((x)*0x4000)))

/* Partition 0 Process Update Register */

#define MC_ME_PRTN0_PUPD               (MC_ME_BASEADDRESS+0x00000104) 
#define MC_ME_X_PRTN0_PUPD(x)          ((MC_ME_BASEADDRESS + 0x00000104 + ((x)*0x4000)))

/* Partition 0 Status Register */

#define MC_ME_PRTN0_STAT               (MC_ME_BASEADDRESS+0x00000108) 
#define MC_ME_X_PRTN0_STAT(x)          ((MC_ME_BASEADDRESS + 0x00000108 + ((x)*0x4000)))

/* Partition 0 COFB Set 0 Clock Status Register */

#define MC_ME_PRTN0_COFB0_STAT         (MC_ME_BASEADDRESS+0x00000110) 
#define MC_ME_X_PRTN0_COFB0_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000110 + ((x)*0x4000)))

/* Partition 0 COFB Set 0 Clock Enable Register */

#define MC_ME_PRTN0_COFB0_CLKEN        (MC_ME_BASEADDRESS+0x00000130) 
#define MC_ME_X_PRTN0_COFB0_CLKEN(x)   ((MC_ME_BASEADDRESS + 0x00000130 + ((x)*0x4000)))

/* Partition 0 Core 0 Process Configuration Register */

#define MC_ME_PRTN0_CORE0_PCONF        (MC_ME_BASEADDRESS+0x00000140) 
#define MC_ME_X_PRTN0_CORE0_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000140 + ((x)*0x4000)))

/* Partition 0 Core 0 Process Update Register */

#define MC_ME_PRTN0_CORE0_PUPD         (MC_ME_BASEADDRESS+0x00000144) 
#define MC_ME_X_PRTN0_CORE0_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000144 + ((x)*0x4000)))

/* Partition 0 Core 0 Status Register */

#define MC_ME_PRTN0_CORE0_STAT         (MC_ME_BASEADDRESS+0x00000148) 
#define MC_ME_X_PRTN0_CORE0_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000148 + ((x)*0x4000)))

/* Partition 0 Core 0 Address Register */

#define MC_ME_PRTN0_CORE0_ADDR         (MC_ME_BASEADDRESS+0x0000014C) 
#define MC_ME_X_PRTN0_CORE0_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000014C + ((x)*0x4000)))

/* Partition 0 Core 1 Process Configuration Register */

#define MC_ME_PRTN0_CORE1_PCONF        (MC_ME_BASEADDRESS+0x00000160) 
#define MC_ME_X_PRTN0_CORE1_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000160 + ((x)*0x4000)))

/* Partition 0 Core 1 Process Update Register */

#define MC_ME_PRTN0_CORE1_PUPD         (MC_ME_BASEADDRESS+0x00000164) 
#define MC_ME_X_PRTN0_CORE1_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000164 + ((x)*0x4000)))

/* Partition 0 Core 1 Status Register */

#define MC_ME_PRTN0_CORE1_STAT         (MC_ME_BASEADDRESS+0x00000168) 
#define MC_ME_X_PRTN0_CORE1_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000168 + ((x)*0x4000)))

/* Partition 0 Core 1 Address Register */

#define MC_ME_PRTN0_CORE1_ADDR         (MC_ME_BASEADDRESS+0x0000016C) 
#define MC_ME_X_PRTN0_CORE1_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000016C + ((x)*0x4000)))

/* Partition 0 Core 2 Process Configuration Register */

#define MC_ME_PRTN0_CORE2_PCONF        (MC_ME_BASEADDRESS+0x00000180) 
#define MC_ME_X_PRTN0_CORE2_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000180 + ((x)*0x4000)))

/* Partition 0 Core 2 Process Update Register */

#define MC_ME_PRTN0_CORE2_PUPD         (MC_ME_BASEADDRESS+0x00000184) 
#define MC_ME_X_PRTN0_CORE2_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000184 + ((x)*0x4000)))

/* Partition 0 Core 2 Status Register */

#define MC_ME_PRTN0_CORE2_STAT         (MC_ME_BASEADDRESS+0x00000188) 
#define MC_ME_X_PRTN0_CORE2_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000188 + ((x)*0x4000)))

/* Partition 0 Core 2 Address Register */

#define MC_ME_PRTN0_CORE2_ADDR         (MC_ME_BASEADDRESS+0x0000018C) 
#define MC_ME_X_PRTN0_CORE2_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000018C + ((x)*0x4000)))

/* Partition 0 Core 3 Process Configuration Register */

#define MC_ME_PRTN0_CORE3_PCONF        (MC_ME_BASEADDRESS+0x000001A0) 
#define MC_ME_X_PRTN0_CORE3_PCONF(x)   ((MC_ME_BASEADDRESS + 0x000001A0 + ((x)*0x4000)))

/* Partition 0 Core 3 Process Update Register */

#define MC_ME_PRTN0_CORE3_PUPD         (MC_ME_BASEADDRESS+0x000001A4) 
#define MC_ME_X_PRTN0_CORE3_PUPD(x)    ((MC_ME_BASEADDRESS + 0x000001A4 + ((x)*0x4000)))

/* Partition 0 Core 3 Status Register */

#define MC_ME_PRTN0_CORE3_STAT         (MC_ME_BASEADDRESS+0x000001A8) 
#define MC_ME_X_PRTN0_CORE3_STAT(x)    ((MC_ME_BASEADDRESS + 0x000001A8 + ((x)*0x4000)))

/* Partition 0 Core 3 Address Register */

#define MC_ME_PRTN0_CORE3_ADDR         (MC_ME_BASEADDRESS+0x000001AC) 
#define MC_ME_X_PRTN0_CORE3_ADDR(x)    ((MC_ME_BASEADDRESS + 0x000001AC + ((x)*0x4000)))

/* Partition 1 Process Configuration Register */

#define MC_ME_PRTN1_PCONF              (MC_ME_BASEADDRESS+0x00000300) 
#define MC_ME_X_PRTN1_PCONF(x)         ((MC_ME_BASEADDRESS + 0x00000300 + ((x)*0x4000)))

/* Partition 1 Process Update Register */

#define MC_ME_PRTN1_PUPD               (MC_ME_BASEADDRESS+0x00000304) 
#define MC_ME_X_PRTN1_PUPD(x)          ((MC_ME_BASEADDRESS + 0x00000304 + ((x)*0x4000)))

/* Partition 1 Status Register */

#define MC_ME_PRTN1_STAT               (MC_ME_BASEADDRESS+0x00000308) 
#define MC_ME_X_PRTN1_STAT(x)          ((MC_ME_BASEADDRESS + 0x00000308 + ((x)*0x4000)))

/* Partition 1 Core 0 Process Configuration Register */

#define MC_ME_PRTN1_CORE0_PCONF        (MC_ME_BASEADDRESS+0x00000340) 
#define MC_ME_X_PRTN1_CORE0_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000340 + ((x)*0x4000)))

/* Partition 1 Core 0 Process Update Register */

#define MC_ME_PRTN1_CORE0_PUPD         (MC_ME_BASEADDRESS+0x00000344) 
#define MC_ME_X_PRTN1_CORE0_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000344 + ((x)*0x4000)))

/* Partition 1 Core 0 Status Register */

#define MC_ME_PRTN1_CORE0_STAT         (MC_ME_BASEADDRESS+0x00000348) 
#define MC_ME_X_PRTN1_CORE0_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000348 + ((x)*0x4000)))

/* Partition 1 Core 0 Address Register */

#define MC_ME_PRTN1_CORE0_ADDR         (MC_ME_BASEADDRESS+0x0000034C) 
#define MC_ME_X_PRTN1_CORE0_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000034C + ((x)*0x4000)))

/* Partition 1 Core 1 Process Configuration Register */

#define MC_ME_PRTN1_CORE1_PCONF        (MC_ME_BASEADDRESS+0x00000360) 
#define MC_ME_X_PRTN1_CORE1_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000360 + ((x)*0x4000)))

/* Partition 1 Core 1 Process Update Register */

#define MC_ME_PRTN1_CORE1_PUPD         (MC_ME_BASEADDRESS+0x00000364) 
#define MC_ME_X_PRTN1_CORE1_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000364 + ((x)*0x4000)))

/* Partition 1 Core 1 Status Register */

#define MC_ME_PRTN1_CORE1_STAT         (MC_ME_BASEADDRESS+0x00000368) 
#define MC_ME_X_PRTN1_CORE1_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000368 + ((x)*0x4000)))

/* Partition 1 Core 1 Address Register */

#define MC_ME_PRTN1_CORE1_ADDR         (MC_ME_BASEADDRESS+0x0000036C) 
#define MC_ME_X_PRTN1_CORE1_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000036C + ((x)*0x4000)))

/* Partition 1 Core 2 Process Configuration Register */

#define MC_ME_PRTN1_CORE2_PCONF        (MC_ME_BASEADDRESS+0x00000380) 
#define MC_ME_X_PRTN1_CORE2_PCONF(x)   ((MC_ME_BASEADDRESS + 0x00000380 + ((x)*0x4000)))

/* Partition 1 Core 2 Process Update Register */

#define MC_ME_PRTN1_CORE2_PUPD         (MC_ME_BASEADDRESS+0x00000384) 
#define MC_ME_X_PRTN1_CORE2_PUPD(x)    ((MC_ME_BASEADDRESS + 0x00000384 + ((x)*0x4000)))

/* Partition 1 Core 2 Status Register */

#define MC_ME_PRTN1_CORE2_STAT         (MC_ME_BASEADDRESS+0x00000388) 
#define MC_ME_X_PRTN1_CORE2_STAT(x)    ((MC_ME_BASEADDRESS + 0x00000388 + ((x)*0x4000)))

/* Partition 1 Core 2 Address Register */

#define MC_ME_PRTN1_CORE2_ADDR         (MC_ME_BASEADDRESS+0x0000038C) 
#define MC_ME_X_PRTN1_CORE2_ADDR(x)    ((MC_ME_BASEADDRESS + 0x0000038C + ((x)*0x4000)))

/* Partition 1 Core 3 Process Configuration Register */

#define MC_ME_PRTN1_CORE3_PCONF        (MC_ME_BASEADDRESS+0x000003A0) 
#define MC_ME_X_PRTN1_CORE3_PCONF(x)   ((MC_ME_BASEADDRESS + 0x000003A0 + ((x)*0x4000)))

/* Partition 1 Core 3 Process Update Register */

#define MC_ME_PRTN1_CORE3_PUPD         (MC_ME_BASEADDRESS+0x000003A4) 
#define MC_ME_X_PRTN1_CORE3_PUPD(x)    ((MC_ME_BASEADDRESS + 0x000003A4 + ((x)*0x4000)))

/* Partition 1 Core 3 Status Register */

#define MC_ME_PRTN1_CORE3_STAT         (MC_ME_BASEADDRESS+0x000003A8) 
#define MC_ME_X_PRTN1_CORE3_STAT(x)    ((MC_ME_BASEADDRESS + 0x000003A8 + ((x)*0x4000)))

/* Partition 1 Core 3 Address Register */

#define MC_ME_PRTN1_CORE3_ADDR         (MC_ME_BASEADDRESS+0x000003AC) 
#define MC_ME_X_PRTN1_CORE3_ADDR(x)    ((MC_ME_BASEADDRESS + 0x000003AC + ((x)*0x4000)))

/* Field definitions for CTL_KEY */

#define MC_ME_KEY_VALUE(x)             (((x)&0x0000FFFF)<<0)  
#define MC_ME_KEY_MSB                  (15)
#define MC_ME_KEY_LSB                  (0)
#define MC_ME_KEY_MASK                 (0x0000FFFF) 
#define MC_ME_KEY                      ((MC_ME_KEY_MASK) << (MC_ME_KEY_LSB)) 

/* Field definitions for MODE_CONF */

#define MC_ME_DEST_RST_VALUE(x)        (((x)&0x00000001)<<0)  
#define MC_ME_DEST_RST_BIT             (0)  
#define MC_ME_DEST_RST                 ((1) << (MC_ME_DEST_RST_BIT)) 

#define MC_ME_FUNC_RST_VALUE(x)        (((x)&0x00000001)<<1)  
#define MC_ME_FUNC_RST_BIT             (1)  
#define MC_ME_FUNC_RST                 ((1) << (MC_ME_FUNC_RST_BIT)) 

#define MC_ME_STANDBY_VALUE(x)         (((x)&0x00000001)<<15)  
#define MC_ME_STANDBY_BIT              (15)  
#define MC_ME_STANDBY                  ((1) << (MC_ME_STANDBY_BIT)) 

/* Field definitions for MODE_UPD */

#define MC_ME_MODE_UPD_VALUE(x)        (((x)&0x00000001)<<0)  
#define MC_ME_MODE_UPD_BIT             (0)  
#undef MC_ME_MODE_UPD
#define MC_ME_MODE_UPD                 ((1) << (MC_ME_MODE_UPD_BIT)) 

/* Field definitions for MODE_STAT */

#define MC_ME_PREV_MODE_VALUE(x)       (((x)&0x00000001)<<0)  
#define MC_ME_PREV_MODE_BIT            (0)  
#define MC_ME_PREV_MODE                ((1) << (MC_ME_PREV_MODE_BIT)) 

/* Field definitions for MAIN_COREID */

#define MC_ME_CIDX_VALUE(x)            (((x)&0x00000007)<<0)  
#define MC_ME_CIDX_MSB                 (2)
#define MC_ME_CIDX_LSB                 (0)
#define MC_ME_CIDX_MASK                (0x00000007) 
#define MC_ME_CIDX                     ((MC_ME_CIDX_MASK) << (MC_ME_CIDX_LSB)) 

#define MC_ME_PIDX_VALUE(x)            (((x)&0x0000001F)<<8)  
#define MC_ME_PIDX_MSB                 (12)
#define MC_ME_PIDX_LSB                 (8)
#define MC_ME_PIDX_MASK                (0x0000001F) 
#define MC_ME_PIDX                     ((MC_ME_PIDX_MASK) << (MC_ME_PIDX_LSB)) 

/* Field definitions for PRTN0_PCONF */

#define MC_ME_PCE_VALUE(x)             (((x)&0x00000001)<<0)  
#define MC_ME_PCE_BIT                  (0)  
#define MC_ME_PCE                      ((1) << (MC_ME_PCE_BIT)) 

/* Field definitions for PRTN0_PUPD */

#define MC_ME_PCUD_VALUE(x)            (((x)&0x00000001)<<0)  
#define MC_ME_PCUD_BIT                 (0)  
#define MC_ME_PCUD                     ((1) << (MC_ME_PCUD_BIT)) 

/* Field definitions for PRTN0_STAT */

#define MC_ME_PCS_VALUE(x)             (((x)&0x00000001)<<0)  
#define MC_ME_PCS_BIT                  (0)  
#define MC_ME_PCS                      ((1) << (MC_ME_PCS_BIT)) 

/* Field definitions for PRTN0_COFB0_STAT */

#define MC_ME_BLOCK0_VALUE(x)          (((x)&0x00000001)<<0)  
#define MC_ME_BLOCK0_BIT               (0)  
#define MC_ME_BLOCK0                   ((1) << (MC_ME_BLOCK0_BIT)) 

#define MC_ME_BLOCK1_VALUE(x)          (((x)&0x00000001)<<1)  
#define MC_ME_BLOCK1_BIT               (1)  
#define MC_ME_BLOCK1                   ((1) << (MC_ME_BLOCK1_BIT)) 

#define MC_ME_BLOCK2_VALUE(x)          (((x)&0x00000001)<<2)  
#define MC_ME_BLOCK2_BIT               (2)  
#define MC_ME_BLOCK2                   ((1) << (MC_ME_BLOCK2_BIT)) 

#define MC_ME_BLOCK3_VALUE(x)          (((x)&0x00000001)<<3)  
#define MC_ME_BLOCK3_BIT               (3)  
#define MC_ME_BLOCK3                   ((1) << (MC_ME_BLOCK3_BIT)) 

#define MC_ME_BLOCK4_VALUE(x)          (((x)&0x00000001)<<4)  
#define MC_ME_BLOCK4_BIT               (4)  
#define MC_ME_BLOCK4                   ((1) << (MC_ME_BLOCK4_BIT)) 

#define MC_ME_BLOCK5_VALUE(x)          (((x)&0x00000001)<<5)  
#define MC_ME_BLOCK5_BIT               (5)  
#define MC_ME_BLOCK5                   ((1) << (MC_ME_BLOCK5_BIT)) 

#define MC_ME_BLOCK6_VALUE(x)          (((x)&0x00000001)<<6)  
#define MC_ME_BLOCK6_BIT               (6)  
#define MC_ME_BLOCK6                   ((1) << (MC_ME_BLOCK6_BIT)) 

#define MC_ME_BLOCK7_VALUE(x)          (((x)&0x00000001)<<7)  
#define MC_ME_BLOCK7_BIT               (7)  
#define MC_ME_BLOCK7                   ((1) << (MC_ME_BLOCK7_BIT)) 

#define MC_ME_BLOCK8_VALUE(x)          (((x)&0x00000001)<<8)  
#define MC_ME_BLOCK8_BIT               (8)  
#define MC_ME_BLOCK8                   ((1) << (MC_ME_BLOCK8_BIT)) 

#define MC_ME_BLOCK9_VALUE(x)          (((x)&0x00000001)<<9)  
#define MC_ME_BLOCK9_BIT               (9)  
#define MC_ME_BLOCK9                   ((1) << (MC_ME_BLOCK9_BIT)) 

#define MC_ME_BLOCK10_VALUE(x)         (((x)&0x00000001)<<10)  
#define MC_ME_BLOCK10_BIT              (10)  
#define MC_ME_BLOCK10                  ((1) << (MC_ME_BLOCK10_BIT)) 

#define MC_ME_BLOCK11_VALUE(x)         (((x)&0x00000001)<<11)  
#define MC_ME_BLOCK11_BIT              (11)  
#define MC_ME_BLOCK11                  ((1) << (MC_ME_BLOCK11_BIT)) 

#define MC_ME_BLOCK12_VALUE(x)         (((x)&0x00000001)<<12)  
#define MC_ME_BLOCK12_BIT              (12)  
#define MC_ME_BLOCK12                  ((1) << (MC_ME_BLOCK12_BIT)) 

#define MC_ME_BLOCK13_VALUE(x)         (((x)&0x00000001)<<13)  
#define MC_ME_BLOCK13_BIT              (13)  
#define MC_ME_BLOCK13                  ((1) << (MC_ME_BLOCK13_BIT)) 

#define MC_ME_BLOCK14_VALUE(x)         (((x)&0x00000001)<<14)  
#define MC_ME_BLOCK14_BIT              (14)  
#define MC_ME_BLOCK14                  ((1) << (MC_ME_BLOCK14_BIT)) 

#define MC_ME_BLOCK15_VALUE(x)         (((x)&0x00000001)<<15)  
#define MC_ME_BLOCK15_BIT              (15)  
#define MC_ME_BLOCK15                  ((1) << (MC_ME_BLOCK15_BIT)) 

#define MC_ME_BLOCK16_VALUE(x)         (((x)&0x00000001)<<16)  
#define MC_ME_BLOCK16_BIT              (16)  
#define MC_ME_BLOCK16                  ((1) << (MC_ME_BLOCK16_BIT)) 

#define MC_ME_BLOCK17_VALUE(x)         (((x)&0x00000001)<<17)  
#define MC_ME_BLOCK17_BIT              (17)  
#define MC_ME_BLOCK17                  ((1) << (MC_ME_BLOCK17_BIT)) 

#define MC_ME_BLOCK18_VALUE(x)         (((x)&0x00000001)<<18)  
#define MC_ME_BLOCK18_BIT              (18)  
#define MC_ME_BLOCK18                  ((1) << (MC_ME_BLOCK18_BIT)) 

#define MC_ME_BLOCK19_VALUE(x)         (((x)&0x00000001)<<19)  
#define MC_ME_BLOCK19_BIT              (19)  
#define MC_ME_BLOCK19                  ((1) << (MC_ME_BLOCK19_BIT)) 

#define MC_ME_BLOCK20_VALUE(x)         (((x)&0x00000001)<<20)  
#define MC_ME_BLOCK20_BIT              (20)  
#define MC_ME_BLOCK20                  ((1) << (MC_ME_BLOCK20_BIT)) 

#define MC_ME_BLOCK21_VALUE(x)         (((x)&0x00000001)<<21)  
#define MC_ME_BLOCK21_BIT              (21)  
#define MC_ME_BLOCK21                  ((1) << (MC_ME_BLOCK21_BIT)) 

#define MC_ME_BLOCK22_VALUE(x)         (((x)&0x00000001)<<22)  
#define MC_ME_BLOCK22_BIT              (22)  
#define MC_ME_BLOCK22                  ((1) << (MC_ME_BLOCK22_BIT)) 

#define MC_ME_BLOCK23_VALUE(x)         (((x)&0x00000001)<<23)  
#define MC_ME_BLOCK23_BIT              (23)  
#define MC_ME_BLOCK23                  ((1) << (MC_ME_BLOCK23_BIT)) 

#define MC_ME_BLOCK24_VALUE(x)         (((x)&0x00000001)<<24)  
#define MC_ME_BLOCK24_BIT              (24)  
#define MC_ME_BLOCK24                  ((1) << (MC_ME_BLOCK24_BIT)) 

#define MC_ME_BLOCK25_VALUE(x)         (((x)&0x00000001)<<25)  
#define MC_ME_BLOCK25_BIT              (25)  
#define MC_ME_BLOCK25                  ((1) << (MC_ME_BLOCK25_BIT)) 

#define MC_ME_BLOCK26_VALUE(x)         (((x)&0x00000001)<<26)  
#define MC_ME_BLOCK26_BIT              (26)  
#define MC_ME_BLOCK26                  ((1) << (MC_ME_BLOCK26_BIT)) 

#define MC_ME_BLOCK27_VALUE(x)         (((x)&0x00000001)<<27)  
#define MC_ME_BLOCK27_BIT              (27)  
#define MC_ME_BLOCK27                  ((1) << (MC_ME_BLOCK27_BIT)) 

#define MC_ME_BLOCK28_VALUE(x)         (((x)&0x00000001)<<28)  
#define MC_ME_BLOCK28_BIT              (28)  
#define MC_ME_BLOCK28                  ((1) << (MC_ME_BLOCK28_BIT)) 

#define MC_ME_BLOCK29_VALUE(x)         (((x)&0x00000001)<<29)  
#define MC_ME_BLOCK29_BIT              (29)  
#define MC_ME_BLOCK29                  ((1) << (MC_ME_BLOCK29_BIT)) 

#define MC_ME_BLOCK30_VALUE(x)         (((x)&0x00000001)<<30)  
#define MC_ME_BLOCK30_BIT              (30)  
#define MC_ME_BLOCK30                  ((1) << (MC_ME_BLOCK30_BIT)) 

#define MC_ME_BLOCK31_VALUE(x)         (((x)&0x00000001)<<31)  
#define MC_ME_BLOCK31_BIT              (31)  
#define MC_ME_BLOCK31                  ((1) << (MC_ME_BLOCK31_BIT)) 

/* Field definitions for PRTN0_COFB0_CLKEN */

#define MC_ME_REQ0_VALUE(x)            (((x)&0x00000001)<<0)  
#define MC_ME_REQ0_BIT                 (0)  
#define MC_ME_REQ0                     ((1) << (MC_ME_REQ0_BIT)) 

#define MC_ME_REQ1_VALUE(x)            (((x)&0x00000001)<<1)  
#define MC_ME_REQ1_BIT                 (1)  
#define MC_ME_REQ1                     ((1) << (MC_ME_REQ1_BIT)) 

#define MC_ME_REQ2_VALUE(x)            (((x)&0x00000001)<<2)  
#define MC_ME_REQ2_BIT                 (2)  
#define MC_ME_REQ2                     ((1) << (MC_ME_REQ2_BIT)) 

#define MC_ME_REQ3_VALUE(x)            (((x)&0x00000001)<<3)  
#define MC_ME_REQ3_BIT                 (3)  
#define MC_ME_REQ3                     ((1) << (MC_ME_REQ3_BIT)) 

#define MC_ME_REQ4_VALUE(x)            (((x)&0x00000001)<<4)  
#define MC_ME_REQ4_BIT                 (4)  
#define MC_ME_REQ4                     ((1) << (MC_ME_REQ4_BIT)) 

#define MC_ME_REQ5_VALUE(x)            (((x)&0x00000001)<<5)  
#define MC_ME_REQ5_BIT                 (5)  
#define MC_ME_REQ5                     ((1) << (MC_ME_REQ5_BIT)) 

#define MC_ME_REQ6_VALUE(x)            (((x)&0x00000001)<<6)  
#define MC_ME_REQ6_BIT                 (6)  
#define MC_ME_REQ6                     ((1) << (MC_ME_REQ6_BIT)) 

#define MC_ME_REQ7_VALUE(x)            (((x)&0x00000001)<<7)  
#define MC_ME_REQ7_BIT                 (7)  
#define MC_ME_REQ7                     ((1) << (MC_ME_REQ7_BIT)) 

#define MC_ME_REQ8_VALUE(x)            (((x)&0x00000001)<<8)  
#define MC_ME_REQ8_BIT                 (8)  
#define MC_ME_REQ8                     ((1) << (MC_ME_REQ8_BIT)) 

#define MC_ME_REQ9_VALUE(x)            (((x)&0x00000001)<<9)  
#define MC_ME_REQ9_BIT                 (9)  
#define MC_ME_REQ9                     ((1) << (MC_ME_REQ9_BIT)) 

#define MC_ME_REQ10_VALUE(x)           (((x)&0x00000001)<<10)  
#define MC_ME_REQ10_BIT                (10)  
#define MC_ME_REQ10                    ((1) << (MC_ME_REQ10_BIT)) 

#define MC_ME_REQ11_VALUE(x)           (((x)&0x00000001)<<11)  
#define MC_ME_REQ11_BIT                (11)  
#define MC_ME_REQ11                    ((1) << (MC_ME_REQ11_BIT)) 

#define MC_ME_REQ12_VALUE(x)           (((x)&0x00000001)<<12)  
#define MC_ME_REQ12_BIT                (12)  
#define MC_ME_REQ12                    ((1) << (MC_ME_REQ12_BIT)) 

#define MC_ME_REQ13_VALUE(x)           (((x)&0x00000001)<<13)  
#define MC_ME_REQ13_BIT                (13)  
#define MC_ME_REQ13                    ((1) << (MC_ME_REQ13_BIT)) 

#define MC_ME_REQ14_VALUE(x)           (((x)&0x00000001)<<14)  
#define MC_ME_REQ14_BIT                (14)  
#define MC_ME_REQ14                    ((1) << (MC_ME_REQ14_BIT)) 

#define MC_ME_REQ15_VALUE(x)           (((x)&0x00000001)<<15)  
#define MC_ME_REQ15_BIT                (15)  
#define MC_ME_REQ15                    ((1) << (MC_ME_REQ15_BIT)) 

#define MC_ME_REQ16_VALUE(x)           (((x)&0x00000001)<<16)  
#define MC_ME_REQ16_BIT                (16)  
#define MC_ME_REQ16                    ((1) << (MC_ME_REQ16_BIT)) 

#define MC_ME_REQ17_VALUE(x)           (((x)&0x00000001)<<17)  
#define MC_ME_REQ17_BIT                (17)  
#define MC_ME_REQ17                    ((1) << (MC_ME_REQ17_BIT)) 

#define MC_ME_REQ18_VALUE(x)           (((x)&0x00000001)<<18)  
#define MC_ME_REQ18_BIT                (18)  
#define MC_ME_REQ18                    ((1) << (MC_ME_REQ18_BIT)) 

#define MC_ME_REQ19_VALUE(x)           (((x)&0x00000001)<<19)  
#define MC_ME_REQ19_BIT                (19)  
#define MC_ME_REQ19                    ((1) << (MC_ME_REQ19_BIT)) 

#define MC_ME_REQ20_VALUE(x)           (((x)&0x00000001)<<20)  
#define MC_ME_REQ20_BIT                (20)  
#define MC_ME_REQ20                    ((1) << (MC_ME_REQ20_BIT)) 

#define MC_ME_REQ21_VALUE(x)           (((x)&0x00000001)<<21)  
#define MC_ME_REQ21_BIT                (21)  
#define MC_ME_REQ21                    ((1) << (MC_ME_REQ21_BIT)) 

#define MC_ME_REQ22_VALUE(x)           (((x)&0x00000001)<<22)  
#define MC_ME_REQ22_BIT                (22)  
#define MC_ME_REQ22                    ((1) << (MC_ME_REQ22_BIT)) 

#define MC_ME_REQ23_VALUE(x)           (((x)&0x00000001)<<23)  
#define MC_ME_REQ23_BIT                (23)  
#define MC_ME_REQ23                    ((1) << (MC_ME_REQ23_BIT)) 

#define MC_ME_REQ24_VALUE(x)           (((x)&0x00000001)<<24)  
#define MC_ME_REQ24_BIT                (24)  
#define MC_ME_REQ24                    ((1) << (MC_ME_REQ24_BIT)) 

#define MC_ME_REQ25_VALUE(x)           (((x)&0x00000001)<<25)  
#define MC_ME_REQ25_BIT                (25)  
#define MC_ME_REQ25                    ((1) << (MC_ME_REQ25_BIT)) 

#define MC_ME_REQ26_VALUE(x)           (((x)&0x00000001)<<26)  
#define MC_ME_REQ26_BIT                (26)  
#define MC_ME_REQ26                    ((1) << (MC_ME_REQ26_BIT)) 

#define MC_ME_REQ27_VALUE(x)           (((x)&0x00000001)<<27)  
#define MC_ME_REQ27_BIT                (27)  
#define MC_ME_REQ27                    ((1) << (MC_ME_REQ27_BIT)) 

#define MC_ME_REQ28_VALUE(x)           (((x)&0x00000001)<<28)  
#define MC_ME_REQ28_BIT                (28)  
#define MC_ME_REQ28                    ((1) << (MC_ME_REQ28_BIT)) 

#define MC_ME_REQ29_VALUE(x)           (((x)&0x00000001)<<29)  
#define MC_ME_REQ29_BIT                (29)  
#define MC_ME_REQ29                    ((1) << (MC_ME_REQ29_BIT)) 

#define MC_ME_REQ30_VALUE(x)           (((x)&0x00000001)<<30)  
#define MC_ME_REQ30_BIT                (30)  
#define MC_ME_REQ30                    ((1) << (MC_ME_REQ30_BIT)) 

#define MC_ME_REQ31_VALUE(x)           (((x)&0x00000001)<<31)  
#define MC_ME_REQ31_BIT                (31)  
#define MC_ME_REQ31                    ((1) << (MC_ME_REQ31_BIT)) 

/* Field definitions for PRTN0_CORE0_PCONF */

#define MC_ME_CCE_VALUE(x)             (((x)&0x00000001)<<0)  
#define MC_ME_CCE_BIT                  (0)  
#define MC_ME_CCE                      ((1) << (MC_ME_CCE_BIT)) 

/* Field definitions for PRTN0_CORE0_PUPD */

#define MC_ME_CCUPD_VALUE(x)           (((x)&0x00000001)<<0)  
#define MC_ME_CCUPD_BIT                (0)  
#define MC_ME_CCUPD                    ((1) << (MC_ME_CCUPD_BIT)) 

/* Field definitions for PRTN0_CORE0_STAT */

#define MC_ME_CCS_VALUE(x)             (((x)&0x00000001)<<0)  
#define MC_ME_CCS_BIT                  (0)  
#define MC_ME_CCS                      ((1) << (MC_ME_CCS_BIT)) 

#define MC_ME_WFI_VALUE(x)             (((x)&0x00000001)<<31)  
#define MC_ME_WFI_BIT                  (31)  
#define MC_ME_WFI                      ((1) << (MC_ME_WFI_BIT)) 

/* Field definitions for PRTN0_CORE0_ADDR */

#define MC_ME_ADDR_VALUE(x)            (((x)&0x3FFFFFFF)<<2)  
#define MC_ME_ADDR_MSB                 (31)
#define MC_ME_ADDR_LSB                 (2)
#define MC_ME_ADDR_MASK                (0x3FFFFFFF) 
#define MC_ME_ADDR                     ((MC_ME_ADDR_MASK) << (MC_ME_ADDR_LSB)) 

