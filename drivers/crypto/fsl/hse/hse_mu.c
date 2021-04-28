// SPDX-License-Identifier: BSD-3-Clause
/*
 * HSE MU interface for secure boot in u-boot
 *
 * Copyright 2020-2021 NXP
 */

#include <common.h>
#include <linux/io.h>
#include <errno.h>

#include <hse/hse_mu.h>
#include <hse/hse_abi.h>

#define MU0B_BASE    0x40210000ul
#define MU1B_BASE    0x40211000ul
#define MU2B_BASE    0x40212000ul
#define MU3B_BASE    0x40213000ul

#define HSE_STATUS_MASK     0xFFFF0000ul /* HSE global status FSR mask */

/**
 * hse_err_decode - HSE Error Code Translation
 * @srv_rsp: HSE service response
 *
 * Return: 0 on service request success, error code otherwise
 */
static inline int hse_err_decode(u32 *srv_rsp)
{
	switch (*srv_rsp) {
	case HSE_SRV_RSP_OK:
		return CMD_RET_SUCCESS;
	case HSE_SRV_RSP_VERIFY_FAILED:
		printf("ERROR: auth tag/signature verification failed!\n");
		goto ret_err;
	case HSE_SRV_RSP_INVALID_ADDR:
		printf("ERROR: invalid service descriptor address!\n");
		goto ret_err;
	case HSE_SRV_RSP_INVALID_PARAM:
		printf("ERROR: invalid service descriptor parameter!\n");
		goto ret_err;
	case HSE_SRV_RSP_KEY_INVALID:
		printf("ERROR: key flags do not match requested operation!\n");
		goto ret_err;
	case HSE_SRV_RSP_NOT_ALLOWED:
		printf("ERROR: operation not allowed!\n");
		goto ret_err;
	default:
		printf("ERROR: unknown error, EFAULT!\n");
		goto ret_err;
	}
ret_err:
	return CMD_RET_FAILURE;
}

/**
 * struct hse_mu_regs - HSE Messaging Unit Registers
 * @ver: Version ID Register, offset 0x0
 * @par: Parameter Register, offset 0x4
 * @cr: Control Register, offset 0x8
 * @sr: Status Register, offset 0xC
 * @fcr: Flag Control Register, offset 0x100
 * @fsr: Flag Status Register, offset 0x104
 * @gier: General Interrupt Enable Register, offset 0x110
 * @gcr: General Control Register, offset 0x114
 * @gsr: General Status Register, offset 0x118
 * @tcr: Transmit Control Register, offset 0x120
 * @tsr: Transmit Status Register, offset 0x124
 * @rcr: Receive Control Register, offset 0x128
 * @rsr: Receive Status Register, offset 0x12C
 * @tr[n]: Transmit Register n, offset 0x200 + 4*n
 * @rr[n]: Receive Register n, offset 0x280 + 4*n
 */
struct hse_mu_regs {
	const u32 ver;
	const u32 par;
	u32 cr;
	u32 sr;
	u8 reserved0[240]; /* 0xF0 */
	u32 fcr;
	const u32 fsr;
	u8 reserved1[8]; /* 0x8 */
	u32 gier;
	u32 gcr;
	u32 gsr;
	u8 reserved2[4]; /* 0x4 */
	u32 tcr;
	const u32 tsr;
	u32 rcr;
	const u32 rsr;
	u8 reserved3[208]; /* 0xD0 */
	u32 tr[16];
	u8 reserved4[64]; /* 0x40 */
	const u32 rr[16];
};

static struct hse_mu_regs * const regs = (struct hse_mu_regs *)MU0B_BASE;

/**
 * hse_mu_check_status - check the HSE global status
 *
 * Return: 16 MSB of MU instance FSR
 */
u16 hse_mu_check_status(void)
{
	u32 fsrval;

	fsrval = ioread32(&regs->fsr);
	fsrval = (fsrval & HSE_STATUS_MASK) >> 16u;

	return (u16)fsrval;
}

/**
 * hse_mu_channel_available - check service channel status
 * @channel: channel index
 *
 * The 16 LSB of MU instance FSR are used by HSE for signaling channel status
 * as busy after a service request has been sent, until the HSE reply is ready.
 *
 * Return: true for channel available, false for invalid index or channel busy
 */
static bool hse_mu_channel_available(u8 channel)
{
	u32 fsrval, tsrval, rsrval;

	if (unlikely(channel >= HSE_NUM_CHANNELS))
		return false;

	fsrval = ioread32(&regs->fsr) & BIT(channel);
	tsrval = ioread32(&regs->tsr) & BIT(channel);
	rsrval = ioread32(&regs->rsr) & BIT(channel);

	if (fsrval || !tsrval || rsrval)
		return false;

	return true;
}

/**
 * hse_mu_msg_pending - check if a service request response is pending
 * @channel: channel index
 *
 * Return: true for response ready, false otherwise
 */
bool hse_mu_msg_pending(u8 channel)
{
	u32 rsrval;

	if (unlikely(channel >= HSE_NUM_CHANNELS))
		return false;

	rsrval = ioread32(&regs->rsr) & BIT(channel);
	if (!rsrval)
		return false;

	return true;
}

/**
 * hse_mu_msg_send - send a message over MU (non-blocking)
 * @channel: channel index
 * @msg: input message
 *
 * Return: 0 on success, -EINVAL for invalid parameter, -ECHRNG for channel
 *         index out of range, -EBUSY for selected channel busy
 */
int hse_mu_msg_send(u8 channel, u32 msg)
{
	if (unlikely(channel >= HSE_NUM_CHANNELS)) {
		printf("%s: channel %d outside range\n", __func__, channel);
		return -ECHRNG;
	}

	if (unlikely(!hse_mu_channel_available(channel))) {
		printf("%s: channel %d busy\n", __func__, channel);
		return -EBUSY;
	}

	iowrite32(msg, &regs->tr[channel]);

	return 0;
}

/**
 * hse_mu_msg_recv - read a message received over MU (non-blocking)
 * @channel: channel index
 * @msg: output message
 *
 * Return: 0 on success, -EINVAL for invalid parameter, -ECHRNG for channel
 *         index out of range, -ENOMSG for no reply pending on selected channel
 */
int hse_mu_msg_recv(u8 channel, u32 *msg)
{
	if (unlikely(!msg)) {
		printf("%s: msg buff pointer is null\n", __func__);
		return -EINVAL;
	}

	if (unlikely(channel >= HSE_NUM_CHANNELS)) {
		printf("%s: channel %d outside range\n", __func__, channel);
		return -ECHRNG;
	}

	if (unlikely(!hse_mu_msg_pending(channel)))
		return -ENOMSG;

	*msg = ioread32(&regs->rr[channel]);

	return 0;
}

/**
 * hse_send_recv - send a message and wait for reply
 * @channel: channel index
 * @send_buf: input message
 * @recv_buf: ptr to store output message
 *
 * Return: 0 on success, error code otherwise
 */
int hse_send_recv(u8 channel, u32 send_buf, u32 *recv_buf)
{
	int ret;

	ret = hse_mu_msg_send(channel, send_buf);
	if (ret)
		return CMD_RET_FAILURE;

	do {
		ret = hse_mu_msg_recv(channel, recv_buf);
	} while (ret == -ENOMSG);
	if (ret)
		return CMD_RET_FAILURE;

	return hse_err_decode(recv_buf);
}
