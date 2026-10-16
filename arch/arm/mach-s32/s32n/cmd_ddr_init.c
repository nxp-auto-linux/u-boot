// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2026 NXP
 *
 * Ask the FSS firmware to initialize high DDR regions on S32N7 via the
 * NXP DDR SCMI vendor protocol, and record the DRAM banks in the /memory node
 * of the OS's DT.
 *
 * Only the synchronous DDR_PARTIAL_DRAM_INIT is used. Its reply should mean
 * the work is done, but FSS sends it as soon as the request is queued. As a
 * workaround, the command polls GET_LAST_INIT_REQUEST_STATUS until the
 * region is initialized.
 */

#include <common.h>
#include <command.h>
#include <console.h>
#include <dm.h>
#include <errno.h>
#include <misc.h>
#include <s32n79-scmi-ddr.h>
#include <time.h>
#include <vsprintf.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include <linux/types.h>

DECLARE_GLOBAL_DATA_PTR;

#define S32N_DDR_REQ_ALIGN		SZ_128M
#define S32N_DDR_BUSY_TIMEOUT_MS	5000
#define S32N_DDR_INIT_DELAY_MS		100
#define S32N_DDR_POLL_MS		100
/* The base covers other requests queued ahead on the same instance. */
#define S32N_DDR_TIMEOUT_BASE_MS	4000
#define S32N_DDR_TIMEOUT_PER_GIB_MS	500

struct ddr_init_job {
	u64 addr;
	u64 size;
	u32 instance;
	ulong start;
};

/* The jobs of one SMMS instance, run in order. */
struct ddr_init_queue {
	struct ddr_init_job jobs[NXP_DDR_MAX_REGIONS];
	unsigned int count;
};

struct ddr_init_call {
	struct udevice *dev;
	struct nxp_ddr_init_req req;
};

/* The last discovery reply, listed again by 'ddr_init discover'. */
static struct nxp_ddr_discovery ddr_init_disc;
static bool ddr_init_disc_valid;

static const struct nxp_ddr_region *
ddr_init_find_region(const struct nxp_ddr_discovery *disc, u64 addr, u64 size)
{
	const struct nxp_ddr_region *r;
	unsigned int i;

	for (i = 0; i < disc->count; i++) {
		r = &disc->regions[i];
		if (addr >= r->start && addr + size <= r->start + r->size)
			return r;
	}

	return NULL;
}

static void ddr_init_print_regions(const struct nxp_ddr_discovery *disc)
{
	const struct nxp_ddr_region *r;
	unsigned int i;

	printf("ddr_init: available regions:\n");
	if (!disc->count)
		printf("    none\n");
	for (i = 0; i < disc->count; i++) {
		r = &disc->regions[i];
		printf("    0x%llx + 0x%llx (instance %u, ECC %s)\n",
		       r->start, r->size, r->instance,
		       r->ecc_enabled ? "on" : "off");
	}
}

static void ddr_init_print_job(const struct ddr_init_job *job)
{
	printf("ddr_init: @0x%llx size=0x%llx inst=%u ...",
	       job->addr, job->size, job->instance);
}

static bool ddr_init_timed_out(const struct ddr_init_job *job)
{
	return get_timer(job->start) > S32N_DDR_TIMEOUT_BASE_MS +
	       DIV_ROUND_UP(job->size, SZ_1G) * S32N_DDR_TIMEOUT_PER_GIB_MS;
}

static void ddr_init_queue_add(struct ddr_init_queue *queues,
			       u64 addr, u64 size, u32 instance)
{
	struct ddr_init_queue *q = &queues[instance];

	q->jobs[q->count++] = (struct ddr_init_job){
		.addr = addr,
		.size = size,
		.instance = instance,
	};
}

static int ddr_init_request(struct ddr_init_call *call)
{
	return misc_call(call->dev, NXP_DDR_CMD_PARTIAL_INIT,
			 &call->req, sizeof(call->req), NULL, 0);
}

static int ddr_init_submit(struct udevice *dev, struct ddr_init_job *job)
{
	struct ddr_init_call call = {
		.dev = dev,
		.req = { .addr = job->addr, .size = job->size },
	};
	int ret;

	ddr_init_print_job(job);

	/* On timeout, ret keeps the last -EBUSY. */
	read_poll_timeout(ddr_init_request, &call, ret, ret != -EBUSY,
			  S32N_DDR_INIT_DELAY_MS * USEC_PER_MSEC,
			  S32N_DDR_BUSY_TIMEOUT_MS * USEC_PER_MSEC);
	if (ret) {
		printf(" failed (%d)\n", ret);
		return ret;
	}

	job->start = get_timer(0);
	printf(" requested\n");

	return 0;
}

