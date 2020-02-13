// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 *
 * The SerDes module header file.
 */

#ifndef SERDES_H
#define SERDES_H

#include "linux/types.h"
#include "linux/errno.h"

#define EXIT_FAILURE 1

/**
 * @brief	SerDes Modes
 */
typedef enum {
	/*	Lane0=PCIE, Lane1=PCIE */
	SERDES_MODE_PCIE_PCIE = 0,
	/*	Lane0=SGMII (1G), Lane1=PCIE */
	SERDES_MODE_SGMII_PCIE = 1,
	/*	Lane0=PCIE, Lane1=SGMII(1G) */
	SERDES_MODE_PCIE_SGMII = 2,
	/*	Lane0=SGMII(1G/2.5G), Lane1=SGMII(1G/2.5G) */
	SERDES_MODE_SGMII_SGMII = 3,
	SERDES_MODE_MAX = SERDES_MODE_SGMII_SGMII
} serdes_mode_t;

#define SERDES_SS_BASE				0x80000

/*
 * SS Registers
 */
#define SS_PHY_GEN_CTRL				(SERDES_SS_BASE + 0x0U)
#define SS_PHY_LPBK_CTRL			(SERDES_SS_BASE + 0x4U)
#define SS_PHY_SRAM_CSR				(SERDES_SS_BASE + 0x8U)
#define SS_PHY_MPLLA_CTRL			(SERDES_SS_BASE + 0x10U)
#define SS_PHY_MPLLB_CTRL			(SERDES_SS_BASE + 0x14U)
#define SS_PHY_EXT_CTRL_SEL			(SERDES_SS_BASE + 0x18U)
#define SS_PHY_EXT_BS_CTRL			(SERDES_SS_BASE + 0x1cU)
#define SS_PHY_REF_CLK_CTRL			(SERDES_SS_BASE + 0x20U)
#define SS_PHY_EXT_MPLLA_CTRL_1			(SERDES_SS_BASE + 0x30U)
#define SS_PHY_EXT_MPLLA_CTRL_2			(SERDES_SS_BASE + 0x34U)
#define SS_PHY_EXT_MPLLA_CTRL_3			(SERDES_SS_BASE + 0x38U)
#define SS_PHY_EXT_MPLLB_CTRL_1			(SERDES_SS_BASE + 0x40U)
#define SS_PHY_EXT_MPLLB_CTRL_2			(SERDES_SS_BASE + 0x44U)
#define SS_PHY_EXT_MPLLB_CTRL_3			(SERDES_SS_BASE + 0x48U)
#define SS_PHY_EXT_RX_EQ_CTRL_1A		(SERDES_SS_BASE + 0x50U)
#define SS_PHY_EXT_RX_EQ_CTRL_1B		(SERDES_SS_BASE + 0x54U)
#define SS_PHY_EXT_RX_EQ_CTRL_1C		(SERDES_SS_BASE + 0x58U)
#define SS_PHY_EXT_RX_EQ_CTRL_2A		(SERDES_SS_BASE + 0x60U)
#define SS_PHY_EXT_RX_EQ_CTRL_2B		(SERDES_SS_BASE + 0x64U)
#define SS_PHY_EXT_RX_EQ_CTRL_2C		(SERDES_SS_BASE + 0x68U)
#define SS_PHY_EXT_RX_EQ_CTRL_3A		(SERDES_SS_BASE + 0x70U)
#define SS_PHY_EXT_RX_EQ_CTRL_3B		(SERDES_SS_BASE + 0x74U)
#define SS_PHY_EXT_RX_EQ_CTRL_3C		(SERDES_SS_BASE + 0x78U)
#define SS_PHY_EXT_RX_EQ_CTRL_4B		(SERDES_SS_BASE + 0x84U)
#define SS_PHY_EXT_RX_EQ_CTRL_4C		(SERDES_SS_BASE + 0x88U)
#define SS_PHY_EXT_CALI_CTRL_1			(SERDES_SS_BASE + 0x90U)
#define SS_PHY_EXT_CALI_CTRL_2			(SERDES_SS_BASE + 0x94U)
#define SS_PHY_EXT_CALI_CTRL_3			(SERDES_SS_BASE + 0x98U)
#define SS_PHY_EXT_CALI_CTRL_4			(SERDES_SS_BASE + 0x9cU)
#define SS_PHY_EXT_MISC_CTRL_1			(SERDES_SS_BASE + 0xa0U)
#define SS_PHY_EXT_MISC_CTRL_2			(SERDES_SS_BASE + 0xa4U)
#define SS_PHY_EXT_TX_EQ_CTRL_1			(SERDES_SS_BASE + 0xb0U)
#define SS_PHY_EXT_TX_EQ_CTRL_2			(SERDES_SS_BASE + 0xb4U)
#define SS_PHY_EXT_TX_EQ_CTRL_3			(SERDES_SS_BASE + 0xb8U)
#define SS_PHY_XPCS0_RX_OVRD_CTRL		(SERDES_SS_BASE + 0xc0U)
#define SS_PHY_XPCS1_RX_OVRD_CTRL		(SERDES_SS_BASE + 0xd0U)
#define SS_SS_RO_REG_0				(SERDES_SS_BASE + 0xe0U)
#define SS_SS_RO_REG_1				(SERDES_SS_BASE + 0xe4U)
#define SS_SS_RO_REG_2				(SERDES_SS_BASE + 0xe5U)
#define SS_SS_RO_REG_3				(SERDES_SS_BASE + 0xecU)
#define SS_SS_RW_REG_0				(SERDES_SS_BASE + 0xf0U)
#define SS_SS_RW_REG_1				(SERDES_SS_BASE + 0xf4U)
#define SS_SS_RW_REG_2				(SERDES_SS_BASE + 0xf8U)
#define SS_SS_RW_REG_3				(SERDES_SS_BASE + 0xfcU)
#define SS_SS_RW_REG_4				(SERDES_SS_BASE + 0x100U)
#define SS_SS_RW_REG_5				(SERDES_SS_BASE + 0x104U)
#define SS_PCIE_SUBSYSTEM_VERSION		(SERDES_SS_BASE + 0x1000U)
#define SS_LINK_INT_CTRL_STS			(SERDES_SS_BASE + 0x1040U)
#define SS_PFE0_GEN_CTRL_1			(SERDES_SS_BASE + 0x1050U)
#define SS_PFE0_GEN_CTRL_2			(SERDES_SS_BASE + 0x1054U)
#define SS_PFE0_GEN_CTRL_3			(SERDES_SS_BASE + 0x1058U)
#define SS_PFE0_GEN_CTRL_4			(SERDES_SS_BASE + 0x105cU)
#define SS_PE0_PM_CTRL				(SERDES_SS_BASE + 0x1060U)
#define SS_PE0_PM_STS				(SERDES_SS_BASE + 0x1064U)
#define SS_PE0_TX_MSG_HDR_1			(SERDES_SS_BASE + 0x1070U)
#define SS_PE0_TX_MSG_HDR_2			(SERDES_SS_BASE + 0x1074U)
#define SS_PE0_TX_MSG_HDR_3			(SERDES_SS_BASE + 0x1078U)
#define SS_PE0_TX_MSG_HDR_4			(SERDES_SS_BASE + 0x107cU)
#define SS_PE0_TX_MSG_REQ			(SERDES_SS_BASE + 0x1080U)
#define SS_PE0_RX_MSG_HDR_1			(SERDES_SS_BASE + 0x1090U)
#define SS_PE0_RX_MSG_HDR_2			(SERDES_SS_BASE + 0x1094U)
#define SS_PE0_RX_MSG_HDR_3			(SERDES_SS_BASE + 0x1098U)
#define SS_PE0_RX_MSG_HDR_4			(SERDES_SS_BASE + 0x109cU)
#define SS_PE0_RX_MSG_STS			(SERDES_SS_BASE + 0x10a0U)
#define SS_PE0_RX_MSG_CAP_CTRL			(SERDES_SS_BASE + 0x10a4U)
#define SS_PE0_RX_MSG_INT_CTRL			(SERDES_SS_BASE + 0x10a8U)
#define SS_PE0_LINK_DBG_1			(SERDES_SS_BASE + 0x10b0U)
#define SS_PE0_LINK_DBG_2			(SERDES_SS_BASE + 0x10b4U)
#define SS_PE0_AXI_MSTR_DBG_1			(SERDES_SS_BASE + 0x10c0U)
#define SS_PE0_AXI_MSTR_DBG_2			(SERDES_SS_BASE + 0x10c4U)
#define SS_PE0_AXI_SLV_DBG_1			(SERDES_SS_BASE + 0x10d0U)
#define SS_PE0_AXI_SLV_DBG_2			(SERDES_SS_BASE + 0x10d4U)
#define SS_PE0_ERR_STS				(SERDES_SS_BASE + 0x10e0U)
#define SS_PE0_ERR_INT_CTRL			(SERDES_SS_BASE + 0x10e4U)
#define SS_PE0_INT_STS				(SERDES_SS_BASE + 0x10e8U)
#define SS_PE0_MSI_GEN_CTRL			(SERDES_SS_BASE + 0x10ecU)
#define SS_PE0_FSM_TRACK_1			(SERDES_SS_BASE + 0x10f0U)
#define SS_PE0_FSM_TRACK_2			(SERDES_SS_BASE + 0x10f4U)
#define SS_APB_BRIDGE_TO_CTRL			(SERDES_SS_BASE + 0x3000U)
#define SS_PHY_REG_ADDR				(SERDES_SS_BASE + 0x3008U)
#define SS_PHY_REG_DATA				(SERDES_SS_BASE + 0x300cU)
#define SS_RST_CTRL				(SERDES_SS_BASE + 0x3010U)

