// SPDX-License-Identifier: BSD-3-Clause
/*
 * HSE advanced secure boot preparatory command demo
 *
 * Copyright 2020-2022 NXP
 */

#include <common.h>
#include <command.h>
#include <cpu_func.h>
#include <errno.h>
#include <fs.h>
#include <malloc.h>
#include <hse/hse_abi.h>
#include <hse/hse_mu.h>

#ifdef CONFIG_SD_BOOT
DECLARE_GLOBAL_DATA_PTR;

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
	HSE_KEY_TYPE_AES, 6u, HSE_KEY256_BITS }, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_HMAC, 6u, HSE_KEY512_BITS}, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_AES, 7u, HSE_KEY256_BITS }, \
{HSE_ALL_MU_MASK, HSE_KEY_OWNER_ANY, \
	HSE_KEY_TYPE_SHARED_SECRET, 1u, HSE_KEY256_BITS}, \
{0u, 0u, 0u, 0u, 0u}

#define APP_CODE_OFFSET 0x40

#define UUID_BL2_CERT \
	{ 0xea69e2d6, \
	  0x635d, \
	  0x11e4, \
	  0x8d, 0x8c, \
	 {0x9f, 0xba, 0xbe, 0x99, 0x56, 0xa5} }

/* the nvm container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry nvm_orig_cat[] = {
	HSE_NVM_KEY_CATALOG_CFG
};

/* the ram container used to format the hse key catalogs */
static const struct hse_key_group_cfg_entry ram_orig_cat[] = {
	HSE_RAM_KEY_CATALOG_CFG
};

/* return 0 for equal uuids */
static inline int compare_uuids(const struct uuid *uuid1,
				const struct uuid *uuid2)
{
	return memcmp(uuid1, uuid2, sizeof(struct uuid));
}

static u32 get_fip_start(struct hse_private *priv)
{
	return priv->ivt.app_boot + APP_CODE_OFFSET;
}

static u64 get_fip_toc_offset(struct hse_private *priv, struct uuid *search)
{
	struct fip_toc_header *toc_header;
	struct fip_toc_entry *toc_entry;
	uintptr_t fip_hdr_start, fip_hdr_end;

	fip_hdr_start = (uintptr_t)priv->app_boot_hdr.ram_load;
	toc_header = (struct fip_toc_header *)fip_hdr_start;
	toc_entry = (struct fip_toc_entry *)(toc_header + 1);

	/* fip_hdr_end is at the start of the first entry */
	fip_hdr_end = fip_hdr_start + (uintptr_t)toc_entry->offset;

	while ((uintptr_t)toc_entry < fip_hdr_end) {
		if (!compare_uuids(&toc_entry->uuid, search))
			return toc_entry->offset;
		toc_entry++;
	}

	return 0;
}

static u64 get_fip_size(struct hse_private *priv)
{
	struct uuid uuid_null = { 0 };

	return get_fip_toc_offset(priv, &uuid_null);
}

static u64 get_fip_sign_offset(struct hse_private *priv)
{
	struct uuid uuid_bl2_cert = UUID_BL2_CERT;

	return get_fip_toc_offset(priv, &uuid_bl2_cert);
}

static u32 get_fip_sign_mmc(struct hse_private *priv)
{
	u32 sign_offset;

	sign_offset = (u32)get_fip_sign_offset(priv);
	if (!sign_offset)
		return 0;

	return get_fip_start(priv) + sign_offset;
}

static uintptr_t get_fip_sign_sram(struct hse_private *priv)
{
	uintptr_t fip_hdr_start = (uintptr_t)priv->app_boot_hdr.ram_load;
	uintptr_t sign_offset;

	sign_offset = (uintptr_t)get_fip_sign_offset(priv);
	if (!sign_offset)
		return 0;

	return fip_hdr_start + sign_offset;
}

int hse_format_key_store(struct hse_private *priv, u32 *recv_buf)
{
	int ret = 0;

	printf("\tFormatting NVM and RAM key stores...\n");
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

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
		printf("ERROR: key catalog format failed!\n");
		return ret;
	}

	return 0;
}

