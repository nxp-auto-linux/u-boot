/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_MAC57D5XH_MCME_REGS_H__
#define __ARCH_ARM_MACH_MAC57D5XH_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* Mode Entry Module (MC_ME) */
struct mc_me_reg {
    u32 mc_me_gs;           /* Global Status Register                       */
    u32 mc_me_mctl;         /* Mode Control Register                        */
    u32 mc_me_me;           /* Mode Enable Register                         */
    u32 mc_me_is;           /* Interrupt Status Register                    */
    u32 mc_me_im;           /* Interrupt Mask Register                      */
    u32 mc_me_imts;         /* Invalid Mode Transition Status Register      */
    u32 mc_me_dmts;         /* Debug Mode Transition Status Register        */
    u32 reserved_0x1c[1];
    u32 mc_me_reset_mc;     /* RESET Mode Configuration Register            */
    u32 reserved_0x24[1];
    u32 mc_me_safe_mc;      /* SAFE Mode Configuration Register             */
    u32 mc_me_drun_mc;      /* DRUN Mode Configuration Register             */
    u32 mc_me_run_mc0;      /* RUN Mode Configuration Register              */
    u32 mc_me_run_mc1;      /* RUN Mode Configuration Register              */
    u32 mc_me_run_mc2;      /* RUN Mode Configuration Register              */
    u32 mc_me_run_mc3;      /* RUN Mode Configuration Register              */
    u32 reserved_0x40[2];
    u32 mc_me_stop0_mc;     /* STOP0 Mode Configuration Register            */
    u32 reserved_0x4c[2];
    u32 mc_me_standby0_mc;  /* STANDBY0 Mode Configuration                  */
    u32 reserved_0x58[2];
    u32 mc_me_ps0;          /* Peripheral Status Register 0                 */
    u32 mc_me_ps1;          /* Peripheral Status Register 1                 */
    u32 mc_me_ps2;          /* Peripheral Status Register 2                 */
    u32 reserved_0x6c[1];
    u32 mc_me_ps4;          /* Peripheral Status Register 4                 */
    u32 mc_me_ps5;          /* Peripheral Status Register 5                 */
    u32 mc_me_ps6;          /* Peripheral Status Register 6                 */
    u32 reserved_0x7c[1];
    u32 mc_me_run_pc0;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc1;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc2;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc3;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc4;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc5;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc6;      /* Run Peripheral Configuration Register        */
    u32 mc_me_run_pc7;      /* Run Peripheral Configuration Register        */
    u32 mc_me_lp_pc0;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc1;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc2;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc3;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc4;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc5;       /* Low-Power Peripheral Configuration Register  */
    u32 mc_me_lp_pc6;       /* Low -Power Peripheral Configuration Register */
    u32 mc_me_lp_pc7;       /* Low-Power Peripheral Configuration Register  */
    u8  mc_me_pctl3;        /* I2C_1 Peripheral Control Register            */
    u8  mc_me_pctl2;        /* I2C_0 Peripheral Control Register            */
    u8  mc_me_pctl1;        /* DMA_CH_MUX_0 Peripheral Control Register     */
    u8  reserved_0xc3[1];
    u8  mc_me_pctl7;        /* FLEXTIMER_3 Peripheral Control Register      */
    u8  mc_me_pctl6;        /* FLEXTIMER_2 Peripheral Control Register      */
    u8  mc_me_pctl5;        /* FLEXTIMER_1 Peripheral Control               */
    u8  mc_me_pctl4;        /* FLEXTIMER_0 Peripheral Control               */
    u8  mc_me_pctl11;       /* SMC Peripheral Control Register              */
    u8  mc_me_pctl10;       /* CMP_1 Peripheral Control                     */
    u8  mc_me_pctl9;        /* CMP_0 Peripheral Control                     */
    u8  reserved_0xcb[1];
    u8  mc_me_pctl15;       /* SSD_3 Peripheral Control Register            */
    u8  mc_me_pctl14;       /* SSD_2 Peripheral Control Register            */
    u8  mc_me_pctl13;       /* SSD_1 Peripheral Control                     */
    u8  mc_me_pctl12;       /* SSD_0 Peripheral Control                     */
    u8  mc_me_pctl19;       /* SPI_0 Peripheral Control                     */
    u8  mc_me_pctl18;       /* MPR Peripheral Control                       */
    u8  mc_me_pctl17;       /* SSD_5 Peripheral Control                     */
    u8  mc_me_pctl16;       /* SSD_4 Peripheral Control                     */
    u8  mc_me_pctl23;       /* SPI_4 Peripheral Control                     */
    u8  mc_me_pctl22;       /* SPI_3 Peripheral Control                     */
    u8  mc_me_pctl21;       /* SPI_2 Peripheral Control Register            */
    u8  mc_me_pctl20;       /* SPI_1 Peripheral Control                     */
    u8  mc_me_pctl27;       /* LCD Peripheral Control Register              */
    u8  mc_me_pctl26;       /* LINFLEX_2 Peripheral Control                 */
    u8  mc_me_pctl25;       /* LINFLEX_1 Peripheral Control Register        */
    u8  mc_me_pctl24;       /* LINFLEX_0 Peripheral Control Register        */
    u8  reserved_0xdc[1];
    u8  mc_me_pctl30;       /* ADC Peripheral Control Register              */
    u8  mc_me_pctl29;       /* FlexCAN_2 Peripheral Control Register        */
    u8  mc_me_pctl28;       /* FlexCAN_1 Peripheral Control Register        */
    u8  reserved_0xe0[1];
    u8  mc_me_pctl34;       /* CRC Peripheral Control Register              */
    u8  reserved_0xe2[5];
    u8  mc_me_pctl36;       /* PIT Peripheral Control Register              */
    u8  reserved_0xe8[23];
    u8  mc_me_pctl60;       /* SIUL Peripheral Control Register             */
    u8  reserved_0x100[23];
    u8  mc_me_pctl84;       /* RTC_API Peripheral Control Register          */
    u8  reserved_0x118[3];
    u8  mc_me_pctl88;       /* FLEXCAN_0 Peripheral Control                 */
    u8  reserved_0x11c[38];
    u8  mc_me_pctl129;      /* QuadSPI_1 Peripheral Control Register        */
    u8  mc_me_pctl128;      /* QuadSPI_0 Peripheral Control Register        */
    u8  reserved_0x144[3];
    u8  mc_me_pctl132;      /* DRAM_CTRLR Peripheral Control                */
    u8  reserved_0x148[3];
    u8  mc_me_pctl136;      /* 2D-ACE 0 (DCU_0) Peripheral Control Register */
    u8  reserved_0x14c[15];
    u8  mc_me_pctl152;      /* 2D-ACE 1 (DCU_1) Peripheral Control          */
    u8  reserved_0x15c[15];
    u8  mc_me_pctl168;      /* VIU Peripheral Control                       */
    u8  reserved_0x16c[3];
    u8  mc_me_pctl172;      /* GC355 Peripheral Control Register            */
    u8  reserved_0x170[3];
    u8  mc_me_pctl176;      /* TCON Peripheral Control Register             */
    u8  reserved_0x174[3];
    u8  mc_me_pctl180;      /* TCON_LITE Peripheral Control                 */
    u8  reserved_0x178[3];
    u8  mc_me_pctl184;      /* RLE Peripheral Control                       */
    u8  reserved_0x17c[3];
    u8  mc_me_pctl188;      /* SGM Peripheral Control Register              */
    u8  reserved_0x180[2];
    u8  mc_me_pctl193;      /* DMA_1 Peripheral Control Register            */
    u8  reserved_0x183[4];
    u8  mc_me_pctl196;      /* ENET Peripheral Control Register             */
    u8  reserved_0x188[3];
    u8  mc_me_pctl200;      /* LDB Peripheral Control Register              */
    u8  reserved_0x184[3];
    u8  mc_me_pctl204;      /* MLB Peripheral Control Register              */
    u8  reserved_0x190[48];
    u32 mc_me_cs;           /* Core Status Register                         */
    u16 mc_me_cctl1;        /* CORE1 Control Register                       */
    u16 mc_me_cctl0;        /* CORE0 Control Register                       */
    u16 reserved_0x1c7[1];
    u16 mc_me_cctl2;        /* CORE2 Control Register                       */
    u32 reserved_0x1cc[5];
    u32 mc_me_caddr0;       /* CORE0 Address Register                       */
    u32 mc_me_caddr1;       /* CORE1 Address Register                       */
    u32 mc_me_caddr2;       /* CORE2 Address Register                       */
};

