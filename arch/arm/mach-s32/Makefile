# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# (C) Copyright 2011 Freescale Semiconductor, Inc.
# Copyright 2019-2020 NXP

MKIMAGEFLAGS_u-boot.s32 = -T $(CONFIG_MKIMAGE_T)
u-boot.s32: MKIMAGEOUTPUT = u-boot.s32.log
u-boot.s32: u-boot.bin FORCE
	$(eval DTB_RSRVD_SIZE = $(shell \
		echo $$((${CONFIG_SYS_TEXT_BASE} - ${CONFIG_DTB_SRAM_ADDR}))))
	$(eval DTB_SIZE = $(shell stat --printf="%s" u-boot.dtb))

	@if [ ${DTB_SIZE} -gt ${DTB_RSRVD_SIZE} ]; then \
		echo "DTB exceeds the reserved space of" \
		"${DTB_RSRVD_SIZE} bytes between CONFIG_SYS_TEXT_BASE" \
		"and CONFIG_DTB_SRAM_ADDR"; \
		false; \
	fi

	@dd if=u-boot.dtb of=u-boot-with-dtb.bin bs=${DTB_RSRVD_SIZE} count=1
	@dd if=u-boot.bin of=u-boot-with-dtb.bin bs=${DTB_RSRVD_SIZE} seek=1
	@mv u-boot-with-dtb.bin u-boot.bin
	$(call if_changed,mkimage)
