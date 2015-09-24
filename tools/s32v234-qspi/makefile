# generate an u-boot image which can be flashed and booted via QSPI
# Note: a pre-existing u-boot.s32 image is necessary

# we use $+ instead of $^ so the order of files is kept
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
qspi_params.o: qspi_params.c
	gcc -c $^ -o $@
