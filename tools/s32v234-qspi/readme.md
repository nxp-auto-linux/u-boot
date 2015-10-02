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