int hse_import_key(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_key_info *key_info;
	struct hse_import_key_srv *import_key_req;
	int ret;

	key_info = &(priv->key_info);
	import_key_req = &(priv->srv_desc.import_key_req);

	printf("\tImporting RSA public key into NVM key store...\n");
	memset((void *)&priv->key_info, 0, sizeof(struct hse_key_info));
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

	key_info->key_flags = HSE_KF_USAGE_VERIFY;
	key_info->key_bit_len =	BYTES_TO_BITS(ARRAY_SIZE(priv->rsa_modulus));
	key_info->key_counter = 0ul;
	key_info->smr_flags = 0ul;
	key_info->key_type = HSE_KEY_TYPE_RSA_PUB;
	key_info->pub_exponent_size = ARRAY_SIZE(priv->rsa_exponent);

	priv->srv_desc.srv_id = HSE_SRV_ID_IMPORT_KEY;
	import_key_req->key_handle = HSE_BOOT_KEY_HANDLE;
	import_key_req->key_info_addr = (uintptr_t)key_info;
	import_key_req->key_addr[0] = (uintptr_t)priv->rsa_modulus;
	import_key_req->key_addr[1] = (uintptr_t)priv->rsa_exponent;
	import_key_req->key_addr[2] = 0u;
	import_key_req->key_len[0] = ARRAY_SIZE(priv->rsa_modulus);
	import_key_req->key_len[1] = ARRAY_SIZE(priv->rsa_exponent);
	import_key_req->key_len[2] = 0u;
	import_key_req->cipher_key = HSE_INVALID_KEY_HANDLE;
	import_key_req->auth_key = HSE_INVALID_KEY_HANDLE;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		printf("ERROR: rsa public key import failed!\n");
		return ret;
	}

	return 0;
}

int hse_install_cr_entry(struct hse_private *priv, u32 *recv_buf)
{
	int ret;

	printf("\tGenerating Core Reset Entry...\n");
	memset((void *)&priv->cr_entry, 0, sizeof(struct hse_cr_entry));
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

	priv->cr_entry.core_id = HSE_APP_CORE3;
	priv->cr_entry.cr_sanction = HSE_CR_SANCTION_KEEP_CORE_IN_RESET;
	priv->cr_entry.preboot_smr_map = HSE_SMR_ENTRY_1;
	priv->cr_entry.pass_reset = priv->app_boot_hdr.ram_entry;
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
		printf("ERROR: core reset entry install failed!\n");
		return ret;
	}

	return 0;
}

int hse_install_smr_entry(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_smr_entry *smr_entry;
	struct hse_smr_install_srv *smr_install_req;
	u32 fip_size;
	int ret;

	smr_entry = &(priv->smr_entry);
	smr_install_req = &(priv->srv_desc.smr_install_req);

	printf("\tGenerating Secure Memory Region entry...\n");
	memset((void *)&priv->smr_entry, 0, sizeof(struct hse_smr_entry));
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

	/* need to recopy FIP to pass verification */
	memset((void *)(uintptr_t)priv->app_boot_hdr.ram_load - APP_CODE_OFFSET, 0,
	       priv->app_boot_hdr.code_len + APP_CODE_OFFSET);
	hse_mmc_read((void *)(uintptr_t)priv->app_boot_hdr.ram_load - APP_CODE_OFFSET,
		     priv->ivt.app_boot / 512,
		     (priv->app_boot_hdr.code_len / 512) + 1);

	fip_size = get_fip_size(priv);
	if (!fip_size) {
		printf("ERROR: invalid FIP size!\n");
		return -ENOMEM;
	}

	/**
	 * no address of actual code start, need to reference app bl header
	 * fip start is at app_bl_header + 0x40
	 */
	smr_entry->smr_src = priv->ivt.app_boot + APP_CODE_OFFSET;
	smr_entry->smr_dst_addr = priv->app_boot_hdr.ram_load;
	smr_entry->smr_size = fip_size - HSE_FIP_AUTH_LEN;
	smr_entry->config_flags = (HSE_SMR_CFG_FLAG_SD_FLASH |
				   HSE_SMR_CFG_FLAG_INSTALL_AUTH);
	smr_entry->check_period = 0;
	smr_entry->key_handle = HSE_BOOT_KEY_HANDLE;
	smr_entry->sign_sch.sign_scheme = HSE_SIGN_RSASSA_PKCS1_V15;
	smr_entry->sign_sch.sch.hash_algo = HSE_HASH_ALGO_SHA_1;
	smr_entry->auth_tag = get_fip_sign_mmc(priv);
	smr_entry->decrypt_key_handle = HSE_SMR_DECRYPT_KEY_HANDLE_NOT_USED;
	smr_entry->version_offset = 0;

	priv->srv_desc.srv_id = HSE_SRV_ID_SMR_ENTRY_INSTALL;
	smr_install_req->access_mode = HSE_ACCESS_MODE_ONE_PASS;
	smr_install_req->entry_index = 1u;
	smr_install_req->smr_entry_addr = (uintptr_t)smr_entry;
	smr_install_req->smr_data_addr = priv->app_boot_hdr.ram_load;
	smr_install_req->smr_data_len = fip_size - HSE_FIP_AUTH_LEN;
	smr_install_req->smr_auth_tag_addr = (uintptr_t)priv->fip_signature;
	smr_install_req->smr_auth_tag_len = HSE_FIP_AUTH_LEN;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_GENERAL,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		printf("ERROR: smr entry install failed!\n");
		return ret;
	}

	return 0;
}

