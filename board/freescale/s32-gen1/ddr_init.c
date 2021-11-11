// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
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

static u32 ddrc_init_cfg(const struct ddrss_config *config);
static u32 execute_training(const struct ddrss_config *config);
static u32 load_phy_image(u32 start_addr, size_t size,
			       const u16 image[]);

/* Main method needed to initialize ddr subsystem. */
u32 ddr_init(void)
{
	u32 ret = NO_ERR;
	size_t i;

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
		ret = post_train_setup((u8)(STORE_CSR_MASK |
						 INIT_MEM_MASK));
		if (ret != NO_ERR)
			return ret;
	}
	return ret;
}

/* Initialize ddr controller with given settings. */
static u32 ddrc_init_cfg(const struct ddrss_config *config)
{
	u32 ret = NO_ERR;

	ret = load_register_cfg(config->ddrc_size, config->ddrc);
	return ret;
}

/* Execute phy training with given settings. 2D training stage is optional. */
static u32 execute_training(const struct ddrss_config *config)
{
	u32 ret = NO_ERR;
	/* Apply DQ swapping settings */
	ret = load_dq_cfg(config->dq_swap_size, config->dq_swap);
	if (ret != NO_ERR)
		return ret;

	/* Initialize phy module */
	ret = load_register_cfg_16(config->phy_size, config->phy);
	if (ret != NO_ERR)
		return ret;

	/* Load 1D imem image */
	writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	ret = load_phy_image(IMEM_START_ADDR, config->imem_1d_size,
			     config->imem_1d);
	if (ret != NO_ERR)
		return ret;
	writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);

	/* Load 1D imem image */
	writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	ret = load_phy_image(DMEM_START_ADDR, config->dmem_1d_size,
			     config->dmem_1d);
	if (ret != NO_ERR)
		return ret;
	writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);

	/* Configure PLL optimal settings */
	set_optimal_pll();

	writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	writel(APBONLY_RESET_STALL_MASK, APBONLY_MICRORESET);
	writel(APBONLY_STALL_TO_MICRO_MASK, APBONLY_MICRORESET);
	writel(APBONLY_MICRORESET_CLR_MASK, APBONLY_MICRORESET);

	ret = wait_firmware_execution();
	writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	if (ret != NO_ERR)
		return ret;

	/*
	 * Check if 2d training images have been initialized before executing
	 * the second training stage.
	 */
	if (config->imem_2d_size > 0U && config->dmem_2d_size > 0U) {
		/* Load 2d imem image */
		writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
		ret = load_phy_image(IMEM_START_ADDR, config->imem_2d_size,
				     config->imem_2d);
		if (ret != NO_ERR)
			return ret;
		writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);

		/* Load 2d dmem image */
		writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
		ret = load_phy_image(DMEM_START_ADDR, config->dmem_2d_size,
				     config->dmem_2d);
		if (ret != NO_ERR)
			return ret;
		writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);

		/* Configure PLL optimal settings */
		set_optimal_pll();

		writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
		writel(APBONLY_RESET_STALL_MASK, APBONLY_MICRORESET);
		writel(APBONLY_STALL_TO_MICRO_MASK, APBONLY_MICRORESET);
		writel(APBONLY_MICRORESET_CLR_MASK, APBONLY_MICRORESET);

		ret = wait_firmware_execution();
		if (ret != NO_ERR)
			return ret;
	}

	writel(UNLOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	/*  Load pie image after training has executed */
	ret = load_register_cfg_16(config->pie_size, config->pie);
	writel(LOCK_CSR_ACCESS, MICROCONT_MUX_SEL);
	return ret;
}

/* Load register array into memory. */
u32 load_register_cfg_16(size_t size, const struct regconf_16 cfg[])
{
	size_t i;

	for (i = 0; i < size; i++)
		writew(cfg[i].data, (uintptr_t)cfg[i].addr);

	return NO_ERR;
}

/* Load register array into memory. */
u32 load_register_cfg(size_t size, const struct regconf cfg[])
{
	size_t i;

	for (i = 0; i < size; i++)
		writel(cfg[i].data, (uintptr_t)cfg[i].addr);

	return NO_ERR;
}

/* Load dq config array into memory. */
u32 load_dq_cfg(size_t size, const struct dqconf cfg[])
{
	size_t i;

	for (i = 0; i < size; i++)
		writel(cfg[i].data, (uintptr_t)cfg[i].addr);

	return NO_ERR;
}

/* Load image into memory at consecutive addresses */
static u32 load_phy_image(u32 start_addr, size_t size,
			       const u16 image[])
{
	size_t i;
	u32 current_addr = start_addr;

	for (i = 0; i < size; i++) {
		writel(image[i], (uintptr_t)current_addr);
		current_addr += (u32)sizeof(u32);
	}
	return NO_ERR;
}

/* Ensure optimal phy pll settings. */
void set_optimal_pll(void)
{
	/* Configure phy pll for 3200MTS data rate */
	writel(PLLCTRL1_VALUE, MASTER_PLLCTRL1);
	writel(PLLTESTMODE_VALUE, MASTER_PLLTESTMODE);
	writel(PLLCTRL4_VALUE, MASTER_PLLCTRL4);
	writel(PLLCTRL2_VALUE, MASTER_PLLCTRL2);
}
