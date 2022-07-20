// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <fdt_support.h>
#include <malloc.h>
#include <dm/uclass.h>
#include <linux/psci.h>
#include <s32-cc/fdt_wrapper.h>

#define CLUSTER_SHIFT	8U
#define CLUSTER_MASK	GENMASK(11, CLUSTER_SHIFT)

struct cpu_desc {
	u32 psci_id;
	bool on;
};

static struct cpu_desc *cpus;
static u32 n_cpus;

static struct cpu_desc *get_cpu(const unsigned int cpu_id)
{
	if (cpu_id >= n_cpus)
		return NULL;

	return &cpus[cpu_id];
}

static int add_cpu(u32 psci_id)
{
	static u32 allocated;
	struct cpu_desc *backup;

	backup = cpus;
	if (allocated < n_cpus + 1) {
		if (!allocated)
			allocated = 4;
		else
			allocated *= 2;

		cpus = realloc(backup, allocated * sizeof(*cpus));
		if (!cpus) {
			cpus = backup;
			return -ENOMEM;
		}
	}

	cpus[n_cpus].psci_id = psci_id;

	/* U-Boot will start on Core 0 */
	if (!n_cpus)
		cpus[n_cpus].on = true;
	else
		cpus[n_cpus].on = false;

	n_cpus++;

	return 0;
}

static int initialize_cpus_data(void)
{
	static bool initialized;
	int ret = -1;
	fdt_addr_t reg;
	ofnode node;

	if (initialized)
		return 0;

	node = ofnode_path("/cpus");
	if (!ofnode_valid(node)) {
		printf("Couldn't find /cpus node\n");
		return -EINVAL;
	}

	for (;;) {
		node = ofnode_by_prop_value(node, "device_type", "cpu", 4);
		if (!ofnode_valid(node))
			break;

		reg = ofnode_get_addr_size_index_notrans(node, 0, NULL);
		if (reg == FDT_ADDR_T_NONE) {
			ret = -EINVAL;
			break;
		}

		ret = add_cpu(reg);
		if (ret)
			break;
	}

	if (ret) {
		free(cpus);
		cpus = NULL;
		n_cpus = 0;
	} else {
		initialized = true;
	}

	return ret;
}

int cpu_reset(u32 nr)
{
	/* Not available */
	return -1;
}

int cpu_disable(u32 nr)
{
	/* Not available */
	return -1;
}

int is_core_valid(unsigned int core)
{
	int ret;

	ret = initialize_cpus_data();
	if (ret)
		return 0;

	if (!get_cpu(core))
		return 0;

	return 1;
}

int cpu_status(u32 nr)
{
	int ret;
	struct cpu_desc *cpu;

	ret = initialize_cpus_data();
	if (ret)
		return ret;

	printf("CPU %u - Status : ", nr);
	cpu = get_cpu(nr);
	if (!cpu) {
		printf("Unknown\n");
		return -1;
	}

	if (cpu->on)
		printf("Running");
	else
		printf("Inactive");

	printf("\n");

	return 0;
}

int cpu_release(u32 nr, int argc, char * const argv[])
{
	unsigned long psci_ret;
	struct cpu_desc *cpu;
	struct udevice *dev;
	int ret;
	u64 boot_addr;

	/* Probe PSCI driver */
	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "psci", &dev);
	if (ret) {
		printf("Failed to probe PSCI driver\n");
		return ret;
	}

	ret = initialize_cpus_data();
	if (ret)
		return ret;

	cpu = get_cpu(nr);
	if (!cpu)
		return -EINVAL;

	boot_addr = simple_strtoull(argv[0], NULL, 16);

	psci_ret = invoke_psci_fn(PSCI_0_2_FN64_CPU_ON,
				  cpu->psci_id, boot_addr, 0x0);
	if (psci_ret) {
		printf("PSCI call failed with : %lu\n", psci_ret);
		return -EINVAL;
	}

	cpu->on = true;

	return 0;
}
