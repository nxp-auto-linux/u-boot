/*
 * Copyright 2016-2018, 2022 NXP
 *
 */
 /* Functions declared in arch/arm/mach-s32/s32-cc/gicsupport.c */
int gic_irq_status(unsigned int irq);
int gic_register_handler(int irq,
			 void (*handler)(struct pt_regs *pt_regs,
					 unsigned int esr),
			 int type, const char *name);
int gic_deregister_handler(int irq);
