/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2019-2021 NXP
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
#define VR_MII_GEN5_12G_16G_TX_POWER_STATE_CTRL 0x1f8035U
#define VR_MII_GEN5_12G_16G_RX_POWER_STATE_CTRL 0x1F8055U
#define VR_MII_GEN5_12G_16G_TX_EQ_CTRL0		0x1F8036U
#define VR_MII_CONSUMER_10G_TX_TERM_CTRL	0x1F803CU
#define VR_MII_GEN5_12G_16G_RX_GENCTRL1		0x1F8051U
#define VR_MII_GEN5_12G_16G_TX_GENCTRL1		0x1F8031U
#define VR_MII_GEN5_12G_16G_TX_GENCTRL2		0x1F8032U
#define VR_MII_GEN5_12G_16G_RX_GENCTRL2		0x1F8052U
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

/* VR_MII_Gen5_12G_16G_RX_GENCTRL1 */
#define RX_RST_0				BIT(4U)

/* VR_MII_Gen5_12G_16G_TX_GENCTRL1 */
#define TX_CLK_RDY_0				BIT(12)

/* VR_MII_Gen5_12G_16G_TX_GENCTRL2 */
#define TX_REQ_0				BIT(0)

/* VR_MII_Gen5_12G_16G_RX_GENCTRL2 */
#define RX_REQ_0				BIT(0)

/* VR_MII_Gen5_12G_16G_RX_POWER_STATE_CTRL */
#define RX_DISABLE_0				BIT(8)

/* VR_MII_Gen5_12G_16G_TX_POWER_STATE_CTRL */
#define TX_DISABLE_0				BIT(8)

/* VR_MII_Gen5_12G_16G_RX_CDR_CTRL */
#define VCO_LOW_FREQ_0				BIT(8)

#define PCS_MODE_1000_BASE_X			0U
#define PCS_MODE_SGMII				2U
#define MII_AN_CTRL_PCS_MODE(x)			(((x) & 0x3U) << 1)
#define MII_AN_CTRL_MII_CTRL			BIT(8)
#define MII_AN_CTRL_TX_CONFIG			BIT(3)
#define MII_AN_INTR_STS_CL37_ANCMPLT_INTR	BIT(0)

/* SR_MII_CTRL */
#define LBE					BIT(14)
#define MII_CTRL_SS6				BIT(6)
#define MII_CTRL_DUPLEX_MODE			BIT(8)
#define MII_CTRL_RESTART_AN			BIT(9)
#define MII_CTRL_AN_ENABLE			BIT(12)
#define MII_CTRL_SS13				BIT(13)

#define MII_STS_LINK_STS			BIT(2)

#define MPLL_EN_0				BIT(0)
#define MPLLB_SEL_0				BIT(4)

#define MPLLA_MULTIPLIER_VALUE(x)		(((x) & 0xffU) << 0)

#define MII_DBG_CTRL_SUPPRESS_LOS_DET		BIT(4)
#define MII_DBG_CTRL_RX_DT_EN_CTL		BIT(6)

/* Field definitions for VR MII MMD Digital Control1 Register */

#define BYP_PWRUP				BIT(1)
#define EN_2_5G_MODE				BIT(2)
#define CL37_TMR_OVR_RIDE			BIT(3)
#define INIT					BIT(8)
#define MAC_AUTO_SW				BIT(9)
#define CS_EN					BIT(10)
#define PWRSV					BIT(11)
#define EN_VSMMD1				BIT(13)
#define R2TLBE					BIT(14)
#define VR_RST					BIT(15)

/* Field definitions for VR_MII_Gen5_12G_16G_REF_CLK_CTRL Register */
#define REF_CLK_EN				BIT(0)
#define REF_USE_PAD				BIT(1)
#define REF_CLK_DIV2				BIT(2)
#define REF_RANGE(x)				(((x) & 0x7U) << 3)
#define REF_MPLLA_DIV2				BIT(6)
#define REF_MPLLB_DIV2				BIT(7)
#define REF_RPT_CLK_EN				BIT(8)

#define CDR_CTRL_VCO_LOW_FREQ_0			BIT(8)

