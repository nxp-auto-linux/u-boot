#!/bin/bash

# Script used to sign a file with its generated MAC (the MAC will be appended
# to the end of the file). To generate the MAC for a file, one can either use
# an external AES-128 CMAC generator, or the CSE3 CMAC generation command that
# we provide in u-boot. For more details regarding the second case, see Release
# Notes.
# Use the string (MAC in hex format) that is generated as the first argument of
# this script.
#
# Usage ./<signmac.sh> <MAC> <unsigned file> <signed file>
#
# For example, to sign uImage, use a command like the example below:
#
# ./signmac.sh "6c63e25942d296c59a7d7448c0524a3a" <SRC_PATH>/uImage <DST_PATH>/uImage
#
# where the hex string given as the first parameter is the previously generated
# MAC, <SRC_PATH> is the path to the unsigned uImage and <DST_PATH> is the
# path to the signed uImage.

if [ $# -ne 3 ]
    then
        echo "Usage ./$0  <MAC> <unsigned file> <signed file>"
        exit 1
fi

cp $2 $3
echo $1 | xxd -r -p >> $3
