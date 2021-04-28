// SPDX-License-Identifier: BSD-3-Clause
/*
 * HSE advanced secure boot preparatory command demo
 *
 * Copyright 2020-2021 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <command.h>
#include <malloc.h>
#include <log.h>
#include <errno.h>
#include <hse/hse_mu.h>
#include <hse/hse_abi.h>

#ifdef CONFIG_SD_BOOT
DECLARE_GLOBAL_DATA_PTR;

/* public key modulus */
static const u8 rsa2048_orig_mod[] = { 0xf4, 0x72, 0xe2, 0xe9, 0x83, 0xd1,
0x03, 0x36, 0xd3, 0xf5, 0xd5, 0x06, 0xf4, 0x4e, 0x4f, 0xc8, 0xc2, 0xe3, 0xf9,
0x1c, 0x00, 0xfd, 0xb1, 0x03, 0xa8, 0xe3, 0x0c, 0xd9, 0x3b, 0x70, 0x4e, 0xa3,
0x2e, 0x32, 0x9c, 0x71, 0x06, 0x70, 0x03, 0xf9, 0x75, 0x6d, 0x95, 0x10, 0x2a,
0x89, 0xf1, 0x82, 0x49, 0x23, 0xee, 0xab, 0x15, 0xe5, 0x11, 0xa7, 0xc6, 0x1d,
0x78, 0x10, 0xed, 0x40, 0xf1, 0x7e, 0x10, 0x36, 0x29, 0x52, 0x2a, 0xa6, 0xbc,
0xfe, 0x51, 0xba, 0x08, 0xcd, 0x65, 0x00, 0xe6, 0x2e, 0xbe, 0x26, 0x31, 0x43,
0x86, 0xc2, 0xbb, 0x24, 0x13, 0xd3, 0x86, 0xf4, 0x1e, 0xac, 0x0f, 0xf1, 0xdc,
0xe9, 0xf1, 0x01, 0x7f, 0xe7, 0xe1, 0x44, 0xff, 0xfb, 0xea, 0xa5, 0xfa, 0x65,
0x54, 0x17, 0x7c, 0xe0, 0x98, 0x56, 0x11, 0x98, 0x07, 0x51, 0x99, 0x9d, 0x32,
0x3b, 0x54, 0x75, 0xc8, 0x9e, 0x92, 0xb8, 0xd4, 0x4f, 0x1a, 0x12, 0x2c, 0x34,
0x63, 0xa0, 0xa0, 0x83, 0x9f, 0x05, 0x55, 0x96, 0x16, 0xf1, 0x96, 0x61, 0xe9,
0xb1, 0x7a, 0xd9, 0xfe, 0xf3, 0xeb, 0xfb, 0xa0, 0x15, 0xa1, 0xef, 0x82, 0x35,
0x68, 0x34, 0x96, 0xff, 0xa8, 0xba, 0x6f, 0x35, 0xc5, 0x4a, 0x96, 0x59, 0x78,
0x6d, 0xa1, 0x1a, 0xbc, 0x4e, 0x4d, 0xa7, 0xfd, 0x25, 0xda, 0x82, 0xc3, 0xc1,
0x06, 0x26, 0xf7, 0x1e, 0xbe, 0x5d, 0xff, 0x2c, 0xc8, 0x5c, 0x7b, 0xb2, 0x9a,
0x4e, 0xf2, 0x45, 0xd4, 0xc0, 0x13, 0x5b, 0xc1, 0xc3, 0x68, 0x32, 0xe1, 0xa1,
0x98, 0xf2, 0xc9, 0x9c, 0xc1, 0x0e, 0xb9, 0x11, 0x3c, 0x2d, 0x07, 0x8e, 0x8c,
0xcd, 0xbc, 0x51, 0x43, 0x83, 0xbf, 0xf4, 0x81, 0x15, 0x77, 0xd4, 0x52, 0xf6,
0xab, 0xa4, 0xc3, 0x9f, 0x2a, 0xb7, 0x73, 0x1a, 0xee, 0x31, 0x39, 0x1a, 0xae,
0x89, 0x14, 0x89 };

/* public key exponent */
static const u8 rsa2048_orig_exp[] = { 0x01, 0x00, 0x01 };

