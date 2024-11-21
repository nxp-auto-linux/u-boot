/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 NXP
 */

#ifndef __NXP_XSPI_H
#define __NXP_XSPI_H

/* XSPI Register defination */

#define XSPI_MCR 0x0

#define XSPI_MCR_CKN_FA_EN_MASK BIT(26)
#define XSPI_MCR_CKN_FA_EN_SHIFT 26
#define XSPI_MCR_DQS_FA_SEL_MASK GENMASK(25, 24)
#define XSPI_MCR_DQS_FA_SEL_SHIFT 24
#define XSPI_MCR_DQS_FA_SEL(x) ((x) << XSPI_MCR_DQS_FA_SEL_SHIFT)
#define XSPI_MCR_ISD3FA_MASK BIT(17)
#define XSPI_MCR_ISD3FA_SHIFT 17
#define XSPI_MCR_ISD3FA_MASK BIT(17)
#define XSPI_MCR_ISD3FA_SHIFT 17
#define XSPI_MCR_ISD2FA_MASK BIT(16)
#define XSPI_MCR_ISD2FA_SHIFT 16
#define XSPI_MCR_MDIS_MASK BIT(14)
#define XSPI_MCR_MDIS_SHIFT 14
#define XSPI_MCR_CLR_TXF_MASK BIT(11)
#define XSPI_MCR_CLR_TXF_SHIFT 11
#define XSPI_MCR_CLR_RXF_MASK BIT(10)
#define XSPI_MCR_CLR_RXF_SHIFT 10
#define XSPI_MCR_IPS_TG_RST_SHIFT 9
#define XSPI_MCR_DDR_EN_MASK BIT(7)
#define XSPI_MCR_DDR_EN_SHIFT 7
#define XSPI_MCR_DQS_EN_MASK BIT(6)
#define XSPI_MCR_DQS_EN_SHIFT 6
#define XSPI_MCR_DQS_LAT_EN_MASK BIT(5)
#define XSPI_MCR_DQS_LAT_EN_SHIFT 5
#define XSPI_MCR_DQS_OUT_EN_MASK BIT(4)
#define XSPI_MCR_DQS_OUT_EN_SHIFT 4
#define XSPI_MCR_END_CFG_MASK GENMASK(3, 2)
#define XSPI_MCR_END_CFG_SHIFT 2
#define XSPI_MCR_END_CFG(x) ((x) << XSPI_MCR_END_CFG_SHIFT)
#define XSPI_MCR_SWRSTHD_MASK BIT(1)
#define XSPI_MCR_SWRSTHD_SHIFT 1
#define XSPI_MCR_SWRSTSD_MASK BIT(0)
#define XSPI_MCR_SWRSTSD_SHIFT 0

#define XSPI_IPCR 0x8U

#define XSPI_IPCR_SEQID_MASK GENMASK(27, 24)
#define XSPI_IPCR_SEQID_SHIFT 24
#define XSPI_IPCR_SEQID(x) ((x) << XSPI_IPCR_SEQID_SHIFT)
#define XSPI_IPCR_IDATSZ_MASK GENMASK(14, 0)
#define XSPI_IPCR_IDATSZ_SHIFT 0
#define XSPI_IPCR_IDATSZ(x) ((x) << XSPI_IPCR_IDATSZ_SHIFT)

#define XSPI_FLSHCR 0xCU

#define XSPI_FLSHCR_TDH_MASK GENMASK(17, 16)
#define XSPI_FLSHCR_TDH_SHIFT 16
#define XSPI_FLSHCR_TDH(x) ((x) << XSPI_FLSHCR_TDH_SHIFT)
#define XSPI_FLSHCR_TCSH_MASK GENMASK(11, 8)
#define XSPI_FLSHCR_TCSH_SHIFT 8
#define XSPI_FLSHCR_TCSH(x) ((x) << XSPI_FLSHCR_TCSH_SHIFT)
#define XSPI_FLSHCR_TCSS_MASK GENMASK(3, 0)
#define XSPI_FLSHCR_TCSS_SHIFT 0
#define XSPI_FLSHCR_TCSS(x) ((x) << XSPI_FLSHCR_TCSS_SHIFT)

