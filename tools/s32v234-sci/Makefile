
# Path to 32bit ARM compiler
COMP=TODO_PATH_TO_ARM_COMPILER
# Secure callback image final address
# Obtained from uboot compiling output - 4th parameter of "HAB Blocks:"
BASE_ADDR=1048940544 # 0x3e859000
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
