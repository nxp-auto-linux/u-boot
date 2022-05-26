/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * HSE ABI for secure boot in u-boot
 *
 * Copyright 2020-2022 NXP
 */

#ifndef HSE_ABI_H
#define HSE_ABI_H

#include <common.h>
#include <mmc.h>
#include <uuid.h>

#define BYTES_TO_BITS(x)	((x) * 8u)

#define MODULUS_OFFSET  0x21u
#define MODULUS_SIZE    0x100u
#define EXPONENT_OFFSET 0x123u
#define EXPONENT_SIZE   0x3u

#define HSE_SRV_RSP_OK            0x55A5AA33ul
#define HSE_SRV_RSP_VERIFY_FAILED 0x55A5A164ul
#define HSE_SRV_RSP_INVALID_ADDR  0x55A5A26Aul
#define HSE_SRV_RSP_INVALID_PARAM 0x55A5A399ul
#define HSE_SRV_RSP_NOT_ALLOWED   0xAA55A21Cul
#define HSE_SRV_RSP_KEY_INVALID   0xA5AA52B4ul

#define HSE_SYS_IMG_MAX_SIZE 0xC000u
#define HSE_FIP_AUTH_LEN     0x100u
#define HSE_AUTH_TAG_OFFSET  0x200u

#define HSE_IVT_BLK      8

#define HSE_EXT_FLASH_SD   2u
#define HSE_EXT_FLASH_PAGE 512u

#define HSE_CHANNEL_ADMIN   0u
#define HSE_CHANNEL_GENERAL 1u

#define HSE_STATUS_INIT_OK        BIT(8)
#define HSE_STATUS_PRIMARY_SYSIMG BIT(14)
#define HSE_IVT_BOOTSEQ_BIT       BIT(3)

#define HSE_SRV_ID_SET_ATTR                 0x00000001ul
#define HSE_SRV_ID_PUBLISH_SYS_IMAGE        0x00000011ul
#define HSE_SRV_ID_FORMAT_KEY_CATALOGS      0x00000101ul
#define HSE_SRV_ID_IMPORT_KEY               0x00000104ul
#define HSE_SRV_ID_SMR_ENTRY_INSTALL        0x00000501ul
#define HSE_SRV_ID_CORE_RESET_ENTRY_INSTALL 0x00000503ul

#define HSE_ACCESS_MODE_ONE_PASS 0u

#define HSE_APP_CORE3 3u /* A53_0 */

#define HSE_PUBLISH_UPDATED_DATA_SET BIT(0)
#define HSE_PUBLISH_ALL_DATA_SETS    BIT(1)

#define HSE_SGT_OPTION_NONE 0u

#define HSE_SMR_CFG_FLAG_SD_FLASH     0x2u
#define HSE_SMR_CFG_FLAG_INSTALL_AUTH BIT(2)
#define HSE_SMR_ENTRY_1               BIT(1)

#define HSE_CR_SANCTION_KEEP_CORE_IN_RESET 0x7455u
#define HSE_CR_AUTO_START                  0x35A5u

#define HSE_MU_ACTIVATED      0xA5u
#define HSE_MU_DEACTIVATED    0x5Au
#define HSE_MU_CONFIG_ATTR_ID 20u

#define HSE_SIGN_RSASSA_PKCS1_V15 0x93u
#define HSE_HASH_ALGO_SHA_1       2u

#define HSE_INVALID_KEY_HANDLE              0xFFFFFFFFul
#define HSE_BOOT_KEY_HANDLE                 0x010700u
#define HSE_SMR_DECRYPT_KEY_HANDLE_NOT_USED 0ul

#define HSE_MU0_MASK    BIT(0)
#define HSE_MU1_MASK    BIT(1)
#define HSE_MU2_MASK    BIT(2)
#define HSE_MU3_MASK    BIT(3)
#define HSE_ALL_MU_MASK (HSE_MU0_MASK | HSE_MU1_MASK | \
			 HSE_MU2_MASK | HSE_MU3_MASK)

#define HSE_KF_USAGE_VERIFY BIT(3)

#define HSE_KEY_OWNER_ANY  0u
#define HSE_KEY_OWNER_CUST 1u
#define HSE_KEY_OWNER_OEM  2u

#define HSE_KEY_TYPE_AES           0x12u
#define HSE_KEY_TYPE_HMAC          0x20u
#define HSE_KEY_TYPE_SHARED_SECRET 0x30u
#define HSE_KEY_TYPE_ECC_PAIR      0x87u
#define HSE_KEY_TYPE_ECC_PUB       0x88u
#define HSE_KEY_TYPE_ECC_PUB_EXT   0x89u
#define HSE_KEY_TYPE_RSA_PAIR      0x97u
#define HSE_KEY_TYPE_RSA_PUB       0x98u
#define HSE_KEY_TYPE_RSA_PUB_EXT   0x99u

#define HSE_KEY128_BITS  128u
#define HSE_KEY256_BITS  256u
#define HSE_KEY512_BITS  512u
#define HSE_KEY521_BITS  521u
#define HSE_KEY638_BITS  638u
#define HSE_KEY1024_BITS 1024u
#define HSE_KEY2048_BITS 2048u
#define HSE_KEY4096_BITS 4096u

/* see include/mmc.h */
struct mmc *hse_init_mmc_device(int dev, bool force_init);
int hse_mmc_read(void *addr, u32 blk, u32 cnt);
int hse_mmc_write(void *addr, u32 blk, u32 cnt);

/**
 * struct fip_toc_header - FIP ToC header
 * @name: fip name/signature, fixed
 * @serial_number: fip serial number, fixed
 * @flags: flags associated with the data
 */
struct fip_toc_header {
	u32 name;
	u32 serial_number;
	u64 flags;
};

/**
 * struct fip_toc_entry - FIP ToC entry
 * @uuid: ToC entry unique ID
 * @offset: offset of entry in FIP
 * @size: size of entry
 * @flags: flags associated with the data
 */
struct fip_toc_entry {
	struct uuid uuid;
	u64 offset;
	u64 size;
	u64 flags;
};

/**
 * struct app_boot_hdr - app load, entry and size info
 * @header: start of application image
 * @ram_load: address at which to load app code
 * @ram_entry: address at which to jump execution
 * @code_len: length of subsequent code
 */
struct app_boot_hdr {
	u32 header;
	u32 ram_load;
	u32 ram_entry;
	u32 code_len;
};

/**
 * struct ivt - ivt held in flash
 * @header: header showing the start of IVT
 * @dcd_self_test: ptr to cfg data used for BIST
 * @dcd_self_test_backup: ptr to backup cfg data used for BIST
 * @dcd: ptr to DCD cfg data
 * @dcd_backup: ptr to backup DCD cfg data
 * @hse_fw: ptr to HSE-H firmware in flash
 * @hse_fw_backup: ptr to backup HSE-H fw in flash
 * @app_boot: ptr to application boot code in flash
 * @app_boot_backup: ptr to backup application boot code in flash
 * @boot_cfg: cfg data used to select boot configuration
 * @lc_cfg: cfg data used for advancing lifecycle
 * @sys_img: ptr to SYS-IMG file in flash
 * @sys_img_backup: ptr to SYS-IMG backup file in flash
 * @sys_img_ext_flash_type: cfg data to select flash type for ASB
 * @sys_img_flash_page_size: cfg data to select flash erasable page size
 * @app_bsb_ext_flash_type: cfg data to select flash type for BSB
 * @gmac: hash of data from IVT
 */
struct ivt {
	u32 ivt_header;
	u8 reserved1[4];
	u32 dcd_self_test;
	u32 dcd_self_test_backup;
	u32 dcd;
	u32 dcd_backup;
	u32 hse_fw;
	u32 hse_fw_backup;
	u32 app_boot;
	u32 app_boot_backup;
	u32 boot_cfg;
	u32 lc_cfg;
	u8 reserved2[4];
	u32 sys_img;
	u32 sys_img_backup;
	u32 sys_img_ext_flash_type;
	u32 sys_img_flash_page_size;
	u32 app_bsb_ext_flash_type;
	u8 reserved3[168];
	u32 gmac[4];
	/* pad to block size - 512 bytes */
	u8 reserved4[256];
} __packed;

/**
 * struct hse_key_group_cfg_entry - key group entry in key catalog
 * @mu_mask: mu instance for the key group
 * @group_owner: key group owner
 * @key_type: group key type
 * @num_key_slots: max number of keys in slot
 * @max_key_bit_len: max bit length of keys in group
 */
struct hse_key_group_cfg_entry {
	u8 mu_mask;
	u8 group_owner;
	u8 key_type;
	u8 num_key_slots;
	u16 max_key_bit_len;
	u8 reserved[2];
} __packed;

/**
 * struct hse_format_key_catalogs_srv - key catalog format service
 * @nvm_key_catalog_cfg_addr: ptr to NVM key catalog
 * @ram_key_catalog_cfg_addr: ptr to RAM key catalog
 */
struct hse_format_key_catalogs_srv {
	u64 nvm_key_catalog_cfg_addr;
	u64 ram_key_catalog_cfg_addr;
} __packed;

/**
 * struct hse_key_info - key properties
 * @key_flags: define key usage
 * @key_bit_len: for RSA, bit length of public modulus n
 * @key_counter: 28 bit counter to prevent rollback attacks on key
 * @smr_flags: which SMR must be checked before using key; 0 is unused
 * @key_type: key type
 * @pub_exponent_size: size in bytes of RSA public exponent e
 */
struct hse_key_info {
	u16 key_flags;
	u16 key_bit_len;
	u32 key_counter;
	u32 smr_flags;
	u8 key_type;
	u8 pub_exponent_size;
	u8 reserved[2];
} __packed;

/**
 * struct hse_import_key_srv - key import service
 * @key_handle: specified slot in which to add/update the key
 * @key_info_addr: ptr to struct hse_key_info
 * @key_addr[2]: ptr to key values
 *               [0] - RSA public modulus n (big-endian)
 *               [1] - RSA public exponent e (big-endian)
 * @key_len[2]: length in bytes for above key values
 * @cipher_key: decryption key handle, set to HSE_INVALID_KEY_HANDLE
 * @auth_key: auth key handle, set to HSE_INVALID_KEY_HANDLE
 */
struct hse_import_key_srv {
	u32 key_handle;
	u64 key_info_addr;
	u64 key_addr[3];
	u16 key_len[3];
	u8 reserved1[2];
	u32 cipher_key;
	u8 reserved2[48];
	u32 auth_key;
	u8 reserved3[48];
} __packed;

/**
 * struct hse_rsa_pkcs1v15_scheme - parameters for RSASSA_PKCS1_V15
 * @hash_algo: specify hash algorithm
 *             must not be HSE_HASH_ALGO_NULL or HSE_HASH_ALGO_MD5
 */
struct hse_rsa_pkcs1v15_scheme {
	u8 hash_algo;
	u8 reserved[3];
} __packed;

/**
 * struct hse_sign_scheme - parameters for signature generate/verify
 * @sign_scheme: signature scheme
 * @sch: RSASSA_PKCS1_V15 signature scheme parameters
 */
struct hse_sign_scheme {
	u8 sign_scheme;
	u8 reserved1[3];
	struct hse_rsa_pkcs1v15_scheme sch;
	u8 reserved2[8];
} __packed;

/**
 * struct hse_smr_entry - SMR entry
 * @smr_src: ptr to SMR load location in flash
 * @smr_dst_addr: ptr to write location after authentication
 * @smr_size: size in bytes of SMR to be loaded
 * @config_flags: config flags for SMR entry
 * @check_period: required by hse, must be 0
 * @key_handle: key from key catalog used to authenticate SMR
 * @sign_sch: auth scheme used to verify the SMR
 * @auth_tag: location of SMR signature in flash
 * @decrypt_key_handle: specifies parameters for smr decryption
 * @version_offset: unused, must be set to 0
 */
struct hse_smr_entry {
	u32 smr_src;
	u64 smr_dst_addr;
	u32 smr_size;
	u8 config_flags;
	u8 reserved1[3];
	u32 check_period;
	u32 key_handle;
	struct hse_sign_scheme sign_sch;
	u32 auth_tag;
	u8 reserved2[4];
	u32 decrypt_key_handle;
	u8 reserved3[12];
	u32 version_offset;
} __packed;

/**
 * struct hse_cr_entry - core reset entry
 * @core_id: core to be un-gated once SMR authentication is successful
 * @cr_sanction: sanction to apply if SMR authentication fails
 * @preboot_smr_map: smr entries which need to be verified before
 *                   booting to ungate core
 * @pass_reset: first instruction to jump to if verification is successful
 * @start_option: specifies if the core is automatically released from
 *                reset or not
 */
struct hse_cr_entry {
	u8 core_id;
	u8 reserved1[1];
	u16 cr_sanction;
	u32 preboot_smr_map;
	u32 pass_reset;
	u8 reserved2[12];
	u16 start_option;
	u8 reserved3[6];
} __packed;

/**
 * struct hse_smr_install_srv - SMR installation service
 * @access_mode: specify access mode
 * @entry_index: specify SMR entry index to install
 * @smr_entry_addr: ptr to struct hse_smr_entry
 * @smr_data_addr: ptr to SMR data to install
 * @smr_data_len: length of SMR data to install
 * @smr_auth_tag_addr: ptr to SMR data authentication tag to verify
 * @smr_auth_tag_len: length of SMR data authentication tag
 */