#define SUBSYS_MODE_VALUE(x)			(((x) & 0x7) << 0)
#define EXT_PCLK_REQ				(1U << 0)
#define PHY_GEN_CTRL_REF_USE_PAD		(1U << 17)

/*
 *		XPCS registers
 */
#define SERDES_XPCS_0_BASE			0x82000U
#define SERDES_XPCS_0_OFFSET			0x0U
#define SERDES_XPCS_0_ADDR1			0x823fcU
#define SERDES_XPCS_0_ADDR2			0x82000U
#define SERDES_XPCS_1_BASE			0x82800U
#define SERDES_XPCS_1_OFFSET			0x800U
#define SERDES_XPCS_1_ADDR1			0x827fcU
#define SERDES_XPCS_1_ADDR2			0x82400U

#define VR_MII_DIG_CTRL1			0x1f8000U
#define VR_MII_AN_CTRL				0x1f8001U
#define VR_MII_AN_INTR_STS			0x1f8002U
#define VR_MII_DBG_CTRL				0x1f8005U
#define VR_MII_LINK_TIMER_CTRL			0x1f800aU
#define VR_MII_DIG_STS				0x1f8010U
#define VR_MII_GEN5_12G_16G_MPLL_CMN_CTRL	0x1f8070U
#define VR_MII_GEN5_12G_16G_MPLLA_CTRL0		0x1f8071U
#define VR_MII_GEN5_12G_MPLLA_CTRL1		0x1f8072U
#define VR_MII_GEN5_12G_16G_MPLLA_CTRL2		0x1f8073U
#define VR_MII_GEN5_12G_16G_MPLLB_CTRL0		0x1f8074U
#define VR_MII_GEN5_12G_MPLLB_CTRL1		0x1f8075U
#define VR_MII_GEN5_12G_16G_MPLLB_CTRL2		0x1f8076U
#define VR_MII_GEN5_12G_MPLLA_CTRL3		0x1f8077U
#define VR_MII_GEN5_12G_MPLLB_CTRL3		0x1f8078U
#define VR_MII_GEN5_12G_VCO_CAL_REF0		0x1f8096U
#define VR_MII_GEN5_12G_16G_TX_RATE_CTRL	0x1f8034U
#define VR_MII_GEN5_12G_16G_RX_RATE_CTRL	0x1f8054U
#define VR_MII_GEN5_12G_16G_CDR_CTRL		0x1f8056U
#define VR_MII_GEN5_12G_16G_VCO_CAL_LD0		0x1f8092U
#define VR_MII_GEN5_12G_16G_REF_CLK_CTRL	0x1f8091U
#define SR_MII_CTRL				0x1f0000U
#define SR_MII_STS				0x1f0001U
#define SR_MII_DEV_ID1				0x1f0002U
#define SR_MII_DEV_ID2				0x1f0003U

