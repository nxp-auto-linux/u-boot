/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <vsprintf.h>
#include <asm/arch/qspi_common.h>
#include <asm/arch/qspi_driver.h>
#include <asm/arch/siul.h>
#include <asm/io.h>
#include <linux/kernel.h>

/*
 * If we have changed the content of the flash by writing or erasing,
 * we need to invalidate the AHB buffer. If we do not do so, we may read out
 * the wrong data. The spec tells us reset the AHB domain and Serial Flash
 * domain at the same time.
 */
inline void qspi_ahb_invalid(void)
{
	QuadSPI.MCR.B.SWRSTSD = 1;
	QuadSPI.MCR.B.SWRSTHD = 1; // |= QSPI_MCR_SWRSTHD_MASK | QSPI_MCR_SWRSTSD_MASK;

	/*
	 * The minimum delay : 1 AHB + 2 SFCK clocks.
	 * Delay 1 us is enough.
	 */
	udelay(1);

	QuadSPI.MCR.B.SWRSTSD = 0;
	QuadSPI.MCR.B.SWRSTHD = 0; // |= QSPI_MCR_SWRSTHD_MASK | QSPI_MCR_SWRSTSD_MASK;
}

void QSPI_setup_hyp()
{
	qspi_iomux();
	/* TODO Jira issue pending */
#ifdef CONFIG_DEBUG_S32_QSPI_QSPI
	eprintf("ERROR: QSPI_setup_hyp() HAS NO QuadSPI settings and definitions");
#else
	debug("QSPI_setup_hyp() uses baremetal QuadSPI settings and definitions");

	QuadSPI.MCR.B.MDIS = 0;	//clear MDIS bit
	QuadSPI.BUF0IND.R = 0x0;	//set AHB buffer size (64bits)
	QuadSPI.SFA1AD.R = CONFIG_SYS_FLASH_BASE + 0x4000000;	//set top address of FA1 (size 512Mbit)
	QuadSPI.SFA2AD.R = CONFIG_SYS_FLASH_BASE + 0x4000000;	//set top address of FA2 (size 0Mbit)
	QuadSPI.SFB1AD.R = FLASH_BASE_ADR2 + 0x4000000;	//set top address of FB1 (size 512Mbit)
	QuadSPI.SFB2AD.R = FLASH_BASE_ADR2 + 0x4000000;	//set top address of FB2 (size 0Mbit) 0x203FFFFF

	QuadSPI.BUF0IND.R = 0x100;	/* buffer0 size 512 bytes */
	QuadSPI.BUF1IND.R = 0x200;	/* buffer1 size 0 bytes */
	QuadSPI.BUF2IND.R = 0x200;	/* buffer2 size 0 bytes */
	QuadSPI.BUF3CR.R = 0x80000000;	/* All masters use buffer 3 */

	QuadSPI.SFAR.R = CONFIG_SYS_FLASH_BASE;
	QuadSPI.MCR.B.DDR_EN = 1;
	QuadSPI.MCR.B.DQS_EN = 1;
	QuadSPI.SFACR.R = 0x00010003;
	QuadSPI.MCR.B.DQS_LAT_EN = 1;
	QuadSPI.FLSHCR.B.TDH = 1;

	/* Set-up Read command for hyperflash */
	quadspi_read_hyp();	// The command will be used by default for any AHB access to the flash memory
#endif
}				/* QSPI_setup */

//address = -1 means chip erase
void quadspi_erase_hyp(int address)
{
	int address_start = address & ~(FLASH_SECTOR_SIZE-1);

	/* Invalidate all cache data otherwise read won't return correct data. */
	qspi_ahb_invalid();
	invalidate_icache_all();
	if (address == -1)
		invalidate_dcache_range(CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE + CONFIG_SYS_FSL_FLASH0_SIZE);
	else
		invalidate_dcache_range(address_start, address_start + FLASH_SECTOR_SIZE);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xAA);
	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0x554, 0x55);

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0x80);
	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xAA);

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0x554, 0x55);

	if (address == -1)
		quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0x10);
	else
		quadspi_send_instruction_hyp((address & 0xfffffffe), 0x30);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;
}

void quadspi_program_word_hyp(unsigned int address, unsigned int word)
{
	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xAA);
	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0x554, 0x55);

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xA0);
	quadspi_send_instruction_hyp(address, word);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;
}

enum qspi_addr_t{
	qspi_real_address = 1,
	qspi_real_and_all
};

