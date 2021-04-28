#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-Clause-3
#
# Copyright 2020 NXP
#
set -o errexit -o pipefail -o noclobber -o nounset

usage() {
	echo "Usage: $(basename "$0") -k <key_path> -d <device> --hse <hse_fw>"
	echo
	echo "    -k|--key    Full path key pair directory"
	echo "                NOTE: A new key pair will be created in the"
	echo "                      specified directory"
	echo "    -d|--dev    Full path to device (i.e. /dev/sdb)"
	echo "    --hse       Full path to HSE Firmware"
}

while [[ $# -gt 0 ]]; do
	case $1 in
		-k|--key)
			shift
			KEY_PATH=$1
			;;
		-d|--dev)
			shift
			DEV_PATH=$1
			;;
		--hse)
			shift
			HSE_PATH=$1
			;;
		-h|--help)
			usage
			exit
			;;
		*)
			echo "Unknown parameter: $1"
			usage
			exit 1
	esac
	shift
done

UBOOT_MAX_SIZE=1048576 

# switch to script dir
SCRIPT_PATH=${0%/*}
if [ "$0" != "$SCRIPT_PATH" ] && [ "$SCRIPT_PATH" != "" ]; then
	cd "$SCRIPT_PATH/.."
fi

# create the keys - fixed key pair for demo
mkdir -p "$KEY_PATH"

if [ ! -f "$KEY_PATH"/private.pem ]; then
echo "-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEA9HLi6YPRAzbT9dUG9E5PyMLj+RwA/bEDqOMM2TtwTqMuMpxx
BnAD+XVtlRAqifGCSSPuqxXlEafGHXgQ7UDxfhA2KVIqprz+UboIzWUA5i6+JjFD
hsK7JBPThvQerA/x3OnxAX/n4UT/++ql+mVUF3zgmFYRmAdRmZ0yO1R1yJ6SuNRP
GhIsNGOgoIOfBVWWFvGWYemxetn+8+v7oBWh74I1aDSW/6i6bzXFSpZZeG2hGrxO
Taf9JdqCw8EGJvcevl3/LMhce7KaTvJF1MATW8HDaDLhoZjyyZzBDrkRPC0HjozN
vFFDg7/0gRV31FL2q6TDnyq3cxruMTkarokUiQIDAQABAoIBAEH3txC5h/BMHYtb
/9JyR8mv/oG9cu8DlC9Bhrgg+D/gfZke6smH9Sqe7AGsKS6sxDIkxkXZVMYc9rQB
MOA2BHW2vkcMa4IvwpofkDyNo157Asmfxc9aQdKMnyruOlDIxMaPQ4DaGaOLUWGU
g1I9giIMizfRElJqtzSzXVjxhA+tZHnpxs5R4rabav8cVx89IAnrMFopsDTxn4sh
npUhdZHjXWhyiXBAaDFZ5WBB/yvSI2qGX0kDkhNe8rr9N35tAkB8G13YfYg2kOLn
AmhsWPuKOj1OzBwQeJ/t/DHN345oJP8uPeVEiq6z/skalVt2cMxCIxTFtXfK9zC7
2ttH12ECgYEA+0zgYpo9NpogaCEvouJ3/pD2DenfXhamGJ1nIWmQzZyiZrXFacGc
tj0w9OhWqqYEAiZevtNH7E0X43Cv4qDBClAWAMkF5it+BoVG4w1SuYx1sCICkhch
35fYiT0ZPkmXKdwv0q82awwG8T0NOod8bznYlKK8ITXIugcrs671uG0CgYEA+QU1
IvOtdRa+SmFnaBhEOmgll7wSKms2AYoedlr+nA4xT3AcpLv6srwxePRHZEecJhS7
6NMuBn+F8V4+AOB/T+Z5YjVE1J7Upt+HH0bwRNKyGKpeJWguOmpEWLVeYOliQfzG
Oa0LVFMIsGWBwzg6moMYiGXTU9YyZy10CLj1Mw0CgYBOLmOcqYmjxB2S58gvL6/P
NgAm4fzJA9jWzZl5LqOLJFDuDV0GguKjW7QwbVwRKScUGCXfzlGQ9YcaDkPaCNMl
xzahNeks65kApFQibVGwQj+W9W3BT4sNSetm/ugsla5+qm8CZ9pRQWOjh6/m5qHS
5Lc5vssytc3l+jchZqwxnQKBgDw4py4JshQT6PcB8Uj6PV3Gm/jC8b2yacAbj9xd
ix9gS0qDuVmZYDpjSq+Om8lDjB6lewAGOiY2JsVFXkdGA2PmP8qhSUEGHDUy73F/
7VEnhwxx/wya0jssQdUWH3neVvXQVHvUPl4pKnvGQIhei8WcGI272kx3C39qPZpy
aFHtAoGBAJHkdmjiN0arH3W/yjcSylBufrpQZ7ohhym7SP7HArfnA+v/SwlqBXwp
ralzDGa1o7FyMt/h5TuPrsBO0pFnUWii1thbTqcpevXOZTFeWF7U/BcXsjbxD5l1
RUrLYeUXLqUb4sC8q0CdCvNEUqnrPRfWH2olsXFbYA2tkx8KeUb9
-----END RSA PRIVATE KEY-----" > "$KEY_PATH"/private.pem
fi

if [ ! -f "$KEY_PATH"/public.pem ]; then
openssl rsa -in "$KEY_PATH"/private.pem -pubout > "$KEY_PATH"/public.pem
fi

# extract app code
dd if=u-boot.s32 of=u-boot-tosign.s32 bs=512 skip=1057

# find necessary padding for u-boot
PAD=$((UBOOT_MAX_SIZE - $(wc -c < u-boot-tosign.s32)))
dd if=/dev/zero bs=1 count="$PAD" >> u-boot-tosign.s32

# sign extracted app code after padding
openssl dgst -sha1 -sign "$KEY_PATH"/private.pem \
	-out u-boot.sign u-boot-tosign.s32

# verify the signature
openssl dgst -sha1 -verify "$KEY_PATH"/public.pem \
	-signature u-boot.sign u-boot-tosign.s32

# add everything to the sdcard
# write primary ivt and duplicate ivt
dd if=u-boot.s32 of="$DEV_PATH" bs=256 count=1 conv=notrunc,fsync
dd if=u-boot.s32 of="$DEV_PATH" bs=512 skip=8 seek=8 count=1 conv=notrunc,fsync

# write DCD
dd if=u-boot.s32 of="$DEV_PATH" bs=512 \
skip=9 seek=9 count=16 conv=notrunc,fsync

# write hse fw
dd if="$HSE_PATH" of="$DEV_PATH" bs=512 seek=32 conv=notrunc,fsync

# write signature
dd if=u-boot.sign of="$DEV_PATH" bs=512 seek=1040 conv=notrunc,fsync

# write app header
dd if=u-boot.s32 of="$DEV_PATH" bs=512 \
seek=1056 skip=1056 count=1 conv=notrunc,fsync

# write signed u-boot
dd if=u-boot-tosign.s32 of="$DEV_PATH" bs=512 seek=1057 conv=notrunc,fsync
