/*
 * (C) Copyright 2007,
 * Sascha Hauer, Pengutronix
 *
 * (C) Copyright 2009-2013 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h> 
#include <asm/armv7.h> 
#include <asm/pl310.h> 
#include <asm/errno.h> 
#include <asm/io.h> 
#include <libfdt.h> 
#include <stdbool.h> 

#if defined(CONFIG_SECURE_BOOT) 
#include <asm/arch-sac58r/sac58r_secure.h> 
#endif 

#ifdef CONFIG_ARCH_MISC_INIT 
int arch_misc_init(void) 
{ 
#ifdef CONFIG_SECURE_BOOT 
	get_hab_status(); 
#endif 
	return 0; 
} 
#endif /* !CONFIG_ARCH_MISC_INIT */ 

#ifdef CONFIG_SECURE_BOOT 

#define hab_rvt_report_event_p	HAB_RVT_REPORT_EVENT				 


#define hab_rvt_report_status_p	HAB_RVT_REPORT_STATUS


#define hab_rvt_authenticate_image_p	HAB_RVT_AUTHENTICATE_IMAGE			


#define hab_rvt_entry_p	HAB_RVT_ENTRY						 
 

#define hab_rvt_exit_p	HAB_RVT_EXIT
									 


#define IVT_SIZE		0x20 
#define ALIGN_SIZE		0x1000 
#define CSF_PAD_SIZE		0x2000 

/* 
 * +------------+  0x0 (DDR_UIMAGE_START) - 
 * |   Header   |                          | 
 * +------------+  0x40                    | 
 * |            |                          | 
 * |            |                          | 
 * |            |                          | 
 * |            |                          | 
 * | Image Data |                          | 
 * .            |                          | 
 * .            |                           > Stuff to be authenticated ----+ 
 * .            |                          |                                | 
 * |            |                          |                                | 
 * |            |                          |                                | 
 * +------------+                          |                                | 
 * |            |                          |                                | 
 * | Fill Data  |                          |                                | 
 * |            |                          |                                | 
 * +------------+ Align to ALIGN_SIZE      |                                | 
 * |    IVT     |                          |                                | 
 * +------------+ + IVT_SIZE              -                                 | 
 * |            |                                                           | 
 * |  CSF DATA  | <---------------------------------------------------------+ 
 * |            | 
 * +------------+ 
 * |            | 
 * | Fill Data  | 
 * |            | 
 * +------------+ + CSF_PAD_SIZE 
 */ 

int check_hab_enable(void) 
{ 
	u32 reg; 
	int result = 0; 
	struct iim_regs *iim = (struct iim_regs *)IMX_IIM_BASE; 
	struct fuse_bank *bank = &iim->bank[0]; 
	struct fuse_bank0_regs *fuse_bank0 = 
			(struct fuse_bank0_regs *)bank->fuse_regs; 

	reg = readl(&fuse_bank0->cfg5); 
	if (reg & 0x2) 
		result = 1; 

	return result; 
} 

void display_event(uint8_t *event_data, size_t bytes) 
{ 
	uint32_t i; 
	if ((event_data) && (bytes > 0)) { 
		for (i = 0; i < bytes; i++) { 
			if (i == 0) 
				printf("\t0x%02x", event_data[i]); 
			else if ((i % 8) == 0) 
				printf("\n\t0x%02x", event_data[i]); 
			else 
				printf(" 0x%02x", event_data[i]); 
		} 
	} 
} 

int get_hab_status(void) 
{ 
	uint32_t index = 0; /* Loop index */ 
	uint8_t event_data[128]; /* Event data buffer */ 
	size_t bytes = sizeof(event_data); /* Event size in bytes */ 
	hab_config_t config = 0; 
	hab_state_t state = 0; 
	hab_rvt_report_event_t *hab_rvt_report_event; 
	hab_rvt_report_status_t *hab_rvt_report_status; 

	hab_rvt_report_event = hab_rvt_report_event_p; 
	hab_rvt_report_status = hab_rvt_report_status_p; 

	/* Check HAB status */ 
	if (hab_rvt_report_status(&config, &state) != HAB_SUCCESS) { 
		printf("\nHAB Configuration: 0x%02x, HAB State: 0x%02x\n", 
			config, state); 

		/* Display HAB Error events */ 
		while (hab_rvt_report_event(HAB_FAILURE, index, event_data, 
				&bytes) == HAB_SUCCESS) { 
			printf("\n"); 
			printf("--------- HAB Event %d -----------------\n", 
					index + 1); 
			printf("event data:\n"); 
			display_event(event_data, bytes); 
			printf("\n"); 
			bytes = sizeof(event_data); 
			index++; 
		} 
	} 
	/* Display message if no HAB events are found */ 
	else { 
		printf("\nHAB Configuration: 0x%02x, HAB State: 0x%02x\n", 
			config, state); 
		printf("No HAB Events Found!\n\n"); 
	} 
	return 0; 
} 

