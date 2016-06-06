/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define IRAM_BASE_ADDR          0x3F000000	/* internal ram */
#define IRAM_SIZE               0x00080000	/* 512 KB */

#define AIPS0_BASE_ADDR         0x40000000
#define AIPS1_BASE_ADDR         0x40080000

/* AIPS 0 */
#define EIM_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00005000)
#define ERM_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00006000)
#define DAP_ROM_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00007000)
#define CA5_DBG_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00008000)
#define CA5_PMU_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00009000)
#define CA5_ETM_BASE_ADDR       (AIPS0_BASE_ADDR + 0x0000A000)
#define CA5_ROM_BASE_ADDR       (AIPS0_BASE_ADDR + 0x0000C000)
#define CA5_CTI_BASE_ADDR       (AIPS0_BASE_ADDR + 0x0000E000)
#define HTM_ETB_BASE_ADDR       (AIPS0_BASE_ADDR + 0x0000F000)
#define CA5_ITM_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00010000)
#define CA5_ETB_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00011000)
#define CA5_FUNNEL_BASE_ADDR    (AIPS0_BASE_ADDR + 0x00012000)
#define PTC_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00013000)
#define TPIU_BASE_ADDR          (AIPS0_BASE_ADDR + 0x00014000)
#define MAIN_FUNNEL_BASE_ADDR   (AIPS0_BASE_ADDR + 0x00015000)
#define SWO_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00016000)
#define HTM_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00017000)
#define QUADSPI_0_BASE_ADDR     (AIPS0_BASE_ADDR + 0x00020000)
#define QUADSPI_1_BASE_ADDR     (AIPS0_BASE_ADDR + 0x00021000)
#define MDDRC_BASE_ADDR         (AIPS0_BASE_ADDR + 0x00024000)
#define TCON_01_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00050000)
#define TCON_02_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00052000)
#define TCON_03_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00053000)
#define TCON_11_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00054000)
#define TCON_12_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00056000)
#define TCON_13_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00057000)
#define RLE_DEC_BASE_ADDR       (AIPS0_BASE_ADDR + 0x00058000)
#define SGM_BASE_ADDR           (AIPS0_BASE_ADDR + 0x0005C000)
#define ENET_BASE_ADDR          (AIPS0_BASE_ADDR + 0x00064000)
#define LDB_BASE_ADDR           (AIPS0_BASE_ADDR + 0x00068000)
#define MLB50_BASE_ADDR         (AIPS0_BASE_ADDR + 0x0006C000)