#define MII_AN_CTRL_TX_CONFIG			BIT(3)
#define MII_AN_INTR_EN				BIT(0)
#define CL37_ANSGM_STS_GET_SPEED(x)		BIT(((x) & 0xc) >> 2)
#define CL37_ANSGM_STS_SPEED_10M		0U
#define CL37_ANSGM_STS_SPEED_100M		1U
#define CL37_ANSGM_STS_SPEED_1000M		2U
#define CL37_ANSGM_STS_LINK			BIT(4)
#define CL37_ANSGM_STS_FD			BIT(1)

#define PLLA_CAL_EN		BIT(0)
#define PLLA_CAL_DIS		BIT(1)

#define PLLB_CAL_EN		BIT(2)
#define PLLB_CAL_DIS		BIT(3)

#define XPCS0_1000M		BIT(8)
#define XPCS0_2500M		BIT(9)
#define XPCS0_DIS		BIT(10)
#define XPCS0_OWNED		(XPCS0_1000M | XPCS0_2500M | XPCS0_DIS)

#define XPCS1_1000M		BIT(16)
#define XPCS1_2500M		BIT(17)
#define XPCS1_DIS		BIT(18)
#define XPCS1_OWNED		(XPCS1_1000M | XPCS1_2500M | XPCS1_DIS)

#define PHY_CLK_INT		BIT(20)
#define PHY_CTRL_XPCS0_OWNED	BIT(21)
#define PHY_CTRL_XPCS1_OWNED	BIT(22)
#define PHY_CTRL_XPCS_OWNED	(PHY_CTRL_XPCS0_OWNED | PHY_CTRL_XPCS1_OWNED)

/* Init */
void serdes_pcs_pma_init_gen2(void *base, enum serdes_clock_fmhz fmhz,
			      u32 init_flags);
void serdes_pcs_issue_vreset(void *base, u32 xpcs);
int serdes_pcs_wait_for_vreset(void *base, u32 xpcs);
int serdes_pcs_reset_seqence(void *serdes_base, u32 xpcs0_base, u32 xpcs1_base);
int serdes_pcs_wait_for_power_good(void *base, u32 xpcs);

/* Bifurcation PMA control */
int serdes_bifurcation_pll_transit_to_3125Mhz(void *base, u32 xpcs,
					      enum serdes_clock_fmhz fmhz);
int serdes_bifurcation_pll_transit_to_1250Mhz(void *base, u32 xpcs,
					      enum serdes_clock_fmhz fmhz);

/* PMA control */
void serdes_pma_lane_disable(void *base, u32 xpcs);
void serdes_pma_lane_enable(void *base, u32 xpcs);
void serdes_pma_issue_rx_reset(void *base, u32 xpcs);
void serdes_pma_loopback_enable(void *base, u32 xpcs);
void serdes_pma_loopback_disable(void *base, u32 xpcs);
int  serdes_pma_wait_link(void *base, u32 xpcs, u8 sec);

/* PCS control */
int  serdes_pcs_speed_select(void *base, u32 xpcs, u32 div);
void serdes_pcs_mii_bus_control_disable(void *base, u32 xpcs);
void serdes_pcs_mii_bus_control_enable(void *base, u32 xpcs);
void serdes_pcs_an_enable(void *base, u32 xpcs);
void serdes_pcs_an_disable(void *base, u32 xpcs);
void serdes_pcs_an_restart(void *base, u32 xpcs);
void serdes_pcs_an_auto_sw_enable(void *base, u32 xpcs);
void serdes_pcs_an_auto_sw_disable(void *base, u32 xpcs);
void serdes_pcs_an_set_link_timer(void *base, u32 xpcs, u16 link_timer);
int  serdes_pcs_an_decode(void *base, u32 xpcs, bool *link, bool *fduplex,
			  u16 *speed);
void serdes_pcs_set_fd(void *base, u32 xpcs);
void serdes_pcs_set_hd(void *base, u32 xpcs);
void serdes_pcs_loopback_enable(void *base, u32 xpcs);
void serdes_pcs_loopback_disable(void *base, u32 xpcs);

enum serdes_xpcs_mode_gen2 s32_get_xpcs_mode(int serd, int xpcs);
int s32_sgmii_wait_link(int serdes, int xpcs);
#endif

