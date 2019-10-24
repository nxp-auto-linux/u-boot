// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019 NXP
 */

// SIUL SoC specific information.

#define SIUL_MSCR_OBE              (0x00200000 ) //Added    
#define SIUL_MSCR_IBE              (0x00080000)

/*****************************************************************
*
* SIUL Registers
*
******************************************************************/

/* Instance SIUL */

/* SIUL2_0, aka SIUL_CC */
#define SIUL_BASEADDRESS               0x4009C000          
/* SIUL2_1, aka SIUL_OFFCC */
#define SOC_SIUL_OFFCC_BASEADDRESS     0x44010000

/* Register definitions */

/* MIDR1 - MCU ID Reg #1 */

#define SIUL_MIDR1                     (SIUL_BASEADDRESS+0x00000004) 
#define SIUL_X_MIDR1(x)                ((SIUL_BASEADDRESS + 0x00000004 + ((x)*0x8000)))

/* MIDR2 - MCU ID Reg #2 */

#define SIUL_MIDR2                     (SIUL_BASEADDRESS+0x00000008) 
#define SIUL_X_MIDR2(x)                ((SIUL_BASEADDRESS + 0x00000008 + ((x)*0x8000)))

/* DISR0 - DMA/Interrupt Status Flag Reg */

#define SIUL_DISR0                     (SIUL_BASEADDRESS+0x00000010) 
#define SIUL_X_DISR0(x)                ((SIUL_BASEADDRESS + 0x00000010 + ((x)*0x8000)))

/* DIRER0 - DMA/Interrupt Request Enable Reg */

#define SIUL_DIRER0                    (SIUL_BASEADDRESS+0x00000018) 
#define SIUL_X_DIRER0(x)               ((SIUL_BASEADDRESS + 0x00000018 + ((x)*0x8000)))

/* DIRSR0 - DMA/Interrupt Request Select Reg */

#define SIUL_DIRSR0                    (SIUL_BASEADDRESS+0x00000020) 
#define SIUL_X_DIRSR0(x)               ((SIUL_BASEADDRESS + 0x00000020 + ((x)*0x8000)))

/* IREER0 -interrupt Rising-Edge Event Enable Reg */

#define SIUL_IREER0                    (SIUL_BASEADDRESS+0x00000028) 
#define SIUL_X_IREER0(x)               ((SIUL_BASEADDRESS + 0x00000028 + ((x)*0x8000)))

/* IFEER0 -Interrupt Falling-Edge Event Enable Reg */

#define SIUL_IFEER0                    (SIUL_BASEADDRESS+0x00000030) 
#define SIUL_X_IFEER0(x)               ((SIUL_BASEADDRESS + 0x00000030 + ((x)*0x8000)))

/* IFER0 -Interrupt Filter Enable Reg */

#define SIUL_IFER0                     (SIUL_BASEADDRESS+0x00000038) 
#define SIUL_X_IFER0(x)                ((SIUL_BASEADDRESS + 0x00000038 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */
#define SIUL_IFMCR(x)        (((SIUL_BASEADDRESS+0x00000040) +((x)*0x4)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR0                    (SIUL_BASEADDRESS+0x00000040) 
#define SIUL_X_IFMCR0(x)               ((SIUL_BASEADDRESS + 0x00000040 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR1                    (SIUL_BASEADDRESS+0x00000044) 
#define SIUL_X_IFMCR1(x)               ((SIUL_BASEADDRESS + 0x00000044 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR2                    (SIUL_BASEADDRESS+0x00000048) 
#define SIUL_X_IFMCR2(x)               ((SIUL_BASEADDRESS + 0x00000048 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR3                    (SIUL_BASEADDRESS+0x0000004C) 
#define SIUL_X_IFMCR3(x)               ((SIUL_BASEADDRESS + 0x0000004C + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR4                    (SIUL_BASEADDRESS+0x00000050) 
#define SIUL_X_IFMCR4(x)               ((SIUL_BASEADDRESS + 0x00000050 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR5                    (SIUL_BASEADDRESS+0x00000054) 
#define SIUL_X_IFMCR5(x)               ((SIUL_BASEADDRESS + 0x00000054 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR6                    (SIUL_BASEADDRESS+0x00000058) 
#define SIUL_X_IFMCR6(x)               ((SIUL_BASEADDRESS + 0x00000058 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR7                    (SIUL_BASEADDRESS+0x0000005C) 
#define SIUL_X_IFMCR7(x)               ((SIUL_BASEADDRESS + 0x0000005C + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR8                    (SIUL_BASEADDRESS+0x00000060) 
#define SIUL_X_IFMCR8(x)               ((SIUL_BASEADDRESS + 0x00000060 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR9                    (SIUL_BASEADDRESS+0x00000064) 
#define SIUL_X_IFMCR9(x)               ((SIUL_BASEADDRESS + 0x00000064 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR10                   (SIUL_BASEADDRESS+0x00000068) 
#define SIUL_X_IFMCR10(x)              ((SIUL_BASEADDRESS + 0x00000068 + ((x)*0x8000)))

/* IFMCRn - Interrupt Filter Maximum Counter Reg */

#define SIUL_IFMCR11                   (SIUL_BASEADDRESS+0x0000006C) 
#define SIUL_X_IFMCR11(x)              ((SIUL_BASEADDRESS + 0x0000006C + ((x)*0x8000)))

/* IFCPR -Interrupt Filter Clock Prescaler Reg */

#define SIUL_IFCPR                     (SIUL_BASEADDRESS+0x000000C0) 
#define SIUL_X_IFCPR(x)                ((SIUL_BASEADDRESS + 0x000000C0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */
#define SIUL_MSCR(x)         (((SIUL_BASEADDRESS+0x00000240) +((x)*0x4)))
/* IMCRn - Input Multiplexed Signal Configuration Reg */
#define SIUL_IMCR(x)         (((SIUL_BASEADDRESS+0x00000A40) +((x)*0x4)))

