/*
 * (C) Copyright 2016-2018, NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef QSPI_COMMON_H_
#define QSPI_COMMON_H_

void qspi_iomux(void);

int do_qspinor_setup(cmd_tbl_t *cmdtp, int flag, int argc,
		     char * const argv[]);

#endif /* QSPI_COMMON_H_ */
