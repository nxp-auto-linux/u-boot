/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2022 NXP
 */
#ifndef S32GEN1_CLK_FUNCS_H
#define S32GEN1_CLK_FUNCS_H
#include <clk-uclass.h>
#include <inttypes.h>

int s32gen1_set_parent(struct clk *c, struct clk *p);

#endif /* S32GEN1_CLK_FUNCS_H */
