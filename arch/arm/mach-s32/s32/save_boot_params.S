/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2026 NXP
 */
#include <linux/linkage.h>

/*
 * save_boot_params() - stash the boot parameters TF-A passes at reset.
 *
 * start.S branches here from `reset:` with SP == 0, i.e. before U-Boot has set
 * up a stack, so this routine must not touch the stack. TF-A passes the
 * NT_FW_CONFIG (FDT) address in x0; store it into nt_fw_dtb and return to the
 * boot flow. nt_fw_dtb lives in .data (so it survives the .bss clear) and is
 * reached PC-relative (adrp/add), which resolves correctly at the pre-relocation
 * load address; relocation later copies .data, preserving the value.
 *
 * Implemented in assembly on purpose: it needs no stack, and being assembly it
 * is inherently outside C code-coverage instrumentation (which would otherwise
 * add a stack-using prologue here and fault). Overrides the weak
 * save_boot_params in arch/arm/cpu/armv8/start.S.
 */
ENTRY(save_boot_params)
	adrp	x1, nt_fw_dtb
	add	x1, x1, :lo12:nt_fw_dtb
	str	x0, [x1]
	b	save_boot_params_ret		/* return to start.S; no stack used */
ENDPROC(save_boot_params)