void hab_caam_clock_enable(void) 
{ 
	writel(0xFFFFFFFF, GPC_BASE_ADDR + GPC_AIPS1_OFFPF_PCTL_3); 
} 


void hab_caam_clock_disable(void) 
{ 
	writel(0x00000000, GPC_BASE_ADDR + GPC_AIPS1_OFFPF_PCTL_3); 
} 

#ifdef DEBUG_AUTHENTICATE_IMAGE 
void dump_mem(uint32_t addr, int size) 
{ 
	int i; 

	for (i = 0; i < size; i += 4) { 
		if (i != 0) { 
			if (i % 16 == 0) 
				printf("\n"); 
			else 
				printf(" "); 
		} 

		printf("0x%08x", *(uint32_t *)addr); 
		addr += 4; 
	} 

	printf("\n"); 

	return; 
} 
#endif 

uint32_t authenticate_image(uint32_t ddr_start, uint32_t image_size) 
{ 
	uint32_t load_addr = 0; 
	size_t bytes; 
	ptrdiff_t ivt_offset = 0; 
	int result = 0; 
	ulong start; 
	hab_rvt_authenticate_image_t *hab_rvt_authenticate_image; 
	hab_rvt_entry_t *hab_rvt_entry; 
	hab_rvt_exit_t *hab_rvt_exit; 

	hab_rvt_authenticate_image = hab_rvt_authenticate_image_p; 
	hab_rvt_entry = hab_rvt_entry_p; 
	hab_rvt_exit = hab_rvt_exit_p; 

	if (check_hab_enable() == 1) { 
		printf("\nAuthenticate uImage from DDR location 0x%x...\n", 
			ddr_start); 

		hab_caam_clock_enable(); 

		if (hab_rvt_entry() == HAB_SUCCESS) { 
			/* If not already aligned, Align to ALIGN_SIZE */ 
			ivt_offset = (image_size + ALIGN_SIZE - 1) & 
					~(ALIGN_SIZE - 1); 

			start = ddr_start; 
			bytes = ivt_offset + IVT_SIZE + CSF_PAD_SIZE; 

#ifdef DEBUG_AUTHENTICATE_IMAGE 
			printf("\nivt_offset = 0x%x, ivt addr = 0x%x\n", 
			       ivt_offset, ddr_start + ivt_offset); 
			printf("Dumping IVT\n"); 
			dump_mem(ddr_start + ivt_offset, 0x20); 

			printf("Dumping CSF Header\n"); 
			dump_mem(ddr_start + ivt_offset + 0x20, 0x40); 

			get_hab_status(); 

			printf("\nCalling authenticate_image in ROM\n"); 
			printf("\tivt_offset = 0x%x\n\tstart = 0x%08x" 
			       "\n\tbytes = 0x%x\n", ivt_offset, start, bytes); 
#endif 

			load_addr = (uint32_t)hab_rvt_authenticate_image( 
					HAB_CID_UBOOT, 
					ivt_offset, (void **)&start, 
					(size_t *)&bytes, NULL); 
			if (hab_rvt_exit() != HAB_SUCCESS) { 
				printf("hab exit function fail\n"); 
				load_addr = 0; 
			} 
		} else 
			printf("hab entry function fail\n"); 

		hab_caam_clock_disable(); 

		get_hab_status(); 
	} 

	if ((!check_hab_enable()) || (load_addr != 0)) 
		result = 1; 

	return result; 
} 
/* ----------- end of HAB API updates ------------*/ 
#endif 

