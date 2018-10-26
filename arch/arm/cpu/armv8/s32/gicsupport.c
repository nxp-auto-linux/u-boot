/*
 * Copyright 2016,2018 NXP
 * Heinz Wrobel <Heinz.Wrobel@nxp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Basic GIC support to permit dealing with interrupt handlers in ARMv8.
 * This code is currently only tested on S32V234, but should be generic
 * enough to be placed outside a CPU specific directory at some point.
 * We ignore SGI/PPI because that is done in gic_64.S.
 *
 * Some of this code is taken from sources developed by <Jay.Tu@nxp.com>
 */

#include <common.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/gicsupport.h>
#include <asm/gic.h>
#include <asm/proc-armv/system.h>

DECLARE_GLOBAL_DATA_PTR;

#define ENTRIES(x) (sizeof(x)/sizeof(x[0]))

/* A performance implementation would use an indexed table.
 * U-Boot has a need for a small footprint, so we do a simple search
 * and only support a few handlers concurrently.
 */
static struct
{
	int     irq;
	void (*handler)(struct pt_regs *pt_regs, unsigned int esr);
	const char *name;
	int type;
	uint32_t count;
} inthandlers[16];
static uint32_t count_spurious;
static int preinitdone = 0, fullinitdone = 0;

static void interrupt_handler_init(void)
{
	/* We need to decouple this init from the generic
	 * interrupt_init() function because interrupt handlers
	 * may be preregistered by code before the GIC init has
	 * happened! PCIe support is a good example for this.
	 */
	int i;

	if (!preinitdone) {
		preinitdone = 1;
		/* Make sure that we do not have any active handlers */
		for (i = 0; i < ENTRIES(inthandlers); i++) {
			inthandlers[i].irq = -1;
			inthandlers[i].count = 0;
		}
		count_spurious = 0;
	}
}

/* Public function to support handlers. Benign to call before GIC init */
void gic_unmask_irq(unsigned int irq)
{
	uint32_t mask;

	if (fullinitdone) {
		mask = 1 << (irq & 0x1f);
		writel(mask, GICD_BASE + GICD_ISENABLERn + (irq >> 5) * sizeof(uint32_t));
	}
}

/* Public function to support handlers. Benign to call before GIC init */
void gic_mask_irq(unsigned int irq)
{
	uint32_t mask;

	mask = 1 << (irq & 0x1f);
	writel(mask, GICD_BASE + GICD_ICENABLERn + (irq >> 5) * sizeof(uint32_t));
}

/* Public function to support handlers. Benign to call before GIC init */
/* To support edge-triggered interrupts */
void gic_clear_pending_irq(unsigned int irq)
{
	uint32_t mask;

	mask = 1 << (irq & 0x1f);
	writel(mask, GICD_BASE + GICD_ICPENDRn + (irq >> 5) * sizeof(uint32_t));
}

/* Public function to support handlers. Benign to call before GIC init */
int gic_irq_status(unsigned int irq)
{
	uint32_t mask;

	uint32_t v = readl(GICD_BASE + GICD_ISPENDRn + (irq >> 5) * sizeof(uint32_t));
	mask = 1 << (irq & 0x1f);

	return !!(v & mask);

}

/* Public function to support handlers. Benign to call before GIC init */
static void gic_set_type(unsigned int irq, int type)
{
	int shift = (irq & 0x0f) * 2;
	uint32_t mask, icfgr;

	if (fullinitdone) {
		mask = 0x3 << shift;
		icfgr = readl(GICD_BASE + GICD_ICFGR + (irq >> 4) * sizeof(uint32_t));
		icfgr &= ~mask;
		icfgr |= ((type & 0x3) << shift);
		writel(icfgr, GICD_BASE + GICD_ICFGR + (irq >> 4) * sizeof(uint32_t));
	}
}

int gic_register_handler(int irq, void (*handler)(struct pt_regs *pt_regs, unsigned int esr), int type, const char *name)
{
	int i;

	interrupt_handler_init();

	if (fullinitdone)
		gic_mask_irq(irq);

	for (i = 0; i < ENTRIES(inthandlers); i++) {
		if(inthandlers[i].irq < 0) {
			inthandlers[i].handler = handler;
			inthandlers[i].name = name;
			inthandlers[i].type = type;

			/* Done last to avoid race condition */
			inthandlers[i].irq = irq;

			if (fullinitdone) {
				gic_set_type(irq, type);
				gic_unmask_irq(irq);
			}
			break;
		}
	}

	return i >= ENTRIES(inthandlers) ? 0 : 1;
}

int gic_deregister_handler(int irq)
{
	int i;

	if (fullinitdone)
		gic_mask_irq(irq);

	for (i = 0; i < ENTRIES(inthandlers); i++) {
		if (inthandlers[i].irq == irq) {
			inthandlers[i].handler = NULL;
			inthandlers[i].name = NULL;
			inthandlers[i].type = 0;

			/* Done last to avoid race condition */
			inthandlers[i].irq = -1;
			break;
		}
	}

	return i >= ENTRIES(inthandlers) ? 0 : 1;
}

