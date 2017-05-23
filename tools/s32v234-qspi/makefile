# Copyright 2015 Freescale Semiconductor, Inc.
# Copyright 2017 NXP
#
# generate an u-boot image which can be flashed and booted via QSPI
# Note: a pre-existing u-boot.s32 image is necessary

# The QSPI image expected by the bootrom looks like this:
#
#    0x0000  +----------------------------------+
#            |      QuadSPI OTFAD Key Area      |
#            |             512 bytes            |
#    0x0200  +----------------------------------+
#            | QuadSPI Configuration Parameters |
#            |             318 bytes            |
#   (0x033e) +----------------------------------+
#            |            (Free Area)           |
#            |             3266 bytes           |
#    0x1000  +----------------------------------+
#            |    Application Program Image     |
#            +----------------------------------+
#
# See figure 30-3. QuadSPI Memory Image Format from reference manual
# version 1.2_RC2 review (S32V234RM_rev1.2_RC2_review.pdf)
#
# we use $+ instead of $^ so the order of files is kept

flash_type := s26kl
# Uncomment this for s70fl01gs SPI flash or any other custom parameters definition
# flash_type := s70fl01gs

u-boot.s32.qspi: zero.512 qspi_params.bin zero.3266 ../../u-boot.s32
	cat $+ > $@

qspi_params.bin: qspi_params.o
	objcopy -O binary  --only-section=.rodata $^ $@

zero.512:
	dd if=/dev/zero of=$@ bs=1 count=512

zero.3266:
	dd if=/dev/zero of=$@ bs=1 count=3266

# it is OK to use the host compiler for this because
# the data structure in the source is using only uint8_t,
# thus it's endian agnostic
#
# this will break if we'll ever use *int16_t or *int32_t
qspi_params.o: qspi_params_$(flash_type).c
	gcc -c $^ -o $@

u-boot.s32.qspi.swab: u-boot.s32.qspi
	stat -c '%s' $^ | grep -o '[13579]$$' && dd if=/dev/zero of=zero.pad bs=1 count=1 || : > zero.pad
	cat $^ zero.pad > $^.even
	dd if=$^.even of=$@ conv=swab

clean:
	rm -f u-boot.s32.qspi zero.512 zero.3266 zero.pad u-boot.s32.qspi.swab qspi_params.bin qspi_params.o u-boot.s32.qspi.even

swab: u-boot.s32.qspi.swab

.PHONY: clean swab
