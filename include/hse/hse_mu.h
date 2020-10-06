/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * HSE MU interface for secure boot in u-boot
 *
 * Copyright 2020 NXP
 */

#ifndef HSE_MU_H
#define HSE_MU_H

#define HSE_NUM_CHANNELS    16u /* number of available service channels */

u16 hse_mu_check_status(void);
bool hse_mu_msg_pending(u8 channel);

int hse_mu_msg_send(u8 channel, u32 msg);
int hse_mu_msg_recv(u8 channel, u32 *msg);
int hse_send_recv(u8 channel, u32 send_buf, u32 *recv_buf);

#endif /* HSE_MU_H */
