// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2026 NXP
 *
 * Ask the FSS firmware to initialize high DDR regions on S32N7 via the
 * NXP DDR SCMI vendor protocol.
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
#include <dm/uclass.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include <linux/types.h>

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

	printf("ddr_init: uninitialized regions:\n");
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
 * Queue the jobs to run and return how many, or a negative error. 'all' skips
 * the regions with ECC off, which need no init; an explicit range is always
 * initialized.
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
			printf("ddr_init: 0x%llx + 0x%llx is not inside one uninitialized region\n",
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
			printf("ddr_init: @0x%llx size=0x%llx inst=%u skipped (ECC off)\n",
			       r->start, r->size, r->instance);
			continue;
		}
		ddr_init_queue_add(queues, r->start, r->size, r->instance);
		n++;
	}

	return n;
}

static int do_ddr_init(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct ddr_init_queue queues[NXP_DDR_MAX_INSTANCES] = { 0 };
	struct nxp_ddr_discovery disc;
	u64 addr = 0, size = 0;
	struct udevice *dev;
	ulong start;
	int n, ret;
	bool all;

	all = argc == 2 && !strcmp(argv[1], "all");
	if (!all && (argc != 3 || ddr_init_parse_range(argv, &addr, &size)))
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

	n = ddr_init_collect(&disc, all, addr, size, queues);
	if (n < 0)
		return CMD_RET_FAILURE;
	if (!n) {
		printf("ddr_init: nothing to initialize\n");
		return CMD_RET_SUCCESS;
	}

	start = get_timer(0);
	ret = ddr_init_run_jobs(dev, queues);
	if (ret) {
		printf("ddr_init: stopped (%d)\n", ret);
		return CMD_RET_FAILURE;
	}

	printf("ddr_init: %d request(s) done in %lu ms\n", n, get_timer(start));

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(ddr_init, 3, 0, do_ddr_init,
	   "initialize high DDR via the NXP DDR SCMI protocol",
	   "all\n"
	   "    - initialize every region FSS reports as uninitialized, skipping\n"
	   "      those with ECC off; works on the DDR instances in parallel\n"
	   "ddr_init <addr> <size>\n"
	   "    - initialize <size> bytes at <addr>, both 128 MiB-aligned and\n"
	   "      inside one reported region (a range outside prints them).\n"
	   "      <addr> is hex; <size> is hex, decimal or has a K/M/G suffix.\n"
	   "      Example: ddr_init 0x8800000000 1G");
