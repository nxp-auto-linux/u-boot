/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale S32G275 board.
 */

#ifndef __S32G275_H
#define __S32G275_H

#define CONFIG_S32G275

#include <configs/s32-gen1.h>

#ifndef CONFIG_PRAM
#define CONFIG_PRAM	2048	/* 2MB */
#endif

#define MMAP_DSPIn(n) \
	((n) == 0 ? SPI0_BASE_ADDR : \
	 (n) == 1 ? SPI1_BASE_ADDR : \
	 (n) == 2 ? SPI2_BASE_ADDR : \
	 (n) == 3 ? SPI3_BASE_ADDR : \
	 (n) == 4 ? SPI4_BASE_ADDR : \
	 (n) == 5 ? SPI5_BASE_ADDR : \
	 SPI_INVALID_BASE_ADDR)

#endif
