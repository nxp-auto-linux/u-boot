// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ddr_init.h"

static uint32_t ddrc_init_cfg(struct ddrss_config *config);
static uint32_t execute_training(struct ddrss_config *config);
static uint32_t load_register_cfg(size_t size, struct regconf cfg[]);
static void set_optimal_pll(struct ddrss_config *config);
static uint32_t load_image(uint32_t start_addr, size_t size, uint32_t image[]);

__attribute__ ((weak)) void store_csr(void) {}

/* Main method needed to initialize ddr subsystem. */
uint32_t ddr_init(void)
{
	uint32_t ret = NO_ERR;
	size_t i = 0;

	init_image_sizes();

	for (i = 0; i < ddrss_config_size; i++) {
		/* Init DDR controller based on selected parameter values */
		ret = ddrc_init_cfg(&configs[i]);
		if (ret != NO_ERR)
			return ret;

		/* Setup AXI ports parity */
		ret = set_axi_parity();
		if (ret != NO_ERR)
			return ret;

		/* Init PHY module */
		ret = execute_training(&configs[i]);
		if (ret != NO_ERR)
			return ret;

		/* Execute post training setup */
		ret = post_train_setup();
		if (ret != NO_ERR)
			return ret;
	}
	return ret;
}

/* Initialize ddr controller with given settings. */
static uint32_t ddrc_init_cfg(struct ddrss_config *config)
{
	uint32_t ret = NO_ERR;

	ret = load_register_cfg(config->ddrc_cfg_size, config->ddrc_cfg);
	return ret;
}

/* Execute phy training with given settings. 2D training stage is optional. */
static uint32_t execute_training(struct ddrss_config *config)
{
	uint32_t ret = NO_ERR;
	/* Apply DQ swapping settings */
	ret = load_register_cfg(config->dq_swap_cfg_size, config->dq_swap_cfg);
	if (ret != NO_ERR)
		return ret;

	/* Initialize phy module */
	ret = load_register_cfg(config->phy_cfg_size, config->phy_cfg);
	if (ret != NO_ERR)
		return ret;

	/* Load 1D imem image */
	UNLOCK_CSR_ACCESS;
	ret = load_image(IMEM_START_ADDR, config->imem_1d_size,
			 config->imem_1d);
	if (ret != NO_ERR)
		return ret;
	LOCK_CSR_ACCESS;

	/* Load 1D imem image */
	UNLOCK_CSR_ACCESS;
	ret = load_image(DMEM_START_ADDR, config->dmem_1d_size,
			 config->dmem_1d);
	if (ret != NO_ERR)
		return ret;
	LOCK_CSR_ACCESS;

	/* Configure PLL optimal settings */
	set_optimal_pll(config);

	LOCK_CSR_ACCESS;
	writel(0x00000009, 0x40380420);
	writel(0x00000001, 0x40380420);
	writel(0x00000000, 0x40380420);

	ret = wait_firmware_execution();
	UNLOCK_CSR_ACCESS;
	if (ret != NO_ERR)
		return ret;
	store_csr();

	/*
	 * Check if 2d training images have been initialized before executing
	 * the second training stage.
	 */
	if (config->imem_2d_size > 0 && config->dmem_2d_size > 0) {
		/* Load 2d imem image */
		UNLOCK_CSR_ACCESS;
		ret = load_image(IMEM_START_ADDR, config->imem_2d_size,
				 config->imem_2d);
		if (ret != NO_ERR)
			return ret;
		LOCK_CSR_ACCESS;

		/* Load 2d dmem image */
		UNLOCK_CSR_ACCESS;
		ret = load_image(DMEM_START_ADDR, config->dmem_2d_size,
				 config->dmem_2d);
		if (ret != NO_ERR)
			return ret;
		LOCK_CSR_ACCESS;

		/* Configure PLL optimal settings */
		set_optimal_pll(config);

		LOCK_CSR_ACCESS;
		writel(0x00000009, 0x40380420);
		writel(0x00000001, 0x40380420);
		writel(0x00000000, 0x40380420);

		ret = wait_firmware_execution();
		if (ret != NO_ERR)
			return ret;
		store_csr();
	}

	UNLOCK_CSR_ACCESS;
	/*  Load pie image after training has executed */
	ret = load_register_cfg(config->pie_cfg_size, config->pie_cfg);
	LOCK_CSR_ACCESS;
	return ret;
}

/* Load register array into memory. */
static uint32_t load_register_cfg(size_t size, struct regconf cfg[])
{
	uint32_t ret = NO_ERR;
	size_t i;

	for (i = 0; i < size; i++)
		writel(cfg[i].data, (uintptr_t)cfg[i].addr);
	return ret;
}

/* Load image into memory at consecutive addresses */
static uint32_t load_image(uint32_t start_addr, size_t size, uint32_t image[])
{
	uint32_t ret = NO_ERR;
	size_t i;

	for (i = 0; i < size; i++) {
		writel(image[i], (uintptr_t)start_addr);
		start_addr += sizeof(uint32_t);
	}
	return ret;
}

/* Ensure optimal settings for pll, depending on the memory type. */
static void set_optimal_pll(struct ddrss_config *config)
{
	switch (config->memory_type) {
	case LPDDR4:
		writel(0x00000021, 0x403816f0);
		writel(0x00000024, 0x40381708);
		break;
	case DDR3:
		writel(0x00000020, 0x403816f0);
		writel(0x00000124, 0x40381708);
		break;
	default:
		break;
	}
	writel(0x0000017f, 0x4038171c);
	writel(0x00000019, 0x403816dc);
}