int hse_generate_sys_img(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_publish_sys_img_srv *publish_sys_img_req;
	int ret;

	publish_sys_img_req = &(priv->srv_desc.publish_sys_img_req);

	printf("\tGenerating SYS_IMG...\n");
	memset((void *)priv->sys_img, 0, HSE_SYS_IMG_MAX_SIZE);
	memset((void *)&priv->srv_desc, 0, sizeof(struct hse_srv_desc));

	priv->sys_img_len = HSE_SYS_IMG_MAX_SIZE;
	priv->srv_desc.srv_id = HSE_SRV_ID_PUBLISH_SYS_IMAGE;
	publish_sys_img_req->publish_options = HSE_PUBLISH_ALL_DATA_SETS;
	publish_sys_img_req->publish_offset_addr = 
		(uintptr_t)&priv->publish_offset;
	publish_sys_img_req->buff_length_addr = (uintptr_t)&priv->sys_img_len;
	publish_sys_img_req->buff_addr = (uintptr_t)priv->sys_img;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_ADMIN,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		printf("ERROR: sys-img generation failed!\n");
		return ret;
	}

	return 0;
}

int hse_write_sys_img(struct hse_private *priv, bool secure)
{
	int ret;
	u32 sys_img_blk, sys_img_num_blks;

	printf("\tPublishing SYS_IMG...\n");

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	sys_img_blk = priv->ivt.sys_img / 512;
	sys_img_num_blks = HSE_SYS_IMG_MAX_SIZE / 512;

	ret = hse_mmc_write(&priv->sys_img, sys_img_blk, sys_img_num_blks);
	if (ret) {
		printf("ERROR: sys-img publish failed!\n");
		return ret;
	}

	/* external flash type, flash page size */
	priv->ivt.sys_img_ext_flash_type = HSE_EXT_FLASH_SD;
	priv->ivt.sys_img_flash_page_size = HSE_EXT_FLASH_PAGE;

	/* set BOOT_SEQ bit, if using secure boot */
	if (secure)
		priv->ivt.boot_cfg |= HSE_IVT_BOOTSEQ_BIT;

	/* write ivt */
	ret = hse_mmc_write(&priv->ivt, HSE_IVT_BLK, 1);
	if (ret) {
		printf("ERROR: ivt write failed!\n");
		return ret;
	}

	return 0;
}

int hse_enable_mus(struct hse_private *priv, u32 *recv_buf)
{
	struct hse_getset_attr_srv *getset_attr_req;
	int ret;

	printf("\tEnabling MUs...\n");

	getset_attr_req = &(priv->srv_desc.getset_attr_req);

	priv->srv_desc.srv_id = HSE_SRV_ID_SET_ATTR;

	priv->mu_config.mu_instances[0].mu_config = HSE_MU_ACTIVATED;
	priv->mu_config.mu_instances[0].xrdc_domain_id = 0u;
	priv->mu_config.mu_instances[0].shared_mem_chunk_size = 0u;

	priv->mu_config.mu_instances[1].mu_config = HSE_MU_ACTIVATED;
	priv->mu_config.mu_instances[1].xrdc_domain_id = 0u;
	priv->mu_config.mu_instances[1].shared_mem_chunk_size = 0u;

	priv->mu_config.mu_instances[2].mu_config = HSE_MU_ACTIVATED;
	priv->mu_config.mu_instances[2].xrdc_domain_id = 0u;
	priv->mu_config.mu_instances[2].shared_mem_chunk_size = 0u;

	priv->mu_config.mu_instances[3].mu_config = HSE_MU_ACTIVATED;
	priv->mu_config.mu_instances[3].xrdc_domain_id = 0u;
	priv->mu_config.mu_instances[3].shared_mem_chunk_size = 0u;

	getset_attr_req->attr_id = HSE_MU_CONFIG_ATTR_ID;
	getset_attr_req->attr_len = sizeof(struct hse_mu_config);
	getset_attr_req->p_attr = (uintptr_t)&priv->mu_config;

	flush_dcache_range((u64)priv,
			   (u64)priv + sizeof(struct hse_private));

	ret = hse_send_recv(HSE_CHANNEL_ADMIN,
			    (u32)(uintptr_t)&priv->srv_desc,
			    recv_buf);
	if (ret) {
		printf("ERROR: enable MU failed!\n");
		return ret;
	}

	return 0;
}

