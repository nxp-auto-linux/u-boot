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
		
Where to get TRACE32 script
---------------------------
The treerunner-hyper.cmm script is created to flash the u-boot image
into hyperflash on S32V234.

The treerunner-hyper.cmm script can be found in t32-hyperflash-prog-example tool.
On request t32-hyperflash-prog-example tool can be provided.

How to boot the QSPI image from Lauterbach
------------------------------------------
Download the lastest TRACE32 Software from lauterbach.com and install it.

Open TRACE32 PowerView for ARM:
		<t32_dir>/bin/<machine>/t32marm-qt
		
Edit the treerunner-hyper.cmm script to use the QSPI image generated from 
previous steps. (line: flashfile.load <path_to_image>/u-boot.s32.qspi 0x0)

Flash into hyperflash using the treerunner-hyper.cmm script from Lauterbach:
	- In TRACE32 PowerView for ARM: File -> Run Script 
										 -> choose treerunner-hyper.cmm
	- Wait for the script to finish.
	
Reset the board. Enjoy your QSPI loaded u-boot.

Flashing on S32V234 with hyper256_halo.bin
------------------------------------------
If the treerunner-hyper.cmm script is using the hyper256_halo.bin flash driver,
then the resulting image must be byte swapped before being flashed to be usable
and readable by the S32V234 BootROM code.

In order to generate such an image you can run:
		make swab

The resulting image will be called `u-boot.s32.qspi.swab`.
