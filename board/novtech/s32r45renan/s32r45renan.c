// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023-2024 NXP
 */

#include <command.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <miiphy.h>
#include <net.h>
#include <dm/uclass.h>
#include <linux/err.h>

#define S32R_PHY_ADDR_1	0x01
#define RENAN_TXRX_SKEW	0xb400
#define RENAN_PHYCTRL_REG	0x17

static int print_cpld_version(void)
{
	u8 major_ver = 0, minor_ver = 0;
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C_GENERIC, 0, &bus);
	if (ret) {
		printf("Can't find CPLD. error = %d\n", ret);
		return ret;
	}

	ret = dm_i2c_read(bus, 0, &major_ver, sizeof(major_ver));
	if (ret < 0) {
		printf("Failed to read major version from CPLD. error = %d\n", ret);
		return -EIO;
	}

	ret = dm_i2c_read(bus, 1, &minor_ver, sizeof(minor_ver));
	if (ret < 0) {
		printf("Failed to read minor version from CPLD. error = %d\n", ret);
		return -EIO;
	}

	printf("CPLD Version: %d.%d\n", major_ver, minor_ver);

	return 0;
}

int board_early_init_r(void)
{
	return print_cpld_version();
}

int do_mac(struct cmd_tbl *cmdtp, int flag, int argc,
	   char *const argv[])
{
	struct udevice *bus;
	uchar enetaddr[ARP_HLEN];
	int ret;

	if (argc == 1 || argc > 3)
		return CMD_RET_USAGE;

	ret = uclass_get_device_by_seq(UCLASS_I2C_EEPROM, 0, &bus);
	if (ret) {
		printf("%s: Can't find EEPROM. error = %d\n", __func__, ret);
		return ret;
	}

	switch(argv[1][0]) {
		case 'r':
		case 's':
		case 'i':
		case 'n':
		case 'e':
		case 'd':
		case 'p':
		default:
			printf("Command '%s' not implemented\n", argv[1]);
			return CMD_RET_SUCCESS;
		case '0':
		case '1':
			string_to_enetaddr(argv[2], enetaddr);
			if (!is_valid_ethaddr(enetaddr)) {
				printf("Invalid MAC address: %s\n", argv[2]);
				return CMD_RET_FAILURE;
			}

			char eth_nr = argv[1][0] - '0';

			ret = i2c_eeprom_write(bus, ARP_HLEN * eth_nr,
					       enetaddr, ARP_HLEN);
			if (ret < 0) {
				printf("The write to eeprom failed\n");
				return CMD_RET_FAILURE;
			}

			break;
	}

	return CMD_RET_SUCCESS;
}

int mac_read_from_eeprom(void)
{
	uchar enetaddr[ARP_HLEN];
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C_EEPROM, 0, &bus);
	if (ret) {
		printf("%s: Can't find EEPROM. error = %d\n", __func__, ret);
		return ret;
	}

	ret = i2c_eeprom_read(bus, 0, enetaddr, ARP_HLEN);
	if (ret < 0) {
		printf("%s: Read from EEPROM failed. error = %d\n", __func__, ret);
		return ret;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("ethaddr", enetaddr);

	ret = i2c_eeprom_read(bus, ARP_HLEN, enetaddr, ARP_HLEN);
	if (ret < 0) {
		printf("%s: Read from EEPROM failed. error = %d\n", __func__, ret);
		return ret;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("eth1addr", enetaddr);

	return 0;
}
