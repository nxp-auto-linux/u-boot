/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020 NXP
 */
#ifndef S32_DMA_MEM_H
#define S32_DMA_MEM_H

/**
 * @brief Clear a memory region of size 'size' starting with address 'addr'
 * (using DMA)
 *
 * @param[in] addr Start address of memory to clear
 * @param[in] size Size of memory area to clear
 *
 * @return  0 on error or size of memory cleared on success
 */
int dma_mem_clr(int addr, int size);

#endif