#define XSPI_BUF0CR 0x010U

#define XSPI_BUF0CR_ADATSZ_MASK GENMASK(17, 8)
#define XSPI_BUF0CR_ADATSZ_SHIFT 8
#define XSPI_BUF0CR_ADATSZ(x) ((x) << XSPI_BUF0CR_ADATSZ_SHIFT)
#define XSPI_BUF0CR_MSTRID_MASK GENMASK(3, 0)
#define XSPI_BUF0CR_MSTRID_SHIFT 0
#define XSPI_BUF0CR_MSTRID(x) ((x) << XSPI_BUF0CR_MSTRID_SHIFT)

#define XSPI_BUF1CR 0x014U

#define XSPI_BUF2CR 0x018U

#define XSPI_BUF3CR 0x1CU

#define XSPI_BUF3CR_ALLMST_MASK BIT(31)
#define XSPI_BUF3CR_ALLMST_SHIFT 31
#define XSPI_BUF3CR_ADATSZ_MASK GENMASK(17, 8)
#define XSPI_BUF3CR_ADATSZ_SHIFT 8
#define XSPI_BUF3CR_ADATSZ(x) ((x) << XSPI_BUF3CR_ADATSZ_SHIFT)
#define XSPI_BUF3CR_MSTRID_MASK GENMASK(3, 0)
#define XSPI_BUF3CR_MSTRID_SHIFT 0
#define XSPI_BUF3CR_MSTRID(x) ((x) << XSPI_BUF3CR_MSTRID_SHIFT)

#define XSPI_BUF0IND 0x030U

#define XSPI_BUF1IND 0x034U

#define XSPI_BUF2IND 0x038U

#define XSPI_DLLCRA 0x60U

#define XSPI_DLLCRA_DLLEN_MASK BIT(31)
#define XSPI_DLLCRA_DLLEN_SHIFT 31
#define XSPI_DLLCRA_FREQEN_MASK BIT(30)
#define XSPI_DLLCRA_FREQEN_SHIFT 30
#define XSPI_DLLCRA_DLL_REFCNTR_MASK GENMASK(27, 24)
#define XSPI_DLLCRA_DLL_REFCNTR_SHIFT 24
#define XSPI_DLLCRA_DLL_REFCNTR(x) ((x) << XSPI_DLLCRA_DLL_REFCNTR_SHIFT)
#define XSPI_DLLCRA_DLLRES_MASK GENMASK(23, 20)
#define XSPI_DLLCRA_DLLRES_SHIFT 20
#define XSPI_DLLCRA_DLLRES(x) ((x) << XSPI_DLLCRA_DLLRES_SHIFT)
#define XSPI_DLLCRA_SLV_DLY_COARSE_MASK GENMASK(11, 8)
#define XSPI_DLLCRA_SLV_DLY_COARSE_SHIFT 8
#define XSPI_DLLCRA_SLV_DLY_COARSE(x) ((x) << XSPI_DLLCRA_SLV_DLY_COARSE_SHIFT)
#define XSPI_DLLCRA_SLAVE_AUTO_UPDT_MASK BIT(3)
#define XSPI_DLLCRA_SLAVE_AUTO_UPDT_SHIFT 3
#define XSPI_DLLCRA_SLV_EN_MASK BIT(2)
#define XSPI_DLLCRA_SLV_EN_SHIFT 2
#define XSPI_DLLCRA_SLV_DLL_BYPASS_MASK BIT(1)
#define XSPI_DLLCRA_SLV_DLL_BYPASS_SHIFT 1
#define XSPI_DLLCRA_SLV_UPD_MASK BIT(0)
#define XSPI_DLLCRA_SLV_UPD_SHIFT 0

#define XSPI_SFACR 0x104U