/* MC_ME registers definitions */

/* MC_ME_ME */
#define MC_ME_ME_RESET_FUNC             (1 << 0)
#define MC_ME_ME_SAFE                   (1 << 2)
#define MC_ME_ME_DRUN                   (1 << 3)
#define MC_ME_ME_RUN0                   (1 << 4)
#define MC_ME_ME_RUN1                   (1 << 5)
#define MC_ME_ME_RUN2                   (1 << 6)
#define MC_ME_ME_RUN3                   (1 << 7)
#define MC_ME_ME_STOP0                  (1 << 10)
#define MC_ME_ME_STANDBY0               (1 << 13)
#define MC_ME_ME_RESET_DEST             (1 << 15)

/* MC_ME_RUN_PCn */
#define MC_ME_RUN_PCn_RESET             (1 << 0)
#define MC_ME_RUN_PCn_SAFE              (1 << 2)
#define MC_ME_RUN_PCn_DRUN              (1 << 3)
#define MC_ME_RUN_PCn_RUN0              (1 << 4)
#define MC_ME_RUN_PCn_RUN1              (1 << 5)
#define MC_ME_RUN_PCn_RUN2              (1 << 6)
#define MC_ME_RUN_PCn_RUN3              (1 << 7)

/*
 * MC_ME_DRUN_MC/ MC_ME_RUN0_MC/ MC_ME_RUN1_MC
 * MC_ME_RUN2_MC/ MC_ME_RUN3_MC
 * MC_ME_STANDBY0_MC/MC_ME_STOP0_MC
 */
