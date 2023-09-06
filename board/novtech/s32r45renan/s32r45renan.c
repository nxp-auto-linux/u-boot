// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2023 NXP
 */

#include <command.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <miiphy.h>
#include <net.h>
#include <dm/uclass.h>
#include <linux/err.h>

#define S32R_PHY_ADDR_1 0x01
#define RENAN_TXRX_SKEW	0xb400
#define RENAN_PHYCTRL_REG	0x17
#define RENAN_I2C_CPLD_VERSIONADDR	0x13
#define RENAN_I2C_EEPROM_ENETADDR	0x55

static int _i2c_get_chip(int chipaddr, struct udevice **ebus)
{

	int ret;
	struct udevice *bus;
	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret < 0) {
		printf("Failed to get the I2C device.\n");
		return ret;
	}

	ret = i2c_get_chip(bus, chipaddr, 1, ebus);
	if (ret < 0) {
		printf("Failed to get the chip %d\n", chipaddr);
		return ret;
	}

	return 0;
}

static int print_cpld_version(void)
{
	u8 major_ver = 0, minor_ver = 0;
	struct udevice *bus;
	int ret;

	ret = _i2c_get_chip(RENAN_I2C_CPLD_VERSIONADDR, &bus);
	if (ret < 0)
		return ret;

	ret = dm_i2c_read(bus, 0, &major_ver, sizeof(major_ver));
	if (ret < 0) {
		printf("Failed to read from CPLD\n");
		return -EIO;
	}

	ret = dm_i2c_read(bus, 1, &minor_ver, sizeof(minor_ver));
	if (ret < 0) {
		printf("Failed to read from CPLD\n");
		return -EIO;
	}

	printf("CPLD Version: %d.%d\n", major_ver, minor_ver);

	return 0;
}

int board_early_init_r(void)
{
	return print_cpld_version();
}

int do_mac(cmd_tbl_t *cmdtp, int flag, int argc,
		char *const argv[])
{
	struct udevice *bus;
	uchar enetaddr[ARP_HLEN];
	int ret;

	ret = _i2c_get_chip(RENAN_I2C_EEPROM_ENETADDR, &bus);
	if (ret < 0)
		return ret;

	if (argc == 1 || argc > 3)
		return CMD_RET_USAGE;


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

	ret = _i2c_get_chip(RENAN_I2C_EEPROM_ENETADDR, &bus);
	if (ret < 0)
		return ret;

	ret = i2c_eeprom_read(bus, 0, enetaddr, ARP_HLEN);
	if (ret < 0) {
		printf("%s: Read from EEPROM failed\n", __func__);
		return ret;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("ethaddr", enetaddr);

	ret = i2c_eeprom_read(bus, ARP_HLEN, enetaddr, ARP_HLEN);
	if (ret < 0) {
		printf("%s: Read from EEPROM failed\n", __func__);
		return ret;
	}

	if (is_valid_ethaddr(enetaddr))
		eth_env_set_enetaddr("eth1addr", enetaddr);

	return 0;
}

/**
 * Due to the hardware design of the reset circuit on S32R45,
 * the PHY does not properly reset after a software reboot.
 * PHY's control register is not correctly initialized afterwards
 * and GMAC clock settings fail to apply.
 */
int last_stage_init(void)
{
	struct udevice *gmac0;
	struct mii_dev *bus;
	struct phy_device *phy;
	phy_interface_t mode;
	const char *phy_mode;
	int ret;

	if (!IS_ENABLED(CONFIG_DWC_ETH_QOS_S32CC))
		return 0;

	ret = uclass_get_device_by_seq(UCLASS_ETH, 0, &gmac0);
	if (ret)
		return ret;

	bus = miiphy_get_dev_by_name(gmac0->name);
	if (!bus)
		return -ENODEV;

	phy_mode = dev_read_prop(gmac0, "phy-mode", NULL);
	if (!phy_mode) {
		pr_err("%s: phy-mode not found\n", gmac0->name);
		return -EINVAL;
	}

	mode = phy_get_interface_by_name(phy_mode);
	/* phy_connect will reset the PHY */
	phy = phy_connect(bus, S32R_PHY_ADDR_1, gmac0, mode);
	if (IS_ERR(phy))
		pr_err("%s: %s: phy reset failed\n", __func__, gmac0->name);
	/* Is not critical if the phy has not been found.
	 * The error code is ignored on purpose.
	 */

	ret = bus->write(bus, S32R_PHY_ADDR_1, MDIO_DEVAD_NONE,
			RENAN_PHYCTRL_REG, RENAN_TXRX_SKEW);
	if (ret)
		pr_warn("%s: RGMII internal delays not applied. No traffic will pass through the PHY\n",
			gmac0->name);

	return 0;
}

