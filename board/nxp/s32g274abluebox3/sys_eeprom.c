// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2006, 2008-2009, 2011 Freescale Semiconductor
 * Copyright 2024 NXP
 * York Sun (yorksun@freescale.com)
 * Haiying Wang (haiying.wang@freescale.com)
 * Timur Tabi (timur@freescale.com)
 */

#include <common.h>
#include <command.h>
#include <env.h>
#include <i2c.h>
#include <init.h>
#include <linux/delay.h>
#include <u-boot/crc.h>

/* Provide a local default in case there is no config */
#ifndef CONFIG_SYS_EEPROM_BUS_NUM
#define CONFIG_SYS_EEPROM_BUS_NUM 0
#endif

/* some boards with non-256-bytes EEPROM have special define */
/* for MAX_NUM_PORTS in board-specific file */
#ifndef MAX_NUM_PORTS
#define MAX_NUM_PORTS	16
#endif
#define NXID_VERSION	1

/**
 * static eeprom: EEPROM layout for NXID format
 *
 * See application note AN3638 for details.
 */
static struct __packed eeprom {
	u8 id[4];         /* 0x00 - 0x03 EEPROM Tag 'NXID' */
	u8 sn[12];        /* 0x04 - 0x0F Serial Number */
	u8 errata[5];     /* 0x10 - 0x14 Errata Level */
	u8 date[6];       /* 0x15 - 0x1a Build Date */
	u8 res_0;         /* 0x1b        Reserved */
	u32 version;      /* 0x1c - 0x1f NXID Version */
	u8 tempcal[8];    /* 0x20 - 0x27 Temperature Calibration Factors */
	u8 tempcalsys[2]; /* 0x28 - 0x29 System Temperature Calibration Factors */
	u8 tempcalflags;  /* 0x2a        Temperature Calibration Flags */
	u8 res_1[21];     /* 0x2b - 0x3f Reserved */
	u8 mac_count;     /* 0x40        Number of MAC addresses */
	u8 mac_flag;      /* 0x41        MAC table flags */
	u8 mac[MAX_NUM_PORTS][6];     /* 0x42 - 0xa1 MAC addresses */
	u8 res_2[90];     /* 0xa2 - 0xfb Reserved */
	u32 crc;          /* 0xfc - 0xff CRC32 checksum */
} e;

/* Set to 1 if we've read EEPROM into memory */
static int has_been_read;

/* Is this a valid NXID EEPROM? */
#define is_valid ((e.id[0] == 'N') && (e.id[1] == 'X') && \
		  (e.id[2] == 'I') && (e.id[3] == 'D'))

/**
 * show_eeprom - display the contents of the EEPROM
 */
static void show_eeprom(void)
{
	int i;
	unsigned int crc;

	/* EEPROM tag ID, should be NXID */
	printf("ID: %c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
	       be32_to_cpu(e.version));

	/* Serial number */
	printf("SN: %s\n", e.sn);

	/* Errata level. */
	printf("Errata: %s\n", e.errata);

	/* Build date, BCD date values, as YYMMDDhhmmss */
	printf("Build date: 20%02x/%02x/%02x %02x:%02x:%02x %s\n",
	       e.date[0], e.date[1], e.date[2],
	       e.date[3] & 0x7F, e.date[4], e.date[5],
	       e.date[3] & 0x80 ? "PM" : "");

	/* Show MAC addresses  */
	for (i = 0; i < min(e.mac_count, (u8)MAX_NUM_PORTS); i++) {
		u8 *p = e.mac[i];

		printf("Eth%u: %02x:%02x:%02x:%02x:%02x:%02x\n", i,
		       p[0], p[1], p[2], p[3],	p[4], p[5]);
	}

	crc = crc32(0, (void *)&e, sizeof(e) - 4);

	if (crc == be32_to_cpu(e.crc))
		printf("CRC: %08x\n", be32_to_cpu(e.crc));
	else
		printf("CRC: %08x (should be %08x)\n",
		       be32_to_cpu(e.crc), crc);

#ifdef DEBUG
	printf("EEPROM dump: (0x%x bytes)\n", sizeof(e));
	for (i = 0; i < sizeof(e); i++) {
		if ((i % 16) == 0)
			printf("%02X: ", i);
		printf("%02X ", ((u8 *)&e)[i]);
		if (((i % 16) == 15) || (i == sizeof(e) - 1))
			printf("\n");
	}
#endif
}

/**
 * read_eeprom - read the EEPROM into memory
 */
