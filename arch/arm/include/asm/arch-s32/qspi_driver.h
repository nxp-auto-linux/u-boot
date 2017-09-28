/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef QSPI_DRIVER_H_
#define QSPI_DRIVER_H_

#include "S32V234.h"
/* TODO: use our own clean definitions; QuadSPI should be defined
 * inside u-boot code in u-boot style" */

/* TODO: we use hacks to adapt easier to u-boot's code; for instance,
 * uint32_t should be changed into u32 */
#define uint32_t u32

/* minimum size to write into hyperflash */
#define FLASH_HALF_PAGE_SIZE	16
#define FLASH_MIN_PROG_SIZE	8

#define QuadSPI_X QuadSPI_0
#define ARDB (*(volatile unsigned int *) 0x71000000)

void QSPI_setup_hyp(void);
unsigned int quadspi_status_hyp(void);
void quadspi_read_ip_hyp(unsigned long address, unsigned long *dest,
			 unsigned long bytes);
void quadspi_program_word_hyp(unsigned int address, unsigned int word);
void quadspi_program_hyp(unsigned int address, uintptr_t data,
			 unsigned int bytes);
void quadspi_program_dma_hyp(unsigned int address, unsigned int *data,
			     unsigned int bytes);
void quadspi_read_hyp(void);
void quadspi_erase_hyp(int address);
void quadspi_id_entry_hyp(void);
void quadspi_cfi_entry_hyp(void);
void quadspi_send_instruction_hyp(unsigned int address, unsigned int cmd);
void quadspi_program(unsigned long src, unsigned long base, unsigned long size);


////////////////// HELPER Functions ////////////////////////////////
void quadspi_set_lut(uint32_t index, uint32_t value);

/////////////////required defines to compile //////////////////////////////////
#define BURST_SIZE 		512
#define FLASH_PGSZ		128
#define FLASH_DMA_PGSZ		512

/* QUADSPI Instructions */
#define CMD        1
#define ADDR       2
#define DUMMY      3
#define MODE       4
#define MODE2      5
#define MODE4      6
#define READ       7
#define WRITE      8
#define JMP_ON_CS  9
#define ADDR_DDR   10
#define MODE_DDR   11
#define MODE2_DDR  12
#define MODE4_DDR  13
#define READ_DDR   14
#define WRITE_DDR  15
#define DATA_LEARN 16
#define CMD_DDR    17
#define CADDR      18
#define CADDR_DDR  19
#define STOP       0

#define QSPI_LUT(CMD1,PAD1,OP1,CMD0,PAD0,OP0)	((((CMD1)&0x3f)<<26)|(((PAD1)&3)<<24)|(((OP1)&0xff)<<16)|(((CMD0)&0x3f)<<10)|(((PAD0)&3)<<8)|((OP0)&0xff))

#define QuadSPI_SR_BUSY_SHIFT                (0)
#define QuadSPI_SR_BUSY_MASK                 ((1) << (QuadSPI_SR_BUSY_SHIFT))
#define QuadSPI_MCR_CLR_TXF_MASK             0x800u
#define QSPI_SR_RXWE_SHIFT                   (16)
#define QSPI_SR_RXWE_MASK                    ((1) << (QSPI_SR_RXWE_SHIFT))
#define QuadSPI_MCR_CLR_TXF_MASK             0x800u
#define QuadSPI_MCR_CLR_TXF_SHIFT            (11)
#define QuadSPI_SR_TXFULL_MASK               0x8000000u
#define QuadSPI_SR_TXFULL_SHIFT              (27)

#endif /* QSPI_DRIVER_H_ */
