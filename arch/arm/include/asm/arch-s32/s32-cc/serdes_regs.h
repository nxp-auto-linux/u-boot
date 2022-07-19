// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2022 NXP
 *
 * The SerDes module header file.
 */

#ifndef SERDES_REGS_H
#define SERDES_REGS_H

#include <linux/errno.h>
#include <linux/types.h>
#include <s32-cc/serdes_hwconfig.h>

/* use a mask to fix DEVICE_TYPE for EP */
#define SERDES_MODE(mode) (mode & 0xe)
#define IS_SERDES_PCIE(mode) (mode & (PCIE_EP | PCIE_RC))
#define IS_SERDES_SGMII(mode) (mode & (SGMII))

#define SERDES_SS_BASE				0x80000

#define SS_PHY_EXT_MPLLA_CTRL_2			(SERDES_SS_BASE + 0x34U)
#define SS_PHY_EXT_MPLLA_CTRL_3			(SERDES_SS_BASE + 0x38U)
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
#define SS_PHY_EXT_TX_EQ_CTRL_1			(SERDES_SS_BASE + 0xb0U)
#define SS_PHY_EXT_TX_EQ_CTRL_2			(SERDES_SS_BASE + 0xb4U)
#define SS_PHY_EXT_TX_EQ_CTRL_3			(SERDES_SS_BASE + 0xb8U)
#define SS_PHY_XPCS0_RX_OVRD_CTRL		(SERDES_SS_BASE + 0xc0U)

/* Subsystem Read Only Registers 0-3 */
#define SS_SS_RO_REG_0				(SERDES_SS_BASE + 0xe0U)
#define SS_SS_RO_REG_1				(SERDES_SS_BASE + 0xe4U)
#define SS_SS_RO_REG_2				(SERDES_SS_BASE + 0xe5U)
#define SS_SS_RO_REG_3				(SERDES_SS_BASE + 0xecU)

/* Subsystem Read Write Registers 0-5 */
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

/* RESET CONTROL Register */
#define SS_RST_CTRL				(SERDES_SS_BASE + 0x3010U)

/* Field definitions for SS_RST_CTRL */
#define COLD_RST				BIT(1)

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

#define MPLLA_STATE_BIT        (31)
#define MPLLA_STATE            BIT(MPLLA_STATE_BIT)

/* Field definitions for PCIE_PHY_MPLLB_CTRL */

#define MPLLB_FORCE_EN_BIT      (0)
#define MPLLB_FORCE_EN          BIT(MPLLB_FORCE_EN_BIT)

#define MPLLB_STATE_BIT        (31)
#define MPLLB_STATE            BIT(MPLLB_STATE_BIT)

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

#endif

