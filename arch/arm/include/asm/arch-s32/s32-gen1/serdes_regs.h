// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2021 NXP
 *
 * The SerDes module header file.
 */

#ifndef SERDES_REGS_H
#define SERDES_REGS_H

#include <asm/arch-s32/s32-gen1/serdes_hwconfig.h>
#include "linux/types.h"
#include "linux/errno.h"

/* use a mask to fix DEVICE_TYPE for EP */
#define SERDES_MODE(mode) (mode & 0xe)
#define IS_SERDES_PCIE(mode) (mode & (PCIE_EP | PCIE_RC))
#define IS_SERDES_SGMII(mode) (mode & (SGMII))

#define SERDES_SS_BASE				0x80000

/*
 * SS Registers
 */

/* PHY General Control */
#define SS_PHY_GEN_CTRL				(SERDES_SS_BASE + 0x0U)

#define SS_PHY_LPBK_CTRL			(SERDES_SS_BASE + 0x4U)
#define SS_PHY_SRAM_CSR				(SERDES_SS_BASE + 0x8U)

/* PHY MPLLA Control */
#define SS_PHY_MPLLA_CTRL			(SERDES_SS_BASE + 0x10U)
/* PHY MPLLB Control */
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

/* Subsystem Read Only Registers 0-3 */
#define SS_SS_RO_REG_0				(SERDES_SS_BASE + 0xe0U)
#define SS_SS_RO_REG_1				(SERDES_SS_BASE + 0xe4U)
#define SS_SS_RO_REG_2				(SERDES_SS_BASE + 0xe5U)
#define SS_SS_RO_REG_3				(SERDES_SS_BASE + 0xecU)

/* Subsystem Read Write Registers 0-5 */
#define SS_SS_RW_REG_0				(SERDES_SS_BASE + 0xf0U)
#define SS_SS_RW_REG_1				(SERDES_SS_BASE + 0xf4U)
#define SS_SS_RW_REG_2				(SERDES_SS_BASE + 0xf8U)
#define SS_SS_RW_REG_3				(SERDES_SS_BASE + 0xfcU)
#define SS_SS_RW_REG_4				(SERDES_SS_BASE + 0x100U)
#define SS_SS_RW_REG_5				(SERDES_SS_BASE + 0x104U)

#define SS_PCIE_SUBSYSTEM_VERSION		(SERDES_SS_BASE + 0x1000U)
#define SS_LINK_INT_CTRL_STS			(SERDES_SS_BASE + 0x1040U)

/* PCIe Controller 0 General Control 1-4 */
#define SS_PE0_GEN_CTRL_1			(SERDES_SS_BASE + 0x1050U)
#define SS_PE0_GEN_CTRL_2			(SERDES_SS_BASE + 0x1054U)
#define SS_PE0_GEN_CTRL_3			(SERDES_SS_BASE + 0x1058U)
#define SS_PE0_GEN_CTRL_4			(SERDES_SS_BASE + 0x105cU)

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

/* PCIe Controller 0 Link Debug 1 */
#define SS_PE0_LINK_DBG_1			(SERDES_SS_BASE + 0x10b0U)
/* PCIe Controller 0 Link Debug 2 */
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

/* PHY Register Address Register */
#define SS_PHY_REG_ADDR				(SERDES_SS_BASE + 0x3008U)
/* PHY Register Data Register */
#define SS_PHY_REG_DATA				(SERDES_SS_BASE + 0x300cU)
/* RESET CONTROL Register */
#define SS_RST_CTRL				(SERDES_SS_BASE + 0x3010U)


/* Field definitions for PHY_GEN_CTRL */

#define PHY_GEN_CTRL_REF_REPEAT_CLK_EN_BIT	(16)
#define PHY_GEN_CTRL_REF_REPEAT_CLK_EN		BIT(REF_REPEAT_CLK_EN_BIT)

#define PHY_GEN_CTRL_REF_USE_PAD_BIT	(17)
#define PHY_GEN_CTRL_REF_USE_PAD	BIT(PHY_GEN_CTRL_REF_USE_PAD_BIT)

#define PHY_GEN_CTRL_RX_SRIS_MODE_BIT	(9)
#define PHY_GEN_CTRL_RX_SRIS_MODE_MASK	BIT(PHY_GEN_CTRL_RX_SRIS_MODE_BIT)

#define EXT_PCLK_REQ				(1U << 0)

/* Field definitions for PHY_EXT_MPLLA/B Registers */

#define MPLLA_DIV8_CLK_EN			(1U << 8)
#define MPLLA_DIV10_CLK_EN			(1U << 9)
#define MPLLA_DIV_CLK_EN			(1U << 10)
#define MPLLA_TX_CLK_DIV(x)			(((x) & 0x7U) << 11)
#define MPLLA_CAL_DISABLE			(1U << 15)

