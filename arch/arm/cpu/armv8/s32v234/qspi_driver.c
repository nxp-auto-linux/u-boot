/*
 * (C) Copyright 2015, 2016
 * Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/qspi_driver.h>
#include <asm/arch/siul.h>
#include <asm/io.h>

void QSPI_setup_hyp()
{
#warning "QSPI_setup_hyp() SIUL2 settings are not tested"
	/* 0x0020d700 CTRL_CLK_BASE */
	writel(SIUL2_PK6_MSCR_MUX_MODE_QSPI_A_SCK | SIUL2_PORT_MSCR_CTRL_QSPI_CLK_BASE, SIUL2_MSCRn(SIUL2_PK6_MSCR)); /* 0x0020C301; QSPI0_A_SCK - V25 - PK6 */
	writel(SIUL2_PK5_MSCR_MUX_MODE_QSPI_A_CS0 | SIUL2_PORT_MSCR_CTRL_QSPI_CLK_BASE, SIUL2_MSCRn(SIUL2_PK5_MSCR)); /* 0x0020FF01;//0x0029F301; //QSPI0_A_CS0 w Pull up enabled - U25 - PK5 */

	/* A_DQS */
	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DQS, SIUL2_MSCRn(SIUL2_PK7_MSCR));	// 0x0008D700; //QSPI0_A_DQS - U22 - PK7
	writel(SIUL2_PK7_IMCR_MUX_MODE_QSPI_A_DQS, SIUL2_IMCRn(SIUL2_PK7_IMCR_QSPI_A_DQS));

	/* note: an alternative A_DATA0_3/4_7 CTRL is 0x0028C301/0x0028C302 */
	/* A_DATA 0-3 */
	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA0_3, SIUL2_MSCRn(SIUL2_PK11_MSCR)); //QSPI0_A_D3 - V22 - PK11
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PK11_IMCR_QSPI_A_DATA3));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA0_3, SIUL2_MSCRn(SIUL2_PK10_MSCR));	//QSPI0_A_D2 - V21 - PK10
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PK10_IMCR_QSPI_A_DATA2));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA0_3, SIUL2_MSCRn(SIUL2_PK9_MSCR));	//QSPI0_A_D1 - U23 - PK9
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PK9_IMCR_QSPI_A_DATA1));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA0_3, SIUL2_MSCRn(SIUL2_PK8_MSCR));	//QSPI0_A_D0 - V23 - PK8
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PK8_IMCR_QSPI_A_DATA0));

	/* A_DATA 4-7 */
	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA4_7, SIUL2_MSCRn(SIUL2_PL2_MSCR));	//QSPI0_B_D3?      - R21 - PL2
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PL2_IMCR_QSPI_A_DATA7));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA4_7, SIUL2_MSCRn(SIUL2_PL1_MSCR));	//QSPI0_B_D2? - U24 - PL1
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PL1_IMCR_QSPI_A_DATA6));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA4_7, SIUL2_MSCRn(SIUL2_PL0_MSCR));	//QSPI0_B_D1? - U21 - PL0
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PL0_IMCR_QSPI_A_DATA5));

	writel(SIUL2_PORT_MSCR_CTRL_QSPI_A_DATA4_7, SIUL2_MSCRn(SIUL2_PK15_MSCR));	//QSPI0_B_D0? - W23 - PK15
	writel(SIUL2_PORT_IMCR_MUX_MODE_QSPI_A_DATA0_7, SIUL2_IMCRn(SIUL2_PK15_IMCR_QSPI_A_DATA4));

	writel(SIUL2_PK13_MSCR_MUX_MODE_QSPI_CK2 | SIUL2_PORT_MSCR_CTRL_QSPI_CLK_BASE, SIUL2_MSCRn(SIUL2_PK13_MSCR));	//QSPI0_CK2 - B_SCK? V24 - PK13

#ifdef CONFIG_DEBUG_S32V234_QSPI_QSPI
	#warning "QSPI_setup_hyp() HAS NO QuadSPI settings and definitions"
