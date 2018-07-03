/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

typedef u32 uint32_t;
typedef volatile u32 vuint32_t;

typedef volatile u16 vuint16_t;

typedef volatile u8 vuint8_t;

#ifndef CONFIG_DEBUG_S32_QSPI_QSPI
/* ============================================================================
   =============================== Module: QuadSPI ============================
   ============================================================================ */

typedef union QuadSPI_MCR_union_tag {  /* Module Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t SWRSTSD:1;
    vuint32_t SWRSTHD:1;
    vuint32_t END_CFG:2;
    vuint32_t  :1;
    vuint32_t DQS_LAT_EN:1;
    vuint32_t DQS_EN:1;
    vuint32_t DDR_EN:1;
    vuint32_t  :2;
    vuint32_t CLR_RXF:1;
    vuint32_t CLR_TXF:1;
    vuint32_t  :2;
    vuint32_t MDIS:1;
    vuint32_t  :1;
    vuint32_t  :4;
    vuint32_t  :4;
    vuint32_t SCLKCFG:8;
  } B;
} QuadSPI_MCR_tag;

typedef union QuadSPI_IPCR_union_tag { /* IP Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t IDATSZ:16;
    vuint32_t PAR_EN:1;
    vuint32_t  :7;
    vuint32_t SEQID:4;
    vuint32_t  :4;
  } B;
} QuadSPI_IPCR_tag;

typedef union QuadSPI_FLSHCR_union_tag { /* Flash Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t TCSS:4;
    vuint32_t  :4;
    vuint32_t TCSH:4;
    vuint32_t  :4;
    vuint32_t TDH:2;
    vuint32_t  :14;
  } B;
} QuadSPI_FLSHCR_tag;

typedef union QuadSPI_BUF0CR_union_tag { /* Buffer0 Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t MSTRID:5;
    vuint32_t  :3;
    vuint32_t ADATSZ:8;                /* AHB data transfer size */
    vuint32_t  :15;
    vuint32_t HP_EN:1;
  } B;
} QuadSPI_BUF0CR_tag;

typedef union QuadSPI_BUF1CR_union_tag { /* Buffer1 Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t MSTRID:5;
    vuint32_t  :3;
    vuint32_t ADATSZ:8;                /* AHB data transfer size */
    vuint32_t  :16;
  } B;
} QuadSPI_BUF1CR_tag;

typedef union QuadSPI_BUF2CR_union_tag { /* Buffer2 Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t MSTRID:5;
    vuint32_t  :3;
    vuint32_t ADATSZ:8;                /* AHB data transfer size */
    vuint32_t  :16;
  } B;
} QuadSPI_BUF2CR_tag;

typedef union QuadSPI_BUF3CR_union_tag { /* Buffer3 Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t MSTRID:5;
    vuint32_t  :3;
    vuint32_t ADATSZ:8;                /* AHB data transfer size */
    vuint32_t  :15;
    vuint32_t ALLMST:1;
  } B;
} QuadSPI_BUF3CR_tag;

typedef union QuadSPI_BFGENCR_union_tag { /* Buffer Generic Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t  :12;
    vuint32_t SEQID:4;
    vuint32_t PAR_EN:1;
    vuint32_t  :15;
  } B;
} QuadSPI_BFGENCR_tag;

typedef union QuadSPI_SOCCR_union_tag { /* SOC Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t SOCCFG:32;
  } B;
} QuadSPI_SOCCR_tag;

typedef union QuadSPI_BUF0IND_union_tag { /* Buffer0 Top Index Register */
  vuint32_t R;
  struct {
    vuint32_t  :3;
    vuint32_t TPINDX0:29;
  } B;
} QuadSPI_BUF0IND_tag;

typedef union QuadSPI_BUF1IND_union_tag { /* Buffer1 Top Index Register */
  vuint32_t R;
  struct {
    vuint32_t  :3;
    vuint32_t TPINDX1:29;
  } B;
} QuadSPI_BUF1IND_tag;

typedef union QuadSPI_BUF2IND_union_tag { /* Buffer2 Top Index Register */
  vuint32_t R;
  struct {
    vuint32_t  :3;
    vuint32_t TPINDX2:29;
  } B;
} QuadSPI_BUF2IND_tag;