/* MSCRn - Multiplexed Single Configuration Reg */
#define SIUL_MSCR0                     (SIUL_BASEADDRESS+0x00000240) 
#define SIUL_X_MSCR0(x)                ((SIUL_BASEADDRESS + 0x00000240 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR1                     (SIUL_BASEADDRESS+0x00000244) 
#define SIUL_X_MSCR1(x)                ((SIUL_BASEADDRESS + 0x00000244 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR2                     (SIUL_BASEADDRESS+0x00000248) 
#define SIUL_X_MSCR2(x)                ((SIUL_BASEADDRESS + 0x00000248 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR3                     (SIUL_BASEADDRESS+0x0000024C) 
#define SIUL_X_MSCR3(x)                ((SIUL_BASEADDRESS + 0x0000024C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR4                     (SIUL_BASEADDRESS+0x00000250) 
#define SIUL_X_MSCR4(x)                ((SIUL_BASEADDRESS + 0x00000250 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR5                     (SIUL_BASEADDRESS+0x00000254) 
#define SIUL_X_MSCR5(x)                ((SIUL_BASEADDRESS + 0x00000254 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR6                     (SIUL_BASEADDRESS+0x00000258) 
#define SIUL_X_MSCR6(x)                ((SIUL_BASEADDRESS + 0x00000258 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR7                     (SIUL_BASEADDRESS+0x0000025C) 
#define SIUL_X_MSCR7(x)                ((SIUL_BASEADDRESS + 0x0000025C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR8                     (SIUL_BASEADDRESS+0x00000260) 
#define SIUL_X_MSCR8(x)                ((SIUL_BASEADDRESS + 0x00000260 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR9                     (SIUL_BASEADDRESS+0x00000264) 
#define SIUL_X_MSCR9(x)                ((SIUL_BASEADDRESS + 0x00000264 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR10                    (SIUL_BASEADDRESS+0x00000268) 
#define SIUL_X_MSCR10(x)               ((SIUL_BASEADDRESS + 0x00000268 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR11                    (SIUL_BASEADDRESS+0x0000026C) 
#define SIUL_X_MSCR11(x)               ((SIUL_BASEADDRESS + 0x0000026C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR12                    (SIUL_BASEADDRESS+0x00000270) 
#define SIUL_X_MSCR12(x)               ((SIUL_BASEADDRESS + 0x00000270 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR13                    (SIUL_BASEADDRESS+0x00000274) 
#define SIUL_X_MSCR13(x)               ((SIUL_BASEADDRESS + 0x00000274 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR14                    (SIUL_BASEADDRESS+0x00000278) 
#define SIUL_X_MSCR14(x)               ((SIUL_BASEADDRESS + 0x00000278 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR15                    (SIUL_BASEADDRESS+0x0000027C) 
#define SIUL_X_MSCR15(x)               ((SIUL_BASEADDRESS + 0x0000027C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR16                    (SIUL_BASEADDRESS+0x00000280) 
#define SIUL_X_MSCR16(x)               ((SIUL_BASEADDRESS + 0x00000280 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR17                    (SIUL_BASEADDRESS+0x00000284) 
#define SIUL_X_MSCR17(x)               ((SIUL_BASEADDRESS + 0x00000284 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR18                    (SIUL_BASEADDRESS+0x00000288) 
#define SIUL_X_MSCR18(x)               ((SIUL_BASEADDRESS + 0x00000288 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR19                    (SIUL_BASEADDRESS+0x0000028C) 
#define SIUL_X_MSCR19(x)               ((SIUL_BASEADDRESS + 0x0000028C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR20                    (SIUL_BASEADDRESS+0x00000290) 
#define SIUL_X_MSCR20(x)               ((SIUL_BASEADDRESS + 0x00000290 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR21                    (SIUL_BASEADDRESS+0x00000294) 
#define SIUL_X_MSCR21(x)               ((SIUL_BASEADDRESS + 0x00000294 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR22                    (SIUL_BASEADDRESS+0x00000298) 
#define SIUL_X_MSCR22(x)               ((SIUL_BASEADDRESS + 0x00000298 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR23                    (SIUL_BASEADDRESS+0x0000029C) 
#define SIUL_X_MSCR23(x)               ((SIUL_BASEADDRESS + 0x0000029C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR24                    (SIUL_BASEADDRESS+0x000002A0) 
#define SIUL_X_MSCR24(x)               ((SIUL_BASEADDRESS + 0x000002A0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR25                    (SIUL_BASEADDRESS+0x000002A4) 
#define SIUL_X_MSCR25(x)               ((SIUL_BASEADDRESS + 0x000002A4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR26                    (SIUL_BASEADDRESS+0x000002A8) 
#define SIUL_X_MSCR26(x)               ((SIUL_BASEADDRESS + 0x000002A8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR27                    (SIUL_BASEADDRESS+0x000002AC) 
#define SIUL_X_MSCR27(x)               ((SIUL_BASEADDRESS + 0x000002AC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR28                    (SIUL_BASEADDRESS+0x000002B0) 
#define SIUL_X_MSCR28(x)               ((SIUL_BASEADDRESS + 0x000002B0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR29                    (SIUL_BASEADDRESS+0x000002B4) 
#define SIUL_X_MSCR29(x)               ((SIUL_BASEADDRESS + 0x000002B4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR30                    (SIUL_BASEADDRESS+0x000002B8) 
#define SIUL_X_MSCR30(x)               ((SIUL_BASEADDRESS + 0x000002B8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR31                    (SIUL_BASEADDRESS+0x000002BC) 
#define SIUL_X_MSCR31(x)               ((SIUL_BASEADDRESS + 0x000002BC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR32                    (SIUL_BASEADDRESS+0x000002C0) 
#define SIUL_X_MSCR32(x)               ((SIUL_BASEADDRESS + 0x000002C0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR33                    (SIUL_BASEADDRESS+0x000002C4) 
#define SIUL_X_MSCR33(x)               ((SIUL_BASEADDRESS + 0x000002C4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR34                    (SIUL_BASEADDRESS+0x000002C8) 
#define SIUL_X_MSCR34(x)               ((SIUL_BASEADDRESS + 0x000002C8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR35                    (SIUL_BASEADDRESS+0x000002CC) 
#define SIUL_X_MSCR35(x)               ((SIUL_BASEADDRESS + 0x000002CC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR36                    (SIUL_BASEADDRESS+0x000002D0) 
#define SIUL_X_MSCR36(x)               ((SIUL_BASEADDRESS + 0x000002D0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR37                    (SIUL_BASEADDRESS+0x000002D4) 
#define SIUL_X_MSCR37(x)               ((SIUL_BASEADDRESS + 0x000002D4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR38                    (SIUL_BASEADDRESS+0x000002D8) 
#define SIUL_X_MSCR38(x)               ((SIUL_BASEADDRESS + 0x000002D8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR39                    (SIUL_BASEADDRESS+0x000002DC) 
#define SIUL_X_MSCR39(x)               ((SIUL_BASEADDRESS + 0x000002DC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR40                    (SIUL_BASEADDRESS+0x000002E0) 
#define SIUL_X_MSCR40(x)               ((SIUL_BASEADDRESS + 0x000002E0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR41                    (SIUL_BASEADDRESS+0x000002E4) 
#define SIUL_X_MSCR41(x)               ((SIUL_BASEADDRESS + 0x000002E4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR42                    (SIUL_BASEADDRESS+0x000002E8) 
#define SIUL_X_MSCR42(x)               ((SIUL_BASEADDRESS + 0x000002E8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR43                    (SIUL_BASEADDRESS+0x000002EC) 
#define SIUL_X_MSCR43(x)               ((SIUL_BASEADDRESS + 0x000002EC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR44                    (SIUL_BASEADDRESS+0x000002F0) 
#define SIUL_X_MSCR44(x)               ((SIUL_BASEADDRESS + 0x000002F0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR45                    (SIUL_BASEADDRESS+0x000002F4) 
#define SIUL_X_MSCR45(x)               ((SIUL_BASEADDRESS + 0x000002F4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR46                    (SIUL_BASEADDRESS+0x000002F8) 
#define SIUL_X_MSCR46(x)               ((SIUL_BASEADDRESS + 0x000002F8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR47                    (SIUL_BASEADDRESS+0x000002FC) 
#define SIUL_X_MSCR47(x)               ((SIUL_BASEADDRESS + 0x000002FC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR48                    (SIUL_BASEADDRESS+0x00000300) 
#define SIUL_X_MSCR48(x)               ((SIUL_BASEADDRESS + 0x00000300 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR49                    (SIUL_BASEADDRESS+0x00000304) 
#define SIUL_X_MSCR49(x)               ((SIUL_BASEADDRESS + 0x00000304 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR50                    (SIUL_BASEADDRESS+0x00000308) 
#define SIUL_X_MSCR50(x)               ((SIUL_BASEADDRESS + 0x00000308 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR51                    (SIUL_BASEADDRESS+0x0000030C) 
#define SIUL_X_MSCR51(x)               ((SIUL_BASEADDRESS + 0x0000030C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR52                    (SIUL_BASEADDRESS+0x00000310) 
#define SIUL_X_MSCR52(x)               ((SIUL_BASEADDRESS + 0x00000310 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR53                    (SIUL_BASEADDRESS+0x00000314) 
#define SIUL_X_MSCR53(x)               ((SIUL_BASEADDRESS + 0x00000314 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR54                    (SIUL_BASEADDRESS+0x00000318) 
#define SIUL_X_MSCR54(x)               ((SIUL_BASEADDRESS + 0x00000318 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR55                    (SIUL_BASEADDRESS+0x0000031C) 
#define SIUL_X_MSCR55(x)               ((SIUL_BASEADDRESS + 0x0000031C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR56                    (SIUL_BASEADDRESS+0x00000320) 
#define SIUL_X_MSCR56(x)               ((SIUL_BASEADDRESS + 0x00000320 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR57                    (SIUL_BASEADDRESS+0x00000324) 
#define SIUL_X_MSCR57(x)               ((SIUL_BASEADDRESS + 0x00000324 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR58                    (SIUL_BASEADDRESS+0x00000328) 
#define SIUL_X_MSCR58(x)               ((SIUL_BASEADDRESS + 0x00000328 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR59                    (SIUL_BASEADDRESS+0x0000032C) 
#define SIUL_X_MSCR59(x)               ((SIUL_BASEADDRESS + 0x0000032C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR60                    (SIUL_BASEADDRESS+0x00000330) 
#define SIUL_X_MSCR60(x)               ((SIUL_BASEADDRESS + 0x00000330 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR61                    (SIUL_BASEADDRESS+0x00000334) 
#define SIUL_X_MSCR61(x)               ((SIUL_BASEADDRESS + 0x00000334 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR62                    (SIUL_BASEADDRESS+0x00000338) 
#define SIUL_X_MSCR62(x)               ((SIUL_BASEADDRESS + 0x00000338 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR63                    (SIUL_BASEADDRESS+0x0000033C) 
#define SIUL_X_MSCR63(x)               ((SIUL_BASEADDRESS + 0x0000033C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR64                    (SIUL_BASEADDRESS+0x00000340) 
#define SIUL_X_MSCR64(x)               ((SIUL_BASEADDRESS + 0x00000340 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR65                    (SIUL_BASEADDRESS+0x00000344) 
#define SIUL_X_MSCR65(x)               ((SIUL_BASEADDRESS + 0x00000344 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR66                    (SIUL_BASEADDRESS+0x00000348) 
#define SIUL_X_MSCR66(x)               ((SIUL_BASEADDRESS + 0x00000348 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR67                    (SIUL_BASEADDRESS+0x0000034C) 
#define SIUL_X_MSCR67(x)               ((SIUL_BASEADDRESS + 0x0000034C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR68                    (SIUL_BASEADDRESS+0x00000350) 
#define SIUL_X_MSCR68(x)               ((SIUL_BASEADDRESS + 0x00000350 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR69                    (SIUL_BASEADDRESS+0x00000354) 
#define SIUL_X_MSCR69(x)               ((SIUL_BASEADDRESS + 0x00000354 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR70                    (SIUL_BASEADDRESS+0x00000358) 
#define SIUL_X_MSCR70(x)               ((SIUL_BASEADDRESS + 0x00000358 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR71                    (SIUL_BASEADDRESS+0x0000035C) 
#define SIUL_X_MSCR71(x)               ((SIUL_BASEADDRESS + 0x0000035C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR72                    (SIUL_BASEADDRESS+0x00000360) 
#define SIUL_X_MSCR72(x)               ((SIUL_BASEADDRESS + 0x00000360 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR73                    (SIUL_BASEADDRESS+0x00000364) 
#define SIUL_X_MSCR73(x)               ((SIUL_BASEADDRESS + 0x00000364 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR74                    (SIUL_BASEADDRESS+0x00000368) 
#define SIUL_X_MSCR74(x)               ((SIUL_BASEADDRESS + 0x00000368 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR75                    (SIUL_BASEADDRESS+0x0000036C) 
#define SIUL_X_MSCR75(x)               ((SIUL_BASEADDRESS + 0x0000036C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR76                    (SIUL_BASEADDRESS+0x00000370) 
#define SIUL_X_MSCR76(x)               ((SIUL_BASEADDRESS + 0x00000370 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR77                    (SIUL_BASEADDRESS+0x00000374) 
#define SIUL_X_MSCR77(x)               ((SIUL_BASEADDRESS + 0x00000374 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR78                    (SIUL_BASEADDRESS+0x00000378) 
#define SIUL_X_MSCR78(x)               ((SIUL_BASEADDRESS + 0x00000378 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR79                    (SIUL_BASEADDRESS+0x0000037C) 
#define SIUL_X_MSCR79(x)               ((SIUL_BASEADDRESS + 0x0000037C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR80                    (SIUL_BASEADDRESS+0x00000380) 
#define SIUL_X_MSCR80(x)               ((SIUL_BASEADDRESS + 0x00000380 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR81                    (SIUL_BASEADDRESS+0x00000384) 
#define SIUL_X_MSCR81(x)               ((SIUL_BASEADDRESS + 0x00000384 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR82                    (SIUL_BASEADDRESS+0x00000388) 
#define SIUL_X_MSCR82(x)               ((SIUL_BASEADDRESS + 0x00000388 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR83                    (SIUL_BASEADDRESS+0x0000038C) 
#define SIUL_X_MSCR83(x)               ((SIUL_BASEADDRESS + 0x0000038C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR84                    (SIUL_BASEADDRESS+0x00000390) 
#define SIUL_X_MSCR84(x)               ((SIUL_BASEADDRESS + 0x00000390 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR85                    (SIUL_BASEADDRESS+0x00000394) 
#define SIUL_X_MSCR85(x)               ((SIUL_BASEADDRESS + 0x00000394 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR86                    (SIUL_BASEADDRESS+0x00000398) 
#define SIUL_X_MSCR86(x)               ((SIUL_BASEADDRESS + 0x00000398 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR87                    (SIUL_BASEADDRESS+0x0000039C) 
#define SIUL_X_MSCR87(x)               ((SIUL_BASEADDRESS + 0x0000039C + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR88                    (SIUL_BASEADDRESS+0x000003A0) 
#define SIUL_X_MSCR88(x)               ((SIUL_BASEADDRESS + 0x000003A0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR89                    (SIUL_BASEADDRESS+0x000003A4) 
#define SIUL_X_MSCR89(x)               ((SIUL_BASEADDRESS + 0x000003A4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR90                    (SIUL_BASEADDRESS+0x000003A8) 
#define SIUL_X_MSCR90(x)               ((SIUL_BASEADDRESS + 0x000003A8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR91                    (SIUL_BASEADDRESS+0x000003AC) 
#define SIUL_X_MSCR91(x)               ((SIUL_BASEADDRESS + 0x000003AC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR92                    (SIUL_BASEADDRESS+0x000003B0) 
#define SIUL_X_MSCR92(x)               ((SIUL_BASEADDRESS + 0x000003B0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR93                    (SIUL_BASEADDRESS+0x000003B4) 
#define SIUL_X_MSCR93(x)               ((SIUL_BASEADDRESS + 0x000003B4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR94                    (SIUL_BASEADDRESS+0x000003B8) 
#define SIUL_X_MSCR94(x)               ((SIUL_BASEADDRESS + 0x000003B8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR95                    (SIUL_BASEADDRESS+0x000003BC) 
#define SIUL_X_MSCR95(x)               ((SIUL_BASEADDRESS + 0x000003BC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR96                    (SIUL_BASEADDRESS+0x000003C0) 
#define SIUL_X_MSCR96(x)               ((SIUL_BASEADDRESS + 0x000003C0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR97                    (SIUL_BASEADDRESS+0x000003C4) 
#define SIUL_X_MSCR97(x)               ((SIUL_BASEADDRESS + 0x000003C4 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR98                    (SIUL_BASEADDRESS+0x000003C8) 
#define SIUL_X_MSCR98(x)               ((SIUL_BASEADDRESS + 0x000003C8 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR99                    (SIUL_BASEADDRESS+0x000003CC) 
#define SIUL_X_MSCR99(x)               ((SIUL_BASEADDRESS + 0x000003CC + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR100                   (SIUL_BASEADDRESS+0x000003D0) 
#define SIUL_X_MSCR100(x)              ((SIUL_BASEADDRESS + 0x000003D0 + ((x)*0x8000)))

/* MSCRn - Multiplexed Single Configuration Reg */