#define XSPI_SFACR_FORCE_A10_MASK BIT(22)
#define XSPI_SFACR_FORCE_A10_SHIFT 22
#define XSPI_SFACR_WA_4B_EN_MASK BIT(21)
#define XSPI_SFACR_WA_4B_EN_SHIFT 21
#define XSPI_SFACR_CAS_INTRLVD_SHIFT 20
#define XSPI_SFACR_RX_BP_EN_SHIFT 18
#define XSPI_SFACR_BYTE_SWAP_MASK BIT(17)
#define XSPI_SFACR_BYTE_SWAP_SHIFT 17
#define XSPI_SFACR_WA_MASK BIT(16)
#define XSPI_SFACR_WA_SHIFT 16
#define XSPI_SFACR_CAS_MASK GENMASK(3, 0)
#define XSPI_SFACR_CAS_SHIFT 0
#define XSPI_SFACR_CAS(x) ((x) << XSPI_SFACR_CAS_SHIFT)

#define XSPI_SMPR 0x108U

#define XSPI_SMPR_DLLFSMPFA_MASK GENMASK(26, 24)
#define XSPI_SMPR_DLLFSMPFA_SHIFT 24
#define XSPI_SMPR_DLLFSMPFA(x) ((x) << 24)

#define XSPI_RBCT 0x110U

#define XSPI_RBCT_WMRK_MASK GENMASK(8, 0)
#define XSPI_RBCT_WMRK_SHIFT 0
#define XSPI_RBCT_WMRK(x) ((x) << XSPI_RBCT_WMRK_SHIFT)

#define XSPI_DLLSR 0x12CU

#define XSPI_DLLSR_DLLA_LOCK_MASK BIT(15)
#define XSPI_DLLSR_DLLA_LOCK_SHIFT 15
#define XSPI_DLLSR_SLVA_LOCK_MASK BIT(14)
#define XSPI_DLLSR_SLVA_LOCK_SHIFT 14
#define XSPI_DLLSR_DLLA_RANGE_ERR_SHIFT 13

#define XSPI_TBSR 0x150U

#define XSPI_TBSR_TRCTR_MASK GENMASK(31, 16)
#define XSPI_TBSR_TRCTR_SHIFT 16
#define XSPI_TBSR_TRCTR(x) ((x) << XSPI_TBSR_TRCTR_SHIFT)

#define XSPI_TBDR 0x154U

#define XSPI_TBDR_TXDATA_MASK GENMASK(31, 0)
#define XSPI_TBDR_TXDATA_SHIFT 0
#define XSPI_TBDR_TXDATA(x) ((x) << XSPI_TBDR_TXDATA_SHIFT)

#define XSPI_TBCT 0x158U

#define XSPI_TBCT_WMRK_MASK GENMASK(7, 0)
#define XSPI_TBCT_WMRK_SHIFT 0
#define XSPI_TBCT_WMRK(x) ((x) << XSPI_TBCT_WMRK_SHIFT)

#define XSPI_SR 0x15CU

#define XSPI_SR_TXFULL_MASK BIT(27)
#define XSPI_SR_TXFULL_SHIFT 27
#define XSPI_SR_BUSY_MASK BIT(0)
#define XSPI_SR_BUSY_SHIFT 0

#define XSPI_FR 0x160U

#define XSPI_FR_ILLINE_MASK BIT(23)
#define XSPI_FR_ILLINE_SHIFT 23

#define XSPI_SFA1AD 0x180U

#define XSPI_SFA2AD 0x184U

#define XSPI_RBDR 0x200U

#define XSPI_LUTKEY 0x300U

#define XSPI_LCKCR 0x304U

#define XSPI_LCKCR_UNLOCK_MASK BIT(1)
#define XSPI_LCKCR_UNLOCK_SHIFT 1
#define XSPI_LCKCR_LOCK_MASK BIT(0)
#define XSPI_LCKCR_LOCK_SHIFT 0

#define XSPI_LUT			0x310

