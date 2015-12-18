# The final U-boot image is expected to look like this 
#
#    0x3E802000    +----------------------------------+
#                  |           Cortex-M4 binary       |
#                  |              5368 bytes          |
#    0x3E8034F8    +----------------------------------+
#                  |            (Freee Area)          |
#                  |            117512 bytes          |
#    0x3E820000    +----------------------------------+
#                  |            u-boot.bin            |
#                  +----------------------------------+
#
# we use $+ instead of $^ so the order of files is kept

all: u-boot.s32.smp

nop.bin:
	./nop_generator.sh

u-boot.bin.smp: exec_m4.bin nop.bin  ../../u-boot.bin
	cat $+ > $@

u-boot.s32.smp: u-boot.bin.smp
	../mkimage -n ../../board/freescale/s32v234evb/s32v234evb.cfg.cfgtmp -T imximage  -e 0x3e802000 -d $^ $@

clean:
	rm -f u-boot.bin.smp u-boot.s32.smp nop.bin


.PHONY: clean