typedef union QuadSPI_SFAR_union_tag { /* Serial Flash Address Register */
  vuint32_t R;
  struct {
    vuint32_t SFADR:32;
  } B;
} QuadSPI_SFAR_tag;

typedef union QuadSPI_SFACR_union_tag { /* Serial Flash Address Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t CAS:4;                   /* Column Address Space */
    vuint32_t  :12;
    vuint32_t WA:1;                    /* Word Addressable */
    vuint32_t  :15;
  } B;
} QuadSPI_SFACR_tag;

typedef union QuadSPI_SMPR_union_tag { /* Sampling Register */
  vuint32_t R;
  struct {
    vuint32_t HSENA:1;                 /* Half Speed serial flash clock Enable */
    vuint32_t HSPHS:1;                 /* Half Speed Phase selection for SDR instructions. */
    vuint32_t HSDLY:1;                 /* Half Speed Delay selection for SDR instructions. */
    vuint32_t  :2;
    vuint32_t FSPHS:1;                 /* Full Speed Phase selection for SDR instructions. */
    vuint32_t FSDLY:1;                 /* Full Speed Delay selection for SDR instructions. Select the delay with respect to the reference edge for the sample point valid for full speed commands: */
    vuint32_t  :9;
    vuint32_t DDRSMP:3;                /* DDR Sampling point */
    vuint32_t  :13;
  } B;
} QuadSPI_SMPR_tag;

typedef union QuadSPI_RBSR_union_tag { /* RX Buffer Status Register */
  vuint32_t R;
  struct {
    vuint32_t  :8;
    vuint32_t RDBFL:6;
    vuint32_t  :2;
    vuint32_t RDCTR:16;
  } B;
} QuadSPI_RBSR_tag;

typedef union QuadSPI_RBCT_union_tag { /* RX Buffer Control Register */
  vuint32_t R;
  struct {
    vuint32_t WMRK:5;
    vuint32_t  :3;
    vuint32_t RXBRD:1;
    vuint32_t  :23;
  } B;
} QuadSPI_RBCT_tag;

typedef union QuadSPI_TBSR_union_tag { /* TX Buffer Status Register */
  vuint32_t R;
  struct {
    vuint32_t  :8;
    vuint32_t TRBFL:6;
    vuint32_t  :2;
    vuint32_t TRCTR:16;
  } B;
} QuadSPI_TBSR_tag;

typedef union QuadSPI_TBDR_union_tag { /* TX Buffer Data Register */
  vuint32_t R;
  struct {
    vuint32_t TXDATA:32;
  } B;
} QuadSPI_TBDR_tag;

typedef union QuadSPI_TBCT_union_tag { /* Tx Buffer Control Register */
  vuint32_t R;
  struct {
    vuint32_t WMRK:5;
    vuint32_t  :27;
  } B;
} QuadSPI_TBCT_tag;

typedef union QuadSPI_SR_union_tag {   /* Status Register */
  vuint32_t R;
  struct {
    vuint32_t BUSY:1;
    vuint32_t IP_ACC:1;
    vuint32_t AHB_ACC:1;
    vuint32_t KEY_FET:1;
    vuint32_t  :1;
    vuint32_t AHBGNT:1;
    vuint32_t AHBTRN:1;
    vuint32_t AHB0NE:1;
    vuint32_t AHB1NE:1;
    vuint32_t AHB2NE:1;
    vuint32_t AHB3NE:1;
    vuint32_t AHB0FUL:1;
    vuint32_t AHB1FUL:1;
    vuint32_t AHB2FUL:1;
    vuint32_t AHB3FUL:1;
    vuint32_t  :1;
    vuint32_t RXWE:1;
    vuint32_t  :2;
    vuint32_t RXFULL:1;
    vuint32_t  :3;
    vuint32_t RXDMA:1;
    vuint32_t TXEDA:1;                 /* Tx Buffer Enough Data Available */
    vuint32_t TXWA:1;                  /* TX Buffer watermark Available */
    vuint32_t TXDMA:1;                 /* TXDMA */
    vuint32_t TXFULL:1;
    vuint32_t  :1;
    vuint32_t DLPSMP:3;
  } B;
} QuadSPI_SR_tag;