static int read_eeprom(void)
{
	struct udevice *dev;
	int ret;

	if (has_been_read)
		return 0;

	ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
				      CONFIG_SYS_I2C_EEPROM_ADDR,
				      CONFIG_SYS_I2C_EEPROM_ADDR_LEN, &dev);
	if (!ret)
		ret = dm_i2c_read(dev, 0, (void *)&e, sizeof(e));

#ifdef DEBUG
	show_eeprom();
#endif

	has_been_read = (ret == 0) ? 1 : 0;

	return ret;
}

/**
 *  update_crc - update the CRC
 *
 *  This function should be called after each update to the EEPROM structure,
 *  to make sure the CRC is always correct.
 */
static void update_crc(void)
{
	u32 crc;

	crc = crc32(0, (void *)&e, sizeof(e) - 4);
	e.crc = cpu_to_be32(crc);
}

/**
 * prog_eeprom - write the EEPROM from memory
 */
static int prog_eeprom(void)
{
	int ret = 0;
	int i;
	void *p;

	/* Set the reserved values to 0xFF   */
	e.res_0 = 0xFF;
	memset(e.res_1, 0xFF, sizeof(e.res_1));
	update_crc();

	/*
	 * The AT24C02 datasheet says that data can only be written in page
	 * mode, which means 8 bytes at a time, and it takes up to 5ms to
	 * complete a given write.
	 */
	for (i = 0, p = &e; i < sizeof(e); i += 8, p += 8) {
		struct udevice *dev;

		ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
					      CONFIG_SYS_I2C_EEPROM_ADDR,
					      CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
					      &dev);
		if (!ret)
			ret = dm_i2c_write(dev, i, p, min((int)(sizeof(e) - i),
							  8));

		if (ret)
			break;
		mdelay(5);	/* 5ms write cycle timing */
	}

	if (!ret) {
		/* Verify the write by reading back the EEPROM and comparing */
		struct udevice *dev;
		struct eeprom e2;

		ret = i2c_get_chip_for_busnum(CONFIG_SYS_EEPROM_BUS_NUM,
					      CONFIG_SYS_I2C_EEPROM_ADDR,
					      CONFIG_SYS_I2C_EEPROM_ADDR_LEN,
					      &dev);
		if (!ret)
			ret = dm_i2c_read(dev, 0, (void *)&e2, sizeof(e2));

		if (!ret && memcmp(&e, &e2, sizeof(e)))
			ret = -1;
	}

	if (ret) {
		printf("Programming failed.\n");
		has_been_read = 0;
		return ret;
	}

	printf("Programming passed.\n");
	return 0;
}

/**
 * h2i - converts hex character into a number
 *
 * This function takes a hexadecimal character (e.g. '7' or 'C') and returns
 * the integer equivalent.
 */
static inline u8 h2i(char p)
{
	if ((p >= '0') && (p <= '9'))
		return p - '0';

	if ((p >= 'A') && (p <= 'F'))
		return (p - 'A') + 10;

	if ((p >= 'a') && (p <= 'f'))
		return (p - 'a') + 10;

	return 0;
}

/**
 * set_date - stores the build date into the EEPROM
 *
 * This function takes a pointer to a string in the format "YYMMDDhhmmss"
 * (2-digit year, 2-digit month, etc), converts it to a 6-byte BCD string,
 * and stores it in the build date field of the EEPROM local copy.
 */
static void set_date(const char *string)
{
	unsigned int i;

	if (strlen(string) != 12) {
		printf("Usage: mac date YYMMDDhhmmss\n");
		return;
	}

	for (i = 0; i < 6; i++)
		e.date[i] = h2i(string[2 * i]) << 4 | h2i(string[2 * i + 1]);

	update_crc();
}

/**
 * set_mac_address - stores a MAC address into the EEPROM
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number) and
 * stores it in one of the MAC address fields of the EEPROM local copy.
 */
static void set_mac_address(unsigned int index, const char *string)
{
	char *p = (char *)string;
	unsigned int i;

	if (index >= MAX_NUM_PORTS || !string) {
		printf("Usage: mac <n> XX:XX:XX:XX:XX:XX\n");
		return;
	}

	for (i = 0; *p && (i < 6); i++) {
		e.mac[index][i] = hextoul(p, &p);
		if (*p == ':')
			p++;
	}

	update_crc();
}

