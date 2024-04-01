.. SPDX-License-Identifier: GPL-2.0
.. Copyright 2023-2024 NXP

S32-CC U-Boot Memory map
========================

==================  ==============  ==============================  =======
Config              Default Value   Description                     Size
==================  ==============  ==============================  =======
SYS_DATA_BASE           0xFFA0_0000     Base address of Data Section	-
SYS_MALLOC_LEN		0xFFA0_41B0     Start of Malloc (Heap) area	32KB
SYS_INIT_SP_ADDR	0xFFA0_C1B0     SP start address		16KB
DTB_ADDR		0xFFA9_8000     Address of Device-Tree-Blob	60KB
SYS_TEXT_BASE		0xFFAA_0000     Base address of Text Section	-