typedef union QuadSPI_FR_union_tag {   /* Flag Register */
  vuint32_t R;
  struct {
    vuint32_t TFF:1;
    vuint32_t  :3;
    vuint32_t IPGEF:1;
    vuint32_t  :1;
    vuint32_t IPIEF:1;
    vuint32_t IPAEF:1;
    vuint32_t  :3;
    vuint32_t IUEF:1;
    vuint32_t ABOF:1;
    vuint32_t AIBSEF:1;
    vuint32_t AITEF:1;
    vuint32_t ABSEF:1;
    vuint32_t RBDF:1;
    vuint32_t RBOF:1;
    vuint32_t  :5;
    vuint32_t ILLINE:1;
    vuint32_t  :2;
    vuint32_t TBUF:1;
    vuint32_t TBFF:1;
    vuint32_t  :1;
    vuint32_t KFEF:1;                  /* Key Fetch Error Flag */
    vuint32_t IAKFEF:1;
    vuint32_t DLPFF:1;
  } B;
} QuadSPI_FR_tag;

typedef union QuadSPI_RSER_union_tag { /* Interrupt and DMA Request Select and Enable Register */
  vuint32_t R;
  struct {
    vuint32_t TFIE:1;
    vuint32_t  :3;
    vuint32_t IPGEIE:1;
    vuint32_t  :1;
    vuint32_t IPIEIE:1;
    vuint32_t IPAEIE:1;
    vuint32_t  :3;
    vuint32_t IUEIE:1;
    vuint32_t ABOIE:1;
    vuint32_t AIBSIE:1;
    vuint32_t AITIE:1;
    vuint32_t ABSEIE:1;
    vuint32_t RBDIE:1;
    vuint32_t RBOIE:1;
    vuint32_t  :3;
    vuint32_t RBDDE:1;
    vuint32_t  :1;
    vuint32_t ILLINIE:1;
    vuint32_t  :1;
    vuint32_t TBFDE:1;                 /* TX Buffer Fill DMA Enable */
    vuint32_t TBUIE:1;
    vuint32_t TBFIE:1;
    vuint32_t  :1;
    vuint32_t KFEIE:1;
    vuint32_t IAKFIE:1;
    vuint32_t DLPFIE:1;
  } B;
} QuadSPI_RSER_tag;

typedef union QuadSPI_SPNDST_union_tag { /* Sequence Suspend Status Register */
  vuint32_t R;
  struct {
    vuint32_t SUSPND:1;
    vuint32_t  :5;
    vuint32_t SPDBUF:2;
    vuint32_t  :1;
    vuint32_t DATLFT:7;
    vuint32_t  :16;
  } B;
} QuadSPI_SPNDST_tag;

typedef union QuadSPI_SPTRCLR_union_tag { /* Sequence Pointer Clear Register */
  vuint32_t R;
  struct {
    vuint32_t BFPTRC:1;
    vuint32_t  :7;
    vuint32_t IPPTRC:1;
    vuint32_t  :23;
  } B;
} QuadSPI_SPTRCLR_tag;

typedef union QuadSPI_SFA1AD_union_tag { /* Serial Flash A1 Top Address */
  vuint32_t R;
  struct {
    vuint32_t  :10;
    vuint32_t TPADA1:22;
  } B;
} QuadSPI_SFA1AD_tag;

typedef union QuadSPI_SFA2AD_union_tag { /* Serial Flash A2 Top Address */
  vuint32_t R;
  struct {
    vuint32_t  :10;
    vuint32_t TPADA2:22;
  } B;
} QuadSPI_SFA2AD_tag;

typedef union QuadSPI_SFB1AD_union_tag { /* Serial Flash B1Top Address */
  vuint32_t R;
  struct {
    vuint32_t  :10;
    vuint32_t TPADB1:22;
  } B;
} QuadSPI_SFB1AD_tag;

typedef union QuadSPI_SFB2AD_union_tag { /* Serial Flash B2Top Address */
  vuint32_t R;
  struct {
    vuint32_t  :10;
    vuint32_t TPADB2:22;
  } B;
} QuadSPI_SFB2AD_tag;

typedef union QuadSPI_DLPR_union_tag { /* Data Learn Pattern Register */
  vuint32_t R;
  struct {
    vuint32_t DLPV:32;
  } B;
} QuadSPI_DLPR_tag;