/* AIPS 1 */
#define SEMA42_BASE_ADDR        (AIPS1_BASE_ADDR + 0x00003000)
#define MSCM_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00005000)
#define SWT0_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00008000)
#define SWT1_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00009000)
#define SWT2_BASE_ADDR          (AIPS1_BASE_ADDR + 0x0000A000)
#define STM0_BASE_ADDR          (AIPS1_BASE_ADDR + 0x0000C000)
#define STM1_BASE_ADDR          (AIPS1_BASE_ADDR + 0x0000D000)
#define DMA0_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00010000)
#define DMA0_TCD_BASE_ADDR      (AIPS1_BASE_ADDR + 0x00011000)
#define DMA1_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00012000)
#define DMA1_TCD_BASE_ADDR      (AIPS1_BASE_ADDR + 0x00013000)
#define XRDC_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00014000)
#define CSE_BASE_ADDR           (AIPS1_BASE_ADDR + 0x00018000)
#define DMA_MUX0_BASE_ADDR      (AIPS1_BASE_ADDR + 0x00021000)
#define I2C0_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00022000)
#define I2C1_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00023000)
#define FTM0_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00024000)
#define FTM1_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00025000)
#define FTM2_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00026000)
#define FTM3_BASE_ADDR          (AIPS1_BASE_ADDR + 0x00027000)
#define DSPI0_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00033000)
#define DSPI1_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00034000)
#define DSPI2_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00035000)
#define DSPI3_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00036000)
#define DSPI4_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00037000)
#define UART0_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00038000)
#define UART1_BASE_ADDR         (AIPS1_BASE_ADDR + 0x00039000)
#define UART2_BASE_ADDR         (AIPS1_BASE_ADDR + 0x0003A000)
#define FLEXCAN1_BASE_ADDR      (AIPS1_BASE_ADDR + 0x0003C000)
#define FLEXCAN2_BASE_ADDR      (AIPS1_BASE_ADDR + 0x0003D000)
#define ADC_BASE_ADDR           (AIPS1_BASE_ADDR + 0x0003E000)
#define PIT_BASE_ADDR           (AIPS1_BASE_ADDR + 0x00044000)
#define MC_RGM0_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00050000)
#define MC_RGM1_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00052000)
#define MC_RGM2_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00053000)
#define MC_CGM0_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00054000)
#define MC_CGM1_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00056000)
#define MC_CGM2_BASE_ADDR       (AIPS1_BASE_ADDR + 0x00057000)
#define MC_ME0_BASE_ADDR        (AIPS1_BASE_ADDR + 0x00058000)
#define MC_ME1_BASE_ADDR        (AIPS1_BASE_ADDR + 0x0005A000)
#define MC_ME2_BASE_ADDR        (AIPS1_BASE_ADDR + 0x0005B000)
#define SIUL2_BASE_ADDR         (AIPS1_BASE_ADDR + 0x0005C000)
#define SSCM0_BASE_ADDR         (AIPS1_BASE_ADDR + 0x0006C000)
#define SSCM1_BASE_ADDR         (AIPS1_BASE_ADDR + 0x0006D000)
#define SSCM2_BASE_ADDR         (AIPS1_BASE_ADDR + 0x0006E000)
#define FLEXCAN0_BASE_ADDR      (AIPS1_BASE_ADDR + 0x0007F000)

/* TODO Remove this after the IOMUX framework is implemented */
#define IOMUXC_BASE_ADDR SIUL2_BASE_ADDR

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define FEC_QUIRK_ENET_MAC
#define I2C_QUIRK_REG

/* MSCM interrupt rounter */
#define MSCM_IRSPRC_CP0_EN      1
#define MSCM_IRSPRC_NUM         112

/* DDRMC */
#define DDRMC_PHY_DQ_TIMING     0x00002613
#define DDRMC_PHY_DQS_TIMING    0x00002615
#define DDRMC_PHY_CTRL          0x01210080
#define DDRMC_PHY_MASTER_CTRL   0x0001012a
#define DDRMC_PHY_SLAVE_CTRL    0x00012020

#define DDRMC_PHY50_DDR3_MODE   (1 << 12)
#define DDRMC_PHY50_EN_SW_HALF_CYCLE    (1 << 8)