#define SIUL_MSCR101                   (SIUL_BASEADDRESS+0x000003D4) 
#define SIUL_X_MSCR101(x)              ((SIUL_BASEADDRESS + 0x000003D4 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO0_3                   (SIUL_BASEADDRESS+0x00001300) 
#define SIUL_X_GPDO0_3(x)              ((SIUL_BASEADDRESS + 0x00001300 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO4_7                   (SIUL_BASEADDRESS+0x00001304) 
#define SIUL_X_GPDO4_7(x)              ((SIUL_BASEADDRESS + 0x00001304 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO8_11                  (SIUL_BASEADDRESS+0x00001308) 
#define SIUL_X_GPDO8_11(x)             ((SIUL_BASEADDRESS + 0x00001308 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO12_15                 (SIUL_BASEADDRESS+0x0000130C) 
#define SIUL_X_GPDO12_15(x)            ((SIUL_BASEADDRESS + 0x0000130C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO16_19                 (SIUL_BASEADDRESS+0x00001310) 
#define SIUL_X_GPDO16_19(x)            ((SIUL_BASEADDRESS + 0x00001310 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO20_23                 (SIUL_BASEADDRESS+0x00001314) 
#define SIUL_X_GPDO20_23(x)            ((SIUL_BASEADDRESS + 0x00001314 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO24_27                 (SIUL_BASEADDRESS+0x00001318) 
#define SIUL_X_GPDO24_27(x)            ((SIUL_BASEADDRESS + 0x00001318 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO28_31                 (SIUL_BASEADDRESS+0x0000131C) 
#define SIUL_X_GPDO28_31(x)            ((SIUL_BASEADDRESS + 0x0000131C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO32_35                 (SIUL_BASEADDRESS+0x00001320) 
#define SIUL_X_GPDO32_35(x)            ((SIUL_BASEADDRESS + 0x00001320 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO36_39                 (SIUL_BASEADDRESS+0x00001324) 
#define SIUL_X_GPDO36_39(x)            ((SIUL_BASEADDRESS + 0x00001324 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO40_43                 (SIUL_BASEADDRESS+0x00001328) 
#define SIUL_X_GPDO40_43(x)            ((SIUL_BASEADDRESS + 0x00001328 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO44_47                 (SIUL_BASEADDRESS+0x0000132C) 
#define SIUL_X_GPDO44_47(x)            ((SIUL_BASEADDRESS + 0x0000132C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO48_51                 (SIUL_BASEADDRESS+0x00001330) 
#define SIUL_X_GPDO48_51(x)            ((SIUL_BASEADDRESS + 0x00001330 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO52_55                 (SIUL_BASEADDRESS+0x00001334) 
#define SIUL_X_GPDO52_55(x)            ((SIUL_BASEADDRESS + 0x00001334 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO56_59                 (SIUL_BASEADDRESS+0x00001338) 
#define SIUL_X_GPDO56_59(x)            ((SIUL_BASEADDRESS + 0x00001338 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO60_63                 (SIUL_BASEADDRESS+0x0000133C) 
#define SIUL_X_GPDO60_63(x)            ((SIUL_BASEADDRESS + 0x0000133C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO64_67                 (SIUL_BASEADDRESS+0x00001340) 
#define SIUL_X_GPDO64_67(x)            ((SIUL_BASEADDRESS + 0x00001340 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO68_71                 (SIUL_BASEADDRESS+0x00001344) 
#define SIUL_X_GPDO68_71(x)            ((SIUL_BASEADDRESS + 0x00001344 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO72_75                 (SIUL_BASEADDRESS+0x00001348) 
#define SIUL_X_GPDO72_75(x)            ((SIUL_BASEADDRESS + 0x00001348 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO76_79                 (SIUL_BASEADDRESS+0x0000134C) 
#define SIUL_X_GPDO76_79(x)            ((SIUL_BASEADDRESS + 0x0000134C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO80_83                 (SIUL_BASEADDRESS+0x00001350) 
#define SIUL_X_GPDO80_83(x)            ((SIUL_BASEADDRESS + 0x00001350 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO84_87                 (SIUL_BASEADDRESS+0x00001354) 
#define SIUL_X_GPDO84_87(x)            ((SIUL_BASEADDRESS + 0x00001354 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO88_91                 (SIUL_BASEADDRESS+0x00001358) 
#define SIUL_X_GPDO88_91(x)            ((SIUL_BASEADDRESS + 0x00001358 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO92_95                 (SIUL_BASEADDRESS+0x0000135C) 
#define SIUL_X_GPDO92_95(x)            ((SIUL_BASEADDRESS + 0x0000135C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO96_99                 (SIUL_BASEADDRESS+0x00001360) 
#define SIUL_X_GPDO96_99(x)            ((SIUL_BASEADDRESS + 0x00001360 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO100_103               (SIUL_BASEADDRESS+0x00001364) 
#define SIUL_X_GPDO100_103(x)          ((SIUL_BASEADDRESS + 0x00001364 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO104_107               (SIUL_BASEADDRESS+0x00001368) 
#define SIUL_X_GPDO104_107(x)          ((SIUL_BASEADDRESS + 0x00001368 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO108_111               (SIUL_BASEADDRESS+0x0000136C) 
#define SIUL_X_GPDO108_111(x)          ((SIUL_BASEADDRESS + 0x0000136C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO112_115               (SIUL_BASEADDRESS+0x00001370) 
#define SIUL_X_GPDO112_115(x)          ((SIUL_BASEADDRESS + 0x00001370 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO116_119               (SIUL_BASEADDRESS+0x00001374) 
#define SIUL_X_GPDO116_119(x)          ((SIUL_BASEADDRESS + 0x00001374 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO120_123               (SIUL_BASEADDRESS+0x00001378) 
#define SIUL_X_GPDO120_123(x)          ((SIUL_BASEADDRESS + 0x00001378 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO124_127               (SIUL_BASEADDRESS+0x0000137C) 
#define SIUL_X_GPDO124_127(x)          ((SIUL_BASEADDRESS + 0x0000137C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO128_131               (SIUL_BASEADDRESS+0x00001380) 
#define SIUL_X_GPDO128_131(x)          ((SIUL_BASEADDRESS + 0x00001380 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO132_135               (SIUL_BASEADDRESS+0x00001384) 
#define SIUL_X_GPDO132_135(x)          ((SIUL_BASEADDRESS + 0x00001384 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO136_139               (SIUL_BASEADDRESS+0x00001388) 
#define SIUL_X_GPDO136_139(x)          ((SIUL_BASEADDRESS + 0x00001388 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO140_143               (SIUL_BASEADDRESS+0x0000138C) 
#define SIUL_X_GPDO140_143(x)          ((SIUL_BASEADDRESS + 0x0000138C + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO144_147               (SIUL_BASEADDRESS+0x00001390) 
#define SIUL_X_GPDO144_147(x)          ((SIUL_BASEADDRESS + 0x00001390 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO148_151               (SIUL_BASEADDRESS+0x00001394) 
#define SIUL_X_GPDO148_151(x)          ((SIUL_BASEADDRESS + 0x00001394 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO152_155               (SIUL_BASEADDRESS+0x00001398) 
#define SIUL_X_GPDO152_155(x)          ((SIUL_BASEADDRESS + 0x00001398 + ((x)*0x8000)))

/* GPDO0n_3n - GPIO Pad Data Output Reg */

#define SIUL_GPDO156_159               (SIUL_BASEADDRESS+0x0000139C) 
#define SIUL_X_GPDO156_159(x)          ((SIUL_BASEADDRESS + 0x0000139C + ((x)*0x8000)))

/* GPDO160_163 - GPIO Pad Data Output Reg */