static int do_hse_secboot_enable(cmd_tbl_t *cmdtp, int flag,
				 int argc, char * const argv[])
{
	struct hse_private *priv;
	char *pubkey_file;
	char mmcdevpart[4];
	u16 hse_status_ret;
	u32 hse_recv;
	u64 hse_resmem;
	long long len_read;
	int hse_nodeoffset, ret;
	uintptr_t fip_sign_sram;

	/* check if hse has been initialised */
	hse_status_ret = hse_mu_check_status();
	if (!(hse_status_ret & HSE_STATUS_INIT_OK)) {
		printf("ERROR: HSE not initialised or missing firmware!\n");
		return CMD_RET_FAILURE;
	}

	if (argc < 2 || !argv[1]) {
		printf("USAGE: hse_secboot_enable <public_key_file>.der\n");
		printf("\n");
		printf("    <public_key_file>.der - rsa public key in DER format\n");
		printf("                            in the FAT partition\n");
		return CMD_RET_FAILURE;
	}
	pubkey_file = argv[1];

	/* find mem reserved for hse */
	hse_nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						       "nxp,s32cc-hse-rmem");
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

	/* read ivt block */
	ret = hse_mmc_read((void *)&priv->ivt, HSE_IVT_BLK, 1);
	if (ret) {
		printf("ERROR: ivt read failed!\n");
		return ret;
	}

	/* read app boot code header */
	ret = hse_mmc_read((void *)&priv->app_boot_hdr,
			   (priv->ivt.app_boot / 512), 1);
	if (ret) {
		printf("ERROR: app boot code header read failed!\n");
		return ret;
	}

	fip_sign_sram = get_fip_sign_sram(priv);
	if (!fip_sign_sram) {
		printf("ERROR: FIP signature read failed!\n");
		return -ENOMEM;
	}
	memcpy((void *)priv->fip_signature, (void *)fip_sign_sram, HSE_FIP_AUTH_LEN);

	/* read public key file */
	snprintf(mmcdevpart, sizeof(mmcdevpart), "%s:%s",
		 env_get("mmcdev"), env_get("mmcpart"));
	ret = fs_set_blk_dev("mmc", mmcdevpart, FS_TYPE_FAT);
	if (ret) {
		printf("ERROR: could not set block device!\n");
		return ret;
	}
	ret = fs_read(pubkey_file, (uintptr_t)priv->rsa_pubkey, 0, 0, &len_read);
	if (ret < 0) {
		printf("ERROR: could not read public key file!\n");
		return ret;
	}

	memcpy(priv->rsa_modulus,
	       (priv->rsa_pubkey + MODULUS_OFFSET),
	       MODULUS_SIZE);
	memcpy(priv->rsa_exponent,
	       (priv->rsa_pubkey + EXPONENT_OFFSET),
	       EXPONENT_SIZE);

	ret = hse_enable_mus(priv, &hse_recv);
	if (ret)
		return ret;

	/* check if sys_img already exists */
	if (!(hse_status_ret & HSE_STATUS_PRIMARY_SYSIMG)) {
		printf("\tNo SYS_IMG, formatting key store...\n");

		ret = hse_format_key_store(priv, &hse_recv);
		if (ret)
			return ret;
	}

	ret = hse_import_key(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_install_smr_entry(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_install_cr_entry(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_generate_sys_img(priv, &hse_recv);
	if (ret) 
		return ret;

	ret = hse_write_sys_img(priv, true);
	if (ret) 
		return ret;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(hse_secboot_enable, 2, 0, do_hse_secboot_enable,
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
		printf("ERROR: HSE not initialised or missing firmware!\n");
		return CMD_RET_FAILURE;
	}

	/* check if sys_img already exists */
	if (hse_status_ret & HSE_STATUS_PRIMARY_SYSIMG) {
		printf("CHECK: SYS_IMG already loaded\n");
		return CMD_RET_SUCCESS;
	}

	/* find mem reserved for hse */
	hse_nodeoffset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
						       "nxp,s32cc-hse-rmem");
	if (hse_nodeoffset < 0) {
		printf("ERROR: hse_reserved node not found! ERRNO: %d\n", hse_nodeoffset);
		return hse_nodeoffset;
	}

	hse_resmem = fdt_get_base_address(gd->fdt_blob, hse_nodeoffset);
	if (hse_resmem < 0) {
		printf("ERROR: could not get base address of hse_reserved node!\n");
		return hse_resmem;
	}

	priv = (struct hse_private *)hse_resmem;
	memset((void *)priv, 0, sizeof(struct hse_private));

	/* read ivt */
	ret = hse_mmc_read((void *)&priv->ivt, HSE_IVT_BLK, 1);
	if (ret) {
		printf("ERROR: ivt read failed!\n");
		return ret;
	}

	ret = hse_enable_mus(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_format_key_store(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_generate_sys_img(priv, &hse_recv);
	if (ret)
		return ret;

	ret = hse_write_sys_img(priv, false);
	if (ret)
		return ret;

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(hse_keystore_format, 1, 0, do_hse_keystore_format,
	   "format the keystore",
	   "Format keystore for use in Linux kernel driver");

#endif /* CONFIG_SD_BOOT */