/* Return 1 while @job runs, 0 once it succeeded, or a negative error. */
static int ddr_init_check_job(const struct ddr_init_job *job,
			      const struct nxp_ddr_init_status *st)
{
	const struct nxp_ddr_instance_status *s;

	if (st && job->instance >= st->count)
		return -ENODEV;

	/* FSS reports our oldest queued job, or the last one done. */
	s = st ? &st->inst[job->instance] : NULL;
	if (s && !s->pending && s->start == job->addr && s->size == job->size) {
		ddr_init_print_job(job);
		if (s->status != NXP_DDR_REQ_STATUS_SUCCESS) {
			printf(" failed\n");
			return -EIO;
		}
		printf(" done in %lu ms\n", get_timer(job->start));
		return 0;
	}

	if (ddr_init_timed_out(job)) {
		ddr_init_print_job(job);
		printf(" timed out\n");
		return -ETIMEDOUT;
	}

	return 1;
}

/* Poll until @job is done, failed or timed out. */
static int ddr_init_wait(struct udevice *dev, const struct ddr_init_job *job)
{
	struct nxp_ddr_init_status st;
	int ret;

	for (;;) {
		/* -EBUSY: no answer this time; only the timeout applies. */
		ret = misc_call(dev, NXP_DDR_CMD_LAST_INIT_STATUS, NULL, 0,
				&st, sizeof(st));
		if (ret && ret != -EBUSY)
			return ret;

		ret = ddr_init_check_job(job, ret ? NULL : &st);
		if (ret <= 0)
			return ret;

		if (ctrlc())
			return -EINTR;
		mdelay(S32N_DDR_POLL_MS);
	}
}

/* Run the jobs of @q in order. The first one is already started. */
static int ddr_init_drain(struct udevice *dev, struct ddr_init_queue *q,
			  bool stop)
{
	unsigned int i;
	int ret;

	for (i = 0; i < q->count; i++) {
		if (i) {
			if (stop)
				break;
			ret = ddr_init_submit(dev, &q->jobs[i]);
			if (ret)
				return ret;
		}

		ret = ddr_init_wait(dev, &q->jobs[i]);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Start the first job of every instance, so the instances work in parallel,
 * then wait for each instance's jobs in turn. After an error, start nothing
 * new but still wait for the jobs already started, and return the first
 * error.
 */
static int ddr_init_run_jobs(struct udevice *dev,
			     struct ddr_init_queue *queues)
{
	struct ddr_init_queue *q;
	int ret, err = 0;
	u32 inst;

	for (inst = 0; inst < NXP_DDR_MAX_INSTANCES; inst++) {
		q = &queues[inst];
		if (q->count && !err)
			err = ddr_init_submit(dev, &q->jobs[0]);
		/* Not started: nothing to wait for. */
		if (err)
			q->count = 0;
	}

	for (inst = 0; inst < NXP_DDR_MAX_INSTANCES; inst++) {
		ret = ddr_init_drain(dev, &queues[inst], err);
		/* Ctrl-C was consumed, so stop here. */
		if (ret == -EINTR)
			return ret;
		if (!err)
			err = ret;
	}

	return err;
}

static int ddr_init_parse_range(char *const argv[], u64 *addr, u64 *size)
{
	char *endp;

	*addr = simple_strtoull(argv[1], NULL, 16);
	*size = ustrtoull(argv[2], &endp, 0);
	if (!*size || !IS_ALIGNED(*addr, S32N_DDR_REQ_ALIGN) ||
	    !IS_ALIGNED(*size, S32N_DDR_REQ_ALIGN) ||
	    check_uptr_overflow(*addr, *size)) {
		printf("ddr_init: 0x%llx + 0x%llx is not a valid 128 MiB-aligned range\n",
		       *addr, *size);
		return -EINVAL;
	}

	return 0;
}

/*
 * Queue the jobs to run and return how many, or a negative error. 'all' queues
 * no job for the regions with ECC off, but they are still published; an
 * explicitly requested range is always initialized.
 */
static int ddr_init_collect(const struct nxp_ddr_discovery *disc, bool all,
			    u64 addr, u64 size, struct ddr_init_queue *queues)
{
	const struct nxp_ddr_region *r;
	unsigned int i;
	int n = 0;

	if (!all) {
		r = ddr_init_find_region(disc, addr, size);
		if (!r) {
			printf("ddr_init: 0x%llx + 0x%llx is not inside one available region\n",
			       addr, size);
			ddr_init_print_regions(disc);
			return -EINVAL;
		}
		if (!r->ecc_enabled)
			printf("ddr_init: inst %u has ECC off, initializing anyway\n",
			       r->instance);
		ddr_init_queue_add(queues, addr, size, r->instance);
		return 1;
	}

	for (i = 0; i < disc->count; i++) {
		r = &disc->regions[i];
		if (!r->ecc_enabled) {
			printf("ddr_init: @0x%llx size=0x%llx inst=%u no init needed (ECC off)\n",
			       r->start, r->size, r->instance);
			continue;
		}
		ddr_init_queue_add(queues, r->start, r->size, r->instance);
		n++;
	}

	return n;
}

static bool ddr_init_bank_is_free(int bank)
{
	return !gd->bd->bi_dram[bank].start && !gd->bd->bi_dram[bank].size;
}

/*
 * Merge into [*addr, *end) one bank that overlaps or is adjacent to it, and
 * free that bank. Return false if there is none.
 */
static bool ddr_init_merge_bank(u64 *addr, u64 *end)
{
	struct bd_info *bd = gd->bd;
	u64 b_start, b_end;
	int bank;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (ddr_init_bank_is_free(bank))
			continue;

		b_start = bd->bi_dram[bank].start;
		b_end = b_start + bd->bi_dram[bank].size;
		if (*addr <= b_end && b_start <= *end) {
			*addr = min(*addr, b_start);
			*end = max(*end, b_end);
			bd->bi_dram[bank].start = 0;
			bd->bi_dram[bank].size = 0;
			return true;
		}
	}

	return false;
}

/*
 * Store an initialized region as a DRAM bank in the OS DT's /memory node.
 * The region gains no MMU mapping: U-Boot must not touch it.
 */
static int ddr_init_publish_bank(u64 addr, u64 size)
{
	struct bd_info *bd = gd->bd;
	u64 end = addr + size;
	int bank, free_bank = -1;

	/* A merged range can reach banks that did not touch the old one. */
	while (ddr_init_merge_bank(&addr, &end))
		;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (ddr_init_bank_is_free(bank)) {
			free_bank = bank;
			break;
		}
	}

	if (free_bank < 0) {
		printf("ddr_init: no free DRAM bank for 0x%llx + 0x%llx (CONFIG_NR_DRAM_BANKS=%d)\n",
		       addr, end - addr, CONFIG_NR_DRAM_BANKS);
		return -ENOSPC;
	}

	bd->bi_dram[free_bank].start = addr;
	bd->bi_dram[free_bank].size = end - addr;

	return 0;
}