#define DDRMC_CR00_DRAM_CLASS_DDR3      (0x6 << 8)
#define DDRMC_CR00_DRAM_CLASS_LPDDR2    (0x5 << 8)
#define DDRMC_CR00_START                1
#define DDRMC_CR02_DRAM_TINIT(v)        ((v) & 0xffffff)
#define DDRMC_CR10_TRST_PWRON(v)        (v)
#define DDRMC_CR11_CKE_INACTIVE(v)      (v)
#define DDRMC_CR12_WRLAT(v)             (((v) & 0x1f) << 8)
#define DDRMC_CR12_CASLAT_LIN(v)        ((v) & 0x3f)
#define DDRMC_CR13_TRC(v)               (((v) & 0xff) << 24)
#define DDRMC_CR13_TRRD(v)              (((v) & 0xff) << 16)
#define DDRMC_CR13_TCCD(v)              (((v) & 0x1f) << 8)
#define DDRMC_CR13_TBST_INT_INTERVAL(v) ((v) & 0x7)
#define DDRMC_CR14_TFAW(v)              (((v) & 0x3f) << 24)
#define DDRMC_CR14_TRP(v)               (((v) & 0x1f) << 16)
#define DDRMC_CR14_TWTR(v)              (((v) & 0xf) << 8)
#define DDRMC_CR14_TRAS_MIN(v)          ((v) & 0xff)
#define DDRMC_CR16_TMRD(v)              (((v) & 0x1f) << 24)
#define DDRMC_CR16_TRTP(v)              (((v) & 0xf) << 16)
#define DDRMC_CR17_TRAS_MAX(v)          (((v) & 0x1ffff) << 8)
#define DDRMC_CR17_TMOD(v)              ((v) & 0xff)
#define DDRMC_CR18_TCKESR(v)            (((v) & 0x1f) << 8)
#define DDRMC_CR18_TCKE(v)              ((v) & 0x7)
#define DDRMC_CR20_AP_EN                (1 << 24)
#define DDRMC_CR21_TRCD_INT(v)          (((v) & 0xff) << 16)
#define DDRMC_CR21_TRAS_LOCKOUT         (1 << 8)
#define DDRMC_CR21_CCMAP_EN             1
#define DDRMC_CR22_TDAL(v)              (((v) & 0x3f) << 16)
#define DDRMC_CR23_BSTLEN(v)            (((v) & 0x7) << 24)
#define DDRMC_CR23_TDLL(v)              ((v) & 0xff)
#define DDRMC_CR24_TRP_AB(v)            ((v) & 0x1f)
#define DDRMC_CR25_TREF_EN              (1 << 16)
#define DDRMC_CR26_TREF(v)              (((v) & 0xffff) << 16)
#define DDRMC_CR26_TRFC(v)              ((v) & 0x3ff)
#define DDRMC_CR28_TREF_INT(v)          ((v) & 0xffff)
#define DDRMC_CR29_TPDEX(v)             ((v) & 0xffff)
#define DDRMC_CR30_TXPDLL(v)            ((v) & 0xffff)
#define DDRMC_CR31_TXSNR(v)             (((v) & 0xffff) << 16)
#define DDRMC_CR31_TXSR(v)              ((v) & 0xffff)
#define DDRMC_CR33_EN_QK_SREF           (1 << 16)
#define DDRMC_CR34_CKSRX(v)             (((v) & 0xf) << 16)
#define DDRMC_CR34_CKSRE(v)             (((v) & 0xf) << 8)
#define DDRMC_CR38_FREQ_CHG_EN          (1 << 8)
#define DDRMC_CR39_PHY_INI_COM(v)       (((v) & 0xffff) << 16)
#define DDRMC_CR39_PHY_INI_STA(v)       (((v) & 0xff) << 8)
#define DDRMC_CR39_FRQ_CH_DLLOFF(v)     ((v) & 0x3)
#define DDRMC_CR41_PHY_INI_STRT_INI_DIS 1
#define DDRMC_CR48_MR1_DA_0(v)          (((v) & 0xffff) << 16)
#define DDRMC_CR48_MR0_DA_0(v)          ((v) & 0xffff)
#define DDRMC_CR66_ZQCL(v)              (((v) & 0xfff) << 16)
#define DDRMC_CR66_ZQINIT(v)            ((v) & 0xfff)
#define DDRMC_CR67_ZQCS(v)              ((v) & 0xfff)
#define DDRMC_CR69_ZQ_ON_SREF_EX(v)     (((v) & 0xf) << 8)
#define DDRMC_CR70_REF_PER_ZQ(v)        (v)
#define DDRMC_CR72_ZQCS_ROTATE          (1 << 24)
#define DDRMC_CR73_APREBIT(v)           (((v) & 0xf) << 24)
#define DDRMC_CR73_COL_DIFF(v)          (((v) & 0x7) << 16)
#define DDRMC_CR73_ROW_DIFF(v)          (((v) & 0x3) << 8)
#define DDRMC_CR74_BANKSPLT_EN          (1 << 24)
#define DDRMC_CR74_ADDR_CMP_EN          (1 << 16)
#define DDRMC_CR74_CMD_AGE_CNT(v)       (((v) & 0xff) << 8)
#define DDRMC_CR74_AGE_CNT(v)           ((v) & 0xff)
#define DDRMC_CR75_RW_PG_EN             (1 << 24)
#define DDRMC_CR75_RW_EN                (1 << 16)
#define DDRMC_CR75_PRI_EN               (1 << 8)
#define DDRMC_CR75_PLEN                 1
#define DDRMC_CR76_NQENT_ACTDIS(v)      (((v) & 0x7) << 24)
#define DDRMC_CR76_D_RW_G_BKCN(v)       (((v) & 0x3) << 16)
#define DDRMC_CR76_W2R_SPLT_EN          (1 << 8)
#define DDRMC_CR76_CS_EN                1
#define DDRMC_CR77_CS_MAP               (1 << 24)
#define DDRMC_CR77_DI_RD_INTLEAVE       (1 << 8)
#define DDRMC_CR77_SWAP_EN              1
#define DDRMC_CR78_BUR_ON_FLY_BIT(v)    ((v) & 0xf)
#define DDRMC_CR79_CTLUPD_AREF          (1 << 24)
#define DDRMC_CR82_INT_MASK             0x1fffffff
#define DDRMC_CR87_ODT_WR_MAPCS0        (1 << 24)
#define DDRMC_CR87_ODT_RD_MAPCS0        (1 << 16)
#define DDRMC_CR88_TODTL_CMD(v)         (((v) & 0x1f) << 16)
#define DDRMC_CR89_AODT_RWSMCS(v)       ((v) & 0xf)
#define DDRMC_CR91_R2W_SMCSDL(v)        (((v) & 0x7) << 16)
#define DDRMC_CR96_WLMRD(v)             (((v) & 0x3f) << 8)
#define DDRMC_CR96_WLDQSEN(v)           ((v) & 0x3f)
#define DDRMC_CR105_RDLVL_DL_0(v)       (((v) & 0xff) << 8)
#define DDRMC_CR110_RDLVL_DL_1(v)       ((v) & 0xff)
#define DDRMC_CR114_RDLVL_GTDL_2(v)     (((v) & 0xffff) << 8)
#define DDRMC_CR117_AXI0_W_PRI(v)       (((v) & 0x3) << 8)
#define DDRMC_CR117_AXI0_R_PRI(v)       ((v) & 0x3)
#define DDRMC_CR118_AXI1_W_PRI(v)       (((v) & 0x3) << 24)
#define DDRMC_CR118_AXI1_R_PRI(v)       (((v) & 0x3) << 16)
#define DDRMC_CR120_AXI0_PRI1_RPRI(v)   (((v) & 0xf) << 24)
#define DDRMC_CR120_AXI0_PRI0_RPRI(v)   (((v) & 0xf) << 16)
#define DDRMC_CR121_AXI0_PRI3_RPRI(v)   (((v) & 0xf) << 8)
#define DDRMC_CR121_AXI0_PRI2_RPRI(v)   ((v) & 0xf)
#define DDRMC_CR122_AXI1_PRI1_RPRI(v)   (((v) & 0xf) << 24)
#define DDRMC_CR122_AXI1_PRI0_RPRI(v)   (((v) & 0xf) << 16)
#define DDRMC_CR122_AXI0_PRIRLX(v)      ((v) & 0x3ff)
#define DDRMC_CR123_AXI1_PRI3_RPRI(v)   (((v) & 0xf) << 8)
#define DDRMC_CR123_AXI1_PRI2_RPRI(v)   ((v) & 0xf)
#define DDRMC_CR124_AXI1_PRIRLX(v)      ((v) & 0x3ff)
#define DDRMC_CR126_PHY_RDLAT(v)        (((v) & 0x3f) << 8)
#define DDRMC_CR132_WRLAT_ADJ(v)        (((v) & 0x1f) << 8)
#define DDRMC_CR132_RDLAT_ADJ(v)        ((v) & 0x3f)
#define DDRMC_CR139_PHY_WRLV_RESPLAT(v) (((v) & 0xff) << 24)
#define DDRMC_CR139_PHY_WRLV_LOAD(v)    (((v) & 0xff) << 16)
#define DDRMC_CR139_PHY_WRLV_DLL(v)     (((v) & 0xff) << 8)
#define DDRMC_CR139_PHY_WRLV_EN(v)      ((v) & 0xff)
#define DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(v)    (((v) & 0x1f) << 27)
#define DDRMC_CR154_PAD_ZQ_MODE(v)      (((v) & 0x3) << 21)
#define DDRMC_CR155_AXI0_AWCACHE        (1 << 10)
#define DDRMC_CR155_PAD_ODT_BYTE1(v)    ((v) & 0x7)
#define DDRMC_CR158_TWR(v)              ((v) & 0x3f)

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* System Reset Controller (SRC) */
struct src {
    u32 scr;
    u32 sbmr1;
    u32 srsr;
    u32 secr;
    u32 gpsr;
    u32 sicr;
    u32 simr;
    u32 sbmr2;
    u32 gpr0;
    u32 gpr1;
    u32 gpr2;
    u32 gpr3;
    u32 gpr4;
    u32 hab0;
    u32 hab1;
    u32 hab2;
    u32 hab3;
    u32 hab4;
    u32 hab5;
    u32 misc0;
    u32 misc1;
    u32 misc2;
    u32 misc3;
};

