#!/bin/bash

#remove previous nop file
rm -f nop.bin

for iter in {1..29378}
do
	echo -n -e '\x1F\x20\x03\xD5' >> nop.bin
done