#define MPPLB_CAL_DISABLE			(1U << 15)
#define MPLLB_MULTIPLIER(n)			(((n) & 0xffU) << 0)

#define MPLLB_CTRL2_MPLLB_DIV_MULT(n)		(((n) & 0xffU) << 0)
#define MPLLB_CTRL2_MPLLB_DIV8_CLK_EN		(1U << 8)
#define MPLLB_DIV10_CLK_EN			BIT(9)
#define MPLLB_CTRL2_MPLLB_DIV_CLK_EN		(1U << 10)
#define MPLLB_TX_CLK_DIV(n)		(((n) & 0x7U) << 11)

#define MPLL_STATE_BIT         (30)
#define MPLL_STATE             BIT(MPLL_STATE_BIT)

#define MPLLA_STATE_BIT        (31)
#define MPLLA_STATE            BIT(MPLLA_STATE_BIT)

/* Field definitions for PCIE_PHY_MPLLB_CTRL */

#define MPLLB_FORCE_EN_BIT      (0)
#define MPLLB_FORCE_EN          BIT(MPLLB_FORCE_EN_BIT)

#define MPLLB_SSC_EN_BIT       (1)
#define MPLLB_SSC_EN           BIT(MPLLB_SSC_EN_BIT)

#define MPLLB_STATE_BIT        (31)
#define MPLLB_STATE            BIT(MPLLB_STATE_BIT)

/* Field definitions for SS_RW_REG_0 */

#define SUBSYS_MODE_LSB		(0)
#define SUBSYS_MODE_MASK	(0x7)
#define SUBSYS_MODE		((SUBSYS_MODE_MASK) << \
		(SUBSYS_MODE_LSB))

#define PHY0_CR_PARA_SEL_BIT		(9)
#define PHY0_CR_PARA_SEL		BIT(PHY0_CR_PARA_SEL_BIT)

/* Field definitions for PE0_GEN_CTRL_1 */

#define DEVICE_TYPE_OVERRIDE       0x10
#define DEVICE_TYPE_EP             0x0
#define DEVICE_TYPE_RC             0x4

#define DEVICE_TYPE_LSB            (0)
#define DEVICE_TYPE_MASK           (0x0000000F)
#define DEVICE_TYPE                ((DEVICE_TYPE_MASK) << \
		(DEVICE_TYPE_LSB))

/* Field definitions for PE0_LINK_DBG_2 */

#define SMLH_LTSSM_STATE_LSB       (0)
#define SMLH_LTSSM_STATE_MASK      (0x0000003F)
#define SMLH_LTSSM_STATE           ((SMLH_LTSSM_STATE_MASK) << \
		(SMLH_LTSSM_STATE_LSB))

#define SMLH_LINK_UP_BIT           (6)
#define SMLH_LINK_UP               BIT(SMLH_LINK_UP_BIT)

#define RDLH_LINK_UP_BIT           (7)
#define RDLH_LINK_UP               BIT(RDLH_LINK_UP_BIT)

#define RATE_LSB           (8)
#define RATE_MASK          (0x00000003)
#define RATE               ((RATE_MASK) << (RATE_LSB))

/* Field definitions for PHY_REG_ADDR */

#define PHY_REG_ADDR_FIELD_LSB       (0)
#define PHY_REG_ADDR_FIELD_MASK      (0x0000FFFF)
#define PHY_REG_ADDR_FIELD           ((PHY_REG_ADDR_FIELD_MASK) << \
		(PHY_REG_ADDR_FIELD_LSB))

#define PHY_REG_EN_BIT         (31)
#define PHY_REG_EN             BIT(PHY_REG_EN_BIT)

/* Field definitions for PHY_REG_DATA */

#define PHY_REG_DATA_FIELD_LSB       (0)
#define PHY_REG_DATA_FIELD_MASK      (0x0000FFFF)
#define PHY_REG_DATA_FIELD           ((PHY_REG_DATA_FIELD_MASK) << \
		(PHY_REG_DATA_FIELD_LSB))

/* PHY registers */

#define RAWLANE0_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3019)
#define RAWLANE1_DIG_PCS_XF_RX_EQ_DELTA_IQ_OVRD_IN	(0x3119)

int s32_serdes_set_mode(void *dbi, int id, enum serdes_mode mode);
enum serdes_mode s32_get_serdes_mode_from_target(void *dbi, int id);

int rgm_issue_reset(u32 pid);
int rgm_release_reset(u32 pid);

#endif

