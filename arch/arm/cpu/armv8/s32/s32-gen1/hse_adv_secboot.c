// SPDX-License-Identifier: BSD-3-Clause
/*
 * HSE advanced secure boot preparatory command demo
 *
 * Copyright 2020 NXP
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <log.h>
#include <errno.h>
#include <hse/hse_mu.h>
#include <hse/hse_abi.h>

/* public key modulus */
static const u8 rsa2048_pub_modulus[] = { 0xf4, 0x72, 0xe2, 0xe9, 0x83, 0xd1,
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
static const u8 rsa2048_pub_exponent[] = { 0x01, 0x00, 0x01 };

/* the nvm container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry nvm_catalog[] = {
	HSE_NVM_KEY_CATALOG_CFG
};

/* the ram container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry ram_catalog[] = {
	HSE_RAM_KEY_CATALOG_CFG
};

#ifdef CONFIG_SD_BOOT

int hse_format_key_store(struct hse_srv_desc *srv_desc, u32 *recv_buf)
{
	int ret = 0;

	printf("\tFormatting NVM and RAM key stores...\n");

	srv_desc->srv_id = HSE_SRV_ID_FORMAT_KEY_CATALOGS;
	srv_desc->format_catalogs_req.nvm_key_catalog_cfg_addr =
			(uintptr_t)&nvm_catalog;
	srv_desc->format_catalogs_req.ram_key_catalog_cfg_addr =
			(uintptr_t)&ram_catalog;

	flush_dcache_range((u64)srv_desc,
			   (u64)srv_desc + sizeof(struct hse_srv_desc));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: key catalog format failed!\n");
		goto hse_send_fail;
	}
	memset((void *)srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	return ret;
}

int hse_import_key(struct hse_srv_desc *srv_desc, u32 *recv_buf)
{
	struct hse_key_info *key_info;
	int ret = 0;

	printf("\tImporting RSA public key into NVM key store...\n");

	key_info = (struct hse_key_info *)
		   calloc(1, sizeof(struct hse_key_info));
	if (!key_info) {
		log_err("ERROR: key info alloc failed!\n");
		ret = -ENOMEM;
		goto alloc_fail;
	}

	key_info->key_flags = (HSE_KF_USAGE_VERIFY |
			       HSE_KF_USAGE_AUTHORIZATION);
	key_info->key_bit_len = BYTES_TO_BITS(ARRAY_SIZE(rsa2048_pub_modulus));
	key_info->key_counter = 0ul;
	key_info->smr_flags = 0ul;
	key_info->key_type = HSE_KEY_TYPE_RSA_PUB;
	key_info->pub_exponent_size = ARRAY_SIZE(rsa2048_pub_exponent);

	srv_desc->srv_id = HSE_SRV_ID_IMPORT_KEY;
	srv_desc->import_key_req.key_handle = HSE_BOOT_KEY_HANDLE;
	srv_desc->import_key_req.key_info_addr = (uintptr_t)key_info;
	srv_desc->import_key_req.key_addr[0] = (uintptr_t)&rsa2048_pub_modulus;
	srv_desc->import_key_req.key_addr[1] = (uintptr_t)&rsa2048_pub_exponent;
	srv_desc->import_key_req.key_addr[2] = 0u;
	srv_desc->import_key_req.key_len[0] = ARRAY_SIZE(rsa2048_pub_modulus);
	srv_desc->import_key_req.key_len[1] = ARRAY_SIZE(rsa2048_pub_exponent);
	srv_desc->import_key_req.key_len[2] = 0u;
	srv_desc->import_key_req.cipher_key = HSE_INVALID_KEY_HANDLE;
	srv_desc->import_key_req.auth_key = HSE_INVALID_KEY_HANDLE;

	flush_dcache_range((u64)srv_desc,
			   (u64)srv_desc + sizeof(struct hse_srv_desc));
	flush_dcache_range((u64)key_info,
			   (u64)key_info + sizeof(struct hse_key_info));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: rsa public key import failed!\n");
		goto hse_send_fail;
	}
	memset((void *)srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	free(key_info);
alloc_fail:
	return ret;
}

int hse_install_cr_entry(struct hse_srv_desc *srv_desc, u32 *recv_buf)
{
	struct hse_cr_entry *cr_entry;
	int ret = 0;

	printf("\tGenerating Core Reset Entry...\n");

	cr_entry = (struct hse_cr_entry *)
		   calloc(1, sizeof(struct hse_cr_entry));
	if (!cr_entry) {
		log_err("ERROR: core reset entry alloc failed!\n");
		ret = -ENOMEM;
		goto alloc_fail;
	}

	cr_entry->core_id = HSE_APP_CORE3;
	cr_entry->fail_verify_flag = 0u;
	cr_entry->cr_sanction = HSE_CR_SANCTION_KEEP_CORE_IN_RESET;
	cr_entry->pass_reset = CONFIG_SYS_TEXT_BASE;
	cr_entry->smr_entries_bits = HSE_SMR_ENTRY_1;
	cr_entry->fail_reset = 0u;

	srv_desc->srv_id = HSE_SRV_ID_CORE_RESET_ENTRY_INSTALL;
	srv_desc->cr_install_req.cr_entry_index = 1u;
	srv_desc->cr_install_req.cr_entry_addr = (uintptr_t)cr_entry;

	flush_dcache_range((u64)srv_desc,
			   (u64)srv_desc + sizeof(struct hse_srv_desc));
	flush_dcache_range((u64)cr_entry,
			   (u64)cr_entry + sizeof(struct hse_cr_entry));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: core reset entry install failed!\n");
		goto hse_send_fail;
	}
	memset((void *)srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	free(cr_entry);
alloc_fail:
	return ret;
}

int hse_install_smr_entry(struct hse_srv_desc *srv_desc, u32 *recv_buf,
			  u32 smr_src)
{
	struct hse_smr_entry *smr_entry;
	void *sign_ram_cpy, *uboot_ram_cpy;
	u32 *sign_len_ram_cpy;
	int ret = 0;

	printf("\tGenerating Secure Memory Region entry...\n");

	sign_ram_cpy = calloc(1, HSE_UBOOT_AUTH_LEN);
	if (!sign_ram_cpy) {
		log_err("ERROR: signature alloc failed!\n");
		ret = -ENOMEM;
		goto alloc_fail;
	}

	uboot_ram_cpy = calloc(1, HSE_UBOOT_MAX_SIZE);
	if (!uboot_ram_cpy) {
		log_err("ERROR: u-boot alloc failed!\n");
		ret = -ENOMEM;
		goto sign_alloc_fail;
	}

	sign_len_ram_cpy = (u32 *)calloc(1, sizeof(u32));
	if (!sign_len_ram_cpy) {
		log_err("ERROR: signature length alloc failed!!\n");
		ret = -ENOMEM;
		goto uboot_alloc_fail;
	}
	*sign_len_ram_cpy = HSE_UBOOT_AUTH_LEN - 0x100;

	ret = hse_mmc_read(sign_ram_cpy, 1025, 1);
	if (ret) {
		log_err("ERROR: signature read failed!\n");
		goto sign_len_alloc_fail;
	}

	ret = hse_mmc_read(uboot_ram_cpy, 1042, 2048);
	if (ret) {
		log_err("ERROR: u-boot read failed!\n");
		goto sign_len_alloc_fail;
	}

	smr_entry = (struct hse_smr_entry *)
		    calloc(1, sizeof(struct hse_smr_entry));
	if (!smr_entry) {
		log_err("ERROR: smr entry alloc failed!\n");
		ret = -ENOMEM;
		goto sign_len_alloc_fail;
	}

	/**
	 * the SMR will contain the app bl header of size 0x40, so we subtract
	 * 0x40 from CONFIG_SYS_TEXT_BASE
	 */
	smr_entry->smr_src = smr_src;
	smr_entry->smr_dst_addr = CONFIG_SYS_TEXT_BASE - 0x40;
	smr_entry->smr_size = HSE_UBOOT_MAX_SIZE;
	smr_entry->config_flags = (HSE_SMR_CFG_FLAG_SD_FLASH |
				   HSE_SMR_CFG_FLAG_INSTALL_AUTH);
	smr_entry->verif_method = HSE_SMR_VERIF_PRE_BOOT_MASK;
	smr_entry->key_handle = HSE_BOOT_KEY_HANDLE;
	smr_entry->sign_sch.sign_scheme = HSE_SIGN_RSASSA_PKCS1_V15;
	smr_entry->sign_sch.sch.hash_algo = HSE_HASH_ALGO_SHA_1;
	smr_entry->auth_tag = HSE_AUTH_TAG_SD;

	srv_desc->srv_id = HSE_SRV_ID_SMR_ENTRY_INSTALL;
	srv_desc->smr_install_req.access_mode = HSE_ACCESS_MODE_ONE_PASS;
	srv_desc->smr_install_req.entry_index = 1u;
	srv_desc->smr_install_req.smr_entry_addr = (uintptr_t)smr_entry;
	srv_desc->smr_install_req.smr_data_addr = (uintptr_t)uboot_ram_cpy;
	srv_desc->smr_install_req.smr_data_len = HSE_UBOOT_MAX_SIZE;
	srv_desc->smr_install_req.smr_auth_tag_addr = (uintptr_t)sign_ram_cpy;
	srv_desc->smr_install_req.smr_auth_tag_len_addr =
					(uintptr_t)sign_len_ram_cpy;

	flush_dcache_range((u64)smr_entry,
			   (u64)smr_entry + sizeof(struct hse_smr_entry));
	flush_dcache_range((u64)srv_desc,
			   (u64)srv_desc + sizeof(struct hse_srv_desc));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: smr entry install failed!\n");
		goto hse_send_fail;
	}
	memset((void *)srv_desc, 0, sizeof(struct hse_srv_desc));

