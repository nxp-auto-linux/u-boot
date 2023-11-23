// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

#include <common.h>
#include <dm/device_compat.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

static inline phys_addr_t pfe_hw_block_pa_of(phys_addr_t addr, u32 pa_off)
{
	if (check_uptr_overflow(addr, (uintptr_t)pa_off))
		return 0U;

	return addr + pa_off;
}

static inline phys_addr_t pfe_hw_block_pa(struct pfe_hw *pfe, u32 pa_off)
{
	return pfe_hw_block_pa_of(pfe->cbus_baseaddr, pa_off);
}

/* external API */
int pfe_hw_hif_chnl_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *cfg)
{
	return pfe_hw_hif_init(&ext->hw->hif, ext->hw->hif_base, ext->hw->hif_chnl, cfg);
}

int pfe_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *hw_cfg)
{
	struct pfe_hw *pfe;
	int ret = 0;

	pfe = kzalloc(sizeof(*pfe), GFP_KERNEL);
	if (!pfe)
		return -ENOMEM;

	ext->hw = pfe;
	pfe->cfg = hw_cfg;

	/* Map CBUS address space */
	pfe->cbus_baseaddr = hw_cfg->cbus_base;
	pfe->base = pfe_hw_phys_addr(hw_cfg->cbus_base);
	if (!pfe->base) {
		dev_err(hw_cfg->dev, "Can't map PPFE CBUS\n");
		ret = -ENOMEM;
		goto exit;
	}

	pfe->hif_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_HIF_BASE_ADDR));
	if (!pfe->hif_base) {
		dev_err(hw_cfg->dev, "Can't get PFE HIF base address\n");
		ret = -EINVAL;
		goto exit;
	}

	pfe->bdr_buffers_va = pfe_hw_phys_addr(hw_cfg->bdrs_addr);
	pfe->bdr_buffers_size = hw_cfg->bdrs_size;

	/* Create HW components */
	pfe->hif_chnl = hw_cfg->hif_chnl_id;
	pfe->ihc_hif_id = hw_cfg->ihc_hif_id;
	pfe->master_hif_id = hw_cfg->master_hif_id;

	return 0;

exit:
	pfe_hw_remove(ext);
	return ret;
}

void pfe_hw_remove(struct pfe_hw_ext *ext)
{
	struct pfe_hw *pfe = ext->hw;

	if (!pfe)
		return;

	pfe->cbus_baseaddr = 0x0ULL;
	kfree(pfe);
	ext->hw = NULL;
}

void pfe_hw_print_stats(struct pfe_hw_ext *ext)
{
	printf("HIF statistics\n");

	if (ext->hw_chnl)
		pfe_hw_chnl_print_stats(ext->hw_chnl);
	else
		printf("Not available, HIF channel is not created yet\n");
}

int pfe_hw_grace_reset(struct pfe_hw_ext *ext)
{
	int ret = 0;

	printf("HIF graceful reset\n");

	return ret;
}
