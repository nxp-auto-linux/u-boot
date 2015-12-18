Cortex-M4 SDHC booting image generation
=======================================

In order to obtain an image suitable for SDHC booting from M4, you need
an M4 binary that starts the A53 master core or if you need SMP support,
the A53 slave cores (1, 2 and 3) should also be started.


How to generate the SDHC image
==============================
After generating the Cortex-M4 elf, extract the binary. The current
folder contains an M4 binary that starts all the A53 cores which start
to execute the u-boot.

Build u-boot as usual and afterwards from the current directory run:

`make clean`
`make all`
`ls u-boot.s32.smp`

The resulting image will be called `u-boot.s32.smp`.
Write the image on SD as the Release Notes document specifies.