hse_send_fail:
	free(smr_entry);
sign_len_alloc_fail:
	free(sign_len_ram_cpy);
uboot_alloc_fail:
	free(uboot_ram_cpy);
sign_alloc_fail:
	free(sign_ram_cpy);
alloc_fail:
	return ret;
}

int hse_generate_sys_img(struct hse_srv_desc *srv_desc, u32 *recv_buf,
			 void *sys_img_buf)
{
	u32 *sys_img_buf_size_ram;
	int ret = 0;

	/* unused, but required by HSE */
	u8 publish_state;
	u32 publish_offset;

	printf("\tGenerating SYS_IMG...\n");

	sys_img_buf_size_ram = (u32 *)calloc(1, sizeof(u32));
	if (!sys_img_buf_size_ram) {
		log_err("ERROR: sys-img length alloc failed!\n");
		ret = -ENOMEM;
		goto alloc_fail;
	}
	*sys_img_buf_size_ram = HSE_SYS_IMG_MAX_SIZE;

	srv_desc->srv_id = HSE_SRV_ID_PUBLISH_SYS_IMAGE;
	srv_desc->publish_sys_img_req.publish_options =
					HSE_PUBLISH_ALL_DATA_SETS;
	srv_desc->publish_sys_img_req.publish_state_addr =
					(uintptr_t)&publish_state;
	srv_desc->publish_sys_img_req.publish_offset_addr =
					(uintptr_t)&publish_offset;
	srv_desc->publish_sys_img_req.buff_length_addr =
					(uintptr_t)sys_img_buf_size_ram;
	srv_desc->publish_sys_img_req.buff_addr = (uintptr_t)sys_img_buf;

	flush_dcache_range((u64)srv_desc,
			   (u64)srv_desc + sizeof(struct hse_srv_desc));
	flush_dcache_range((u64)sys_img_buf_size_ram,
			   (u64)sys_img_buf_size_ram + sizeof(u32));
	flush_dcache_range((u64)sys_img_buf,
			   (u64)sys_img_buf + HSE_SYS_IMG_MAX_SIZE);

	ret = hse_send_recv(HSE_CHANNEL_ADMIN,
			    (u32)(uintptr_t)srv_desc,
			    recv_buf);
	if (ret) {
		log_err("ERROR: sys-img publish failed!\n");
		goto hse_send_fail;
	}

hse_send_fail:
	free(sys_img_buf_size_ram);
alloc_fail:
	return ret;
}

