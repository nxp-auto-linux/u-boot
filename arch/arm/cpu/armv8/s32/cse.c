/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Idenfifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <asm/arch/cse.h>
#include <common.h>
#include <malloc.h>
#include <libfdt.h>
#include <fs.h>
#include <errno.h>

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

#ifdef CONFIG_FSL_CSE3

/*
 * Load cse blob file from sdhc at CSE_BLOB_BASE address
 */
static int mmc_load_cse_blob(void)
{
	const char mmc_dev_part[] = __stringify(CONFIG_SYS_MMC_ENV_DEV) ":"
			__stringify(CONFIG_MMC_PART);
	const char *filename;
	loff_t bytes = 0, pos = 0, len_read;

	filename = getenv("cse_file");
	if (!filename)
	{
		printf("Failed to find \"cse_file\" environment variable! Default enviroment should be loaded.\n");
		return -ENOENT;
	}

	if (fs_set_blk_dev("mmc", mmc_dev_part, FS_TYPE_FAT))
		return -ENOENT;

	if (fs_read(filename, CSE_BLOB_BASE, pos, bytes, &len_read))
		return -ENOENT;

	printf("%llu bytes read\n", len_read);

	return 0;
}

extern void dma_mem_clr(void *, uint32_t);
int cse_init(void)
{
	uint32_t firmware;
	uint32_t err;

	/* check if CSE module is enabled */
	if (readl(OCOTP_CFG3) & OCOTP_CFG3_EXPORT_CONTROL) {
		printf("The security module (CSE3) is disabled.\n");
		return -ENODEV;
	}

	/* check if secure boot is enabled on chip and if secure boot was
	completed successfully in bootrom */
	if ((readl(OCOTP_CFG5) & OCOTP_CFG5_SEC_BOOT_MODE) &&
	    (readl(CSE_SR) & CSE_SR_BOK)) {
		goto init_rng;
	}

	/* check if the firmware was not loaded before */
	if (!readl(CSE_KIA0)) {
		goto cse_firmware_loading;
	} else {
		/* check if init_cse was already done */
		writel(CSE_CMD_INIT_RNG, CSE_CMD);

		if (cse_wait(CSE_TIMEOUT))
			return -ETIME;

		err = readl(CSE_ECR);

		if (err == CSE_SEQ_ERR)
			goto init_cse;
		else
			goto init_rng;
	}

cse_firmware_loading:

	/* check if CSE_BLOB_BASE is in SRAM and if it is, clear the area
	 * between CSE_BLOB_BASE and CSE_BLOB_BASE + CSE_BLOB_SIZE */
	if (IS_ADDR_IN_IRAM(CSE_BLOB_BASE))
		dma_mem_clr((void *)CSE_BLOB_BASE, CSE_BLOB_SIZE);

	if (mmc_load_cse_blob()) {
		printf("CSE firmware loading failed\n");
		return -ENOENT;
	}

	writel(KIA_BASE, CSE_KIA0);
	writel(KIA_BASE, CSE_KIA1);

init_cse:
	if (readl(CSE_SR) & CSE_SR_BSY) {
		cse_cancel_cmd();
		if (cse_wait(CSE_TIMEOUT))
			return -EIO;
	}

	/* Init CSE3 */
	writel(virt_to_phys(&firmware), CSE_P1);
	writel(CSE_CMD_INIT_CSE, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -EIO;

	err = readl(CSE_ECR);

	if (err && (err != CSE_SEQ_ERR))
		return -EIO;

	/* Open KRAM */
	writel(KRAM_ADDR, CSE_P1);
	writel(CSE_CMD_OPEN_SEC_RAM, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -EIO;

	err = readl(CSE_ECR);

	if (err && (err != CSE_SEQ_ERR))
		return -EIO;

init_rng:
	/* Init RNG */
	writel(CSE_CMD_INIT_RNG, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -EIO;
	if (readl(CSE_ECR))
		return -EIO;

	return 0;
}

#ifdef CONFIG_SECURE_BOOT
int cse_auth(ulong start_addr, unsigned long len, int key_id,
			uint8_t *exp_mac)
{
	/* Verify MAC */
	writel(key_id, CSE_P1);
	writel(virt_to_phys(&len), CSE_P2);
	writel(start_addr, CSE_P3);
	writel(virt_to_phys(exp_mac), CSE_P4);
	writel(MAC_LEN * 8, CSE_P5);
	writel(CSE_CMD_VERIFY_MAC, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -ETIME;
	if (readl(CSE_ECR))
		return -EIO;
	if (readl(CSE_P5))
		return -EIO;

	return 0;
}

int cse_genmac(ulong start_addr, unsigned long len, unsigned long key_id,
		uint8_t mac[])
{
	/* CSE init */
	int ret = cse_init();
	if (ret) {
		printf("CSE init failed\n");
		return ret;
	}

	/* Generate MAC */
	writel(key_id, CSE_P1);
	writel(virt_to_phys(&len), CSE_P2);
	writel(start_addr, CSE_P3);
	writel(virt_to_phys(mac), CSE_P4);
	writel(CSE_CMD_GENERATE_MAC, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -ETIME;
	if (readl(CSE_ECR))
		return -EIO;

	return 0;
}

int do_genmac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uint8_t mac[MAC_LEN];
	int i;
	int err = 0;
	ulong start_addr;
	unsigned long len, key_id;

	if (argc != 4) {
		printf("Usage : %s start_address length key_id\n", argv[0]);
		printf("Example: %s 0xC0000000 0x$filesize 0x4\n", argv[0]);
		return err;
	}

	start_addr = simple_strtoul(argv[1], NULL, 16);
	len = simple_strtoul(argv[2], NULL, 10);
	key_id = simple_strtoul(argv[3], NULL, 16);

	err = cse_genmac(start_addr, len, key_id, mac);
	if (err) {
		return err;
	}

	/* Print generated MAC */
	printf("Generated MAC is: ");
	for (i = 0; i < MAC_LEN; i++)
		printf("%02x", mac[i]);
	printf("\nGenerated MAC (hex) is: ");
	for (i = 0; i < MAC_LEN; i++)
		printf("\\x%02x", mac[i]);
	printf("\n");

	return err;
}

U_BOOT_CMD(
		genmac, CONFIG_SYS_MAXARGS, 1, do_genmac,
		"generate MAC",
		""
	);

#endif /* CONFIG_SECURE_BOOT */
#endif /* CONFIG_FSL_CSE3 */