int do_mac(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	char cmd;

	if (argc == 1) {
		show_eeprom();
		return 0;
	}

	cmd = argv[1][0];

	if (cmd == 'r') {
		read_eeprom();
		return 0;
	}

	if (cmd == 'i') {
		memcpy(e.id, "NXID", sizeof(e.id));
		e.version = cpu_to_be32(NXID_VERSION);
		update_crc();
		return 0;
	}

	if (!is_valid) {
		printf("Please read the EEPROM ('r') and/or set the ID ('i') first.\n");
		return 0;
	}

	if (argc == 2) {
		switch (cmd) {
		case 's':	/* save */
			prog_eeprom();
			break;
		default:
			return cmd_usage(cmdtp);
		}

		return 0;
	}

	/* We know we have at least one parameter  */

	switch (cmd) {
	case 'n':	/* serial number */
		memset(e.sn, 0, sizeof(e.sn));
		strlcpy((char *)e.sn, argv[2], sizeof(e.sn));
		update_crc();
		break;
	case 'e':	/* errata */
		memset(e.errata, 0, 5);
		strlcpy((char *)e.errata, argv[2], sizeof(e.errata));
		update_crc();
		break;
	case 'd':	/* date BCD format YYMMDDhhmmss */
		set_date(argv[2]);
		break;
	case 'p':	/* MAC table size */
		e.mac_count = hextoul(argv[2], NULL);
		update_crc();
		break;
	case '0' ... '9':	/* "mac 0" through "mac 22" */
		set_mac_address(dectoul(argv[1], NULL), argv[2]);
		break;
	case 'h':	/* help */
	default:
		return cmd_usage(cmdtp);
	}

	return 0;
}

/**
 * mac_read_from_eeprom - read the MAC addresses from EEPROM
 *
 * This function reads the MAC addresses from EEPROM and sets the
 * appropriate environment variables for each one read.
 *
 * The environment variables are only set if they haven't been set already.
 * This ensures that any user-saved variables are never overwritten.
 *
 * This function must be called after relocation.
 *
 * For NXID v1 EEPROMs, we support loading and up-converting the older NXID v0
 * format.  In a v0 EEPROM, there are only eight MAC addresses and the CRC is
 * located at a different offset.
 */
int mac_read_from_eeprom(void)
{
	unsigned int i;
	u32 crc, crc_offset = offsetof(struct eeprom, crc);
	u32 *crcp; /* Pointer to the CRC in the data read from the EEPROM */

	puts("EEPROM: ");

	if (read_eeprom()) {
		printf("Read failed.\n");
		return 0;
	}

	if (!is_valid) {
		printf("Invalid ID (%02x %02x %02x %02x)\n",
		       e.id[0], e.id[1], e.id[2], e.id[3]);
		return 0;
	}

	/*
	 * If we've read an NXID v0 EEPROM, then we need to set the CRC offset
	 * to where it is in v0.
	 */
	if (e.version == 0)
		crc_offset = 0x72;

	crc = crc32(0, (void *)&e, crc_offset);
	crcp = (void *)&e + crc_offset;
	if (crc != be32_to_cpu(*crcp)) {
		printf("CRC mismatch (%08x != %08x)\n", crc, be32_to_cpu(e.crc));
		return 0;
	}

	/*
	 * MAC address #9 in v1 occupies the same position as the CRC in v0.
	 * Erase it so that it's not mistaken for a MAC address.  We'll
	 * update the CRC later.
	 */
	if (e.version == 0)
		memset(e.mac[8], 0xff, 6);

	for (i = 0; i < min(e.mac_count, (u8)MAX_NUM_PORTS); i++) {
		if (memcmp(&e.mac[i], "\0\0\0\0\0\0", 6) &&
		    memcmp(&e.mac[i], "\xFF\xFF\xFF\xFF\xFF\xFF", 6)) {
			char ethaddr[18];
			char enetvar[9];

			sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
				e.mac[i][0],
				e.mac[i][1],
				e.mac[i][2],
				e.mac[i][3],
				e.mac[i][4],
				e.mac[i][5]);
			sprintf(enetvar, i ? "eth%daddr" : "ethaddr", i);
			/* Only initialize environment variables that are blank
			 * (i.e. have not yet been set)
			 */
			if (!env_get(enetvar))
				env_set(enetvar, ethaddr);
		}
	}

	printf("%c%c%c%c v%u\n", e.id[0], e.id[1], e.id[2], e.id[3],
	       be32_to_cpu(e.version));

	/*
	 * Now we need to upconvert the data into v1 format.  We do this last so
	 * that at boot time, U-Boot will still say "NXID v0".
	 */
	if (e.version == 0) {
		e.version = cpu_to_be32(NXID_VERSION);
		update_crc();
	}

	return 0;
}