bool is_flash_addr(unsigned int address, enum qspi_addr_t addr_type)
{
	bool isflash = 0;

	isflash |= (address >= CONFIG_SYS_FLASH_BASE);
	isflash |= (address == qspi_real_and_all) && (address == -1);
	if (!isflash) {
		printf("Incorrect address '0x%.8x'.\n"
		       "Must an address above or equal to '0x%.8x' (or '-1',"
		       " if the command accepts it)\n", address,
		       CONFIG_SYS_FLASH_BASE);
		return 0;
	}
	return 1;
}

void quadspi_program_hyp(unsigned int address, uintptr_t pdata,
			 unsigned int bytes)
{
	int i, j, k, m;	// i: number total of bytes to flash
			// k: number of bytes to flash at the next command
			// m :number of dword (32 bits) to flash at the next command
	unsigned int *data;

	data = (unsigned int*)pdata;
	i = bytes;

	/* Invalidate all cache data otherwise read won't return correct data. */
	qspi_ahb_invalid();
	invalidate_icache_all();
	invalidate_dcache_range(address, address + i);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	while (i > 0) {
		/* 128 is the circular TX Buffer depth. */
		k = i >= 128 ? 128 : i;

		/* 512-byte address boundary should not be crossed so write the minimum
		 * number between k and the left number of bytes in the current burst area.
		 */
		k = min(k, (int)(BURST_SIZE - address % BURST_SIZE));

		/* Compute the number of dwords. */
		m = k >> 2;

		quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xAA);
		quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0x554, 0x55);
		quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0xA0);

		/* TOOD Jira issue pending */
#ifndef CONFIG_DEBUG_S32_QSPI
		debug("quadspi_program_hyp() uses baremetal QuadSPI settings and definitions");
#endif
		//prepare write/program instruction
		QuadSPI.SFAR.R = address;
		quadspi_set_lut(60,
				QSPI_LUT(ADDR_DDR, 3, 0x18, CMD_DDR, 3, 0x00));
		quadspi_set_lut(61,
				QSPI_LUT(WRITE_DDR, 3, 2, CADDR_DDR, 3, 0x10));
		quadspi_set_lut(62, 0);
		// tx buffer
		QuadSPI.MCR.B.CLR_TXF = 1;
		QuadSPI.FR.R = 0x08000000;
		// load write data
		for (j = 0; j < m; j++)
			QuadSPI.TBDR.R = *data++;
		QuadSPI.IPCR.R = (15 << 24) | (k);	//send the write command
		while (QuadSPI.SR.R & QuadSPI_SR_BUSY_MASK) ;	//wait for cmd to be sent
		while ((quadspi_status_hyp() & 0x8000) == 0) ;	//check status, wait to be done

		address += k;
		i -= k;
	}

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;
}

void quadspi_read_hyp(void)
{
	quadspi_set_lut(0, QSPI_LUT(ADDR_DDR, 3, 24, CMD_DDR, 3, 0xA0));
	quadspi_set_lut(1, QSPI_LUT(DUMMY, 3, 15, CADDR_DDR, 3, 16));
	quadspi_set_lut(2, QSPI_LUT(STOP, 3, 0, READ_DDR, 3, 128));
	QuadSPI.BFGENCR.R = 0x00000;
}

/* TODO: is quadspi_read_ip_hyp not useful? */
void quadspi_read_ip_hyp(unsigned long address, unsigned long *dest,
			 unsigned long bytes)
{
	int i, j, k, m;
	QuadSPI.SFAR.R = address & 0xFFFFFFFE;	//clear less significative bit
	quadspi_set_lut(60, QSPI_LUT(ADDR_DDR, 3, 24, CMD_DDR, 3, 0xA0));
	quadspi_set_lut(61, QSPI_LUT(DUMMY, 3, 15, CADDR_DDR, 3, 16));
	quadspi_set_lut(62, QSPI_LUT(STOP, 0, 0, READ_DDR, 3, 128));

	i = bytes & 0xFFFFFFFE;
	k = i % 128;
	if (k == 0)
		k = 128;

	m = k >> 2;
	if (k & 3)
		m++;

	do {
		QuadSPI.MCR.B.CLR_RXF = 1;
		QuadSPI.FR.R = 0x10000;
		QuadSPI.IPCR.R = (15 << 24) | (k);	//fill the RX buffer
		while (QuadSPI.SR.R & QuadSPI_SR_BUSY_MASK) ;	//wait for cmd to be sent
		while (QuadSPI.RBSR.B.RDBFL != m) ;	//wait for buffer to be filled

		for (j = 0; j < m; j++)
			*dest++ = QuadSPI.RBDR[j].R;

		QuadSPI.SFAR.R = QuadSPI.SFAR.R + (k);
		i -= k;
		k = 128;
		m = 32;
	} while (i > 0);
}

