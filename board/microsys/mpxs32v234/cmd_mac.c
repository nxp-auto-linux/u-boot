/* -*-C-*- */
/*
 * (C) Copyright 2017 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>

int board_get_mac(struct eth_device *dev, unsigned char *mac);

int do_mac(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	if (argc < 3)
		return CMD_RET_USAGE;

	struct eth_device *dev = NULL;

	dev = eth_get_dev_by_name(argv[2]);

	if (!dev) {
		puts("Error: Ethernet device not found!\n");
		return CMD_RET_FAILURE;
	}

	if (strncmp(argv[1], "get\0", 4) == 0) {
		unsigned char mac[6];

		board_get_mac(dev, mac);
		printf("%s %pM\n", dev->name, mac);
		return CMD_RET_SUCCESS;
	}

	return CMD_RET_FAILURE;
}

U_BOOT_CMD(
		mac, CONFIG_SYS_MAXARGS, 1, do_mac,
		"get MAC addresses",
		"get <eth>              get MAC address of device <eth>\n"
	  );
