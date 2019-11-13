/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019 NXP */

#include <image.h>
#include "imagetool.h"
#include "s32gen1image.h"

#define UNSPECIFIED	-1

#ifdef CONFIG_FLASH_BOOT
#define S32G2XX_COMMAND_SEQ_FILL_OFF 20
#endif

static struct dcd_command *last_command;

static table_entry_t dcd_commands[] = {
	{WRITE_DATA,			"WRITE_DATA",			"",},
	{WRITE_SET_BITMASK,		"WRITE_SET_BITMASK",		"",},
	{WRITE_CLEAR_BITMASK,		"WRITE_CLEAR_BITMASK",		"",},
	{CHECK_BITS_ARE_SET,		"CHECK_BITS_ARE_SET",		"",},
	{CHECK_BITS_ARE_CLEAR,		"CHECK_BITS_ARE_CLEAR",		"",},
	{CHECK_ANY_BIT_IS_SET,		"CHECK_ANY_BIT_IS_SET",		"",},
	{CHECK_ANY_BIT_IS_CLEAR,	"CHECK_ANY_BIT_IS_CLEAR",	"",},
	{NOP,				"NOP",				"",},
	{INVALID,			"",				"",},
};

static void s32gen1_add_dcd_command_header(__u8 tag, __u8 params)
{
	size_t length = be16_to_cpu(last_command->length);

	if (tag == DCD_WRITE_COMMAND_TAG &&
	    last_command->tag == tag &&
	    last_command->params == params)
		return;

	last_command = (struct dcd_command*)((__u8*)last_command + length);

	last_command->tag = tag;
	last_command->length = cpu_to_be16(4);
	last_command->params = params;
}

static void s32gen1_add_dcd_command(int command_type, int access_width,
				    int register_address, int register_data,
				    int count,
				    struct program_image *program_image)
{
	size_t length = 0;
	unsigned int offset;
	__u8 tag = 0;
	__u8 params = 0;
	size_t dcd_length;

	switch (command_type) {
	case WRITE_DATA:
		tag = DCD_WRITE_COMMAND_TAG;
		params = access_width;
		break;
	case WRITE_SET_BITMASK:
		tag = DCD_WRITE_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case WRITE_CLEAR_BITMASK:
		tag = DCD_WRITE_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 access_width;
		break;
	case CHECK_BITS_ARE_SET:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case CHECK_BITS_ARE_CLEAR:
		tag = DCD_CHECK_COMMAND_TAG;
		params = access_width;
		break;
	case CHECK_ANY_BIT_IS_SET:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 DCD_COMMAND_PARAMS_DATA_SET	|
			 access_width;
		break;
	case CHECK_ANY_BIT_IS_CLEAR:
		tag = DCD_CHECK_COMMAND_TAG;
		params = DCD_COMMAND_PARAMS_DATA_MASK	|
			 access_width;
		break;
	case NOP:
		tag = DCD_NOP_COMMAND_TAG;
		break;
	}

	s32gen1_add_dcd_command_header(tag, params);

	switch (tag) {
	case DCD_WRITE_COMMAND_TAG:
		offset = (be16_to_cpu(last_command->length) - 4) / 8;
		last_command->s.write[offset].addr =
						cpu_to_le32(register_address);
		last_command->s.write[offset].data =
						cpu_to_le32(register_data);
		length = be16_to_cpu(last_command->length) + 8;
		last_command->length = cpu_to_be16(length);
		break;

	case DCD_CHECK_COMMAND_TAG:
		last_command->s.check.address = cpu_to_le32(register_address);
		last_command->s.check.mask = cpu_to_le32(register_data);
		length = be16_to_cpu(last_command->length) + 8;

		if (count != UNSPECIFIED) {
			last_command->s.check.count = count;
			length = length + 4;
		}

		last_command->length = cpu_to_be16(length);
		break;

	case DCD_NOP_COMMAND_TAG:
		break;
	}

	dcd_length = (__u8*)last_command +
		     be16_to_cpu(last_command->length) -
		     (__u8*)&program_image->dcd;

	if (dcd_length > DCD_MAXIMUM_SIZE) {
		fprintf(stderr, "DCD length of %zu exceeds the maximum of %d\n",
			dcd_length, DCD_MAXIMUM_SIZE);
		exit(EXIT_FAILURE);
	}

	program_image->dcd.length = cpu_to_be16(dcd_length);
}

static void s32gen1_parse_configuration_file(const char *filename,
					     struct program_image *program_image)
{
	FILE *f = NULL;
	char *line = NULL;
	char *token = NULL;
	size_t line_length;

	int command_type;
	int access_width;
	int register_address;
	int register_data;
	int count;

