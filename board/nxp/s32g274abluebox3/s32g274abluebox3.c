// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>

#define SPI_ALIAS "spi0"
#define SJA1110_UC_COMPATIBLE "nxp,sja1110-uc"
#define SJA1110_SWITCH_COMPATIBLE "nxp,sja1110-switch"
#define SJA1110_DSA_COMPATIBLE "nxp,sja1110a"

static void compatible_err(const char *path, const char *compat, int err)
{
	pr_err("Failed to find node by compatible %s in %s. Error: %s\n",
	       compat, path, fdt_strerror(err));
}

static void ft_sja1110_dsa_setup(void *blob)
{
	const char *spi_path;
	int spi_offset;
	int offset;
	int ret;

	if (env_get_yesno("sja1110_dsa") < 1)
		return;

	spi_path = fdt_get_alias(blob, SPI_ALIAS);
	if (!spi_path) {
		pr_err("Alias not found: %s\n", SPI_ALIAS);
		goto out_err;
	}

	spi_offset = fdt_path_offset(blob, spi_path);
	if (spi_offset < 0) {
		pr_err("Failed to get node offset by path %s. Error: %s\n",
		       spi_path, fdt_strerror(spi_offset));
		goto out_err;
	}

	offset = fdt_node_offset_by_compatible(blob, spi_offset,
					       SJA1110_UC_COMPATIBLE);
	if (offset >= 0)
		fdt_del_node(blob, offset);
	else
		compatible_err(spi_path, SJA1110_UC_COMPATIBLE, offset);

	offset = fdt_node_offset_by_compatible(blob, spi_offset,
					       SJA1110_SWITCH_COMPATIBLE);
	if (offset >= 0)
		fdt_del_node(blob, offset);
	else
		compatible_err(spi_path, SJA1110_SWITCH_COMPATIBLE,
			       offset);

	offset = fdt_node_offset_by_compatible(blob, spi_offset,
					       SJA1110_DSA_COMPATIBLE);
	if (offset < 0) {
		compatible_err(spi_path, SJA1110_DSA_COMPATIBLE, offset);
		goto out_err;
	}

	ret = fdt_status_okay(blob, offset);
	if (ret < 0) {
		pr_err("Failed to set status \"okay\". Error: %s\n",
		       fdt_strerror(spi_offset));
		goto out_err;
	}

	return;

out_err:
	pr_err("SJA1110 DSA driver not enabled.\n");
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	fdt_fixup_ethernet(blob);
	ft_sja1110_dsa_setup(blob);

	return 0;
}