unsigned int quadspi_status_hyp(void)
{
	unsigned int data;

	quadspi_send_instruction_hyp(CONFIG_SYS_FLASH_BASE + 0xAAA, 0x70);

	QuadSPI.SFAR.R = CONFIG_SYS_FLASH_BASE + 0x2;
	quadspi_set_lut(60, QSPI_LUT(ADDR_DDR, 3, 0x18, CMD_DDR, 3, 0x80));
	quadspi_set_lut(61, QSPI_LUT(DUMMY, 3, 15, CADDR_DDR, 3, 0x10));
	quadspi_set_lut(62, QSPI_LUT(STOP, 0, 0, READ_DDR, 3, 0x2));

	QuadSPI.MCR.B.CLR_RXF = 1;
	QuadSPI.FR.R = 0x10000;
	QuadSPI.IPCR.R = (15 << 24) | (2);	//fill the RX buffer
	while (QuadSPI.SR.R & QuadSPI_SR_BUSY_MASK) ;
	while (QuadSPI.RBSR.B.RDBFL != 1) ;

	data = QuadSPI.RBDR[0].R;
	return data;
}

void quadspi_send_instruction_hyp(unsigned int address, unsigned int cmd)
{
	QuadSPI.SFAR.R = (address & 0xfffffffe);
	quadspi_set_lut(60, QSPI_LUT(ADDR_DDR, 3, 0x18, CMD_DDR, 3, 0x00));
	quadspi_set_lut(61, QSPI_LUT(CMD_DDR, 3, cmd >> 8, CADDR_DDR, 3, 0x10));
	quadspi_set_lut(62, QSPI_LUT(STOP, 0, 0, CMD_DDR, 3, cmd));
	QuadSPI.IPCR.R = (15 << 24);
	while (QuadSPI.SR.R & QuadSPI_SR_BUSY_MASK) ;
}

void quadspi_set_lut(uint32_t index, uint32_t value)
{
	//Unlock the LUT
	do {
		QuadSPI.LUTKEY.R = 0x5AF05AF0;
		QuadSPI.LCKCR.R = 0x2;	//UNLOCK the LUT
	}
	while (QuadSPI.LCKCR.B.UNLOCK == 0);	//is this required??

	// SEQID 2 - Page Erase
	QuadSPI.LUT[index].R = value;

	//Lock the LUT
	QuadSPI.LUTKEY.R = 0x5AF05AF0;
	QuadSPI.LCKCR.R = 0x1;	//LOCK the LUT
	while (QuadSPI.LCKCR.B.LOCK == 0) ;
}

int do_qspinor_setup(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	printf("SD/eMMC is disabled. Hyperflash is active and can be used!\n");
	QSPI_setup_hyp();
	return 0;
}

volatile static bool flash_lock=1;
static int do_qspinor_prog(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	unsigned int fladdr, bufaddr, size;

	if (argc != 4) {
		printf("This command needs exactly three parameters (flashaddr "
		       "buffaddr and size).\n");
		return 1;
	}

	fladdr = simple_strtol(argv[1], NULL, 16);
	if (!is_flash_addr(fladdr, qspi_real_address))
		return 1;

	if (fladdr % FLASH_HALF_PAGE_SIZE != 0)
		printf("Address should be %d bytes aligned.\n",
		       FLASH_HALF_PAGE_SIZE);

	bufaddr = simple_strtol(argv[2], NULL, 16);
	size = simple_strtol(argv[3], NULL, 16);

	/* It is strongly recommended that a multiple of 16-byte half-pages be
	 * written and each half-page written only once. */
	if (size < FLASH_HALF_PAGE_SIZE || size % FLASH_HALF_PAGE_SIZE != 0) {
		printf("The written size must be multiple of %d.\n",
		       FLASH_HALF_PAGE_SIZE);
		return 1;
	}

	if (!flash_lock) {
		quadspi_program_hyp(fladdr, (uintptr_t)bufaddr, size);
	} else {
		printf("Flash write and erase operations are locked!\n");
	}
	return 0;
}