/* Periodic Interrupt Timer (PIT) */
struct pit_reg {
    u32 mcr;
    u32 recv0[55];
    u32 ltmr64h;
    u32 ltmr64l;
    u32 recv1[6];
    u32 ldval0;
    u32 cval0;
    u32 tctrl0;
    u32 tflg0;
    u32 ldval1;
    u32 cval1;
    u32 tctrl1;
    u32 tflg1;
    u32 ldval2;
    u32 cval2;
    u32 tctrl2;
    u32 tflg2;
    u32 ldval3;
    u32 cval3;
    u32 tctrl3;
    u32 tflg3;
    u32 ldval4;
    u32 cval4;
    u32 tctrl4;
    u32 tflg4;
    u32 ldval5;
    u32 cval5;
    u32 tctrl5;
    u32 tflg5;
    u32 ldval6;
    u32 cval6;
    u32 tctrl6;
    u32 tflg6;
    u32 ldval7;
    u32 cval7;
    u32 tctrl7;
    u32 tflg7;
};

/* Watchdog Timer (WDOG) */
struct wdog_regs {
    u16 wcr;
    u16 wsr;
    u16 wrsr;
    u16 wicr;
    u16 wmcr;
};

/* LPDDR2/DDR3 SDRAM Memory Controller (DDRMC) */
struct ddrmr_regs {
    u32 cr[162];
    u32 rsvd[94];
    u32 phy[53];
};