#define SIUL_GPDO160_163               (SIUL_BASEADDRESS+0x000013A0) 
#define SIUL_X_GPDO160_163(x)          ((SIUL_BASEADDRESS + 0x000013A0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */
#define SIUL_GPDO(x)         (*(vuint8_t *) ((SIUL_BASEADDRESS+0x00001300) +((x)*0x1)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO0                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001300)) 
#define SIUL_X_GPDO0(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001300 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO1                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001301)) 
#define SIUL_X_GPDO1(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001301 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO2                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001302)) 
#define SIUL_X_GPDO2(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001302 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO3                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001303)) 
#define SIUL_X_GPDO3(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001303 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO4                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001304)) 
#define SIUL_X_GPDO4(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001304 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO5                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001305)) 
#define SIUL_X_GPDO5(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001305 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO6                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001306)) 
#define SIUL_X_GPDO6(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001306 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO7                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001307)) 
#define SIUL_X_GPDO7(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001307 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO8                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001308)) 
#define SIUL_X_GPDO8(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001308 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO9                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001309)) 
#define SIUL_X_GPDO9(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001309 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO10                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130A)) 
#define SIUL_X_GPDO10(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO11                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130B)) 
#define SIUL_X_GPDO11(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO12                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130C)) 
#define SIUL_X_GPDO12(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO13                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130D)) 
#define SIUL_X_GPDO13(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO14                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130E)) 
#define SIUL_X_GPDO14(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO15                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000130F)) 
#define SIUL_X_GPDO15(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000130F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO16                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001310)) 
#define SIUL_X_GPDO16(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001310 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO17                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001311)) 
#define SIUL_X_GPDO17(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001311 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO18                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001312)) 
#define SIUL_X_GPDO18(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001312 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO19                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001313)) 
#define SIUL_X_GPDO19(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001313 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO20                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001314)) 
#define SIUL_X_GPDO20(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001314 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO21                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001315)) 
#define SIUL_X_GPDO21(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001315 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO22                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001316)) 
#define SIUL_X_GPDO22(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001316 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO23                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001317)) 
#define SIUL_X_GPDO23(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001317 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO24                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001318)) 
#define SIUL_X_GPDO24(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001318 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO25                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001319)) 
#define SIUL_X_GPDO25(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001319 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO26                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131A)) 
#define SIUL_X_GPDO26(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO27                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131B)) 
#define SIUL_X_GPDO27(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO28                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131C)) 
#define SIUL_X_GPDO28(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO29                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131D)) 
#define SIUL_X_GPDO29(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO30                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131E)) 
#define SIUL_X_GPDO30(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO31                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000131F)) 
#define SIUL_X_GPDO31(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000131F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO32                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001320)) 
#define SIUL_X_GPDO32(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001320 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO33                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001321)) 
#define SIUL_X_GPDO33(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001321 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO34                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001322)) 
#define SIUL_X_GPDO34(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001322 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO35                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001323)) 
#define SIUL_X_GPDO35(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001323 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO36                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001324)) 
#define SIUL_X_GPDO36(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001324 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO37                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001325)) 
#define SIUL_X_GPDO37(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001325 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO38                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001326)) 
#define SIUL_X_GPDO38(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001326 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO39                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001327)) 
#define SIUL_X_GPDO39(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001327 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO40                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001328)) 
#define SIUL_X_GPDO40(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001328 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO41                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001329)) 
#define SIUL_X_GPDO41(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001329 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO42                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132A)) 
#define SIUL_X_GPDO42(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO43                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132B)) 
#define SIUL_X_GPDO43(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO44                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132C)) 
#define SIUL_X_GPDO44(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO45                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132D)) 
#define SIUL_X_GPDO45(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO46                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132E)) 
#define SIUL_X_GPDO46(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO47                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000132F)) 
#define SIUL_X_GPDO47(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000132F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO48                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001330)) 
#define SIUL_X_GPDO48(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001330 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO49                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001331)) 
#define SIUL_X_GPDO49(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001331 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO50                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001332)) 
#define SIUL_X_GPDO50(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001332 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO51                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001333)) 
#define SIUL_X_GPDO51(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001333 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO52                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001334)) 
#define SIUL_X_GPDO52(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001334 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO53                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001335)) 
#define SIUL_X_GPDO53(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001335 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO54                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001336)) 
#define SIUL_X_GPDO54(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001336 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO55                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001337)) 
#define SIUL_X_GPDO55(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001337 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO56                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001338)) 
#define SIUL_X_GPDO56(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001338 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO57                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001339)) 
#define SIUL_X_GPDO57(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001339 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO58                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133A)) 
#define SIUL_X_GPDO58(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO59                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133B)) 
#define SIUL_X_GPDO59(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO60                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133C)) 
#define SIUL_X_GPDO60(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO61                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133D)) 
#define SIUL_X_GPDO61(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO62                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133E)) 
#define SIUL_X_GPDO62(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO63                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000133F)) 
#define SIUL_X_GPDO63(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000133F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO64                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001340)) 
#define SIUL_X_GPDO64(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001340 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO65                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001341)) 
#define SIUL_X_GPDO65(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001341 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO66                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001342)) 
#define SIUL_X_GPDO66(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001342 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO67                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001343)) 
#define SIUL_X_GPDO67(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001343 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO68                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001344)) 
#define SIUL_X_GPDO68(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001344 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO69                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001345)) 
#define SIUL_X_GPDO69(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001345 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO70                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001346)) 
#define SIUL_X_GPDO70(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001346 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO71                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001347)) 
#define SIUL_X_GPDO71(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001347 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO72                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001348)) 
#define SIUL_X_GPDO72(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001348 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO73                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001349)) 
#define SIUL_X_GPDO73(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001349 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO74                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134A)) 
#define SIUL_X_GPDO74(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO75                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134B)) 
#define SIUL_X_GPDO75(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO76                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134C)) 
#define SIUL_X_GPDO76(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO77                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134D)) 
#define SIUL_X_GPDO77(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO78                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134E)) 
#define SIUL_X_GPDO78(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO79                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000134F)) 
#define SIUL_X_GPDO79(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000134F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO80                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001350)) 
#define SIUL_X_GPDO80(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001350 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO81                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001351)) 
#define SIUL_X_GPDO81(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001351 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO82                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001352)) 
#define SIUL_X_GPDO82(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001352 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO83                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001353)) 
#define SIUL_X_GPDO83(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001353 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO84                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001354)) 
#define SIUL_X_GPDO84(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001354 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO85                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001355)) 
#define SIUL_X_GPDO85(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001355 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO86                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001356)) 
#define SIUL_X_GPDO86(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001356 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO87                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001357)) 
#define SIUL_X_GPDO87(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001357 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO88                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001358)) 
#define SIUL_X_GPDO88(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001358 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO89                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001359)) 
#define SIUL_X_GPDO89(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001359 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO90                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135A)) 
#define SIUL_X_GPDO90(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO91                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135B)) 
#define SIUL_X_GPDO91(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO92                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135C)) 
#define SIUL_X_GPDO92(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO93                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135D)) 
#define SIUL_X_GPDO93(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO94                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135E)) 
#define SIUL_X_GPDO94(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO95                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000135F)) 
#define SIUL_X_GPDO95(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000135F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO96                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001360)) 
#define SIUL_X_GPDO96(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001360 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO97                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001361)) 
#define SIUL_X_GPDO97(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001361 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO98                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001362)) 
#define SIUL_X_GPDO98(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001362 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO99                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001363)) 
#define SIUL_X_GPDO99(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001363 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO100                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001364)) 
#define SIUL_X_GPDO100(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001364 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO101                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001365)) 
#define SIUL_X_GPDO101(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001365 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO102                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001366)) 
#define SIUL_X_GPDO102(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001366 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO103                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001367)) 
#define SIUL_X_GPDO103(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001367 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO104                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001368)) 
#define SIUL_X_GPDO104(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001368 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO105                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001369)) 
#define SIUL_X_GPDO105(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001369 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO106                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136A)) 
#define SIUL_X_GPDO106(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO107                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136B)) 
#define SIUL_X_GPDO107(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO108                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136C)) 
#define SIUL_X_GPDO108(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO109                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136D)) 
#define SIUL_X_GPDO109(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO110                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136E)) 
#define SIUL_X_GPDO110(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO111                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000136F)) 
#define SIUL_X_GPDO111(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000136F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO112                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001370)) 
#define SIUL_X_GPDO112(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001370 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO113                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001371)) 
#define SIUL_X_GPDO113(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001371 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO114                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001372)) 
#define SIUL_X_GPDO114(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001372 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO115                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001373)) 
#define SIUL_X_GPDO115(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001373 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO116                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001374)) 
#define SIUL_X_GPDO116(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001374 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO117                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001375)) 
#define SIUL_X_GPDO117(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001375 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO118                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001376)) 
#define SIUL_X_GPDO118(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001376 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO119                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001377)) 
#define SIUL_X_GPDO119(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001377 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO120                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001378)) 
#define SIUL_X_GPDO120(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001378 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO121                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001379)) 
#define SIUL_X_GPDO121(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001379 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO122                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137A)) 
#define SIUL_X_GPDO122(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO123                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137B)) 
#define SIUL_X_GPDO123(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO124                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137C)) 
#define SIUL_X_GPDO124(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO125                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137D)) 
#define SIUL_X_GPDO125(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO126                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137E)) 
#define SIUL_X_GPDO126(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO127                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000137F)) 
#define SIUL_X_GPDO127(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000137F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO128                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001380)) 
#define SIUL_X_GPDO128(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001380 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO129                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001381)) 
#define SIUL_X_GPDO129(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001381 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO130                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001382)) 
#define SIUL_X_GPDO130(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001382 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO131                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001383)) 
#define SIUL_X_GPDO131(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001383 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO132                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001384)) 
#define SIUL_X_GPDO132(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001384 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO133                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001385)) 
#define SIUL_X_GPDO133(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001385 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO134                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001386)) 
#define SIUL_X_GPDO134(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001386 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO135                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001387)) 
#define SIUL_X_GPDO135(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001387 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO136                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001388)) 
#define SIUL_X_GPDO136(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001388 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO137                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001389)) 
#define SIUL_X_GPDO137(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001389 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO138                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138A)) 
#define SIUL_X_GPDO138(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO139                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138B)) 
#define SIUL_X_GPDO139(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO140                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138C)) 
#define SIUL_X_GPDO140(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO141                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138D)) 
#define SIUL_X_GPDO141(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO142                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138E)) 
#define SIUL_X_GPDO142(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO143                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000138F)) 
#define SIUL_X_GPDO143(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000138F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO144                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001390)) 
#define SIUL_X_GPDO144(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001390 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO145                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001391)) 
#define SIUL_X_GPDO145(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001391 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO146                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001392)) 
#define SIUL_X_GPDO146(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001392 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO147                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001393)) 
#define SIUL_X_GPDO147(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001393 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO148                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001394)) 
#define SIUL_X_GPDO148(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001394 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO149                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001395)) 
#define SIUL_X_GPDO149(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001395 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO150                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001396)) 
#define SIUL_X_GPDO150(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001396 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO151                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001397)) 
#define SIUL_X_GPDO151(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001397 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO152                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001398)) 
#define SIUL_X_GPDO152(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001398 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO153                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001399)) 
#define SIUL_X_GPDO153(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001399 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO154                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139A)) 
#define SIUL_X_GPDO154(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO155                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139B)) 
#define SIUL_X_GPDO155(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO156                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139C)) 
#define SIUL_X_GPDO156(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO157                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139D)) 
#define SIUL_X_GPDO157(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO158                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139E)) 
#define SIUL_X_GPDO158(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO159                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000139F)) 
#define SIUL_X_GPDO159(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000139F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO160                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A0)) 
#define SIUL_X_GPDO160(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO161                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A1)) 
#define SIUL_X_GPDO161(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO162                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A2)) 
#define SIUL_X_GPDO162(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO163                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A3)) 
#define SIUL_X_GPDO163(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO164                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A4)) 
#define SIUL_X_GPDO164(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO165                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A5)) 
#define SIUL_X_GPDO165(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO166                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A6)) 
#define SIUL_X_GPDO166(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO167                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A7)) 
#define SIUL_X_GPDO167(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO168                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A8)) 
#define SIUL_X_GPDO168(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO169                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013A9)) 
#define SIUL_X_GPDO169(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013A9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO170                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AA)) 
#define SIUL_X_GPDO170(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO171                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AB)) 
#define SIUL_X_GPDO171(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO172                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AC)) 
#define SIUL_X_GPDO172(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO173                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AD)) 
#define SIUL_X_GPDO173(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO174                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AE)) 
#define SIUL_X_GPDO174(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO175                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013AF)) 
#define SIUL_X_GPDO175(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013AF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO176                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B0)) 
#define SIUL_X_GPDO176(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO177                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B1)) 
#define SIUL_X_GPDO177(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO178                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B2)) 
#define SIUL_X_GPDO178(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO179                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B3)) 
#define SIUL_X_GPDO179(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO180                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B4)) 
#define SIUL_X_GPDO180(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO181                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B5)) 
#define SIUL_X_GPDO181(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO182                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B6)) 
#define SIUL_X_GPDO182(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO183                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B7)) 
#define SIUL_X_GPDO183(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO184                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B8)) 
#define SIUL_X_GPDO184(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO185                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013B9)) 
#define SIUL_X_GPDO185(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013B9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO186                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BA)) 
#define SIUL_X_GPDO186(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO187                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BB)) 
#define SIUL_X_GPDO187(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO188                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BC)) 
#define SIUL_X_GPDO188(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO189                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BD)) 
#define SIUL_X_GPDO189(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO190                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BE)) 
#define SIUL_X_GPDO190(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO191                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013BF)) 
#define SIUL_X_GPDO191(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013BF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO192                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C0)) 
#define SIUL_X_GPDO192(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO193                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C1)) 
#define SIUL_X_GPDO193(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO194                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C2)) 
#define SIUL_X_GPDO194(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO195                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C3)) 
#define SIUL_X_GPDO195(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO196                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C4)) 
#define SIUL_X_GPDO196(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO197                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C5)) 
#define SIUL_X_GPDO197(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO198                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C6)) 
#define SIUL_X_GPDO198(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO199                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C7)) 
#define SIUL_X_GPDO199(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO200                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C8)) 
#define SIUL_X_GPDO200(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO201                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013C9)) 
#define SIUL_X_GPDO201(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013C9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO202                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CA)) 
#define SIUL_X_GPDO202(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO203                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CB)) 
#define SIUL_X_GPDO203(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO204                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CC)) 
#define SIUL_X_GPDO204(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO205                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CD)) 
#define SIUL_X_GPDO205(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO206                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CE)) 
#define SIUL_X_GPDO206(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO207                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013CF)) 
#define SIUL_X_GPDO207(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013CF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO208                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D0)) 
#define SIUL_X_GPDO208(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO209                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D1)) 
#define SIUL_X_GPDO209(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO210                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D2)) 
#define SIUL_X_GPDO210(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO211                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D3)) 
#define SIUL_X_GPDO211(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO212                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D4)) 
#define SIUL_X_GPDO212(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO213                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D5)) 
#define SIUL_X_GPDO213(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO214                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D6)) 
#define SIUL_X_GPDO214(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO215                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D7)) 
#define SIUL_X_GPDO215(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO216                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D8)) 
#define SIUL_X_GPDO216(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO217                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013D9)) 
#define SIUL_X_GPDO217(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013D9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO218                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DA)) 
#define SIUL_X_GPDO218(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO219                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DB)) 
#define SIUL_X_GPDO219(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO220                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DC)) 
#define SIUL_X_GPDO220(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO221                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DD)) 
#define SIUL_X_GPDO221(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO222                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DE)) 
#define SIUL_X_GPDO222(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO223                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013DF)) 
#define SIUL_X_GPDO223(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013DF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO224                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E0)) 
#define SIUL_X_GPDO224(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO225                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E1)) 
#define SIUL_X_GPDO225(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO226                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E2)) 
#define SIUL_X_GPDO226(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO227                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E3)) 
#define SIUL_X_GPDO227(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO228                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E4)) 
#define SIUL_X_GPDO228(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO229                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E5)) 
#define SIUL_X_GPDO229(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO230                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E6)) 
#define SIUL_X_GPDO230(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO231                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E7)) 
#define SIUL_X_GPDO231(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO232                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E8)) 
#define SIUL_X_GPDO232(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO233                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013E9)) 
#define SIUL_X_GPDO233(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013E9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO234                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013EA)) 
#define SIUL_X_GPDO234(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013EA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO235                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013EB)) 
#define SIUL_X_GPDO235(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013EB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO236                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013EC)) 
#define SIUL_X_GPDO236(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013EC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO237                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013ED)) 
#define SIUL_X_GPDO237(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013ED + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO238                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013EE)) 
#define SIUL_X_GPDO238(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013EE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO239                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013EF)) 
#define SIUL_X_GPDO239(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013EF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO240                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F0)) 
#define SIUL_X_GPDO240(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO241                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F1)) 
#define SIUL_X_GPDO241(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO242                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F2)) 
#define SIUL_X_GPDO242(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO243                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F3)) 
#define SIUL_X_GPDO243(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO244                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F4)) 
#define SIUL_X_GPDO244(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO245                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F5)) 
#define SIUL_X_GPDO245(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO246                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F6)) 
#define SIUL_X_GPDO246(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO247                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F7)) 
#define SIUL_X_GPDO247(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO248                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F8)) 
#define SIUL_X_GPDO248(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO249                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013F9)) 
#define SIUL_X_GPDO249(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013F9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO250                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FA)) 
#define SIUL_X_GPDO250(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO251                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FB)) 
#define SIUL_X_GPDO251(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO252                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FC)) 
#define SIUL_X_GPDO252(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO253                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FD)) 
#define SIUL_X_GPDO253(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO254                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FE)) 
#define SIUL_X_GPDO254(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO255                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000013FF)) 
#define SIUL_X_GPDO255(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000013FF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO256                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001400)) 
#define SIUL_X_GPDO256(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001400 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO257                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001401)) 
#define SIUL_X_GPDO257(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001401 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO258                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001402)) 
#define SIUL_X_GPDO258(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001402 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO259                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001403)) 
#define SIUL_X_GPDO259(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001403 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO260                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001404)) 
#define SIUL_X_GPDO260(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001404 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO261                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001405)) 
#define SIUL_X_GPDO261(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001405 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO262                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001406)) 
#define SIUL_X_GPDO262(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001406 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO263                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001407)) 
#define SIUL_X_GPDO263(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001407 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO264                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001408)) 
#define SIUL_X_GPDO264(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001408 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO265                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001409)) 
#define SIUL_X_GPDO265(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001409 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO266                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140A)) 
#define SIUL_X_GPDO266(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO267                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140B)) 
#define SIUL_X_GPDO267(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO268                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140C)) 
#define SIUL_X_GPDO268(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO269                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140D)) 
#define SIUL_X_GPDO269(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO270                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140E)) 
#define SIUL_X_GPDO270(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO271                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000140F)) 
#define SIUL_X_GPDO271(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000140F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO272                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001410)) 
#define SIUL_X_GPDO272(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001410 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO273                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001411)) 
#define SIUL_X_GPDO273(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001411 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO274                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001412)) 
#define SIUL_X_GPDO274(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001412 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO275                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001413)) 
#define SIUL_X_GPDO275(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001413 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO276                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001414)) 
#define SIUL_X_GPDO276(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001414 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO277                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001415)) 
#define SIUL_X_GPDO277(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001415 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO278                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001416)) 
#define SIUL_X_GPDO278(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001416 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO279                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001417)) 
#define SIUL_X_GPDO279(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001417 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO280                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001418)) 
#define SIUL_X_GPDO280(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001418 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO281                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001419)) 
#define SIUL_X_GPDO281(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001419 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO282                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141A)) 
#define SIUL_X_GPDO282(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO283                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141B)) 
#define SIUL_X_GPDO283(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO284                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141C)) 
#define SIUL_X_GPDO284(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO285                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141D)) 
#define SIUL_X_GPDO285(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO286                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141E)) 
#define SIUL_X_GPDO286(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO287                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000141F)) 
#define SIUL_X_GPDO287(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000141F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO288                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001420)) 
#define SIUL_X_GPDO288(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001420 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO289                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001421)) 
#define SIUL_X_GPDO289(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001421 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO290                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001422)) 
#define SIUL_X_GPDO290(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001422 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO291                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001423)) 
#define SIUL_X_GPDO291(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001423 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO292                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001424)) 
#define SIUL_X_GPDO292(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001424 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO293                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001425)) 
#define SIUL_X_GPDO293(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001425 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO294                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001426)) 
#define SIUL_X_GPDO294(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001426 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO295                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001427)) 
#define SIUL_X_GPDO295(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001427 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO296                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001428)) 
#define SIUL_X_GPDO296(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001428 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO297                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001429)) 
#define SIUL_X_GPDO297(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001429 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO298                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142A)) 
#define SIUL_X_GPDO298(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO299                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142B)) 
#define SIUL_X_GPDO299(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO300                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142C)) 
#define SIUL_X_GPDO300(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO301                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142D)) 
#define SIUL_X_GPDO301(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO302                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142E)) 
#define SIUL_X_GPDO302(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO303                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000142F)) 
#define SIUL_X_GPDO303(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000142F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO304                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001430)) 
#define SIUL_X_GPDO304(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001430 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO305                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001431)) 
#define SIUL_X_GPDO305(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001431 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO306                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001432)) 
#define SIUL_X_GPDO306(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001432 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO307                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001433)) 
#define SIUL_X_GPDO307(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001433 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO308                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001434)) 
#define SIUL_X_GPDO308(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001434 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO309                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001435)) 
#define SIUL_X_GPDO309(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001435 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO310                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001436)) 
#define SIUL_X_GPDO310(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001436 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO311                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001437)) 
#define SIUL_X_GPDO311(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001437 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO312                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001438)) 
#define SIUL_X_GPDO312(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001438 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO313                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001439)) 
#define SIUL_X_GPDO313(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001439 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO314                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143A)) 
#define SIUL_X_GPDO314(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO315                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143B)) 
#define SIUL_X_GPDO315(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO316                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143C)) 
#define SIUL_X_GPDO316(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO317                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143D)) 
#define SIUL_X_GPDO317(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO318                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143E)) 
#define SIUL_X_GPDO318(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO319                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000143F)) 
#define SIUL_X_GPDO319(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000143F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO320                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001440)) 
#define SIUL_X_GPDO320(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001440 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO321                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001441)) 
#define SIUL_X_GPDO321(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001441 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO322                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001442)) 
#define SIUL_X_GPDO322(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001442 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO323                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001443)) 
#define SIUL_X_GPDO323(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001443 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO324                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001444)) 
#define SIUL_X_GPDO324(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001444 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO325                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001445)) 
#define SIUL_X_GPDO325(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001445 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO326                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001446)) 
#define SIUL_X_GPDO326(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001446 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO327                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001447)) 
#define SIUL_X_GPDO327(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001447 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO328                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001448)) 
#define SIUL_X_GPDO328(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001448 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO329                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001449)) 
#define SIUL_X_GPDO329(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001449 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO330                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144A)) 
#define SIUL_X_GPDO330(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO331                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144B)) 
#define SIUL_X_GPDO331(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO332                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144C)) 
#define SIUL_X_GPDO332(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO333                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144D)) 
#define SIUL_X_GPDO333(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO334                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144E)) 
#define SIUL_X_GPDO334(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO335                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000144F)) 
#define SIUL_X_GPDO335(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000144F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO336                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001450)) 
#define SIUL_X_GPDO336(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001450 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO337                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001451)) 
#define SIUL_X_GPDO337(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001451 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO338                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001452)) 
#define SIUL_X_GPDO338(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001452 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO339                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001453)) 
#define SIUL_X_GPDO339(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001453 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO340                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001454)) 
#define SIUL_X_GPDO340(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001454 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO341                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001455)) 
#define SIUL_X_GPDO341(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001455 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO342                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001456)) 
#define SIUL_X_GPDO342(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001456 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO343                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001457)) 
#define SIUL_X_GPDO343(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001457 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO344                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001458)) 
#define SIUL_X_GPDO344(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001458 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO345                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001459)) 
#define SIUL_X_GPDO345(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001459 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO346                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145A)) 
#define SIUL_X_GPDO346(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO347                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145B)) 
#define SIUL_X_GPDO347(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO348                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145C)) 
#define SIUL_X_GPDO348(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO349                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145D)) 
#define SIUL_X_GPDO349(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO350                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145E)) 
#define SIUL_X_GPDO350(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO351                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000145F)) 
#define SIUL_X_GPDO351(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000145F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO352                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001460)) 
#define SIUL_X_GPDO352(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001460 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO353                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001461)) 
#define SIUL_X_GPDO353(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001461 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO354                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001462)) 
#define SIUL_X_GPDO354(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001462 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO355                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001463)) 
#define SIUL_X_GPDO355(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001463 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO356                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001464)) 
#define SIUL_X_GPDO356(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001464 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO357                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001465)) 
#define SIUL_X_GPDO357(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001465 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO358                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001466)) 
#define SIUL_X_GPDO358(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001466 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO359                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001467)) 
#define SIUL_X_GPDO359(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001467 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO360                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001468)) 
#define SIUL_X_GPDO360(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001468 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO361                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001469)) 
#define SIUL_X_GPDO361(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001469 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO362                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146A)) 
#define SIUL_X_GPDO362(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO363                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146B)) 
#define SIUL_X_GPDO363(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO364                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146C)) 
#define SIUL_X_GPDO364(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO365                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146D)) 
#define SIUL_X_GPDO365(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO366                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146E)) 
#define SIUL_X_GPDO366(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO367                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000146F)) 
#define SIUL_X_GPDO367(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000146F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO368                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001470)) 
#define SIUL_X_GPDO368(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001470 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO369                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001471)) 
#define SIUL_X_GPDO369(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001471 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO370                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001472)) 
#define SIUL_X_GPDO370(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001472 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO371                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001473)) 
#define SIUL_X_GPDO371(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001473 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO372                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001474)) 
#define SIUL_X_GPDO372(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001474 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO373                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001475)) 
#define SIUL_X_GPDO373(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001475 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO374                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001476)) 
#define SIUL_X_GPDO374(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001476 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO375                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001477)) 
#define SIUL_X_GPDO375(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001477 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO376                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001478)) 
#define SIUL_X_GPDO376(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001478 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO377                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001479)) 
#define SIUL_X_GPDO377(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001479 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO378                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147A)) 
#define SIUL_X_GPDO378(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO379                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147B)) 
#define SIUL_X_GPDO379(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO380                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147C)) 
#define SIUL_X_GPDO380(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO381                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147D)) 
#define SIUL_X_GPDO381(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO382                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147E)) 
#define SIUL_X_GPDO382(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO383                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000147F)) 
#define SIUL_X_GPDO383(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000147F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO384                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001480)) 
#define SIUL_X_GPDO384(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001480 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO385                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001481)) 
#define SIUL_X_GPDO385(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001481 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO386                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001482)) 
#define SIUL_X_GPDO386(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001482 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO387                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001483)) 
#define SIUL_X_GPDO387(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001483 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO388                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001484)) 
#define SIUL_X_GPDO388(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001484 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO389                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001485)) 
#define SIUL_X_GPDO389(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001485 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO390                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001486)) 
#define SIUL_X_GPDO390(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001486 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO391                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001487)) 
#define SIUL_X_GPDO391(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001487 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO392                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001488)) 
#define SIUL_X_GPDO392(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001488 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO393                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001489)) 
#define SIUL_X_GPDO393(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001489 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO394                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148A)) 
#define SIUL_X_GPDO394(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO395                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148B)) 
#define SIUL_X_GPDO395(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO396                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148C)) 
#define SIUL_X_GPDO396(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO397                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148D)) 
#define SIUL_X_GPDO397(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO398                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148E)) 
#define SIUL_X_GPDO398(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO399                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000148F)) 
#define SIUL_X_GPDO399(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000148F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO400                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001490)) 
#define SIUL_X_GPDO400(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001490 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO401                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001491)) 
#define SIUL_X_GPDO401(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001491 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO402                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001492)) 
#define SIUL_X_GPDO402(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001492 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO403                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001493)) 
#define SIUL_X_GPDO403(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001493 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO404                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001494)) 
#define SIUL_X_GPDO404(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001494 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO405                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001495)) 
#define SIUL_X_GPDO405(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001495 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO406                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001496)) 
#define SIUL_X_GPDO406(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001496 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO407                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001497)) 
#define SIUL_X_GPDO407(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001497 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO408                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001498)) 
#define SIUL_X_GPDO408(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001498 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO409                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001499)) 
#define SIUL_X_GPDO409(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001499 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO410                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149A)) 
#define SIUL_X_GPDO410(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149A + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO411                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149B)) 
#define SIUL_X_GPDO411(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149B + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO412                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149C)) 
#define SIUL_X_GPDO412(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149C + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO413                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149D)) 
#define SIUL_X_GPDO413(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149D + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO414                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149E)) 
#define SIUL_X_GPDO414(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149E + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO415                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000149F)) 
#define SIUL_X_GPDO415(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000149F + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO416                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A0)) 
#define SIUL_X_GPDO416(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO417                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A1)) 
#define SIUL_X_GPDO417(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO418                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A2)) 
#define SIUL_X_GPDO418(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO419                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A3)) 
#define SIUL_X_GPDO419(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO420                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A4)) 
#define SIUL_X_GPDO420(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO421                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A5)) 
#define SIUL_X_GPDO421(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO422                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A6)) 
#define SIUL_X_GPDO422(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO423                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A7)) 
#define SIUL_X_GPDO423(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO424                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A8)) 
#define SIUL_X_GPDO424(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO425                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014A9)) 
#define SIUL_X_GPDO425(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014A9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO426                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AA)) 
#define SIUL_X_GPDO426(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO427                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AB)) 
#define SIUL_X_GPDO427(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO428                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AC)) 
#define SIUL_X_GPDO428(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO429                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AD)) 
#define SIUL_X_GPDO429(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO430                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AE)) 
#define SIUL_X_GPDO430(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO431                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014AF)) 
#define SIUL_X_GPDO431(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014AF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO432                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B0)) 
#define SIUL_X_GPDO432(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO433                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B1)) 
#define SIUL_X_GPDO433(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO434                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B2)) 
#define SIUL_X_GPDO434(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO435                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B3)) 
#define SIUL_X_GPDO435(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO436                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B4)) 
#define SIUL_X_GPDO436(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO437                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B5)) 
#define SIUL_X_GPDO437(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO438                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B6)) 
#define SIUL_X_GPDO438(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO439                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B7)) 
#define SIUL_X_GPDO439(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO440                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B8)) 
#define SIUL_X_GPDO440(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO441                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014B9)) 
#define SIUL_X_GPDO441(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014B9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO442                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BA)) 
#define SIUL_X_GPDO442(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO443                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BB)) 
#define SIUL_X_GPDO443(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO444                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BC)) 
#define SIUL_X_GPDO444(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO445                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BD)) 
#define SIUL_X_GPDO445(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO446                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BE)) 
#define SIUL_X_GPDO446(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO447                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014BF)) 
#define SIUL_X_GPDO447(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014BF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO448                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C0)) 
#define SIUL_X_GPDO448(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO449                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C1)) 
#define SIUL_X_GPDO449(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO450                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C2)) 
#define SIUL_X_GPDO450(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO451                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C3)) 
#define SIUL_X_GPDO451(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO452                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C4)) 
#define SIUL_X_GPDO452(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO453                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C5)) 
#define SIUL_X_GPDO453(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO454                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C6)) 
#define SIUL_X_GPDO454(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO455                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C7)) 
#define SIUL_X_GPDO455(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO456                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C8)) 
#define SIUL_X_GPDO456(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO457                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014C9)) 
#define SIUL_X_GPDO457(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014C9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO458                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CA)) 
#define SIUL_X_GPDO458(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO459                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CB)) 
#define SIUL_X_GPDO459(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO460                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CC)) 
#define SIUL_X_GPDO460(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO461                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CD)) 
#define SIUL_X_GPDO461(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO462                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CE)) 
#define SIUL_X_GPDO462(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO463                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014CF)) 
#define SIUL_X_GPDO463(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014CF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO464                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D0)) 
#define SIUL_X_GPDO464(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO465                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D1)) 
#define SIUL_X_GPDO465(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO466                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D2)) 
#define SIUL_X_GPDO466(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO467                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D3)) 
#define SIUL_X_GPDO467(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO468                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D4)) 
#define SIUL_X_GPDO468(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO469                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D5)) 
#define SIUL_X_GPDO469(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO470                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D6)) 
#define SIUL_X_GPDO470(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO471                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D7)) 
#define SIUL_X_GPDO471(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO472                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D8)) 
#define SIUL_X_GPDO472(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO473                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014D9)) 
#define SIUL_X_GPDO473(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014D9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO474                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DA)) 
#define SIUL_X_GPDO474(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO475                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DB)) 
#define SIUL_X_GPDO475(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO476                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DC)) 
#define SIUL_X_GPDO476(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO477                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DD)) 
#define SIUL_X_GPDO477(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO478                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DE)) 
#define SIUL_X_GPDO478(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO479                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014DF)) 
#define SIUL_X_GPDO479(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014DF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO480                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E0)) 
#define SIUL_X_GPDO480(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO481                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E1)) 
#define SIUL_X_GPDO481(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO482                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E2)) 
#define SIUL_X_GPDO482(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO483                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E3)) 
#define SIUL_X_GPDO483(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO484                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E4)) 
#define SIUL_X_GPDO484(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO485                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E5)) 
#define SIUL_X_GPDO485(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO486                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E6)) 
#define SIUL_X_GPDO486(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO487                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E7)) 
#define SIUL_X_GPDO487(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO488                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E8)) 
#define SIUL_X_GPDO488(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO489                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014E9)) 
#define SIUL_X_GPDO489(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014E9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO490                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014EA)) 
#define SIUL_X_GPDO490(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014EA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO491                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014EB)) 
#define SIUL_X_GPDO491(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014EB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO492                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014EC)) 
#define SIUL_X_GPDO492(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014EC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO493                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014ED)) 
#define SIUL_X_GPDO493(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014ED + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO494                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014EE)) 
#define SIUL_X_GPDO494(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014EE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO495                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014EF)) 
#define SIUL_X_GPDO495(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014EF + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO496                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F0)) 
#define SIUL_X_GPDO496(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F0 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO497                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F1)) 
#define SIUL_X_GPDO497(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F1 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO498                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F2)) 
#define SIUL_X_GPDO498(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F2 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO499                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F3)) 
#define SIUL_X_GPDO499(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F3 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO500                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F4)) 
#define SIUL_X_GPDO500(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F4 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO501                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F5)) 
#define SIUL_X_GPDO501(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F5 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO502                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F6)) 
#define SIUL_X_GPDO502(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F6 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO503                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F7)) 
#define SIUL_X_GPDO503(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F7 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO504                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F8)) 
#define SIUL_X_GPDO504(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F8 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO505                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014F9)) 
#define SIUL_X_GPDO505(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014F9 + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO506                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FA)) 
#define SIUL_X_GPDO506(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FA + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO507                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FB)) 
#define SIUL_X_GPDO507(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FB + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO508                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FC)) 
#define SIUL_X_GPDO508(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FC + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO509                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FD)) 
#define SIUL_X_GPDO509(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FD + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO510                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FE)) 
#define SIUL_X_GPDO510(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FE + ((x)*0x8000)))

