/* SPDX-License-Identifier: GPL-2.0-only OR BSD-3-Clause */
/*
 * Copyright 2025 NXP
 */

#ifndef _UFS_NXP_S32N79_H_
#define _UFS_NXP_S32N79_H_

/* MPHY register definitions */
#define MPHY_REG_LSB_MASK			0x00FF
#define MPHY_REG_MSB_MASK			0xFF00

/* MPHY specific register addresses */
#define RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_0	0x3006
#define RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_1	0x3106

/* Vendor specific attributes */
#define CBRATESEL				0x8114

/* TX FSM states */
#define TX_FSM_HIBERN8				1

/* CPort definitions */
#define CPORT_0					0

#endif /* _UFS_NXP_S32N79_H_ */
