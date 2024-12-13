.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause

s32n79sim
==========

U-Boot for the NXP S32N79 simulator

Quick Start
-----------

- Build U-Boot
- Memory layout related build options

Build U-Boot
------------

.. code-block:: bash

   $ make s32n79sim_defconfig
   $ make

Memory layout related build options
-----------------------------------

The memory allocated to TF-A, U-Boot and to the running OS is configurable. The
following configuration options can be used in this scope:
*   `CONFIG_BOARD_DRAM_BASE`: Specifies the starting address of the DRAM
  allocated to the current instance.
*   `CONFIG_BOARD_DRAM_SIZE`: Specifies the size of the DRAM allocated to the
  current instance.
*   `CONFIG_SYS_LOAD_ADDR`: Specifies the address of the OS image.
*   `CONFIG_DTB_ADDR`: Specifies the address of the OS device tree blob.
*   `CONFIG_RAMDISK_ADDR`: Specifies the address of the OS ramdisk.

Other customizable options:
*   `CONFIG_MEM_BOOT`: Enable this option if the kernel images are already
  loaded in memory.