#define PCS_MODE_1000_BASE_X			0U
#define PCS_MODE_SGMII				1U
#define PCS_MODE_QSGMII				2U
#define PCS_MODE_XPCS				3U
#define MII_AN_CTRL_PCS_MODE(x)			(((x) & 0x3U) << 1)
#define MII_AN_CTRL_MII_CTRL			(1U << 8)
#define MII_AN_CTRL_TX_CONFIG			(1U << 3)
#define MII_AN_INTR_STS_CL37_ANCMPLT_INTR	(1U << 0)

#define MII_CTRL_SS6				(1U << 6)
#define MII_CTRL_DUPLEX_MODE			(1U << 8)
#define MII_CTRL_RESTART_AN			(1U << 9)
#define MII_CTRL_AN_ENABLE			(1U << 12)
#define MII_CTRL_SS13				(1U << 13)

#define MII_STS_LINK_STS			(1U << 2)

#define MPLL_CMN_CTRL_MPLL_EN_0			(1U << 0)
#define MPLL_CMN_CTRL_MPLLB_SEL_0		(1U << 4)

#define MPLLA_MULTIPLIER_VALUE(x)		(((x) & 0xffU) << 0)

#define MII_DBG_CTRL_SUPRESS_LOS_DET		(1U << 4)
#define MII_DBG_CTRL_RX_DT_EN_CTL		(1U << 6)