/* UART */
struct linflex_fsl {
    u32 lincr1;
    u32 linier;
    u32 linsr;
    u32 linesr;
    u32 uartcr;
    u32 uartsr;
    u32 lintcsr;
    u32 linocr;
    u32 lintocr;
    u32 linfbrr;
    u32 linibrr;
    u32 lincfr;
    u32 lincr2;
    u32 bidr;
    u32 bdrl;
    u32 bdrm;
    u32 ifer;
    u32 ifmi;
    u32 ifmr;
    u32 ifcr0;
    u32 ifcr1;
    u32 ifcr2;
    u32 ifcr3;
    u32 ifcr4;
    u32 ifcr5;
    u32 ifcr6;
    u32 ifcr7;
    u32 ifcr8;
    u32 ifcr9;
    u32 ifcr10;
    u32 ifcr11;
    u32 ifcr12;
    u32 ifcr13;
    u32 ifcr14;
    u32 ifcr15;
    u32 gcr;
    u32 uartpto;
    u32 uartcto;
    u32 dmatxe;
    u32 dmarxe;
};

/* MSCM Interrupt Router */
struct mscm_ir {
    u32 ircp0ir;
    u32 ircp1ir;
    u32 rsvd1[6];
    u32 ircpgir;
    u32 rsvd2[23];
    u16 irsprc[112];
    u16 rsvd3[848];
};

#endif    /* __ASSEMBLER__*/

#endif    /* __ASM_ARCH_IMX_REGS_H__ */