/* GPDOn - GPIO Pad Data Output Reg */

#define SIUL_GPDO511                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x000014FF)) 
#define SIUL_X_GPDO511(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x000014FF + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI0_3                   (SIUL_BASEADDRESS+0x00001500) 
#define SIUL_X_GPDI0_3(x)              ((SIUL_BASEADDRESS + 0x00001500 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI4_7                   (SIUL_BASEADDRESS+0x00001504) 
#define SIUL_X_GPDI4_7(x)              ((SIUL_BASEADDRESS + 0x00001504 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI8_11                  (SIUL_BASEADDRESS+0x00001508) 
#define SIUL_X_GPDI8_11(x)             ((SIUL_BASEADDRESS + 0x00001508 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI12_15                 (SIUL_BASEADDRESS+0x0000150C) 
#define SIUL_X_GPDI12_15(x)            ((SIUL_BASEADDRESS + 0x0000150C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI16_19                 (SIUL_BASEADDRESS+0x00001510) 
#define SIUL_X_GPDI16_19(x)            ((SIUL_BASEADDRESS + 0x00001510 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI20_23                 (SIUL_BASEADDRESS+0x00001514) 
#define SIUL_X_GPDI20_23(x)            ((SIUL_BASEADDRESS + 0x00001514 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI24_27                 (SIUL_BASEADDRESS+0x00001518) 
#define SIUL_X_GPDI24_27(x)            ((SIUL_BASEADDRESS + 0x00001518 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI28_31                 (SIUL_BASEADDRESS+0x0000151C) 
#define SIUL_X_GPDI28_31(x)            ((SIUL_BASEADDRESS + 0x0000151C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI32_35                 (SIUL_BASEADDRESS+0x00001520) 
#define SIUL_X_GPDI32_35(x)            ((SIUL_BASEADDRESS + 0x00001520 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI36_39                 (SIUL_BASEADDRESS+0x00001524) 
#define SIUL_X_GPDI36_39(x)            ((SIUL_BASEADDRESS + 0x00001524 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI40_43                 (SIUL_BASEADDRESS+0x00001528) 
#define SIUL_X_GPDI40_43(x)            ((SIUL_BASEADDRESS + 0x00001528 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI44_47                 (SIUL_BASEADDRESS+0x0000152C) 
#define SIUL_X_GPDI44_47(x)            ((SIUL_BASEADDRESS + 0x0000152C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI48_51                 (SIUL_BASEADDRESS+0x00001530) 
#define SIUL_X_GPDI48_51(x)            ((SIUL_BASEADDRESS + 0x00001530 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI52_55                 (SIUL_BASEADDRESS+0x00001534) 
#define SIUL_X_GPDI52_55(x)            ((SIUL_BASEADDRESS + 0x00001534 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI56_59                 (SIUL_BASEADDRESS+0x00001538) 
#define SIUL_X_GPDI56_59(x)            ((SIUL_BASEADDRESS + 0x00001538 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI60_63                 (SIUL_BASEADDRESS+0x0000153C) 
#define SIUL_X_GPDI60_63(x)            ((SIUL_BASEADDRESS + 0x0000153C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI64_67                 (SIUL_BASEADDRESS+0x00001540) 
#define SIUL_X_GPDI64_67(x)            ((SIUL_BASEADDRESS + 0x00001540 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI68_71                 (SIUL_BASEADDRESS+0x00001544) 
#define SIUL_X_GPDI68_71(x)            ((SIUL_BASEADDRESS + 0x00001544 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI72_75                 (SIUL_BASEADDRESS+0x00001548) 
#define SIUL_X_GPDI72_75(x)            ((SIUL_BASEADDRESS + 0x00001548 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI76_79                 (SIUL_BASEADDRESS+0x0000154C) 
#define SIUL_X_GPDI76_79(x)            ((SIUL_BASEADDRESS + 0x0000154C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI80_83                 (SIUL_BASEADDRESS+0x00001550) 
#define SIUL_X_GPDI80_83(x)            ((SIUL_BASEADDRESS + 0x00001550 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI84_87                 (SIUL_BASEADDRESS+0x00001554) 
#define SIUL_X_GPDI84_87(x)            ((SIUL_BASEADDRESS + 0x00001554 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI88_91                 (SIUL_BASEADDRESS+0x00001558) 
#define SIUL_X_GPDI88_91(x)            ((SIUL_BASEADDRESS + 0x00001558 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI92_95                 (SIUL_BASEADDRESS+0x0000155C) 
#define SIUL_X_GPDI92_95(x)            ((SIUL_BASEADDRESS + 0x0000155C + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI96_99                 (SIUL_BASEADDRESS+0x00001560) 
#define SIUL_X_GPDI96_99(x)            ((SIUL_BASEADDRESS + 0x00001560 + ((x)*0x8000)))