#else
	#warning "QSPI_setup_hyp() uses baremetal QuadSPI settings and definitions"
	QuadSPI.MCR.B.MDIS = 0;	//clear MDIS bit
	QuadSPI.BUF0IND.R = 0x0;	//set AHB buffer size (64bits)
	QuadSPI.SFA1AD.R = FLASH_BASE_ADR + 0x4000000;	//set top address of FA1 (size 512Mbit)
	QuadSPI.SFA2AD.R = FLASH_BASE_ADR + 0x4000000;	//set top address of FA2 (size 0Mbit)
	QuadSPI.SFB1AD.R = FLASH_BASE_ADR2 + 0x4000000;	//set top address of FB1 (size 512Mbit)
	QuadSPI.SFB2AD.R = FLASH_BASE_ADR2 + 0x4000000;	//set top address of FB2 (size 0Mbit) 0x203FFFFF

	QuadSPI.BUF0IND.R = 0x100;	/* buffer0 size 512 bytes */
	QuadSPI.BUF1IND.R = 0x200;	/* buffer1 size 0 bytes */
	QuadSPI.BUF2IND.R = 0x200;	/* buffer2 size 0 bytes */
	QuadSPI.BUF3CR.R = 0x80000000;	/* All masters use buffer 3 */

	QuadSPI.SFAR.R = FLASH_BASE_ADR;
	QuadSPI.MCR.B.DDR_EN = 1;
	QuadSPI.MCR.B.DQS_EN = 1;
	QuadSPI.SFACR.R = 0x00010003;
	QuadSPI.MCR.B.DQS_LAT_EN = 1;
	QuadSPI.FLSHCR.B.TDH = 1;

	/* Set-up Read command for hyperflash */
	quadspi_read_hyp();	// The command will be used by default for any AHB access to the flash memory
#endif
}				/* QSPI_setup */

//sector = -1 means chip erase
void quadspi_erase_hyp(int sector)
{
	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xAA);
	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0x554, 0x55);

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0x80);
	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xAA);

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0x554, 0x55);

	if (sector == -1)
		quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0x10);
	else
		quadspi_send_instruction_hyp((sector & 0xfffffffe), 0x30);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;
}

void quadspi_program_word_hyp(unsigned int address, unsigned int word)
{
	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xAA);
	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0x554, 0x55);

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xA0);
	quadspi_send_instruction_hyp(address, word);

	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;
}

void quadspi_program_hyp(unsigned int address, unsigned int *data,
			 unsigned int bytes)
{
	int i, j, k, m;		// i: number total of bytes to flash    // k: number of bytes to flash at the next command      // m :number of word (16 bits) to flash at the next command
	//check status, wait to be ready
	while ((quadspi_status_hyp() & 0x8000) == 0) ;

	address = (address & 0xfffffffe);
	i = bytes & 0xFFFFFFFE;
	if ((address % 128) != 0) {
		k = 128 - (address % 128);
	} else if (i > 128)
		k = 128;
	else
		k = i;

	m = k >> 2;
	if (k & 3)
		m++;

	do {

		quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xAA);
		quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0x554, 0x55);
		quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0xA0);

#ifndef CONFIG_DEBUG_S32V234_QSPI
#warning "quadspi_program_hyp() uses baremetal QuadSPI settings and definitions"
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
		if (i > 128)
			k = 128;
		else
			k = i;
//              k = 128;
		m = 32;
	} while (i > 0);

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

	quadspi_send_instruction_hyp(FLASH_BASE_ADR + 0xAAA, 0x70);

	QuadSPI.SFAR.R = FLASH_BASE_ADR + 0x2;
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

static int do_qspinor_setup(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	QSPI_setup_hyp();
	return 0;
}

static int do_qspinor_empty(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	printf("Not yet implemented\n");
	return 0;
}
U_BOOT_CMD(
	qspinor_setup, 1, 1, do_qspinor_setup,
	"setup qspi pinmuxing and qspi registers for access to hyperflash",
	""
);

/* quadspi_erase_hyp */
U_BOOT_CMD(
	qspinor_program, 4, 1, do_qspinor_empty,
	"write a data buffer into hyperflash",
	"qspinor_program ADDR BUFF LEN\n"
	"    - write into flash starting with address ADDR\n"
	"      the first LEN bytes contained in the memory\n"
	"      buffer at address BUFF.\n"
);

/* quadspi_erase_hyp */
U_BOOT_CMD(
	qspinor_erase, 2, 1, do_qspinor_empty,
	"erase the given hyperflash sector (maybe start address?)",
	"qspinor_erase N\n"
	"    - erase flash sector N\n"
);
