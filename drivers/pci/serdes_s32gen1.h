/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2022 NXP
 * S32Gen1 SerDes driver
 */

#ifndef SERDES_S32GEN1_H
#define SERDES_S32GEN1_H

#include <compiler.h>

int wait_read32(void __iomem *address, u32 expected,
		u32 mask, int read_attempts);

#endif /* SERDES_S32GEN1_H */