static int do_qspinor_erase(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	long addr_start;

	if (argc != 2) {
		printf("This command needs exactly one parameter\n");
		return 1;
	}

	addr_start = simple_strtol(argv[1], NULL, 16);
	if (!is_flash_addr(addr_start, qspi_real_and_all))
		return 1;

	if (!flash_lock)
		quadspi_erase_hyp(addr_start);
	else
		printf("Flash write and erase operations are locked!\n");
	return 0;
}

/* we only need our own SW protect until we implement proper protection via HW
 * mechanisms; until then we need not conflict with those commands
 */
#ifdef CONFIG_CUSTOM_CMD_FLASH
/* we clean (set to 0) the LUTs used for write and erase to make sure no
 * accidental writes or erases can happen
 */
void quadspi_rm_write_erase_luts(void)
{
	quadspi_set_lut(60, 0);
	quadspi_set_lut(61, 0);
	quadspi_set_lut(62, 0);
}

static int do_swprotect(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{

	if (argc != 2) {
		printf("This command needs exactly one parameter (on/off).\n");
		return 1;
	}

	if (!strcmp(argv[1],"on")) {
		quadspi_rm_write_erase_luts();
		flash_lock = 1;
		return 0;
	}

	if (!strcmp(argv[1],"off")) {
		flash_lock = 0;
		return 0;
	}

	printf("Unexpected parameter. This command accepts only 'on' and 'off'"
	       "as parameter.\n");
	return 1;
}

/* simple SW protection */
U_BOOT_CMD(
	protect, 2, 1, do_swprotect,
	"protect on/off the flash memory against write and erase operations",
	         "on\n"
	"    - enable protection and forbid erase and write operations\n"
	"protect off\n"
	"    - disable protection allowing write and erase operations\n"
	""
);

/* quadspi_erase_hyp */
U_BOOT_CMD(
	erase, 3, 1, do_qspinor_erase,
	"erase FLASH from address 'START'",
	"erase START / -1\n"
	"    - erase flash starting from START address\n"
	"    - if START=-1, erase the entire chip\n"
);
#else
	#warning "Using U-Boot's protect and erase commands, not our custom ones"
#endif


/* quadspi_erase_hyp */
U_BOOT_CMD(
	flwrite, 4, 1, do_qspinor_prog,
	"write a data buffer into hyperflash",
	"ADDR BUFF HEXLEN\n"
	"    - write into flash starting with address ADDR\n"
	"      the first HEXLEN bytes contained in the memory\n"
	"      buffer at address BUFF.\n"
	"      Note: all numbers are in hexadecimal format\n"
);

#ifdef CONFIG_ENV_IS_IN_FLASH

/*
Wrappers for our flash memory functions, since
common/env_flash.c's saveenv function expects those
with different signatures. Done to enable the saving
of u-boot's environment to flash memory. Temporary
solution until this driver is updated to adhere to
the U-boot Driver Model.
*/

int flash_sect_protect(int protect_on,unsigned long flash_begin_addr
	,unsigned long flash_end_addr){

	if (protect_on) {
		quadspi_rm_write_erase_luts();
		flash_lock = 1;
	}
	else {
		flash_lock = 0;
	}

	return 0;
}

int flash_sect_erase(unsigned long flash_begin_addr
	, unsigned long flash_end_addr){

	if (!is_flash_addr((long)flash_begin_addr, qspi_real_and_all))
		return 1;

	if (!flash_lock)
		quadspi_erase_hyp(flash_begin_addr);
	else
		return -1;

	return 0;
}

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

unsigned long flash_init(){

	int i;

	flash_info[0].size = CONFIG_SYS_MAX_FLASH_SECT * FLASH_SECTOR_SIZE;
	flash_info[0].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	flash_info[0].flash_id = 0;

	flash_info[0].start[0] = CONFIG_SYS_FLASH_BASE;
	flash_info[0].protect[0] = 0;

	for (i = 1; i < flash_info[0].sector_count; i++) {
		flash_info[0].start[i] = flash_info[0].start[i - 1] + CONFIG_ENV_SECT_SIZE; /* 256 KB */
		flash_info[0].protect[i] = 0;
	}

	return flash_info[0].size;
}

int write_buff(flash_info_t* info,uchar* data,
	ulong flash_addr ,ulong bytes){

	quadspi_program_hyp((unsigned int)flash_addr, (uintptr_t)data, (unsigned int)bytes);
	return 0;
}

#endif