#define REF_CLK_CTRL_REF_CLK_EN			(1U << 0)
#define REF_CLK_CTRL_REF_USE_PAD		(1U << 1)
#define REF_CLK_CTRL_REF_CLK_DIV2		(1U << 2)
#define REF_CLK_CTRL_REF_RANGE(x)		(((x) & 0x7U) << 3)
#define REF_CLK_CTRL_REF_MPLLA_DIV2		(1U << 6)
#define REF_CLK_CTRL_REF_MPLLB_DIV2		(1U << 7)
#define REF_CLK_CTRL_REF_RPT_CLK_EN		(1U << 8)

#define EN_2_5G_MODE				(1U << 2)
#define MAC_AUTO_SW				(1U << 9)
#define CS_EN					(1U << 10)
#define PWRSV					(1U << 11)
#define EN_VSMMD1				(1U << 13)
#define R2TLBE					(1U << 14)
#define VR_RST					(1U << 15)

#define MPLLA_DIV8_CLK_EN			(1U << 8)
#define MPLLA_DIV10_CLK_EN			(1U << 9)
#define MPLLA_DIV_CLK_EN			(1U << 10)
#define MPLLA_TX_CLK_DIV(x)			(((x) & 0x7U) << 11)
#define MPLLA_CAL_DISABLE			(1U << 15)

#define MPPLB_CAL_DISABLE			(1U << 15)
#define MPLLB_MULTIPLIER(n)			(((n) & 0xffU) << 0)

#define MPLLB_CTRL2_MPLLB_DIV_MULT(n)		(((n) & 0xffU) << 0)
#define MPLLB_CTRL2_MPLLB_DIV8_CLK_EN		(1U << 8)
#define MPLLB_CTRL2_MPLLB_DIV10_CLK_EN		(1U << 9)
#define MPLLB_CTRL2_MPLLB_DIV_CLK_EN		(1U << 10)
#define MPLLB_CTRL2_MPLLB_TX_CLK_DIV(n)		(((n) & 0x7U) << 11)

#define CDR_CTRL_VCO_LOW_FREQ_0			(1U << 8)



int serdes_set_mode(void *base, serdes_mode_t mode);
int serdes_xpcs_wait_for_power_good(void *base, uint32_t xpcs);
int serdes_xpcs_set_sgmii_speed(void *base, uint32_t xpcs,
				    uint32_t mbps, bool fduplex);
int serdes_xpcs_set_1000_mode(void *base, uint32_t xpcs, bool ext_ref,
				  uint8_t ref_mhz);
int serdes_xpcs_set_2500_mode(void *base, uint32_t xpcs, bool ext_ref,
				  uint8_t ref_mhz);
int serdes_xpcs_set_loopback(void *base, uint32_t xpcs, bool enable);
int serdes_wait_for_link(void *base, uint32_t xpcs, uint8_t timeout);

int s32_serdes1_wait_link(int idx);
int s32_serdes1_setup(int mode);

#endif