/* hse nvm key catalog configuration */
#define HSE_NVM_KEY_CATALOG_CFG \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_AES, 5U, HSE_KEY128_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_AES, 10U, HSE_KEY256_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_HMAC, 5U, HSE_KEY512_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_ECC_PAIR, 2U, HSE_KEY256_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_ECC_PUB, 2U, HSE_KEY256_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_ECC_PUB_EXT, 1U, HSE_KEY256_BITS }, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_RSA_PAIR, 2U, HSE_KEY2048_BITS}, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_RSA_PUB, 2U, HSE_KEY2048_BITS}, \
{ HSE_ALL_MU_MASK, HSE_KEY_OWNER_CUST, \
	HSE_KEY_TYPE_RSA_PUB_EXT, 1U, HSE_KEY2048_BITS}, \
{ 0U, 0U, 0U, 0U, 0U }

/* hse ram key catalog configuration */
#define  HSE_RAM_KEY_CATALOG_CFG \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_RSA_PUB, 1u, HSE_KEY2048_BITS }, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_AES, 12u, HSE_KEY256_BITS }, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_HMAC, 6u, HSE_KEY512_BITS}, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_ECC_PUB, 1u, HSE_KEY256_BITS}, \
{0u, 0u, 0u, 0u, 0u}

/* the nvm container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry nvm_orig_cat[] = {
	HSE_NVM_KEY_CATALOG_CFG
};

/* the ram container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry ram_orig_cat[] = {
	HSE_RAM_KEY_CATALOG_CFG
};

int hse_format_key_store(struct hse_private *priv, u32 *recv_buf)
{
	int ret = 0;

	printf("\tFormatting NVM and RAM key stores...\n");

	memcpy((void *)&priv->nvm_catalog, (void *)&nvm_orig_cat,
	       sizeof(nvm_orig_cat));
	memcpy((void *)&priv->ram_catalog, (void *)&ram_orig_cat,
	       sizeof(ram_orig_cat));

	priv->srv_desc.srv_id = HSE_SRV_ID_FORMAT_KEY_CATALOGS;
	priv->srv_desc.format_catalogs_req.nvm_key_catalog_cfg_addr =
			(uintptr_t)&priv->nvm_catalog;
	priv->srv_desc.format_catalogs_req.ram_key_catalog_cfg_addr =
			(uintptr_t)&priv->ram_catalog;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: key catalog format failed!\n");
		goto hse_send_fail;
	}
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	return ret;
}

int hse_import_key(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_key_info *key_info;
	struct hse_import_key_srv *import_key_req;
	int ret = 0;

	key_info = &(priv->key_info);
	import_key_req = &(priv->srv_desc.import_key_req);

	printf("\tImporting RSA public key into NVM key store...\n");

	memcpy((void *)&priv->rsa2048_pub_modulus, (void *)&rsa2048_orig_mod,
	       sizeof(rsa2048_orig_mod));
	memcpy((void *)&priv->rsa2048_pub_exponent, (void *)&rsa2048_orig_exp,
	       sizeof(rsa2048_orig_exp));

	key_info->key_flags = (HSE_KF_USAGE_VERIFY | HSE_KF_USAGE_AUTHORIZATION);
	key_info->key_bit_len =
		BYTES_TO_BITS(ARRAY_SIZE(priv->rsa2048_pub_modulus));
	key_info->key_counter = 0ul;
	key_info->smr_flags = 0ul;
	key_info->key_type = HSE_KEY_TYPE_RSA_PUB;
	key_info->pub_exponent_size = ARRAY_SIZE(priv->rsa2048_pub_exponent);

	priv->srv_desc.srv_id = HSE_SRV_ID_IMPORT_KEY;
	import_key_req->key_handle = HSE_BOOT_KEY_HANDLE;
	import_key_req->key_info_addr = (uintptr_t)key_info;
	import_key_req->key_addr[0] = (uintptr_t)&priv->rsa2048_pub_modulus;
	import_key_req->key_addr[1] = (uintptr_t)&priv->rsa2048_pub_exponent;
	import_key_req->key_addr[2] = 0u;
	import_key_req->key_len[0] = ARRAY_SIZE(priv->rsa2048_pub_modulus);
	import_key_req->key_len[1] = ARRAY_SIZE(priv->rsa2048_pub_exponent);
	import_key_req->key_len[2] = 0u;
	import_key_req->cipher_key = HSE_INVALID_KEY_HANDLE;
	import_key_req->auth_key = HSE_INVALID_KEY_HANDLE;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: rsa public key import failed!\n");
		goto hse_send_fail;
	}
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	return ret;
}

int hse_install_cr_entry(struct hse_private *priv, u32 *recv_buf)
{
	int ret = 0;

	printf("\tGenerating Core Reset Entry...\n");

	priv->cr_entry.core_id = HSE_APP_CORE3;
	priv->cr_entry.cr_sanction = HSE_CR_SANCTION_KEEP_CORE_IN_RESET;
	priv->cr_entry.preboot_smr_map = HSE_SMR_ENTRY_1;
	priv->cr_entry.pass_reset = CONFIG_SYS_TEXT_BASE;
	priv->cr_entry.start_option = HSE_CR_AUTO_START;

	priv->srv_desc.srv_id = HSE_SRV_ID_CORE_RESET_ENTRY_INSTALL;
	priv->srv_desc.cr_install_req.cr_entry_index = 1u;
	priv->srv_desc.cr_install_req.cr_entry_addr =
					(uintptr_t)&priv->cr_entry;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: core reset entry install failed!\n");
		goto hse_send_fail;
	}
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	return ret;
}

int hse_install_smr_entry(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_smr_entry *smr_entry;
	struct hse_smr_install_srv *smr_install_req;
	int ret = 0;

	smr_entry = &(priv->smr_entry);
	smr_install_req = &(priv->srv_desc.smr_install_req);

	printf("\tGenerating Secure Memory Region entry...\n");

	ret = hse_mmc_read(&priv->uboot_sign, HSE_UBOOT_SIGN_BLK, 1);
	if (ret) {
		log_err("ERROR: signature read failed!\n");
		goto hse_send_fail;
	}

	ret = hse_mmc_read(&priv->uboot_copy, HSE_UBOOT_BIN_BLK, 2048);
	if (ret) {
		log_err("ERROR: u-boot read failed!\n");
		goto hse_send_fail;
	}

	/**
	 * no address of actual code start, need to reference app bl header
	 * CONFIG_DTB_SRAM_ADDR used bc smr contains dtb and code
	 */
	smr_entry->smr_src = priv->ivt.app_boot + 0x200;
	smr_entry->smr_dst_addr = CONFIG_DTB_SRAM_ADDR;
	smr_entry->smr_size = HSE_UBOOT_MAX_SIZE;
	smr_entry->config_flags = (HSE_SMR_CFG_FLAG_SD_FLASH |
				   HSE_SMR_CFG_FLAG_INSTALL_AUTH);
	smr_entry->check_period = 0;
	smr_entry->key_handle = HSE_BOOT_KEY_HANDLE;
	smr_entry->sign_sch.sign_scheme = HSE_SIGN_RSASSA_PKCS1_V15;
	smr_entry->sign_sch.sch.hash_algo = HSE_HASH_ALGO_SHA_1;
	smr_entry->auth_tag = HSE_AUTH_TAG_SD;
	smr_entry->decrypt_key_handle = HSE_SMR_DECRYPT_KEY_HANDLE_NOT_USED;

	priv->srv_desc.srv_id = HSE_SRV_ID_SMR_ENTRY_INSTALL;
	smr_install_req->access_mode = HSE_ACCESS_MODE_ONE_PASS;
	smr_install_req->entry_index = 1u;
	smr_install_req->smr_entry_addr = (uintptr_t)smr_entry;
	smr_install_req->smr_data_addr = (uintptr_t)&priv->uboot_copy;
	smr_install_req->smr_data_len = HSE_UBOOT_MAX_SIZE;
	smr_install_req->smr_auth_tag_addr = (uintptr_t)&priv->uboot_sign;
	smr_install_req->smr_auth_tag_len = HSE_UBOOT_AUTH_LEN;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: smr entry install failed!\n");
		goto hse_send_fail;
	}
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	return ret;
}