#define XSPI_BFGENCR 0x20

#define XSPI_BFGENCR_ALIGN_MASK GENMASK(23, 22)
#define XSPI_BFGENCR_ALIGN_SHIFT 22
#define XSPI_BFGENCR_ALIGN(x) ((x) << 22)
#define XSPI_BFGENCR_WR_FLUSH_EN_MASK BIT(21)
#define XSPI_BFGENCR_SEQID_MASK GENMASK(15, 12)
#define XSPI_BFGENCR_SEQID_SHIFT 12
#define XSPI_BFGENCR_SEQID(x) ((x) << XSPI_BFGENCR_SEQID_SHIFT)

#define XSPI_FRAD0_WORD2 0x808U

#define XSPI_FRAD0_WORD2_MD1ACP_MASK GENMASK(5, 3)
#define XSPI_FRAD0_WORD2_MD1ACP_SHIFT 3
#define XSPI_FRAD0_WORD2_MD1ACP(x) ((x) << 3)
#define XSPI_FRAD0_WORD2_MD0ACP_MASK GENMASK(2, 0)
#define XSPI_FRAD0_WORD2_MD0ACP_SHIFT 0
#define XSPI_FRAD0_WORD2_MD0ACP(x) ((x) << 0)

#define XSPI_FRAD0_WORD3 0x80CU

#define XSPI_FRAD0_WORD3_VLD_MASK BIT(31)
#define XSPI_FRAD0_WORD3_VLD_SHIFT 31

#define XSPI_TG0MDAD 0x900U

#define XSPI_TG1MDAD 0x910U

#define XSPI_MGC 0x920

#define XSPI_MGC_GVLD_MASK BIT(31)
#define XSPI_MGC_GVLD_SHIFT 31
#define XSPI_MGC_GVLDMDAD_MASK BIT(29)
#define XSPI_MGC_GVLDMDAD_SHIFT 29
#define XSPI_MGC_GVLDFRAD_MASK BIT(27)
#define XSPI_MGC_GVLDFRAD_SHIFT 27

#define XSPI_MTO 0x928

#define XSPI_MTO_SFP_ACC_TO_MASK GENMASK(31, 0)

#define XSPI_TG2MDAD_EXT 0x940U

#define XSPI_TG2MDAD_EXT_VLD_MASK BIT(31)

#define XSPI_TG3MDAD_EXT 0x944U

#define XSPI_TG4MDAD_EXT 0x948U

#define XSPI_SFP_TG_IPCR 0x958U

#define XSPI_SFP_TG_IPCR_SEQID_MASK GENMASK(27, 24)
#define XSPI_SFP_TG_IPCR_SEQID_SHIFT 24
#define XSPI_SFP_TG_IPCR_SEQID(x) ((x) << XSPI_SFP_TG_IPCR_SEQID_SHIFT)
#define XSPI_SFP_TG_IPCR_IDATSZ_MASK GENMASK(15, 0)
#define XSPI_SFP_TG_IPCR_IDATSZ_SHIFT 0
#define XSPI_SFP_TG_IPCR_IDATSZ(x) ((x) << XSPI_SFP_TG_IPCR_IDATSZ_SHIFT)

#define XSPI_SFP_TG_SFAR 0x95CU

#define XSPI_FRAD8_WORD2 0x988U

#define XSPI_FRAD8_WORD3 0x98CU

/********* XSPI CMD definitions ***************************/
#define CMD_SDR 0x01U
#define CMD_DDR 0x11U
#define RADDR_SDR 0x02U
#define RADDR_DDR 0x0AU
#define WRITE_SDR 0x08U
#define WRITE_DDR 0x0FU
#define READ_SDR 0x07U
#define READ_DDR 0x0EU
#define DUMMY_CYCLE 0x03U
#define CMD_STOP 0U

#define DEFAULT_XMIT_SIZE 0x40U

#define ENV_ADDR_SIZE 0x10000

