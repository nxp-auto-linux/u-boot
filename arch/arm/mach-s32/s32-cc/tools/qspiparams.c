// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2022 NXP */

#include <s32cc_image_params.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define xstr(s) str(s)
#define str(s) #s
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

enum qspi_memory {
	MX25UW51245G,
	MT35XU512ABA,
	INVALID_QSPI_MEM,
};

const char *mem_names[] = {
	[MX25UW51245G] = str(MX25UW51245G),
	[MT35XU512ABA] = str(MT35XU512ABA),
};

static void show_usage(char *arg)
{
	fprintf(stderr, "This tool is used to generate QSPI parameters ");
	fprintf(stderr, "binary for S32CC platforms.\n");
	fprintf(stderr,
		"The resulted binary will be added to IVT by mkimage tool.\n\n");
	fprintf(stderr, "Usage: %s -m MEMORY -o FILE\n\n", arg);
	fprintf(stderr, "\t-m\t\tSelects the QSPI memory. ");
	fprintf(stderr,
		"Available memories:\n\t\t\tMX25UW51245G and MT35XU512ABA\n");
}

static enum qspi_memory get_memory(const char *mem_name)
{
	size_t i;

	for (i = 0u; i < ARRAY_SIZE(mem_names); i++) {
		if (!strcmp(mem_name, mem_names[i]))
			return (enum qspi_memory)i;
	}

	return INVALID_QSPI_MEM;
}

int main(int argc, char **argv)
{
	int opt;
	enum qspi_memory memory = INVALID_QSPI_MEM;
	struct qspi_params *params;
	char *output = NULL;
	FILE *ofile;

	while ((opt = getopt(argc, argv, "hm:o:")) > 0) {
		switch (opt) {
		case 'm':
			memory = get_memory(optarg);
			if (memory == INVALID_QSPI_MEM) {
				fprintf(stderr, "Invalid memory: %s\n",
					optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'o':
			output = optarg;
			break;
		default:
			show_usage(argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	switch (memory) {
	case MX25UW51245G:
		params = get_macronix_qspi_conf();
		break;
	case MT35XU512ABA:
		params = get_micron_qspi_conf();
		break;
	default:
		fprintf(stderr,
			"-m argument is incorrect or is missing\n");
		exit(EXIT_FAILURE);
	}

	if (!output) {
		fprintf(stderr, "-o argument is mandatory\n");
		exit(EXIT_FAILURE);
	}

	ofile = fopen(output, "wb");
	if (!ofile) {
		perror("Output file error");
		exit(EXIT_FAILURE);
	}

	fwrite(params, sizeof(*params), 1, ofile);
	fclose(ofile);

	return EXIT_SUCCESS;
}