static void gic_dist_init(unsigned long base)
{
	unsigned int gic_irqs, i;
	uint32_t ctlr = readl(base + GICD_CTLR);

	/* We turn off the GIC while we mess with it.
	 * The original init has happened in gic_64.S!
	 */
	writel(ctlr & ~3, base + GICD_CTLR);

	gic_irqs = readl(base + GICD_TYPER) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = 32; i < gic_irqs; i += 16)
		writel(0, base + GICD_ICFGR + i * 4 / 16);

	/*
	 * Set all global interrupts to this CPU only.
	 */
	for (i = 32; i < gic_irqs; i += 4)
		writel(0x01010101, base + GICD_ITARGETSRn + i * 4 / 4);

	/*
	 * Set priority on all global interrupts.
	 */
	for (i = 32; i < gic_irqs; i += 4)
		writel(0xa0a0a0a0, base + GICD_IPRIORITYRn + i * 4 / 4);

	/*
	 * Disable all interrupts.  Leave the PPI and SGIs alone
	 * as these enables are banked registers.
	 */
	for (i = 32; i < gic_irqs; i += 32)
		writel(0xffffffff, base + GICD_ICENABLERn + i * 4 / 32);

	writel(ctlr, base + GICD_CTLR);
}

static void gic_cpu_clean(unsigned long dist_base, unsigned long cpu_base)
{
	/* Initially we need to make sure that we do not have any
	 * left over requests that could cause a mess during
	 * initialization. This happens during sloppy SMP init in
	 * lowlevel.S and should be FIXED. *sigh*
	 */
	writel(0xffffffff, dist_base + GICD_ICENABLERn);

}

static void gic_cpu_init(unsigned long dist_base, unsigned long cpu_base)
{
	/* Accept just about anything */
	writel(0xff, cpu_base + GICC_PMR);

	/* gic_64.S doesn't set the recommended AckCtl value. We do */
	writel(0x1e3, cpu_base + GICC_CTLR);
}

int arch_interrupt_init(void)
{
	int i;

	interrupt_handler_init();

	gic_cpu_clean(GICD_BASE, GICC_BASE);

	gic_dist_init(GICD_BASE);

	/* U-Boot runs with a single CPU only */
	gic_cpu_init(GICD_BASE, GICC_BASE);

	fullinitdone = 1;

	/* Now that we have set up the GIC, we need to start up
	 * any preregistered handlers.
	 */
	for (i = 0; i < ENTRIES(inthandlers); i++) {
		int irq = inthandlers[i].irq;
		if(irq >= 0) {
			if (fullinitdone) {
				gic_set_type(irq, inthandlers[i].type);
				gic_unmask_irq(irq);
			}
		}
	}

	return 0;
}

int interrupt_init (void)
{
	return arch_interrupt_init();
}

void enable_interrupts(void)
{
	local_irq_enable();
}

int disable_interrupts(void)
{
	int flags;

	local_irq_save(flags);

	return flags;
}

void do_irq(struct pt_regs *pt_regs, unsigned int esr)
{
	int i, group = 0;
	uint32_t thisirq = readl(GICC_BASE + GICC_IAR);

	if (thisirq == 1022) {
		/* Group 1 interrupt! */
		group = 1;
		thisirq = readl(GICC_BASE + GICC_AIAR);
	}

	if (thisirq == 1023) {
		count_spurious++;
		return;
	}

	for (i = 0; i < ENTRIES(inthandlers); i++) {
		if(inthandlers[i].irq == thisirq) {
			inthandlers[i].count++;
			(inthandlers[i].handler)(pt_regs, esr);

			break;
		}
	}

	if (group) {
		writel(thisirq, GICC_BASE + GICC_AEOIR);
	}
	else {
		writel(thisirq, GICC_BASE + GICC_EOIR);
	}

	if (i >= ENTRIES(inthandlers)) {
		printf("\"Irq\" handler, esr 0x%08x for GIC irq %d, group %d\n",
		       esr, thisirq, group);
		show_regs(pt_regs);
		panic("Resetting CPU ...\n");
	}
}

#if defined(CONFIG_CMD_IRQ)
int do_irqinfo (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	printf("GIC support is enabled for GIC @ 0x%08x\n", GICD_BASE);
	printf("Spurious: %d\n", count_spurious);
	for (i = 0; i < ENTRIES(inthandlers); i++) {
		if(inthandlers[i].irq >= 0) {
			printf("%20s(%d): %d\n", inthandlers[i].name,
			       inthandlers[i].irq,
			       inthandlers[i].count);
		}
	}

	return 0;
}
#endif
