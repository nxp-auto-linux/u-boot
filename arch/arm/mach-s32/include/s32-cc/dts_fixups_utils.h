/* SPDX-License-Identifier: GPL-2.0+ or BSD-3-Clause */
/* Copyright 2024 NXP */

#ifndef __DTS_FIXUPS_UTILS_H
#define __DTS_FIXUPS_UTILS_H

#include <dm/ofnode.h>

ofnode ofnode_by_alias(const char *alias_fmt, u32 alias_id);
int apply_dm_hwconfig_fixups(void);
int apply_fdt_hwconfig_fixups(void *blob);
int apply_dm_quick_boot_fixups(void);

#endif /* __DTS_FIXUPS_UTILS_H */