/* Publish every queued job and, for 'all', the regions with ECC off. */
static int ddr_init_publish(const struct ddr_init_queue *queues,
			    const struct nxp_ddr_discovery *disc, bool all)
{
	const struct nxp_ddr_region *r;
	unsigned int i;
	u32 inst;
	int ret;

	for (inst = 0; inst < NXP_DDR_MAX_INSTANCES; inst++) {
		for (i = 0; i < queues[inst].count; i++) {
			ret = ddr_init_publish_bank(queues[inst].jobs[i].addr,
						    queues[inst].jobs[i].size);
			if (ret)
				return ret;
		}
	}

	for (i = 0; all && i < disc->count; i++) {
		r = &disc->regions[i];
		if (r->ecc_enabled)
			continue;
		ret = ddr_init_publish_bank(r->start, r->size);
		if (ret)
			return ret;
	}

	return 0;
}

static int do_ddr_init(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct ddr_init_queue queues[NXP_DDR_MAX_INSTANCES] = { 0 };
	struct nxp_ddr_discovery disc;
	u64 addr = 0, size = 0;
	struct udevice *dev;
	ulong start;
	bool all, discover;
	int n, ret;

	all = argc == 2 && !strcmp(argv[1], "all");
	discover = argc == 2 && !strcmp(argv[1], "discover");
	if (discover && ddr_init_disc_valid) {
		ddr_init_print_regions(&ddr_init_disc);
		return CMD_RET_SUCCESS;
	}

	if (!all && !discover &&
	    (argc != 3 || ddr_init_parse_range(argv, &addr, &size)))
		return CMD_RET_USAGE;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(scmi_ddr), &dev);
	if (ret) {
		printf("ddr_init: NXP DDR SCMI device not found (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	ret = misc_call(dev, NXP_DDR_CMD_DISCOVER_REGIONS, NULL, 0,
			&disc, sizeof(disc));
	if (ret) {
		printf("ddr_init: DDR region discovery failed (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	ddr_init_disc = disc;
	ddr_init_disc_valid = true;

	if (discover) {
		ddr_init_print_regions(&disc);
		return CMD_RET_SUCCESS;
	}

	n = ddr_init_collect(&disc, all, addr, size, queues);
	if (n < 0)
		return CMD_RET_FAILURE;

	start = get_timer(0);
	if (n) {
		ret = ddr_init_run_jobs(dev, queues);
		if (ret) {
			printf("ddr_init: stopped (%d)\n", ret);
			return CMD_RET_FAILURE;
		}
	}

	if (ddr_init_publish(queues, &disc, all))
		return CMD_RET_FAILURE;

	if (n)
		printf("ddr_init: %d request(s) done in %lu ms\n", n,
		       get_timer(start));
	else
		printf("ddr_init: nothing to initialize\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ddr_init, 3, 0, do_ddr_init,
	   "initialize high DDR via the NXP DDR SCMI protocol",
	   "discover\n"
	   "    - list the DDR regions FSS reports as available\n"
	   "ddr_init all\n"
	   "    - initialize every region FSS reports as available, in parallel\n"
	   "      on the SMMS DDR controllers (those with ECC off need no init),\n"
	   "      and add them all to the /memory node passed to the OS\n"
	   "ddr_init <addr> <size>\n"
	   "    - initialize <size> bytes at <addr>, both 128 MiB-aligned and\n"
	   "      inside one reported region (a range outside prints them).\n"
	   "      <addr> is hex; <size> is hex, decimal or has a K/M/G suffix.");