typedef union QuadSPI_RBDR_union_tag { /* RX Buffer Data Register */
  vuint32_t R;
  struct {
    vuint32_t RXDATA:32;
  } B;
} QuadSPI_RBDR_tag;

typedef union QuadSPI_LUTKEY_union_tag { /* LUT Key Register */
  vuint32_t R;
  struct {
    vuint32_t KEY:32;
  } B;
} QuadSPI_LUTKEY_tag;

typedef union QuadSPI_LCKCR_union_tag { /* LUT Lock Configuration Register */
  vuint32_t R;
  struct {
    vuint32_t LOCK:1;
    vuint32_t UNLOCK:1;
    vuint32_t  :30;
  } B;
} QuadSPI_LCKCR_tag;

typedef union QuadSPI_LUT_union_tag {  /* Look-up Table register */
  vuint32_t R;
  struct {
    vuint32_t OPRND0:8;                /* Operand for INSTR0. */
    vuint32_t PAD0:2;                  /* Pad information for INSTR0. */
    vuint32_t INSTR0:6;                /* Instruction 0 */
    vuint32_t OPRND1:8;                /* Operand for INSTR1. */
    vuint32_t PAD1:2;                  /* Pad information for INSTR1. */
    vuint32_t INSTR1:6;                /* Instruction 1 */
  } B;
} QuadSPI_LUT_tag;

struct QuadSPI_tag {
  QuadSPI_MCR_tag MCR;                 /* Module Configuration Register */
  uint8_t QuadSPI_reserved0[4];
  QuadSPI_IPCR_tag IPCR;               /* IP Configuration Register */
  QuadSPI_FLSHCR_tag FLSHCR;           /* Flash Configuration Register */
  QuadSPI_BUF0CR_tag BUF0CR;           /* Buffer0 Configuration Register */
  QuadSPI_BUF1CR_tag BUF1CR;           /* Buffer1 Configuration Register */
  QuadSPI_BUF2CR_tag BUF2CR;           /* Buffer2 Configuration Register */
  QuadSPI_BUF3CR_tag BUF3CR;           /* Buffer3 Configuration Register */
  QuadSPI_BFGENCR_tag BFGENCR;         /* Buffer Generic Configuration Register */
  QuadSPI_SOCCR_tag SOCCR;             /* SOC Configuration Register */
  uint8_t QuadSPI_reserved1[8];
  QuadSPI_BUF0IND_tag BUF0IND;         /* Buffer0 Top Index Register */
  QuadSPI_BUF1IND_tag BUF1IND;         /* Buffer1 Top Index Register */
  QuadSPI_BUF2IND_tag BUF2IND;         /* Buffer2 Top Index Register */
  uint8_t QuadSPI_reserved2[196];
  QuadSPI_SFAR_tag SFAR;               /* Serial Flash Address Register */
  QuadSPI_SFACR_tag SFACR;             /* Serial Flash Address Configuration Register */
  QuadSPI_SMPR_tag SMPR;               /* Sampling Register */
  QuadSPI_RBSR_tag RBSR;               /* RX Buffer Status Register */
  QuadSPI_RBCT_tag RBCT;               /* RX Buffer Control Register */
  uint8_t QuadSPI_reserved3[60];
  QuadSPI_TBSR_tag TBSR;               /* TX Buffer Status Register */
  QuadSPI_TBDR_tag TBDR;               /* TX Buffer Data Register */
  QuadSPI_TBCT_tag TBCT;               /* Tx Buffer Control Register */
  QuadSPI_SR_tag SR;                   /* Status Register */
  QuadSPI_FR_tag FR;                   /* Flag Register */
  QuadSPI_RSER_tag RSER;               /* Interrupt and DMA Request Select and Enable Register */
  QuadSPI_SPNDST_tag SPNDST;           /* Sequence Suspend Status Register */
  QuadSPI_SPTRCLR_tag SPTRCLR;         /* Sequence Pointer Clear Register */
  uint8_t QuadSPI_reserved4[16];
  QuadSPI_SFA1AD_tag SFA1AD;           /* Serial Flash A1 Top Address */
  QuadSPI_SFA2AD_tag SFA2AD;           /* Serial Flash A2 Top Address */
  QuadSPI_SFB1AD_tag SFB1AD;           /* Serial Flash B1Top Address */
  QuadSPI_SFB2AD_tag SFB2AD;           /* Serial Flash B2Top Address */
  QuadSPI_DLPR_tag DLPR;               /* Data Learn Pattern Register */
  uint8_t QuadSPI_reserved5[108];
  QuadSPI_RBDR_tag RBDR[32];           /* RX Buffer Data Register */
  uint8_t QuadSPI_reserved6[128];
  QuadSPI_LUTKEY_tag LUTKEY;           /* LUT Key Register */
  QuadSPI_LCKCR_tag LCKCR;             /* LUT Lock Configuration Register */
  uint8_t QuadSPI_reserved7[8];
  QuadSPI_LUT_tag LUT[64];             /* Look-up Table register */
};