/* GPDI0n_3n - GPIO Pad Data Input Reg */

#define SIUL_GPDI100_103               (SIUL_BASEADDRESS+0x00001564) 
#define SIUL_X_GPDI100_103(x)          ((SIUL_BASEADDRESS + 0x00001564 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */
#define SIUL_GPDI(x)         (*(vuint8_t *) ((SIUL_BASEADDRESS+0x00001500) +((x)*0x1)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI0                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001500)) 
#define SIUL_X_GPDI0(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001500 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI1                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001501)) 
#define SIUL_X_GPDI1(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001501 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI2                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001502)) 
#define SIUL_X_GPDI2(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001502 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI3                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001503)) 
#define SIUL_X_GPDI3(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001503 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI4                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001504)) 
#define SIUL_X_GPDI4(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001504 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI5                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001505)) 
#define SIUL_X_GPDI5(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001505 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI6                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001506)) 
#define SIUL_X_GPDI6(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001506 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI7                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001507)) 
#define SIUL_X_GPDI7(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001507 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI8                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001508)) 
#define SIUL_X_GPDI8(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001508 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI9                     (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001509)) 
#define SIUL_X_GPDI9(x)                (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001509 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI10                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150A)) 
#define SIUL_X_GPDI10(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI11                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150B)) 
#define SIUL_X_GPDI11(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI12                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150C)) 
#define SIUL_X_GPDI12(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI13                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150D)) 
#define SIUL_X_GPDI13(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI14                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150E)) 
#define SIUL_X_GPDI14(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI15                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000150F)) 
#define SIUL_X_GPDI15(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000150F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI16                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001510)) 
#define SIUL_X_GPDI16(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001510 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI17                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001511)) 
#define SIUL_X_GPDI17(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001511 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI18                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001512)) 
#define SIUL_X_GPDI18(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001512 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI19                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001513)) 
#define SIUL_X_GPDI19(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001513 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI20                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001514)) 
#define SIUL_X_GPDI20(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001514 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI21                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001515)) 
#define SIUL_X_GPDI21(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001515 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI22                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001516)) 
#define SIUL_X_GPDI22(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001516 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI23                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001517)) 
#define SIUL_X_GPDI23(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001517 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI24                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001518)) 
#define SIUL_X_GPDI24(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001518 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI25                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001519)) 
#define SIUL_X_GPDI25(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001519 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI26                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151A)) 
#define SIUL_X_GPDI26(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI27                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151B)) 
#define SIUL_X_GPDI27(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI28                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151C)) 
#define SIUL_X_GPDI28(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI29                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151D)) 
#define SIUL_X_GPDI29(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI30                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151E)) 
#define SIUL_X_GPDI30(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI31                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000151F)) 
#define SIUL_X_GPDI31(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000151F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI32                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001520)) 
#define SIUL_X_GPDI32(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001520 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI33                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001521)) 
#define SIUL_X_GPDI33(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001521 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI34                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001522)) 
#define SIUL_X_GPDI34(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001522 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI35                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001523)) 
#define SIUL_X_GPDI35(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001523 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI36                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001524)) 
#define SIUL_X_GPDI36(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001524 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI37                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001525)) 
#define SIUL_X_GPDI37(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001525 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI38                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001526)) 
#define SIUL_X_GPDI38(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001526 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI39                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001527)) 
#define SIUL_X_GPDI39(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001527 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI40                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001528)) 
#define SIUL_X_GPDI40(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001528 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI41                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001529)) 
#define SIUL_X_GPDI41(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001529 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI42                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152A)) 
#define SIUL_X_GPDI42(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI43                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152B)) 
#define SIUL_X_GPDI43(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI44                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152C)) 
#define SIUL_X_GPDI44(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI45                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152D)) 
#define SIUL_X_GPDI45(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI46                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152E)) 
#define SIUL_X_GPDI46(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI47                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000152F)) 
#define SIUL_X_GPDI47(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000152F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI48                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001530)) 
#define SIUL_X_GPDI48(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001530 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI49                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001531)) 
#define SIUL_X_GPDI49(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001531 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI50                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001532)) 
#define SIUL_X_GPDI50(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001532 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI51                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001533)) 
#define SIUL_X_GPDI51(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001533 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI52                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001534)) 
#define SIUL_X_GPDI52(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001534 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI53                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001535)) 
#define SIUL_X_GPDI53(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001535 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI54                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001536)) 
#define SIUL_X_GPDI54(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001536 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI55                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001537)) 
#define SIUL_X_GPDI55(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001537 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI56                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001538)) 
#define SIUL_X_GPDI56(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001538 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI57                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001539)) 
#define SIUL_X_GPDI57(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001539 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI58                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153A)) 
#define SIUL_X_GPDI58(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI59                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153B)) 
#define SIUL_X_GPDI59(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI60                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153C)) 
#define SIUL_X_GPDI60(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI61                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153D)) 
#define SIUL_X_GPDI61(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI62                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153E)) 
#define SIUL_X_GPDI62(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI63                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000153F)) 
#define SIUL_X_GPDI63(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000153F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI64                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001540)) 
#define SIUL_X_GPDI64(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001540 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI65                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001541)) 
#define SIUL_X_GPDI65(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001541 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI66                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001542)) 
#define SIUL_X_GPDI66(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001542 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI67                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001543)) 
#define SIUL_X_GPDI67(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001543 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI68                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001544)) 
#define SIUL_X_GPDI68(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001544 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI69                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001545)) 
#define SIUL_X_GPDI69(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001545 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI70                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001546)) 
#define SIUL_X_GPDI70(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001546 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI71                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001547)) 
#define SIUL_X_GPDI71(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001547 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI72                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001548)) 
#define SIUL_X_GPDI72(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001548 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI73                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001549)) 
#define SIUL_X_GPDI73(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001549 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI74                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154A)) 
#define SIUL_X_GPDI74(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI75                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154B)) 
#define SIUL_X_GPDI75(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI76                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154C)) 
#define SIUL_X_GPDI76(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI77                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154D)) 
#define SIUL_X_GPDI77(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI78                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154E)) 
#define SIUL_X_GPDI78(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI79                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000154F)) 
#define SIUL_X_GPDI79(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000154F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI80                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001550)) 
#define SIUL_X_GPDI80(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001550 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI81                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001551)) 
#define SIUL_X_GPDI81(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001551 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI82                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001552)) 
#define SIUL_X_GPDI82(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001552 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI83                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001553)) 
#define SIUL_X_GPDI83(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001553 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI84                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001554)) 
#define SIUL_X_GPDI84(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001554 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI85                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001555)) 
#define SIUL_X_GPDI85(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001555 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI86                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001556)) 
#define SIUL_X_GPDI86(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001556 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI87                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001557)) 
#define SIUL_X_GPDI87(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001557 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI88                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001558)) 
#define SIUL_X_GPDI88(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001558 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI89                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001559)) 
#define SIUL_X_GPDI89(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001559 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI90                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155A)) 
#define SIUL_X_GPDI90(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155A + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI91                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155B)) 
#define SIUL_X_GPDI91(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155B + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI92                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155C)) 
#define SIUL_X_GPDI92(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155C + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI93                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155D)) 
#define SIUL_X_GPDI93(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155D + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI94                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155E)) 
#define SIUL_X_GPDI94(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155E + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI95                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x0000155F)) 
#define SIUL_X_GPDI95(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x0000155F + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI96                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001560)) 
#define SIUL_X_GPDI96(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001560 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI97                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001561)) 
#define SIUL_X_GPDI97(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001561 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI98                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001562)) 
#define SIUL_X_GPDI98(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001562 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI99                    (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001563)) 
#define SIUL_X_GPDI99(x)               (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001563 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI102                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001566)) 
#define SIUL_X_GPDI102(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001566 + ((x)*0x8000)))