#define XSPI_LUT_KEY_VAL		(0x5AF05AF0UL)
#define XSPI_LCKER_LOCK			BIT(0)
#define XSPI_LCKER_UNLOCK		BIT(1)

#define xspi_get_reg_field(x, env, reg_name, field_name) ({  \
	u32 reg;                                \
	reg = xspi_readl_offset((x), (env), reg_name);  \
	reg &= XSPI_##reg_name##_##field_name##_MASK; \
	reg = reg >> XSPI_##reg_name##_##field_name##_SHIFT; \
	reg; \
})

#define xspi_set_reg_field(x, env, val, reg_name, field_name) do {  \
	u32 reg;                                \
	reg = xspi_readl_offset((x), (env), reg_name);  \
	reg &= ~XSPI_##reg_name##_##field_name##_MASK; \
	reg |= ((val) << XSPI_##reg_name##_##field_name##_SHIFT); \
	xspi_writel_offset(x, (env), reg, reg_name);  \
} while (0)

#define xspi_writel_offset(x, env, val, offset)  do {    \
	if ((x)->devtype_data->little_endian) \
		out_le32((void __iomem *)(uintptr_t)(x)->iobase + \
			 ((env) * ENV_ADDR_SIZE) + XSPI_##offset, (val));   \
	else    \
		out_be32((void __iomem *)(uintptr_t)(x)->iobase + \
			 ((env) * ENV_ADDR_SIZE) + XSPI_##offset, (val));   \
} while (0)

#define xspi_readl_offset(x, env, offset)  ({    \
	u32 reg;    \
	if ((x)->devtype_data->little_endian) \
		reg = in_le32((void __iomem *)(uintptr_t)(x)->iobase + \
			      ((env) * ENV_ADDR_SIZE) + XSPI_##offset); \
	else    \
		reg = in_be32((void __iomem *)(uintptr_t)(x)->iobase + \
			      ((env) * ENV_ADDR_SIZE) + XSPI_##offset);  \
	reg;    \
})

#define POLL_TOUT		5000

#define CMD_LUT_FOR_IP_CMD 1
#define CMD_LUT_FOR_AHB_CMD 0

/*
 * Calculate number of required PAD bits for LUT register.
 *
 * The pad stands for the number of IO lines [0:7].
 * For example, the octal read needs eight IO lines,
 * so you should use LUT_PAD(8). This macro
 * returns 3 i.e. use eight (2^3) IP lines for read.
 */
#define LUT_PAD(x) (fls(x) - 1)

/*
 * Macro for constructing the LUT entries with the following
 * register layout:
 *
 *  ---------------------------------------------------
 *  | INSTR1 | PAD1 | OPRND1 | INSTR0 | PAD0 | OPRND0 |
 *  ---------------------------------------------------
 */
#define PAD_SHIFT		8
#define INSTR_SHIFT		10
#define OPRND_SHIFT		16

/* Macros for constructing the LUT register. */
#define LUT_DEF(idx, ins, pad, opr)			  \
	((((ins) << INSTR_SHIFT) | ((pad) << PAD_SHIFT) | \
	(opr)) << (((idx) % 2) * OPRND_SHIFT))

#define XSPI_LUT_PAIRS  5
#define XSPI_LUT_REG(seq, idx) \
	(XSPI_LUT + ((seq) * XSPI_LUT_PAIRS + (idx)) * 4)

/* xspi data structure */
struct nxp_xspi_devtype_data {
	unsigned int rxfifo;
	unsigned int rx_buf_size;
	unsigned int txfifo;
	unsigned int ahb_buf_size;
	unsigned int quirks;
	bool little_endian;
};

struct nxp_xspi {
	struct udevice *dev;
	u32 iobase;
	u32 ahb_addr;
	u32 a1_size;
	u32 a2_size;
	struct {
		bool gmid:1;
		u32 env:3;
	} config;
	struct clk clk;
	struct nxp_xspi_devtype_data *devtype_data;
	unsigned long support_max_rate;
	bool dtr;
};

#endif
