# Creates Secure Callback Image. The Secure Callback function included
# in the image will be executed by the BootROM code on the M4 Core.

# Path to 32bit ARM cross-compiler
COMP=TODO_PATH_TO_ARM_COMPILER
# Secure callback image final address, located right after the u-boot image.
# This value can be obtained during u-boot verbose (V=1) build when
# SECURE_CALLBACK config is enabled. The address is the last value of the
# "HAB Blocks:" compiling output line:
# e.g. "HAB Blocks:   3e81f000 00000000 0003b000 3e85a000"
BASE_ADDR=1048977408 # 0x3e862000
# Secure callback function address
SCADDR=$(($BASE+0x10))
SCADDR=$((SCADDR | 1)) # thumb mode address
# CMAC address
MACADDR=$(($BASE+0x10+$SCSIZE))

sci: sec_call genHeader.pl genCMAC.pl
	# Generate header
	perl genHeader.pl $(BASE_ADDR) $(shell stat -c%s sec_call) > sci.bin
	# Add callback function
	cat sec_call >> sci.bin
	# Generate and append CMAC
	#cat sci.bin | ruby genCMAC.rb > sciCMAC -- needs cmac-rb
	cat sci.bin | perl genCMAC.pl > sciCMAC # -- needs Digest::CMAC
	cat sciCMAC >> sci.bin

sec_call: sec_call.c
	$(COMP)gcc -mthumb -c sec_call.c -fpic -o sec_call.o
	$(COMP)objcopy -j .text -O binary sec_call.o sec_call


clean:
	rm sec_call sec_call.o sciCMAC sci.bin
