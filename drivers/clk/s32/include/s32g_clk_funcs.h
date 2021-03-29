/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2021 NXP
 */
#ifndef S32G_CLK_FUNCS_H
#define S32G_CLK_FUNCS_H
#include <s32gen1_clk_funcs.h>

struct s32gen1_clk *s32g_get_plat_cc_clock(uint32_t id);
struct s32gen1_clk *s32g_get_plat_clock(uint32_t id);

#endif /* S32G_CLK_FUNCS_H */
