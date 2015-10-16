/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/io.h>
#include <asm/arch/cse.h>
#include <common.h>
#include <malloc.h>
#include <libfdt.h>
#include <fs.h>

#define CSE_TIMEOUT		1000000

static inline void cse_cancel_cmd(void)
{
	writel(CSE_CMD_CANCEL, CSE_CMD);
}

static inline int cse_wait(int timeout)
{
	int delay = timeout;

	while (delay--) {
		udelay(1);
		if (!(readl(CSE_SR) & CSE_SR_BSY))
			return 0;
	}

	printf("cse_init: Timed out while waiting for CSE command\n");
	return -1;
}

#ifdef CONFIG_CSE3

int load_cse_firmware(void) {

	char * ifname = "mmc";
	char mmc_dev_part_str[4];
	const char *filename;
	unsigned long bytes = 0;
	unsigned long pos = 0;
	int len_read;

	sprintf(mmc_dev_part_str, "%d:%s", CONFIG_SYS_MMC_ENV_DEV,
		getenv("mmcpart"));

	filename = getenv("cse_file");

	if (fs_set_blk_dev(ifname, mmc_dev_part_str, FS_TYPE_FAT))
		return 1;

	len_read = fs_read(filename, FIRMWARE_BASE, pos, bytes);

	if (len_read <= 0)
		return 1;

	printf("%d bytes read\n", len_read);

	return 0;
}

int cse_init(void)
{
	uint32_t firmware;
	uint32_t err;

	/* check if secure boot is enabled on chip and if secure boot was
	completed successfully in bootrom */
	if((readl(OCOTP_CFG5) & OCOTP_CFG5_SEC_BOOT_MODE) &&
		(readl(CSE_SR) & CSE_SR_BOK)) {
		return 0;
	}

	/* check if the firmware was not loaded before */
	if(!readl(CSE_KIA0)) {
		goto cse_firmware_loading;
	} else {
		/* check if init_cse was already done */
		writel(CSE_CMD_INIT_RNG, CSE_CMD);

		if (cse_wait(CSE_TIMEOUT))
			return -1;

		err = readl(CSE_ECR);

		if(err == CSE_SEQ_ERR) {
			goto init_cse;
		}
		return 0;
	}

cse_firmware_loading:
	if(load_cse_firmware()) {
		printf("CSE firmware loading failed\n");
		return 1;
	}

init_cse:
	writel(KIA_BASE, CSE_KIA0);
	writel(KIA_BASE, CSE_KIA1);

	if (readl(CSE_SR) & CSE_SR_BSY) {
		cse_cancel_cmd();
		if (cse_wait(CSE_TIMEOUT))
			return -1;
	}

	/* Init CSE3 */
	writel(&firmware, CSE_P1);
	writel(CSE_CMD_INIT_CSE, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;

	err = readl(CSE_ECR);

	if (err == CSE_SEQ_ERR)
		return 0;
	if (err)
		return -1;

	/* Init RNG */
	writel(CSE_CMD_INIT_RNG, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;
	if (readl(CSE_ECR))
		return -1;

	return 0;
}

#ifdef CONFIG_SECURE_BOOT
int cse_auth(ulong start_addr, unsigned long len, int key_id, uint8_t *exp_mac)
{
	/* Verify MAC */
	writel(key_id, CSE_P1);
	writel(&len, CSE_P2);
	writel(start_addr, CSE_P3);
	writel(exp_mac, CSE_P4);
	writel(MAC_LEN * 8, CSE_P5);
	writel(CSE_CMD_VERIFY_MAC, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;
	if (readl(CSE_ECR))
		return -1;
	if (readl(CSE_P5)) {
		return -1;
	}

	return 0;
}

int secure_boot(void)
{
	unsigned long uimage_size, fdt_size;
	uint8_t uimage_exp_mac[MAC_LEN];
	uint8_t fdt_exp_mac[MAC_LEN];

	uimage_size =  8 *
	    (unsigned long)image_get_image_size((image_header_t *)load_addr);

	memcpy(uimage_exp_mac, (uint8_t *)load_addr + uimage_size/8, MAC_LEN);

	if (cse_auth(load_addr, uimage_size, SECURE_BOOT_KEY_ID,
		    uimage_exp_mac)) {
		printf("uImage was NOT successfully authenticated!\n");
		return 1;
	}

	printf("uImage was successfully authenticated!\n",
		SECURE_BOOT_KEY_ID);

	fdt_size =  8 *
	    (unsigned long)fdt_totalsize((struct fdt_header *)FDT_ADDR);

	memcpy(fdt_exp_mac, (uint8_t *)FDT_ADDR + fdt_size/8, MAC_LEN);

	if (cse_auth(FDT_ADDR, fdt_size, SECURE_BOOT_KEY_ID,
		    fdt_exp_mac)) {
		printf("fdt was NOT successfully authenticated!\n");
		return 1;
	}

	printf("fdt was successfully authenticated!\n");

	return 0;
}

int do_genmac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t mac[MAC_LEN];
	int i;
	ulong start_addr;
	unsigned long len, key_id;

	start_addr = simple_strtoul(argv[1], NULL, 16);
	len = simple_strtoul(argv[2], NULL, 10);
	key_id = simple_strtoul(argv[3], NULL, 16);

	/* CSE init */
	if (cse_init()) {
		printf("CSE init failed\n");
		return 1;
	}

	/* Generate MAC */
	writel(key_id, CSE_P1);
	writel(&len, CSE_P2);
	writel(start_addr, CSE_P3);
	writel(mac, CSE_P4);
	writel(CSE_CMD_GENERATE_MAC, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;
	if (readl(CSE_ECR))
		return -1;

	/* Print generated MAC */
	printf("Generated MAC is: ");
	for (i = 0; i < MAC_LEN; i++)
		printf("%02x", mac[i]);
	printf("\n");

	return 0;
}

int do_get_uimage_size(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	unsigned long uimage_size;

	uimage_size =  8 *
	    (unsigned long)image_get_image_size((image_header_t *)load_addr);

	printf("uImage size is %lu\n", uimage_size);

	return 0;
}

int do_get_fdt_size(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	unsigned long fdt_size;

	fdt_size =  8 *
	    (unsigned long)fdt_totalsize((struct fdt_header *)FDT_ADDR);

	printf("fdt size is %lu\n", fdt_size);

	return 0;
}

U_BOOT_CMD(
		genmac, CONFIG_SYS_MAXARGS, 1, do_genmac,
		"generate MAC",
		""
	);

U_BOOT_CMD(
		get_uimage_size, CONFIG_SYS_MAXARGS, 1, do_get_uimage_size,
		"get uImage size in bits",
		""
	);

U_BOOT_CMD(
		get_fdt_size, CONFIG_SYS_MAXARGS, 1, do_get_fdt_size,
		"get fdt size in bits",
		""
	);

#endif /* CONFIG_SECURE_BOOT */
#endif /* CONFIG_CSE3 */