int hse_generate_sys_img(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_publish_sys_img_srv *publish_sys_img_req;
	int ret = 0;

	publish_sys_img_req = &(priv->srv_desc.publish_sys_img_req);

	printf("\tGenerating SYS_IMG...\n");

	priv->sys_img_len = HSE_SYS_IMG_MAX_SIZE;

	priv->srv_desc.srv_id = HSE_SRV_ID_PUBLISH_SYS_IMAGE;
	publish_sys_img_req->publish_options = HSE_PUBLISH_ALL_DATA_SETS;
	publish_sys_img_req->publish_offset_addr = 
		(uintptr_t)&priv->publish_offset;
	publish_sys_img_req->buff_length_addr = (uintptr_t)&priv->sys_img_len;
	publish_sys_img_req->buff_addr = (uintptr_t)&priv->sys_img;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_ADMIN,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: sys-img publish failed!\n");
		goto hse_send_fail;
	}

hse_send_fail:
	return ret;
}

int hse_write_sys_img(struct hse_private *priv, bool secure)
{
	int ret = 0;

	printf("\tWriting SYS_IMG to SDcard...\n");

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_mmc_write(&priv->sys_img, HSE_SYS_IMG_BLK, 96);
	if (ret) {
		log_err("ERROR: sys-img write failed!\n");
		ret = CMD_RET_FAILURE;
		goto ret_fail;
	}

	printf("\tUpdating SYS_IMG pointer...\n");

	/* set the sys img address, external flash type, flash page size */
	priv->ivt.sys_img = HSE_SYS_IMG_SD;
	priv->ivt.sys_img_ext_flash_type = HSE_EXT_FLASH_SD;
	priv->ivt.sys_img_flash_page_size = HSE_EXT_FLASH_PAGE;

	/* set BOOT_SEQ bit, if using secure boot */
	if (secure)
		priv->ivt.boot_cfg |= HSE_IVT_BOOTSEQ_BIT;

	/* write primary ivt */
	ret = hse_mmc_write(&priv->ivt, HSE_PIVT_BLK, 1);
	if (ret) {
		log_err("ERROR: primary ivt write failed!\n");
		ret = CMD_RET_FAILURE;
		goto ret_fail;
	}

	/* write duplicate ivt */
	ret = hse_mmc_write(&priv->ivt, HSE_DIVT_BLK, 1);
	if (ret) {
		log_err("ERROR: duplicate ivt write failed!\n");
		ret = CMD_RET_FAILURE;
		goto ret_fail;
	}

	ret = CMD_RET_SUCCESS;
ret_fail:
	return ret;
}

