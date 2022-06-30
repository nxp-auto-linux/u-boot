// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */
#include <common.h>
#include <board_common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <s32-cc/scmi_reset_agent.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

void board_prep_linux(bootm_headers_t *images)
{
	int ret;

	ret = scmi_reset_agent();
	if (ret)
		pr_err("Failed to reset SCMI agent's settings\n");
}

void *board_fdt_blob_setup(void)
{
	void *dtb;

	dtb = (void *)(CONFIG_SYS_TEXT_BASE - CONFIG_S32CC_MAX_DTB_SIZE);

	if (fdt_magic(dtb) != FDT_MAGIC)
		panic("DTB is not passed via %p\n", dtb);

	return dtb;
}

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	/*
	 * Skip these fixups when reusing U-Boot dtb for Linux
	 * as they don't make sense.
	 *
	 * This block should be removed once the bindings and the dtbs
	 * used by Linux and U-Boot are fully compatible.
	 */
	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		printf("Skipping %s...\n", __func__);
		return 0;
	}

	if (IS_ENABLED(CONFIG_NETDEVICES))
		ft_enet_fixup(blob);

	return 0;
}
#endif /* defined(CONFIG_OF_BOARD_SETUP) */
