// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2026 NXP
 *
 * Request the FSS firmware to physically initialize a high DDR region on
 * the S32N79 via the NXP Misc SCMI vendor protocol (INIT_DDR_PARTIAL).
 *
 * The firmware services one request at a time and bounds the per-call
 * region size, so a request is split into 1 GiB chunks.
 *
 * This only asks the FSS to bring the memory up; it does not extend
 * U-Boot's own DRAM banks or patch the /memory node of the OS DTB.
 */

#include <command.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>
#include <s32n79-scmi-misc.h>
#include <stdio.h>
#include <vsprintf.h>
#include <dm/uclass.h>
#include <linux/sizes.h>
#include <linux/types.h>

#define S32N_DDR_CHUNK_SIZE	SZ_1G
#define S32N_DDR_CHUNK_ALIGN	SZ_128M

/*
 * Address ranges owned by each SMMS DDR controller instance on the
 * S32N79. Mirrors the same map used by TF-A and the downstream Linux
 * mem-hotplug-scmi driver; keep them in sync.
 */
static const struct {
	u64 start;
	u64 end;
	u32 instance;
} smms_ranges[] = {
	{ 0x80000000ULL,	0xbfffffffULL,		0 }, /*  1 GiB */
	{ 0xc0000000ULL,	0xffffffffULL,		1 }, /*  1 GiB */
	{ 0x8800000000ULL,	0x8bffffffffULL,	0 }, /* 16 GiB */
	{ 0xc000000000ULL,	0xc3ffffffffULL,	1 }, /* 16 GiB */
};

/*
 * Find the SMMS instance that owns the [addr, addr + size - 1] range.
 * The full request must fall within a single instance's range; spanning
 * two instances or landing outside any known range returns -EINVAL.
 */
static int addr_to_smms_instance(u64 addr, u64 size, u32 *instance)
{
	u64 end = addr + size - 1;
	size_t i;

	if (!size || end < addr)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(smms_ranges); i++) {
		if (addr >= smms_ranges[i].start &&
		    end  <= smms_ranges[i].end) {
			*instance = smms_ranges[i].instance;
			return 0;
		}
	}

	return -EINVAL;
}

static int do_ddr_init(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct nxp_misc_ddr_init_req req;
	struct udevice *dev;
	u64 addr, size, left, chunk;
	u32 instance;
	char *endp;
	int ret;

	if (argc != 3)
		return CMD_RET_USAGE;

	addr = simple_strtoull(argv[1], NULL, 16);
	size = ustrtoull(argv[2], &endp, 0);

	if (!size) {
		printf("ddr_init: size must be non-zero\n");
		return CMD_RET_USAGE;
	}

	if (!IS_ALIGNED(addr, S32N_DDR_CHUNK_ALIGN) ||
	    !IS_ALIGNED(size, S32N_DDR_CHUNK_ALIGN)) {
		printf("ddr_init: <addr> and <size> must both be 128 MiB-aligned\n");
		return CMD_RET_USAGE;
	}

	if (check_uptr_overflow(addr, size)) {
		printf("ddr_init: 0x%llx + 0x%llx overflows\n", addr, size);
		return CMD_RET_USAGE;
	}

	if (addr_to_smms_instance(addr, size, &instance)) {
		printf("ddr_init: [0x%llx, 0x%llx] is not fully owned by a single SMMS instance\n",
		       addr, addr + size - 1);
		return CMD_RET_FAILURE;
	}

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(scmi_misc), &dev);
	if (ret) {
		printf("ddr_init: NXP Misc SCMI device not found (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	left = size;
	while (left) {
		chunk = min_t(u64, left, S32N_DDR_CHUNK_SIZE);

		printf("ddr_init: @0x%llx size=0x%llx inst=%u ...",
		       addr, chunk, instance);

		req.addr = addr;
		req.size = chunk;
		req.instance_id = instance;

		ret = misc_call(dev, NXP_MISC_CMD_INIT_DDR_PARTIAL,
				&req, sizeof(req), NULL, 0);
		if (ret) {
			printf(" failed (%d)\n", ret);
			return CMD_RET_FAILURE;
		}

		printf(" ok\n");

		addr += chunk;
		left -= chunk;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ddr_init, 3, 0, do_ddr_init,
	   "initialize a DDR region via NXP Misc SCMI (INIT_DDR_PARTIAL)",
	   "<addr> <size>\n"
	   "    - initialize the DDR region at <addr> of <size> bytes via FSS.\n"
	   "      <addr> and <size> must be 128 MiB-aligned. <addr> is hex;\n"
	   "      <size> is hex, decimal, or carries a K/M/G[i][B] suffix.\n"
	   "      Examples:\n"
	   "          ddr_init 0x8800000000 0x40000000\n"
	   "          ddr_init 0x8800000000 1G\n"
	   "          ddr_init 0xc000000000 1024MB");