static int do_hse_adv_secboot_prep(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	struct hse_private *priv;
	u16 hse_status_ret;
	u32 hse_recv;
	u64 hse_resmem;
	int hse_nodeoffset, ret;

	/* check if hse has been initialised */
	hse_status_ret = hse_mu_check_status();
	if (!(hse_status_ret & HSE_STATUS_INIT_OK)) {
		/* keep printf to warn user if hse fw is missing */
		printf("ERROR: HSE not initialised or missing firmware!\n");
		ret = CMD_RET_FAILURE;
		goto ret_fail;
	}

	/* find mem reserved for hse */
	hse_nodeoffset = fdt_path_offset(gd->fdt_blob,
				     "/reserved-memory/hse_reserved");
	if (hse_nodeoffset < 0) {
		printf("ERROR: hse_reserved node not found!\n");
		return hse_nodeoffset;
	}

	hse_resmem = fdt_get_base_address(gd->fdt_blob, hse_nodeoffset);
	if (hse_resmem < 0) {
		printf("ERROR: could not get base address of hse_reserved node!\n");
		return hse_resmem;
	}

	priv = (struct hse_private *)hse_resmem;
	memset((void *)priv, 0, sizeof(struct hse_private));

	/* can only read from mmc in blocks of 512B */
	ret = hse_mmc_read((void *)&priv->ivt, HSE_PIVT_BLK, 1);
	if (ret) {
		/* try reading duplicate ivt */
		ret = hse_mmc_read((void *)&priv->ivt, HSE_DIVT_BLK, 1);
		if (ret) {
			log_err("ERROR: ivt read failed!\n");
			goto ret_fail;
		}
	}

	/* check if sys_img already exists */
	if (priv->ivt.sys_img) {
		printf("CHECK: SYS_IMG already exists\n");
		if (priv->ivt.boot_cfg & HSE_IVT_BOOTSEQ_BIT) {
			/* do nothing */
			printf("CHECK: BOOTSEQ bit already set\n");

			ret = CMD_RET_SUCCESS;
			goto ret_fail;
		} else {
			/* set bootseq bit in boot cfg word */
			printf("\tSetting BOOTSEQ bit...\n");
			priv->ivt.boot_cfg |= HSE_IVT_BOOTSEQ_BIT;

			/* write primary ivt */
			ret = hse_mmc_write(&priv->ivt, HSE_PIVT_BLK, 1);
			if (ret) {
				log_err("ERROR: primary ivt write failed!\n");
				ret = CMD_RET_FAILURE;
				goto ret_fail;
			}

			/* write duplicate ivt */
			ret = hse_mmc_write(&priv->ivt, HSE_DIVT_BLK, 1);
			if (ret) {
				log_err("ERROR: duplicate ivt write failed!\n");
				goto ret_fail;
			}

			ret = CMD_RET_SUCCESS;
			goto ret_fail;
		}
	}

	ret = hse_format_key_store(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_import_key(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_install_smr_entry(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_install_cr_entry(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_generate_sys_img(priv, &hse_recv);
	if (ret) 
		goto ret_fail;

	ret = hse_write_sys_img(priv, true);
	if (ret) 
		goto ret_fail;

	ret = CMD_RET_SUCCESS;
ret_fail:
	return ret;
}

U_BOOT_CMD(hse_adv_secboot_prep_demo, 1, 0, do_hse_adv_secboot_prep,
	   "generate device-specific SYS_IMG",
	   "Generate SYS-IMG and place it on SD card");

static int do_hse_keystore_format(cmd_tbl_t *cmdtp, int flag,
				  int argc, char * const argv[])
{
	struct hse_private *priv;
	u16 hse_status_ret;
	u32 hse_recv;
	u64 hse_resmem;
	int hse_nodeoffset, ret;

	/* check if hse has been initialised */
	hse_status_ret = hse_mu_check_status();
	if (!(hse_status_ret & HSE_STATUS_INIT_OK)) {
		/* keep printf to warn user if hse is missing all the time */
		printf("ERROR: HSE not initialised or missing firmware!\n");
		ret = CMD_RET_FAILURE;
		goto ret_fail;
	}

	/* find mem reserved for hse */
	hse_nodeoffset = fdt_path_offset(gd->fdt_blob,
				     "/reserved-memory/hse_reserved");
	if (hse_nodeoffset < 0) {
		log_err("ERROR: hse_reserved node not found!\n");
		return hse_nodeoffset;
	}

	hse_resmem = fdt_get_base_address(gd->fdt_blob, hse_nodeoffset);
	if (hse_resmem < 0) {
		log_err("ERROR: could not get base address of hse_reserved node!\n");
		return hse_resmem;
	}

	priv = (struct hse_private *)hse_resmem;
	memset((void *)priv, 0, sizeof(struct hse_private));

	/* can only read from mmc in blocks of 512B */
	ret = hse_mmc_read((void *)&priv->ivt, HSE_PIVT_BLK, 1);
	if (ret) {
		/* try reading duplicate ivt */
		ret = hse_mmc_read((void *)&priv->ivt, HSE_DIVT_BLK, 1);
		if (ret) {
			log_err("ERROR: ivt read failed!\n");
			goto ret_fail;
		}
	}

	/* check if sys_img already exists */
	if (priv->ivt.sys_img) {
		printf("CHECK: SYS_IMG already exists\n");
		ret = CMD_RET_SUCCESS;
		goto ret_fail;
	}

	ret = hse_format_key_store(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_generate_sys_img(priv, &hse_recv);
	if (ret)
		goto ret_fail;

	ret = hse_write_sys_img(priv, false);
	if (ret)
		goto ret_fail;

	ret = CMD_RET_SUCCESS;
ret_fail:
	return ret;
}

U_BOOT_CMD(hse_keystore_format, 1, 0, do_hse_keystore_format,
	   "format the keystore",
	   "Format keystore for use in Linux kernel driver");

#endif /* CONFIG_SD_BOOT */