static int do_hse_adv_secboot_prep(cmd_tbl_t *cmdtp, int flag,
				   int argc, char * const argv[])
{
	struct ivt *ivt;
	struct hse_srv_desc *srv_desc;
	void *sys_img_buf;
	u16 hse_status_ret;
	u32 recv_buf;
	int ret;

	/* check if hse has been initialised */
	hse_status_ret = hse_mu_check_status();
	if (!(hse_status_ret & HSE_STATUS_INIT_OK)) {
		log_err("ERROR: HSE not initialised!\n");
		ret = CMD_RET_FAILURE;
		goto ret_general;
	}

	/* can only read from mmc in blocks of 512B */
	ret = hse_mmc_read((void *)HSE_READ_LOC, 0, 1);
	if (ret) {
		log_err("ERROR: ivt read failed!\n");
		goto ret_general;
	}

	ivt = (struct ivt *)HSE_READ_LOC;

	/* check if sys_img already exists */
	if (ivt->sys_img) {
		if (ivt->boot_cfg & (1 << 3)) {
			/* do nothing */
			ret = CMD_RET_SUCCESS;
			goto ret_general;
		} else {
			/* set bootseq bit in boot cfg word */
			ivt->boot_cfg |= (1 << 3);

			ret = hse_mmc_write(ivt, 0, 1);
			if (ret) {
				log_err("ERROR: ivt write failed!\n");
				goto ret_general;
			}

			ret = CMD_RET_SUCCESS;
			goto ret_general;
		}
	}

	printf("\tAllocating space for service descriptor...\n");

	srv_desc = (struct hse_srv_desc *)
		   calloc(1, sizeof(struct hse_srv_desc));
	if (!srv_desc) {
		log_err("ERROR: service descriptor alloc failed!\n");
		ret = CMD_RET_FAILURE;
		goto ret_general;
	}

	ret = hse_format_key_store(srv_desc, &recv_buf);
	if (ret) {
		ret = CMD_RET_FAILURE;
		goto srv_desc_alloc_fail;
	}

	hse_import_key(srv_desc, &recv_buf);
	if (ret) {
		ret = CMD_RET_FAILURE;
		goto srv_desc_alloc_fail;
	}

	hse_install_cr_entry(srv_desc, &recv_buf);
	if (ret) {
		ret = CMD_RET_FAILURE;
		goto srv_desc_alloc_fail;
	}

	hse_install_smr_entry(srv_desc, &recv_buf, (u32)(ivt->app_boot));
	if (ret) {
		ret = CMD_RET_FAILURE;
		goto srv_desc_alloc_fail;
	}

	sys_img_buf = calloc(1, HSE_SYS_IMG_MAX_SIZE);
	if (!sys_img_buf) {
		log_err("ERROR: sys-img alloc failed!\n");
		ret = CMD_RET_FAILURE;
		goto srv_desc_alloc_fail;
	}

	hse_generate_sys_img(srv_desc, &recv_buf, sys_img_buf);
	if (ret) {
		ret = CMD_RET_FAILURE;
		goto sys_img_alloc_fail;
	}

	printf("\tWriting SYS_IMG to SDcard...\n");

	flush_dcache_range((u64)sys_img_buf,
			   (u64)sys_img_buf + HSE_SYS_IMG_MAX_SIZE);

	ret = hse_mmc_write(sys_img_buf, 897, 128);
	if (ret) {
		log_err("ERROR: sys-img write failed!\n");
		ret = CMD_RET_FAILURE;
		goto sys_img_alloc_fail;
	}

	printf("\tUpdating SYS_IMG pointer...\n");

	/* set the sys img address, external flash type, flash page size */
	ivt->sys_img = HSE_SYS_IMG_SD;
	ivt->sys_img_ext_flash_type = HSE_EXT_FLASH_SD;
	ivt->sys_img_flash_page_size = HSE_EXT_FLASH_PAGE;

	/* set BOOT_SEQ bit */
	ivt->boot_cfg |= (1 << 3);

	ret = hse_mmc_write(ivt, 0, 1);
	if (ret) {
		log_err("ERROR: IVT write failed!\n");
		ret = CMD_RET_FAILURE;
		goto sys_img_alloc_fail;
	}

	ret = CMD_RET_SUCCESS;
sys_img_alloc_fail:
	free(sys_img_buf);
srv_desc_alloc_fail:
	free(srv_desc);
ret_general:
	return ret;
}

#endif /* CONFIG_SD_BOOT */

U_BOOT_CMD(hse_adv_secboot_prep_demo, 1, 0, do_hse_adv_secboot_prep,
	   "generate device-specific SYS_IMG",
	   "Generate SYS-IMG and place it on SD card");
