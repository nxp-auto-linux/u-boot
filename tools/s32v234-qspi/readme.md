QSPI image generation
=====================

In order to generate an image suitable for QSPI booting on S32V234 an already
existing `u-boot.s32` image is necessary.

The generated image will have the name `u-boot.s32.qspi`.

How to generate the QSPI image
------------------------------
# build u-boot as usual and from this directory run
make
ls u-boot.s32.qspi

Using the generated image
-------------------------

Flash into hyperflash using the treerunner-hyper.cmm script from Lauterbach.

Reset the board. Enjoy your QSPI loaded u-boot.

Flashing on S32V234 with hyper256_halo.bin
------------------------------------------
If the treerunner-hyper.cmm script is using the hyper256_halo.bin flash driver,
then the resulting image must be byte swapped before being flashed to be usable
and readable by the S32V234 BootROM code.

In order to generate such an image you can run:
make swab

The resulting image will be called `u-boot.s32.qspi.swab`.
