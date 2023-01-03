#!/usr/bin/env bash
set -ex

file=/home/user/assembler/RISC-V-disassembly-tools/build/src/elfParser/elfParser

echo "8bits" > hex.txt
od -A x --output-duplicates --skip-bytes=0x000a0120 -t x1 $file >> hex.txt
echo >> hex.txt

echo "16bits" >> hex.txt
od --address-radix=x --output-duplicates --skip-bytes=0x000a0120 -t x2 $file >> hex.txt
echo >> hex.txt

echo "32bits" >> hex.txt
od -A x --output-duplicates --skip-bytes=0x000a0120 -t x4 $file >> hex.txt
echo >> hex.txt

echo "64bits" >> hex.txt
od -A x --output-duplicates --skip-bytes=0x000a0120 -t x8 $file >> hex.txt
echo >> hex.txt
