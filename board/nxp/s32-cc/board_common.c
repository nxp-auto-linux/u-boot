// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2024 NXP
 */
#include <common.h>
#include <image.h>
#include <s32-cc/scmi_reset_agent.h>

void board_cleanup_before_linux(void)
{
	int ret, skip;

	skip = env_get_yesno("skip_scmi_reset_agent");
	if (skip == 1)
		return;

	ret = scmi_reset_agent();
	if (ret)
		pr_err("Failed to reset SCMI agent's settings\n");
}