/* GPDIn - GPIO Pad Data Input Reg */

#define SIUL_GPDI103                   (*(vuint8_t *) (SIUL_BASEADDRESS+0x00001567)) 
#define SIUL_X_GPDI103(x)              (*(vuint8_t *) (SIUL_BASEADDRESS + 0x00001567 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO0                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001700)) 
#define SIUL_X_PGPDO0(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001700 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO1                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001702)) 
#define SIUL_X_PGPDO1(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001702 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO2                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001704)) 
#define SIUL_X_PGPDO2(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001704 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO3                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001706)) 
#define SIUL_X_PGPDO3(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001706 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO4                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001708)) 
#define SIUL_X_PGPDO4(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001708 + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO5                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x0000170A)) 
#define SIUL_X_PGPDO5(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x0000170A + ((x)*0x8000)))

/* PGPDOn - Parallel GPIO Pad Data out Reg */

#define SIUL_PGPDO7                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x0000170E)) 
#define SIUL_X_PGPDO7(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x0000170E + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI0                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001740)) 
#define SIUL_X_PGPDI0(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001740 + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI1                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001742)) 
#define SIUL_X_PGPDI1(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001742 + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI2                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001744)) 
#define SIUL_X_PGPDI2(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001744 + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI3                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001746)) 
#define SIUL_X_PGPDI3(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001746 + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI4                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x00001748)) 
#define SIUL_X_PGPDI4(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x00001748 + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI5                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x0000174A)) 
#define SIUL_X_PGPDI5(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x0000174A + ((x)*0x8000)))

/* PGPDIn - Parallel GPIO Pad Data in Reg */

#define SIUL_PGPDI7                    (*(vuint16_t *) (SIUL_BASEADDRESS+0x0000174E)) 
#define SIUL_X_PGPDI7(x)               (*(vuint16_t *) (SIUL_BASEADDRESS + 0x0000174E + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */
#define SIUL_MPGPDO(x)       (((SIUL_BASEADDRESS+0x00001780) +((x)*0x4)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO0                   (SIUL_BASEADDRESS+0x00001780) 
#define SIUL_X_MPGPDO0(x)              ((SIUL_BASEADDRESS + 0x00001780 + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO1                   (SIUL_BASEADDRESS+0x00001784) 
#define SIUL_X_MPGPDO1(x)              ((SIUL_BASEADDRESS + 0x00001784 + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO2                   (SIUL_BASEADDRESS+0x00001788) 
#define SIUL_X_MPGPDO2(x)              ((SIUL_BASEADDRESS + 0x00001788 + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO3                   (SIUL_BASEADDRESS+0x0000178C) 
#define SIUL_X_MPGPDO3(x)              ((SIUL_BASEADDRESS + 0x0000178C + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO4                   (SIUL_BASEADDRESS+0x00001790) 
#define SIUL_X_MPGPDO4(x)              ((SIUL_BASEADDRESS + 0x00001790 + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO5                   (SIUL_BASEADDRESS+0x00001794) 
#define SIUL_X_MPGPDO5(x)              ((SIUL_BASEADDRESS + 0x00001794 + ((x)*0x8000)))

/* MPGPDOn - Masked Parallel GPIO Pad Data Out Reg */

#define SIUL_MPGPDO6                   (SIUL_BASEADDRESS+0x00001798) 
#define SIUL_X_MPGPDO6(x)              ((SIUL_BASEADDRESS + 0x00001798 + ((x)*0x8000)))

/* Field definitions for MIDR1 */

#define SIUL_PARTNUM_VALUE(x)          (((x)&0x0000FFFF)<<16)  
#define SIUL_PARTNUM_MSB               (31)
#define SIUL_PARTNUM_LSB               (16)
#define SIUL_PARTNUM_MASK              (0x0000FFFF) 
#define SIUL_PARTNUM                   ((SIUL_PARTNUM_MASK) << (SIUL_PARTNUM_LSB)) 

#define SIUL_MAJOR_MASK_VALUE(x)       (((x)&0x0000000F)<<4)  
#define SIUL_MAJOR_MASK_MSB            (7)
#define SIUL_MAJOR_MASK_LSB            (4)
#define SIUL_MAJOR_MASK_MASK           (0x0000000F) 
#define SIUL_MAJOR_MASK                ((SIUL_MAJOR_MASK_MASK) << (SIUL_MAJOR_MASK_LSB)) 

#define SIUL_MINOR_MASK_VALUE(x)       (((x)&0x0000000F)<<0)  
#define SIUL_MINOR_MASK_MSB            (3)
#define SIUL_MINOR_MASK_LSB            (0)
#define SIUL_MINOR_MASK_MASK           (0x0000000F) 
#define SIUL_MINOR_MASK                ((SIUL_MINOR_MASK_MASK) << (SIUL_MINOR_MASK_LSB)) 

/* Field definitions for DISR0 */

#define SIUL_EIF11_VALUE(x)            (((x)&0x00000001)<<11)  
#define SIUL_EIF11_BIT                 (11)  
#define SIUL_EIF11                     ((1) << (SIUL_EIF11_BIT)) 

#define SIUL_EIF10_VALUE(x)            (((x)&0x00000001)<<10)  
#define SIUL_EIF10_BIT                 (10)  
#define SIUL_EIF10                     ((1) << (SIUL_EIF10_BIT)) 

#define SIUL_EIF9_VALUE(x)             (((x)&0x00000001)<<9)  
#define SIUL_EIF9_BIT                  (9)  
#define SIUL_EIF9                      ((1) << (SIUL_EIF9_BIT)) 

#define SIUL_EIF8_VALUE(x)             (((x)&0x00000001)<<8)  
#define SIUL_EIF8_BIT                  (8)  
#define SIUL_EIF8                      ((1) << (SIUL_EIF8_BIT)) 

#define SIUL_EIF7_VALUE(x)             (((x)&0x00000001)<<7)  
#define SIUL_EIF7_BIT                  (7)  
#define SIUL_EIF7                      ((1) << (SIUL_EIF7_BIT)) 

#define SIUL_EIF6_VALUE(x)             (((x)&0x00000001)<<6)  
#define SIUL_EIF6_BIT                  (6)  
#define SIUL_EIF6                      ((1) << (SIUL_EIF6_BIT)) 

#define SIUL_EIF5_VALUE(x)             (((x)&0x00000001)<<5)  
#define SIUL_EIF5_BIT                  (5)  
#define SIUL_EIF5                      ((1) << (SIUL_EIF5_BIT)) 

#define SIUL_EIF4_VALUE(x)             (((x)&0x00000001)<<4)  
#define SIUL_EIF4_BIT                  (4)  
#define SIUL_EIF4                      ((1) << (SIUL_EIF4_BIT)) 

#define SIUL_EIF3_VALUE(x)             (((x)&0x00000001)<<3)  
#define SIUL_EIF3_BIT                  (3)  
#define SIUL_EIF3                      ((1) << (SIUL_EIF3_BIT)) 

#define SIUL_EIF2_VALUE(x)             (((x)&0x00000001)<<2)  
#define SIUL_EIF2_BIT                  (2)  
#define SIUL_EIF2                      ((1) << (SIUL_EIF2_BIT)) 

#define SIUL_EIF1_VALUE(x)             (((x)&0x00000001)<<1)  
#define SIUL_EIF1_BIT                  (1)  
#define SIUL_EIF1                      ((1) << (SIUL_EIF1_BIT)) 

#define SIUL_EIF0_VALUE(x)             (((x)&0x00000001)<<0)  
#define SIUL_EIF0_BIT                  (0)  
#define SIUL_EIF0                      ((1) << (SIUL_EIF0_BIT)) 

/* Field definitions for DIRER0 */

#define SIUL_EIRE11_VALUE(x)           (((x)&0x00000001)<<11)  
#define SIUL_EIRE11_BIT                (11)  
#define SIUL_EIRE11                    ((1) << (SIUL_EIRE11_BIT)) 

#define SIUL_EIRE10_VALUE(x)           (((x)&0x00000001)<<10)  
#define SIUL_EIRE10_BIT                (10)  
#define SIUL_EIRE10                    ((1) << (SIUL_EIRE10_BIT)) 

#define SIUL_EIRE9_VALUE(x)            (((x)&0x00000001)<<9)  
#define SIUL_EIRE9_BIT                 (9)  
#define SIUL_EIRE9                     ((1) << (SIUL_EIRE9_BIT)) 

#define SIUL_EIRE8_VALUE(x)            (((x)&0x00000001)<<8)  
#define SIUL_EIRE8_BIT                 (8)  
#define SIUL_EIRE8                     ((1) << (SIUL_EIRE8_BIT)) 

#define SIUL_EIRE7_VALUE(x)            (((x)&0x00000001)<<7)  
#define SIUL_EIRE7_BIT                 (7)  
#define SIUL_EIRE7                     ((1) << (SIUL_EIRE7_BIT)) 

#define SIUL_EIRE6_VALUE(x)            (((x)&0x00000001)<<6)  
#define SIUL_EIRE6_BIT                 (6)  
#define SIUL_EIRE6                     ((1) << (SIUL_EIRE6_BIT)) 

#define SIUL_EIRE5_VALUE(x)            (((x)&0x00000001)<<5)  
#define SIUL_EIRE5_BIT                 (5)  
#define SIUL_EIRE5                     ((1) << (SIUL_EIRE5_BIT)) 

#define SIUL_EIRE4_VALUE(x)            (((x)&0x00000001)<<4)  
#define SIUL_EIRE4_BIT                 (4)  
#define SIUL_EIRE4                     ((1) << (SIUL_EIRE4_BIT)) 

#define SIUL_EIRE3_VALUE(x)            (((x)&0x00000001)<<3)  
#define SIUL_EIRE3_BIT                 (3)  
#define SIUL_EIRE3                     ((1) << (SIUL_EIRE3_BIT)) 

#define SIUL_EIRE2_VALUE(x)            (((x)&0x00000001)<<2)  
#define SIUL_EIRE2_BIT                 (2)  
#define SIUL_EIRE2                     ((1) << (SIUL_EIRE2_BIT)) 

#define SIUL_EIRE1_VALUE(x)            (((x)&0x00000001)<<1)  
#define SIUL_EIRE1_BIT                 (1)  
#define SIUL_EIRE1                     ((1) << (SIUL_EIRE1_BIT)) 