#define MC_ME_RUNMODE_MC_SYSCLK(val)    (0x0000000F & (val))
#define MC_ME_RUNMODE_MC_RCON           (1 << 4)
#define MC_ME_RUNMODE_MC_FXOSCON        (1 << 5)
#define MC_ME_RUNMODE_MC_SIRCON         (1 << 6)
#define MC_ME_RUNMODE_MC_SXOSCON        (1 << 7)
#define MC_ME_RUNMODE_MC_PLL0ON         (1 << 8)
#define MC_ME_RUNMODE_MC_PLL1ON         (1 << 9)
#define MC_ME_RUNMODE_MC_PLL2ON         (1 << 10)
#define MC_ME_RUNMODE_MC_PLL3ON         (1 << 11)
#define MC_ME_RUNMODE_MC_FLAON(val)     (0x00030000 & ((val) << 16))
#define MC_ME_RUNMODE_MC_MVRON          (1 << 20)
#define MC_ME_RUNMODE_MC_PDO            (1 << 23)
#define MC_ME_RUNMODE_MC_PWRLVL0        (1 << 28)
#define MC_ME_RUNMODE_MC_PWRLVL1        (1 << 29)
#define MC_ME_RUNMODE_MC_PWRLVL2        (1 << 30)

/* MC_ME_MCTL */
#define MC_ME_MCTL_KEY                  (0x00005AF0)
#define MC_ME_MCTL_INVERTEDKEY          (0x0000A50F)
#define MC_ME_MCTL_RESET                (0x0 << 28)
#define MC_ME_MCTL_SAFE                 (0x2 << 28)
#define MC_ME_MCTL_DRUN                 (0x3 << 28)
#define MC_ME_MCTL_RUN0                 (0x4 << 28)
#define MC_ME_MCTL_RUN1                 (0x5 << 28)
#define MC_ME_MCTL_RUN2                 (0x6 << 28)
#define MC_ME_MCTL_RUN3                 (0x7 << 28)
#define MC_ME_MCTL_STOP0                (0x10 << 28)
#define MC_ME_MCTL_STANDBY0             (0x13 << 28)

/* MC_ME_GS */
#define MC_ME_GS_S_SYSCLK_IRC           (0x0 << 0)
#define MC_ME_GS_S_SYSCLK_FXOSC         (0x1 << 0)
#define MC_ME_GS_S_SYSCLK_PLL0          (0x4 << 0)
#define MC_ME_GS_S_SYSCLK_PLL1          (0x5 << 0)
#define MC_ME_GS_S_STSCLK_DISABLE       (0xF << 0)
#define MC_ME_GS_S_RC                   (1 << 4)
#define MC_ME_GS_S_FXOSC                (1 << 5)
#define MC_ME_GS_S_SIRC                 (1 << 6)
#define MC_ME_GS_S_SXOSC                (1 << 7)
#define MC_ME_GS_S_PLL0                 (1 << 8)
#define MC_ME_GS_S_PLL1                 (1 << 9)
#define MC_ME_GS_S_PLL2                 (1 << 10)
#define MC_ME_GS_S_PLL3                 (1 << 11)
#define MC_ME_GS_S_FLA0                 (1 << 16)
#define MC_ME_GS_S_FLA1                 (1 << 17)
#define MC_ME_GS_S_PDO                  (1 << 23)
#define MC_ME_GS_S_MTRANS               (1 << 27)
#define MC_ME_GS_S_CRT_MODE_RESET       (0x0 << 28)
#define MC_ME_GS_S_CRT_MODE_SAFE        (0x2 << 28)
#define MC_ME_GS_S_CRT_MODE_DRUN        (0x3 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN0        (0x4 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN1        (0x5 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN2        (0x6 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN3        (0x7 << 28)

#endif

#endif /*__ARCH_ARM_MACH_MAC57D5XH_MCME_REGS_H__ */
