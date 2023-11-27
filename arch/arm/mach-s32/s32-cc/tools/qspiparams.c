// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2022-2023 NXP */

#include <errno.h>
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

static const char * const mem_names[] = {
	[MX25UW51245G] = str(MX25UW51245G),
	[MT35XU512ABA] = str(MT35XU512ABA),
};

static const unsigned int allowed_freqs[] = {100, 133, 166, 200};

static void show_usage(char *arg)
{
	fprintf(stderr, "This tool is used to generate QSPI parameters ");
	fprintf(stderr, "binary for S32CC platforms.\n");
	fprintf(stderr,
		"The resulted binary will be added to IVT by mkimage tool.\n\n");
	fprintf(stderr, "Usage: %s -m MEMORY -f FREQUENCY -o FILE\n\n", arg);
	fprintf(stderr, "\t-m\t\tSelects the QSPI memory. ");
	fprintf(stderr,
		"Available memories:\n\t\t\tMX25UW51245G and MT35XU512ABA\n");
	fprintf(stderr,
		"\t-f\t\t(Optional) Selects the frequency for MX25UW51245G\n");
}

static void show_available_qspi_freq(void)
{
	size_t i;

	fprintf(stderr, "Available QSPI frequencies are:\n");
	for (i = 0u; i < ARRAY_SIZE(allowed_freqs); i++)
		fprintf(stderr, "%u MHz\n", allowed_freqs[i]);
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

static int check_freq(unsigned int freq)
{
	size_t i;

	for (i = 0u; i < ARRAY_SIZE(allowed_freqs); i++)
		if (freq == allowed_freqs[i])
			return 0;

	return -EINVAL;
}

int main(int argc, char **argv)
{
	int opt;
	enum qspi_memory memory = INVALID_QSPI_MEM;
	unsigned int freq = 0;
	struct qspi_params *params;
	char *output = NULL;
	FILE *ofile;
	errno = 0;

	while ((opt = getopt(argc, argv, "hm:o:f:")) > 0) {
		switch (opt) {
		case 'm':
			memory = get_memory(optarg);
			if (memory == INVALID_QSPI_MEM) {
				fprintf(stderr, "Invalid memory: %s\n",
					optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case 'f':
			freq = strtoul(optarg, NULL, 0);
			if (errno || check_freq(freq)) {
				fprintf(stderr, "Invalid QSPI frequency: %s MHz\n",
					optarg);
				show_available_qspi_freq();
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
		if (freq)
			params->sflash_clk_freq = freq;
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
