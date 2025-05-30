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
*   `CONFIG_SYS_LOAD_RAM_OFFSET`: OS image offset in RAM (default: 0x1000_0000).
*   `CONFIG_DTB_RAM_OFFSET`: OS DTB offset in RAM (default: 0x1300_0000).
*   `CONFIG_RAMDISK_RAM_OFFSET`: Root filesystem offset in RAM (default: 0x1400_0000).

Other customizable options:
*   `CONFIG_MEM_BOOT`: Enable this option if the kernel images are already
  loaded in memory.
*   `CONFIG_DEBUG_UART_BASE`: UART registers base addressv (default: 0x4A03_0000).