#define QuadSPI (*(volatile struct QuadSPI_tag *) QSPI_BASE_ADDR)

/* QuadSPI */
#define QuadSPI_MCR          QuadSPI.MCR.R                 /* Module Configuration Register */
#define QuadSPI_IPCR         QuadSPI.IPCR.R                /* IP Configuration Register */
#define QuadSPI_FLSHCR       QuadSPI.FLSHCR.R              /* Flash Configuration Register */
#define QuadSPI_BUF0CR       QuadSPI.BUF0CR.R              /* Buffer0 Configuration Register */
#define QuadSPI_BUF1CR       QuadSPI.BUF1CR.R              /* Buffer1 Configuration Register */
#define QuadSPI_BUF2CR       QuadSPI.BUF2CR.R              /* Buffer2 Configuration Register */
#define QuadSPI_BUF3CR       QuadSPI.BUF3CR.R              /* Buffer3 Configuration Register */
#define QuadSPI_BFGENCR      QuadSPI.BFGENCR.R             /* Buffer Generic Configuration Register */
#define QuadSPI_SOCCR        QuadSPI.SOCCR.R               /* SOC Configuration Register */
#define QuadSPI_BUF0IND      QuadSPI.BUF0IND.R             /* Buffer0 Top Index Register */
#define QuadSPI_BUF1IND      QuadSPI.BUF1IND.R             /* Buffer1 Top Index Register */
#define QuadSPI_BUF2IND      QuadSPI.BUF2IND.R             /* Buffer2 Top Index Register */
#define QuadSPI_SFAR         QuadSPI.SFAR.R                /* Serial Flash Address Register */
#define QuadSPI_SFACR        QuadSPI.SFACR.R               /* Serial Flash Address Configuration Register */
#define QuadSPI_SMPR         QuadSPI.SMPR.R                /* Sampling Register */
#define QuadSPI_RBSR         QuadSPI.RBSR.R                /* RX Buffer Status Register */
#define QuadSPI_RBCT         QuadSPI.RBCT.R                /* RX Buffer Control Register */
#define QuadSPI_TBSR         QuadSPI.TBSR.R                /* TX Buffer Status Register */
#define QuadSPI_TBDR         QuadSPI.TBDR.R                /* TX Buffer Data Register */
#define QuadSPI_TBCT         QuadSPI.TBCT.R                /* Tx Buffer Control Register */
#define QuadSPI_SR           QuadSPI.SR.R                  /* Status Register */
#define QuadSPI_FR           QuadSPI.FR.R                  /* Flag Register */
#define QuadSPI_RSER         QuadSPI.RSER.R                /* Interrupt and DMA Request Select and Enable Register */
#define QuadSPI_SPNDST       QuadSPI.SPNDST.R              /* Sequence Suspend Status Register */
#define QuadSPI_SPTRCLR      QuadSPI.SPTRCLR.R             /* Sequence Pointer Clear Register */
#define QuadSPI_SFA1AD       QuadSPI.SFA1AD.R              /* Serial Flash A1 Top Address */
#define QuadSPI_SFA2AD       QuadSPI.SFA2AD.R              /* Serial Flash A2 Top Address */
#define QuadSPI_SFB1AD       QuadSPI.SFB1AD.R              /* Serial Flash B1Top Address */
#define QuadSPI_SFB2AD       QuadSPI.SFB2AD.R              /* Serial Flash B2Top Address */
#define QuadSPI_DLPR         QuadSPI.DLPR.R                /* Data Learn Pattern Register */
#define QuadSPI_RBDR0        QuadSPI.RBDR[0].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR1        QuadSPI.RBDR[1].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR2        QuadSPI.RBDR[2].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR3        QuadSPI.RBDR[3].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR4        QuadSPI.RBDR[4].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR5        QuadSPI.RBDR[5].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR6        QuadSPI.RBDR[6].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR7        QuadSPI.RBDR[7].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR8        QuadSPI.RBDR[8].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR9        QuadSPI.RBDR[9].R             /* RX Buffer Data Register */
#define QuadSPI_RBDR10       QuadSPI.RBDR[10].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR11       QuadSPI.RBDR[11].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR12       QuadSPI.RBDR[12].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR13       QuadSPI.RBDR[13].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR14       QuadSPI.RBDR[14].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR15       QuadSPI.RBDR[15].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR16       QuadSPI.RBDR[16].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR17       QuadSPI.RBDR[17].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR18       QuadSPI.RBDR[18].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR19       QuadSPI.RBDR[19].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR20       QuadSPI.RBDR[20].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR21       QuadSPI.RBDR[21].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR22       QuadSPI.RBDR[22].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR23       QuadSPI.RBDR[23].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR24       QuadSPI.RBDR[24].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR25       QuadSPI.RBDR[25].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR26       QuadSPI.RBDR[26].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR27       QuadSPI.RBDR[27].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR28       QuadSPI.RBDR[28].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR29       QuadSPI.RBDR[29].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR30       QuadSPI.RBDR[30].R            /* RX Buffer Data Register */
#define QuadSPI_RBDR31       QuadSPI.RBDR[31].R            /* RX Buffer Data Register */
#define QuadSPI_LUTKEY       QuadSPI.LUTKEY.R              /* LUT Key Register */
#define QuadSPI_LCKCR        QuadSPI.LCKCR.R               /* LUT Lock Configuration Register */
#define QuadSPI_LUT0         QuadSPI.LUT[0].R              /* Look-up Table register */
#define QuadSPI_LUT1         QuadSPI.LUT[1].R              /* Look-up Table register */
#define QuadSPI_LUT2         QuadSPI.LUT[2].R              /* Look-up Table register */
#define QuadSPI_LUT3         QuadSPI.LUT[3].R              /* Look-up Table register */
#define QuadSPI_LUT4         QuadSPI.LUT[4].R              /* Look-up Table register */
#define QuadSPI_LUT5         QuadSPI.LUT[5].R              /* Look-up Table register */
#define QuadSPI_LUT6         QuadSPI.LUT[6].R              /* Look-up Table register */
#define QuadSPI_LUT7         QuadSPI.LUT[7].R              /* Look-up Table register */
#define QuadSPI_LUT8         QuadSPI.LUT[8].R              /* Look-up Table register */
#define QuadSPI_LUT9         QuadSPI.LUT[9].R              /* Look-up Table register */
#define QuadSPI_LUT10        QuadSPI.LUT[10].R             /* Look-up Table register */
#define QuadSPI_LUT11        QuadSPI.LUT[11].R             /* Look-up Table register */
#define QuadSPI_LUT12        QuadSPI.LUT[12].R             /* Look-up Table register */
#define QuadSPI_LUT13        QuadSPI.LUT[13].R             /* Look-up Table register */
#define QuadSPI_LUT14        QuadSPI.LUT[14].R             /* Look-up Table register */
#define QuadSPI_LUT15        QuadSPI.LUT[15].R             /* Look-up Table register */
#define QuadSPI_LUT16        QuadSPI.LUT[16].R             /* Look-up Table register */
#define QuadSPI_LUT17        QuadSPI.LUT[17].R             /* Look-up Table register */
#define QuadSPI_LUT18        QuadSPI.LUT[18].R             /* Look-up Table register */
#define QuadSPI_LUT19        QuadSPI.LUT[19].R             /* Look-up Table register */
#define QuadSPI_LUT20        QuadSPI.LUT[20].R             /* Look-up Table register */
#define QuadSPI_LUT21        QuadSPI.LUT[21].R             /* Look-up Table register */
#define QuadSPI_LUT22        QuadSPI.LUT[22].R             /* Look-up Table register */
#define QuadSPI_LUT23        QuadSPI.LUT[23].R             /* Look-up Table register */
#define QuadSPI_LUT24        QuadSPI.LUT[24].R             /* Look-up Table register */
#define QuadSPI_LUT25        QuadSPI.LUT[25].R             /* Look-up Table register */
#define QuadSPI_LUT26        QuadSPI.LUT[26].R             /* Look-up Table register */
#define QuadSPI_LUT27        QuadSPI.LUT[27].R             /* Look-up Table register */
#define QuadSPI_LUT28        QuadSPI.LUT[28].R             /* Look-up Table register */
#define QuadSPI_LUT29        QuadSPI.LUT[29].R             /* Look-up Table register */
#define QuadSPI_LUT30        QuadSPI.LUT[30].R             /* Look-up Table register */
#define QuadSPI_LUT31        QuadSPI.LUT[31].R             /* Look-up Table register */
#define QuadSPI_LUT32        QuadSPI.LUT[32].R             /* Look-up Table register */
#define QuadSPI_LUT33        QuadSPI.LUT[33].R             /* Look-up Table register */
#define QuadSPI_LUT34        QuadSPI.LUT[34].R             /* Look-up Table register */
#define QuadSPI_LUT35        QuadSPI.LUT[35].R             /* Look-up Table register */
#define QuadSPI_LUT36        QuadSPI.LUT[36].R             /* Look-up Table register */
#define QuadSPI_LUT37        QuadSPI.LUT[37].R             /* Look-up Table register */
#define QuadSPI_LUT38        QuadSPI.LUT[38].R             /* Look-up Table register */
#define QuadSPI_LUT39        QuadSPI.LUT[39].R             /* Look-up Table register */
#define QuadSPI_LUT40        QuadSPI.LUT[40].R             /* Look-up Table register */
#define QuadSPI_LUT41        QuadSPI.LUT[41].R             /* Look-up Table register */
#define QuadSPI_LUT42        QuadSPI.LUT[42].R             /* Look-up Table register */
#define QuadSPI_LUT43        QuadSPI.LUT[43].R             /* Look-up Table register */
#define QuadSPI_LUT44        QuadSPI.LUT[44].R             /* Look-up Table register */
#define QuadSPI_LUT45        QuadSPI.LUT[45].R             /* Look-up Table register */
#define QuadSPI_LUT46        QuadSPI.LUT[46].R             /* Look-up Table register */
#define QuadSPI_LUT47        QuadSPI.LUT[47].R             /* Look-up Table register */
#define QuadSPI_LUT48        QuadSPI.LUT[48].R             /* Look-up Table register */
#define QuadSPI_LUT49        QuadSPI.LUT[49].R             /* Look-up Table register */
#define QuadSPI_LUT50        QuadSPI.LUT[50].R             /* Look-up Table register */
#define QuadSPI_LUT51        QuadSPI.LUT[51].R             /* Look-up Table register */
#define QuadSPI_LUT52        QuadSPI.LUT[52].R             /* Look-up Table register */
#define QuadSPI_LUT53        QuadSPI.LUT[53].R             /* Look-up Table register */
#define QuadSPI_LUT54        QuadSPI.LUT[54].R             /* Look-up Table register */
#define QuadSPI_LUT55        QuadSPI.LUT[55].R             /* Look-up Table register */
#define QuadSPI_LUT56        QuadSPI.LUT[56].R             /* Look-up Table register */
#define QuadSPI_LUT57        QuadSPI.LUT[57].R             /* Look-up Table register */
#define QuadSPI_LUT58        QuadSPI.LUT[58].R             /* Look-up Table register */
#define QuadSPI_LUT59        QuadSPI.LUT[59].R             /* Look-up Table register */
#define QuadSPI_LUT60        QuadSPI.LUT[60].R             /* Look-up Table register */
#define QuadSPI_LUT61        QuadSPI.LUT[61].R             /* Look-up Table register */
#define QuadSPI_LUT62        QuadSPI.LUT[62].R             /* Look-up Table register */
#define QuadSPI_LUT63        QuadSPI.LUT[63].R             /* Look-up Table register */
#endif /* ifndef CONFIG_DEBUG_S32_QSPI_QSPI */
