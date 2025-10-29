/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024-2025 NXP
 */

#ifndef __S32_FDT_H__
#define __S32_FDT_H__

int ft_fixup_memory(void *blob, struct bd_info *bd);
int ft_fixup_atf(void *new_blob);
int ft_add_reserved_memory_node(void *new_blob, const char *name);

#endif
