// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <miiphy.h>
#include <asm/arch/s32-gen1/ncore.h>

#if defined(CONFIG_TARGET_S32G2XXAEVB) || defined(CONFIG_TARGET_S32G3XXAEVB) ||\
	defined(CONFIG_NXP_S32GRDB_BOARD)
#include <dm/uclass.h>
#include <misc.h>
#include <s32gen1_siul2_nvram.h>
#endif

#include "board_common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_FSL_DRAM_SIZE1 + 0x100;

	return 0;
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#if CONFIG_IS_ENABLED(NETDEVICES)
	ft_enet_fixup(blob);
#endif

	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */
