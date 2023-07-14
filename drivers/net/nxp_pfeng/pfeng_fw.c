// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet accelerator FW support
 *
 * Copyright 2023 NXP
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <elf.h>
#include <env.h>
#include <fs.h>
#include <spi_flash.h>

#include "pfeng.h"

#define PFENG_ENV_VAR_FW_SOURCE	"pfengfw"

#define PFENG_FW_MAX_ELF_SIZE 0xffffU
#define PFENG_FW_MAX_QSPI_ADDR 0xf0000000U

#if CONFIG_IS_ENABLED(NXP_PFENG_FW_LOC_SDCARD)
static int pfeng_fw_check_sd_fw_size(loff_t length)
{
	if (length < 0 || length > (loff_t)PFENG_FW_MAX_ELF_SIZE)
		return -EINVAL;

	return 0;
}

static int pfeng_fw_load(char *fname, char *iface, char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret;
	void *addr = NULL;
	loff_t length = 0, read = 0;

	log_debug("Loading PFEng fw from %s@%s:%s:%d\n", iface, part, fname, ftype);

	ret = fs_set_blk_dev(iface, part, ftype);
	if (ret < 0)
		goto exit;

	ret = fs_size(fname, &length);
	if (ret < 0)
		goto exit;

	ret = pfeng_fw_check_sd_fw_size(length);
	if (ret)
		goto exit;

	addr = valloc(length);
	if (!addr) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = fs_set_blk_dev(iface, part, ftype);
	if (ret < 0)
		goto exit;

	ret = fs_read(fname, (ulong)addr, 0, length, &read);
	if (ret < 0)
		goto exit;

	if (length != read) {
		ret = -EIO;
		goto exit;
	}

	priv->fw_class_data = addr;
	priv->fw_class_size = length;

	log_debug("Found PFEng firmware: %s@%s:%s size %lld\n",
		  iface, part, fname, read);

exit:
	if (ret) {
		if (addr)
			free(addr);

		log_err("PFEng firmware file '%s@%s:%s' loading failed: %d\n",
			iface, part, fname, ret);
		priv->fw_class_data = NULL;
		priv->fw_class_size = 0;
	}
	return ret;
}
#endif /* NXP_PFENG_FW_LOC_SDCARD */

#if CONFIG_IS_ENABLED(NXP_PFENG_FW_LOC_QSPI)
static int pfeng_fw_check_elf_size(size_t elf_size)
{
	if (elf_size > PFENG_FW_MAX_ELF_SIZE) {
		log_err("Too large ELF file, size: %zu\n", elf_size);
		return -EFBIG;
	}

	return 0;
}

static int pfeng_fw_check_qspi_addr(unsigned long qspi_addr)
{
	if (qspi_addr > PFENG_FW_MAX_QSPI_ADDR) {
		log_err("Too large QSPI address: %lu\n", qspi_addr);
		return -EINVAL;
	}

	return 0;
}

static int setup_flash_device(struct spi_flash **flash)
{
	struct udevice *new = NULL;
	int	ret;

	/* Use default QSPI device. Speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE,
				     &new);
	if (ret) {
		log_err("spi_flash_probe_bus_cs() failed\n");
		return ret;
	}

	*flash = dev_get_uclass_priv(new);
	return 0;
}

static void swab_elf_hdr(Elf32_Ehdr *elf_hdr)
{
	elf_hdr->e_type = be16_to_cpup(&elf_hdr->e_type);
	elf_hdr->e_machine = be16_to_cpup(&elf_hdr->e_machine);
	elf_hdr->e_version = be32_to_cpup(&elf_hdr->e_version);
	elf_hdr->e_entry = be32_to_cpup(&elf_hdr->e_entry);
	elf_hdr->e_phoff = be32_to_cpup(&elf_hdr->e_phoff);
	elf_hdr->e_shoff = be32_to_cpup(&elf_hdr->e_shoff);
	elf_hdr->e_flags = be32_to_cpup(&elf_hdr->e_flags);
	elf_hdr->e_ehsize = be16_to_cpup(&elf_hdr->e_ehsize);
	elf_hdr->e_phentsize = be16_to_cpup(&elf_hdr->e_phentsize);
	elf_hdr->e_phnum = be16_to_cpup(&elf_hdr->e_phnum);
	elf_hdr->e_shentsize = be16_to_cpup(&elf_hdr->e_shentsize);
	elf_hdr->e_shnum = be16_to_cpup(&elf_hdr->e_shnum);
	elf_hdr->e_shstrndx = be16_to_cpup(&elf_hdr->e_shstrndx);
}

static int load_pfe_fw(struct spi_flash *flash, u32 qspi_addr,
		       void **fw_buffer, size_t *elf_size)
{
	int ret;
	Elf32_Ehdr elf_hdr = {0};

	ret = spi_flash_read(flash, qspi_addr, sizeof(elf_hdr), &elf_hdr);
	if (ret) {
		log_err("Failed to read PFE FW Header from QSPI\n");
		return -EIO;
	}
	swab_elf_hdr(&elf_hdr);

	*elf_size = elf_hdr.e_shoff +
		    ((Elf32_Word)elf_hdr.e_shentsize * (Elf32_Word)elf_hdr.e_shnum);

	ret = pfeng_fw_check_elf_size(*elf_size);
	if (ret)
		goto exit;

	if (!valid_elf_image((unsigned long)&elf_hdr)) {
		log_err("PFEng firmware is not valid\n");
		ret = -EINVAL;
		goto exit;
	}

	*fw_buffer = valloc(*elf_size);
	if (!*fw_buffer) {
		log_err("Failed to allocate 0x%lx bytes for PFE FW\n",
			*elf_size);
		ret = -ENOMEM;
		goto exit;
	}

	ret = spi_flash_read(flash, qspi_addr, *elf_size, *fw_buffer);
	if (ret) {
		log_err("Failed to load PFE FW from QSPI\n");
		goto exit;
	}

exit:
	if (ret && *fw_buffer)
		free(*fw_buffer);

	return ret;
}

static int pfeng_fw_load(char *fname, char *iface, const char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret = 0;
	void *fw_buffer = NULL;
	struct spi_flash *flash;
	u32 qspi_addr;
	size_t elf_size;
	unsigned long val;

	val = simple_strtoul(part, NULL, 16);
	ret = pfeng_fw_check_qspi_addr(val);
	if (ret)
		goto exit;

	qspi_addr = val;
	ret = setup_flash_device(&flash);
	if (ret)
		goto exit;

	ret = load_pfe_fw(flash, qspi_addr, &fw_buffer, &elf_size);
	if (ret)
		goto exit;

	ret = pfeng_fw_check_elf_size(elf_size);
	if (ret)
		goto exit;

	priv->fw_class_data = fw_buffer;
	priv->fw_class_size = (u32)elf_size;

	if (!priv->fw_class_data) {
		log_err("PFE firmware not found at qspi@%p\n",
			priv->fw_class_data);
		ret = -EINVAL;
		goto exit;
	}

	log_debug("DEB: Found PFEng firmware: qspi@%p\n", priv->fw_class_data);

exit:

	if (ret) {
		if (fw_buffer)
			free(fw_buffer);

		log_err("PFE firmware file '%s@%s:%s' loading failed: %d\n",
			iface, part, fname, ret);
		priv->fw_class_data = NULL;
		priv->fw_class_size = 0;
	}

	return ret;
}
#endif /* NXP_PFENG_FW_LOC_QSPI */

int pfeng_fw_set_from_env_and_load(struct pfeng_priv *priv)
{
	char *env_fw, *dup_env_fw = NULL;
	char *fw_int = NULL, *fw_name = NULL, *fw_part = NULL;
	int fw_type = FS_TYPE_ANY;
	char *p;
	int ret;

	/* Parse fw destination from environment */
	env_fw = env_get(PFENG_ENV_VAR_FW_SOURCE);
	if (env_fw && strlen(env_fw)) {
		/* We want to modify content, so duplicate */
		dup_env_fw = strdup(env_fw);
		if (!dup_env_fw) {
			ret = -ENOMEM;
			goto exit;
		}

		env_fw = dup_env_fw;
		/* check for interface (mmc/usb/...) */
		p = strchr(env_fw, '@');
		if (p) {
			fw_int = env_fw;
			*p = '\0';
			env_fw = ++p;
		}

		/* check for dev@part:fwname */
		p = strrchr(env_fw, ':');
		if (p) {
			*p = '\0';
			fw_part = env_fw;
			fw_name = ++p;
		} else {
#if CONFIG_IS_ENABLED(NXP_PFENG_FW_LOC_QSPI)
			fw_part = env_fw;
#endif
		}
	}

	if (!fw_name && !fw_part) {
		/* use the default config variant */
#if CONFIG_IS_ENABLED(NXP_PFENG_FW_LOC_SDCARD)
		fw_name = CONFIG_NXP_PFENG_FW_NAME;
#endif
		fw_part = CONFIG_NXP_PFENG_FW_PART;
	}

#if CONFIG_IS_ENABLED(NXP_PFENG_FW_LOC_SDCARD)
	if (!fw_int)
		fw_int = "mmc";
#endif

	/* FW load */
	ret = pfeng_fw_load(fw_name, fw_int, fw_part, fw_type, priv);
exit:
	free(dup_env_fw);
	return ret;
}