struct hse_smr_install_srv {
	u8 access_mode;
	u8 entry_index;
	u8 reserved1[2];
	u64 smr_entry_addr;
	u64 smr_data_addr;
	u32 smr_data_len;
	u64 smr_auth_tag_addr;
	u8 reserved2[8];
	u16 smr_auth_tag_len;
	u8 reserved3[18];
} __packed;

/**
 * struct hse_cr_install_srv - Core Reset installation service
 * @cr_entry_index: specify CR entry index to install
 * @cr_entry_addr: ptr to struct hse_cr_entry
 */
struct hse_cr_install_srv {
	u8 cr_entry_index;
	u8 reserved[3];
	u64 cr_entry_addr;
} __packed;

/**
 * struct hse_publish_sys_img_srv - SYS-IMG publish service
 * @publish_options: specify which data sets to publish in SYS-IMG
 * @publish_offset_addr: ptr to store SYS-IMG offset in flash, unused
 * @buff_length_addr: as input, specify length of output buffer
 *                    as output, number of bytes written by HSE into buffer
 * @buff_addr: ptr to buffer in which to store SYS-IMG
 */
struct hse_publish_sys_img_srv {
	u8 publish_options;
	u8 reserved1[3];
	u64 publish_offset_addr;
	u64 buff_length_addr;
	u64 buff_addr;
} __packed;

/**
 * struct hse_mu_instance_config - configure a MU interface
 * @mu_config: specify MU state; MU0 cannot be deactivated
 * @xrdc_domain_id: domain id to acces host interface memory chunk
 * @shared_mem_chunk_size: which chunk of host interface memory to reserve
 */
struct hse_mu_instance_config {
	u8 mu_config;
	u8 xrdc_domain_id;
	u16 shared_mem_chunk_size;
	u8 reserved[60];
} __packed;

/**
 * struct hse_mu_config - configure all MU interfaces
 * @mu_instances: contains configuration for all MU interfaces
 */
struct hse_mu_config {
	struct hse_mu_instance_config mu_instances[4];
} __packed;

/**
 * struct hse_getset_attr_srv - get attribute
 * @attr_id: attribute ID
 * @attr_len: attribute length, in bytes
 * @attr: DMA address of the attribute
 */
struct hse_getset_attr_srv {
	u16 attr_id;
	u8 reserved[2];
	u32 attr_len;
	u64 p_attr;
} __packed;

/**
 * struct hse_srv_desc - HSE service descriptor
 * @srv_id: specify service for HSE to execute
 * @union: specify service parameters
 */
struct hse_srv_desc {
	u32 srv_id;
	u8 reserved[4];
	union {
		struct hse_getset_attr_srv getset_attr_req;
		struct hse_format_key_catalogs_srv format_catalogs_req;
		struct hse_import_key_srv import_key_req;
		struct hse_cr_install_srv cr_install_req;
		struct hse_smr_install_srv smr_install_req;
		struct hse_publish_sys_img_srv publish_sys_img_req;
	};
} __packed;

/**
 * struct hse_private - hse required data, stored at start of ddr
 * @ivt: ivt stored for modifications required for secboot
 * @app_boot_hdr: application load, entry and size data
 * @srv_desc: service descriptor
 * @key_info: key data for insertion into catalog
 * @cr_entry: core reset entry data
 * @smr_entry: secure memory region data
 * @mu_config: mu configuration data
 * @rsa_pubkey: contents of public key file, in DER format
 * @rsa_modulus: rsa public key modulus, extracted from file
 * @rsa_exponent: rsa public key exponent, extracted from file
 * @fip_sign: fip signature in ddr
 * @sys_img: hse-generated system image
 * @sys_img_len: system image length
 * @publish_offset: offset at which to write the updated sys_img
 * @nvm_catalog: nvm key catalog
 * @ram_catalog: ram key catalog
 */
struct hse_private {
	struct ivt ivt;
	struct app_boot_hdr app_boot_hdr;
	struct hse_srv_desc srv_desc;
	struct hse_key_info key_info;
	struct hse_cr_entry cr_entry;
	struct hse_smr_entry smr_entry;
	struct hse_mu_config mu_config;
	u8 rsa_pubkey[512];
	u8 rsa_modulus[256];
	u8 rsa_exponent[3];
	u8 reserved;
	u8 fip_signature[256];
	u8 sys_img[HSE_SYS_IMG_MAX_SIZE];
	u32 sys_img_len;
	u32 publish_offset;
	struct hse_key_group_cfg_entry nvm_catalog[20];
	struct hse_key_group_cfg_entry ram_catalog[11];
};

#endif /* HSE_ABI_H */
