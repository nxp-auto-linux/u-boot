#! /usr/bin/perl -w
use strict;
# Generates Secure Callback Image header

# Secure callback image final address
my $base_addr=$ARGV[0];
# Secure callback function size
my $sc_len=$ARGV[1];
#open(my $out, '>:raw', 'sci.bin') or die "Unable to open: $!";
print pack("C", 0xDF); # Tag
# Whole image size: Secure Callback + Header + CMAC
# Write this value in u-boot config file under SECURE_CALLBACK config
print pack("n", $sc_len+0x20);
print pack("C", 0x50); # Version
# Secure Callback function address - thumb mode
print pack("V", ($base_addr+0x10)|1);
# CMAC address
print pack("V", $base_addr+0x10+$sc_len);
# Authentication Length = Image Size - CMAC
print pack("V", $sc_len+0x10);
#close($out);

