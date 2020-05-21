/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019-2020 NXP
 *
 * The SerDes module header file.
 */

#ifndef SERDES_XPCS_REGS_H
#define SERDES_XPCS_REGS_H

#include <common.h>
#include "linux/types.h"
#include "linux/errno.h"

#define EXIT_FAILURE 1

/*
 *		XPCS registers
 */
#define SERDES_XPCS_0_BASE			0x82000U
#define SERDES_XPCS_0_ADDR1			0x823fcU
#define SERDES_XPCS_0_ADDR2			0x82000U

#define SERDES_XPCS_1_BASE			0x82800U
#define SERDES_XPCS_1_ADDR1			0x82bfcU
#define SERDES_XPCS_1_ADDR2			0x82800U

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

#define MII_DBG_CTRL_SUPPRESS_LOS_DET		(1U << 4)
#define MII_DBG_CTRL_RX_DT_EN_CTL		(1U << 6)

/* Field definitions for VR MII MMD Digital Control1 Register */

#define BYP_PWRUP				BIT(1)
#define EN_2_5G_MODE				(1U << 2)
#define MAC_AUTO_SW				(1U << 9)
#define CS_EN					(1U << 10)
#define PWRSV					(1U << 11)
#define EN_VSMMD1				(1U << 13)
#define R2TLBE					(1U << 14)
#define VR_RST					(1U << 15)

/* Field definitions for VR_MII_Gen5_12G_16G_REF_CLK_CTRL Register */

#define REF_CLK_CTRL_REF_CLK_EN			(1U << 0)
#define REF_CLK_CTRL_REF_USE_PAD		(1U << 1)
#define REF_CLK_CTRL_REF_CLK_DIV2		(1U << 2)
#define REF_CLK_CTRL_REF_RANGE(x)		(((x) & 0x7U) << 3)
#define REF_CLK_CTRL_REF_MPLLA_DIV2		(1U << 6)
#define REF_CLK_CTRL_REF_MPLLB_DIV2		(1U << 7)
#define REF_CLK_CTRL_REF_RPT_CLK_EN		(1U << 8)

#define CDR_CTRL_VCO_LOW_FREQ_0			(1U << 8)


int serdes_xpcs_wait_for_power_good(void *base, uint32_t xpcs);
int serdes_xpcs_set_sgmii_speed(void *base, uint32_t xpcs,
				    uint32_t mbps, bool fduplex);
int serdes_xpcs_set_1000_mode(void *base, u32 xpcs,
			      enum serdes_clock clktype,
			      enum serdes_clock_fmhz fmhz);
int serdes_xpcs_set_2500_mode(void *base, u32 xpcs,
			      enum serdes_clock clktype,
			      enum serdes_clock_fmhz fmhz);
int serdes_xpcs_set_loopback(void *base, uint32_t xpcs, bool enable);
int serdes_wait_for_link(void *base, uint32_t xpcs, uint8_t timeout);

int s32_serdes1_wait_link(int idx);
#endif