#define SIUL_EIRE0_VALUE(x)            (((x)&0x00000001)<<0)  
#define SIUL_EIRE0_BIT                 (0)  
#define SIUL_EIRE0                     ((1) << (SIUL_EIRE0_BIT)) 

/* Field definitions for DIRSR0 */

#define SIUL_DIRS11_VALUE(x)           (((x)&0x00000001)<<11)  
#define SIUL_DIRS11_BIT                (11)  
#define SIUL_DIRS11                    ((1) << (SIUL_DIRS11_BIT)) 

#define SIUL_DIRS10_VALUE(x)           (((x)&0x00000001)<<10)  
#define SIUL_DIRS10_BIT                (10)  
#define SIUL_DIRS10                    ((1) << (SIUL_DIRS10_BIT)) 

#define SIUL_DIRS9_VALUE(x)            (((x)&0x00000001)<<9)  
#define SIUL_DIRS9_BIT                 (9)  
#define SIUL_DIRS9                     ((1) << (SIUL_DIRS9_BIT)) 

#define SIUL_DIRS8_VALUE(x)            (((x)&0x00000001)<<8)  
#define SIUL_DIRS8_BIT                 (8)  
#define SIUL_DIRS8                     ((1) << (SIUL_DIRS8_BIT)) 

#define SIUL_DIRS7_VALUE(x)            (((x)&0x00000001)<<7)  
#define SIUL_DIRS7_BIT                 (7)  
#define SIUL_DIRS7                     ((1) << (SIUL_DIRS7_BIT)) 

#define SIUL_DIRS6_VALUE(x)            (((x)&0x00000001)<<6)  
#define SIUL_DIRS6_BIT                 (6)  
#define SIUL_DIRS6                     ((1) << (SIUL_DIRS6_BIT)) 

#define SIUL_DIRS5_VALUE(x)            (((x)&0x00000001)<<5)  
#define SIUL_DIRS5_BIT                 (5)  
#define SIUL_DIRS5                     ((1) << (SIUL_DIRS5_BIT)) 

#define SIUL_DIRS4_VALUE(x)            (((x)&0x00000001)<<4)  
#define SIUL_DIRS4_BIT                 (4)  
#define SIUL_DIRS4                     ((1) << (SIUL_DIRS4_BIT)) 

#define SIUL_DIRS3_VALUE(x)            (((x)&0x00000001)<<3)  
#define SIUL_DIRS3_BIT                 (3)  
#define SIUL_DIRS3                     ((1) << (SIUL_DIRS3_BIT)) 

#define SIUL_DIRS2_VALUE(x)            (((x)&0x00000001)<<2)  
#define SIUL_DIRS2_BIT                 (2)  
#define SIUL_DIRS2                     ((1) << (SIUL_DIRS2_BIT)) 

#define SIUL_DIRS1_VALUE(x)            (((x)&0x00000001)<<1)  
#define SIUL_DIRS1_BIT                 (1)  
#define SIUL_DIRS1                     ((1) << (SIUL_DIRS1_BIT)) 

#define SIUL_DIRS0_VALUE(x)            (((x)&0x00000001)<<0)  
#define SIUL_DIRS0_BIT                 (0)  
#define SIUL_DIRS0                     ((1) << (SIUL_DIRS0_BIT)) 

/* Field definitions for IREER0 */

#define SIUL_IREE11_VALUE(x)           (((x)&0x00000001)<<11)  
#define SIUL_IREE11_BIT                (11)  
#define SIUL_IREE11                    ((1) << (SIUL_IREE11_BIT)) 

#define SIUL_IREE10_VALUE(x)           (((x)&0x00000001)<<10)  
#define SIUL_IREE10_BIT                (10)  
#define SIUL_IREE10                    ((1) << (SIUL_IREE10_BIT)) 

#define SIUL_IREE9_VALUE(x)            (((x)&0x00000001)<<9)  
#define SIUL_IREE9_BIT                 (9)  
#define SIUL_IREE9                     ((1) << (SIUL_IREE9_BIT)) 

#define SIUL_IREE8_VALUE(x)            (((x)&0x00000001)<<8)  
#define SIUL_IREE8_BIT                 (8)  
#define SIUL_IREE8                     ((1) << (SIUL_IREE8_BIT)) 

#define SIUL_IREE7_VALUE(x)            (((x)&0x00000001)<<7)  
#define SIUL_IREE7_BIT                 (7)  
#define SIUL_IREE7                     ((1) << (SIUL_IREE7_BIT)) 

#define SIUL_IREE6_VALUE(x)            (((x)&0x00000001)<<6)  
#define SIUL_IREE6_BIT                 (6)  
#define SIUL_IREE6                     ((1) << (SIUL_IREE6_BIT)) 

#define SIUL_IREE5_VALUE(x)            (((x)&0x00000001)<<5)  
#define SIUL_IREE5_BIT                 (5)  
#define SIUL_IREE5                     ((1) << (SIUL_IREE5_BIT)) 

#define SIUL_IREE4_VALUE(x)            (((x)&0x00000001)<<4)  
#define SIUL_IREE4_BIT                 (4)  
#define SIUL_IREE4                     ((1) << (SIUL_IREE4_BIT)) 

#define SIUL_IREE3_VALUE(x)            (((x)&0x00000001)<<3)  
#define SIUL_IREE3_BIT                 (3)  
#define SIUL_IREE3                     ((1) << (SIUL_IREE3_BIT)) 

#define SIUL_IREE2_VALUE(x)            (((x)&0x00000001)<<2)  
#define SIUL_IREE2_BIT                 (2)  
#define SIUL_IREE2                     ((1) << (SIUL_IREE2_BIT)) 

#define SIUL_IREE1_VALUE(x)            (((x)&0x00000001)<<1)  
#define SIUL_IREE1_BIT                 (1)  
#define SIUL_IREE1                     ((1) << (SIUL_IREE1_BIT)) 

#define SIUL_IREE0_VALUE(x)            (((x)&0x00000001)<<0)  
#define SIUL_IREE0_BIT                 (0)  
#define SIUL_IREE0                     ((1) << (SIUL_IREE0_BIT)) 

/* Field definitions for IFEER0 */

#define SIUL_IFEE11_VALUE(x)           (((x)&0x00000001)<<11)  
#define SIUL_IFEE11_BIT                (11)  
#define SIUL_IFEE11                    ((1) << (SIUL_IFEE11_BIT)) 

#define SIUL_IFEE10_VALUE(x)           (((x)&0x00000001)<<10)  
#define SIUL_IFEE10_BIT                (10)  
#define SIUL_IFEE10                    ((1) << (SIUL_IFEE10_BIT)) 

#define SIUL_IFEE9_VALUE(x)            (((x)&0x00000001)<<9)  
#define SIUL_IFEE9_BIT                 (9)  
#define SIUL_IFEE9                     ((1) << (SIUL_IFEE9_BIT)) 

#define SIUL_IFEE8_VALUE(x)            (((x)&0x00000001)<<8)  
#define SIUL_IFEE8_BIT                 (8)  
#define SIUL_IFEE8                     ((1) << (SIUL_IFEE8_BIT)) 

#define SIUL_IFEE7_VALUE(x)            (((x)&0x00000001)<<7)  
#define SIUL_IFEE7_BIT                 (7)  
#define SIUL_IFEE7                     ((1) << (SIUL_IFEE7_BIT)) 

#define SIUL_IFEE6_VALUE(x)            (((x)&0x00000001)<<6)  
#define SIUL_IFEE6_BIT                 (6)  
#define SIUL_IFEE6                     ((1) << (SIUL_IFEE6_BIT)) 

#define SIUL_IFEE5_VALUE(x)            (((x)&0x00000001)<<5)  
#define SIUL_IFEE5_BIT                 (5)  
#define SIUL_IFEE5                     ((1) << (SIUL_IFEE5_BIT)) 

#define SIUL_IFEE4_VALUE(x)            (((x)&0x00000001)<<4)  
#define SIUL_IFEE4_BIT                 (4)  
#define SIUL_IFEE4                     ((1) << (SIUL_IFEE4_BIT)) 

#define SIUL_IFEE3_VALUE(x)            (((x)&0x00000001)<<3)  
#define SIUL_IFEE3_BIT                 (3)  
#define SIUL_IFEE3                     ((1) << (SIUL_IFEE3_BIT)) 

#define SIUL_IFEE2_VALUE(x)            (((x)&0x00000001)<<2)  
#define SIUL_IFEE2_BIT                 (2)  
#define SIUL_IFEE2                     ((1) << (SIUL_IFEE2_BIT)) 

#define SIUL_IFEE1_VALUE(x)            (((x)&0x00000001)<<1)  
#define SIUL_IFEE1_BIT                 (1)  
#define SIUL_IFEE1                     ((1) << (SIUL_IFEE1_BIT)) 

#define SIUL_IFEE0_VALUE(x)            (((x)&0x00000001)<<0)  
#define SIUL_IFEE0_BIT                 (0)  
#define SIUL_IFEE0                     ((1) << (SIUL_IFEE0_BIT)) 

/* Field definitions for IFER0 */

#define SIUL_IFE11_VALUE(x)            (((x)&0x00000001)<<11)  
#define SIUL_IFE11_BIT                 (11)  
#define SIUL_IFE11                     ((1) << (SIUL_IFE11_BIT)) 

#define SIUL_IFE10_VALUE(x)            (((x)&0x00000001)<<10)  
#define SIUL_IFE10_BIT                 (10)  
#define SIUL_IFE10                     ((1) << (SIUL_IFE10_BIT)) 

#define SIUL_IFE9_VALUE(x)             (((x)&0x00000001)<<9)  
#define SIUL_IFE9_BIT                  (9)  
#define SIUL_IFE9                      ((1) << (SIUL_IFE9_BIT)) 

#define SIUL_IFE8_VALUE(x)             (((x)&0x00000001)<<8)  
#define SIUL_IFE8_BIT                  (8)  
#define SIUL_IFE8                      ((1) << (SIUL_IFE8_BIT)) 

#define SIUL_IFE7_VALUE(x)             (((x)&0x00000001)<<7)  
#define SIUL_IFE7_BIT                  (7)  
#define SIUL_IFE7                      ((1) << (SIUL_IFE7_BIT)) 

#define SIUL_IFE6_VALUE(x)             (((x)&0x00000001)<<6)  
#define SIUL_IFE6_BIT                  (6)  
#define SIUL_IFE6                      ((1) << (SIUL_IFE6_BIT)) 

#define SIUL_IFE5_VALUE(x)             (((x)&0x00000001)<<5)  
#define SIUL_IFE5_BIT                  (5)  
#define SIUL_IFE5                      ((1) << (SIUL_IFE5_BIT)) 

#define SIUL_IFE4_VALUE(x)             (((x)&0x00000001)<<4)  
#define SIUL_IFE4_BIT                  (4)  
#define SIUL_IFE4                      ((1) << (SIUL_IFE4_BIT)) 

#define SIUL_IFE3_VALUE(x)             (((x)&0x00000001)<<3)  
#define SIUL_IFE3_BIT                  (3)  
#define SIUL_IFE3                      ((1) << (SIUL_IFE3_BIT)) 

#define SIUL_IFE2_VALUE(x)             (((x)&0x00000001)<<2)  
#define SIUL_IFE2_BIT                  (2)  
#define SIUL_IFE2                      ((1) << (SIUL_IFE2_BIT)) 

#define SIUL_IFE1_VALUE(x)             (((x)&0x00000001)<<1)  
#define SIUL_IFE1_BIT                  (1)  
#define SIUL_IFE1                      ((1) << (SIUL_IFE1_BIT)) 

#define SIUL_IFE0_VALUE(x)             (((x)&0x00000001)<<0)  
#define SIUL_IFE0_BIT                  (0)  
#define SIUL_IFE0                      ((1) << (SIUL_IFE0_BIT)) 

/* Field definitions for IFMCR0 */

#define SIUL_MAXCNT_VALUE(x)           (((x)&0x0000000F)<<0)  
#define SIUL_MAXCNT_MSB                (3)
#define SIUL_MAXCNT_LSB                (0)
#define SIUL_MAXCNT_MASK               (0x0000000F) 
#define SIUL_MAXCNT                    ((SIUL_MAXCNT_MASK) << (SIUL_MAXCNT_LSB)) 

/* Field definitions for IFCPR */

#define SIUL_IFCP_VALUE(x)             (((x)&0x0000000F)<<0)  
#define SIUL_IFCP_MSB                  (3)
#define SIUL_IFCP_LSB                  (0)
#define SIUL_IFCP_MASK                 (0x0000000F) 
#define SIUL_IFCP                      ((SIUL_IFCP_MASK) << (SIUL_IFCP_LSB)) 