	f = fopen(filename, "r");
	if (f == NULL) {
		fprintf(stderr, "Can't open file %s: %s\n", filename,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	while ((getline(&line, &line_length, f)) > 0) {

		command_type = INVALID;
		access_width = 0;
		register_address = 0;
		register_data = 0;
		count = UNSPECIFIED;

		token = strtok(line, " \r\n");
		if (token != NULL)
			command_type = get_table_entry_id(dcd_commands, "",
							  token);
		token = strtok(NULL, " \r\n");
		if (token != NULL)
			access_width = strtoul(token, NULL, 0);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			register_address = strtoul(token, NULL, 16);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			register_data = strtoul(token, NULL, 16);

		token = strtok(NULL, " \r\n");
		if (token != NULL)
			count = strtoul(token, NULL, 0);


		if (command_type != INVALID)
			s32gen1_add_dcd_command(command_type,
						access_width,
						register_address,
						register_data,
						count,
						program_image);
	}

	fclose(f);
}

static void s32gen1_print_header(const void *header)
{
	return;
}

#ifndef CONFIG_FLASH_BOOT
static void enforce_reserved_range(void *image_start, int image_length,
				   void *reserved_start, void *reserved_end)
{
	void *image_end = (void*)((__u8*)image_start + image_length);

	if (image_start < reserved_end && image_end > reserved_start) {
		fprintf(stderr, "Loading data of size 0x%x at 0x%p forbidden.",
			image_length, image_end);
		fprintf(stderr, " Range 0x%p --- 0x%p is reserved!\n",
			reserved_start, reserved_end);
		exit(EXIT_FAILURE);
	}
}

#else
static struct qspi_params s32g2xx_qspi_conf = {
	.header   = 0x5a5a5a5a,
	.mcr      = 0x010f004c,
	.flshcr   = 0x00000303,
	.bufgencr = 0x00000000,
	.dllcr    = 0x01200006,
	.paritycr = 0x00000000,
	.sfacr    = 0x00000800,
	.smpr     = 0x00000020,
	.dlcr     = 0x40ff40ff,
	.sflash_1_size = 0x20000000,
	.sflash_2_size = 0x20000000,
	.dlpr = 0xaa553443,
	.sfar = 0x00000000,
	.ipcr = 0x00000000,
	.tbdr = 0x00000000,
	.dll_bypass_en   = 0x01,
	.dll_slv_upd_en  = 0x00,
	.dll_auto_upd_en = 0x00,
	.ipcr_trigger_en = 0x00,
	.sflash_clk_freq = 0x85,
	.reserved = {0x00, 0x00, 0x00},
	.command_seq = {0xec, 0x07, 0x13, 0x07,
			0x20, 0x0b, 0x14, 0x0f,
			0x01, 0x1f, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00},
	.flash_write_data = {0x06, 0x00, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0x00,
			     0x00, 0x00, 0x00, 0x00,
			     0x72, 0x00, 0x80, 0x81,
			     0x00, 0x00, 0x00, 0x00,
			     0x01, 0x00, 0x00, 0x00,}
};

static void s32g2xx_set_qspi_params(struct qspi_params *qspi_params)
{
	memcpy(qspi_params, &s32g2xx_qspi_conf, sizeof(*qspi_params));
	memset(&qspi_params->command_seq[S32G2XX_COMMAND_SEQ_FILL_OFF], 0xff,
	       sizeof(qspi_params->command_seq) - S32G2XX_COMMAND_SEQ_FILL_OFF);
}
#endif

static void s32gen1_set_header(void *header, struct stat *sbuf, int unused,
			       struct image_tool_params *tool_params)
{
	struct program_image *program_image = (struct program_image*)header;
	size_t code_length;

	if (!program_image) {
		fprintf(stderr, "Pointer to 'struct program_image' is NULL!\n");
		exit(EXIT_FAILURE);
	}

	last_command = (struct dcd_command*)&program_image->dcd.commands[0];
	s32gen1_parse_configuration_file(tool_params->imagename,
					 program_image);

	program_image->ivt.tag = IVT_TAG;
	program_image->ivt.length = cpu_to_be16(sizeof(struct ivt));
	program_image->ivt.version = IVT_VERSION;

	program_image->ivt.dcd_pointer = offsetof(struct program_image, dcd);
	program_image->ivt.boot_configuration_word = BCW_BOOT_TARGET_A53_0;
	program_image->ivt.application_boot_code_pointer =
						offsetof(struct program_image,
							 application_boot_code);

	program_image->dcd.tag = DCD_TAG;
	program_image->dcd.version = DCD_VERSION;

	program_image->application_boot_code.tag = APPLICATION_BOOT_CODE_TAG;
	program_image->application_boot_code.version =
						APPLICATION_BOOT_CODE_VERSION;
	program_image->application_boot_code.ram_start_pointer =
						CONFIG_SYS_TEXT_BASE;
	program_image->application_boot_code.ram_entry_pointer =
						CONFIG_SYS_TEXT_BASE;

	code_length = sbuf->st_size
			- offsetof(struct program_image, application_boot_code)
			- offsetof(struct application_boot_code, code);

	if (code_length % 0x40) {
		code_length &= ~0x3f;
		code_length += 0x40;
	}
	program_image->application_boot_code.code_length = code_length;

#ifndef CONFIG_FLASH_BOOT
	enforce_reserved_range((void*)(__u64)
			       program_image->application_boot_code.ram_start_pointer,
			       program_image->application_boot_code.code_length,
			       (void*)SRAM_RESERVED_0_START,
			       (void*)SRAM_RESERVED_0_END);

	enforce_reserved_range((void*)(__u64)
			       program_image->application_boot_code.ram_start_pointer,
			       program_image->application_boot_code.code_length,
			       (void*)SRAM_RESERVED_1_START,
			       (void*)SRAM_RESERVED_1_END);
#else
	s32g2xx_set_qspi_params(&program_image->qspi_params);
#endif

	return;
}

static int s32gen1_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_S32GEN1IMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int s32gen1_vrec_header(struct image_tool_params *tool_params,
			       struct image_type_params *type_params)
{
	struct program_image *program_image;

	program_image = malloc(sizeof(struct program_image));
	if (program_image == NULL) {
		perror("Call to malloc() failed");
		exit(EXIT_FAILURE);
	}

	type_params->header_size = sizeof(struct program_image) -
			(512 - offsetof(struct application_boot_code, code));
	type_params->hdr = (void*)program_image;

	return 0;
}

U_BOOT_IMAGE_TYPE(
	s32gen1_image,
	"NXP S32GEN1 Boot Image",
	0,
	NULL,
	NULL,
	NULL,
	s32gen1_print_header,
	s32gen1_set_header,
	NULL,
	s32gen1_check_image_type,
	NULL,
	s32gen1_vrec_header
);
