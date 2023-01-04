#!/usr/bin/env bash
set -ex

file=/home/user/assembler/RISC-V-disassembly-tools/build/src/elfParser/elfParser

echo "64bits .debug_aranges" > hex.txt
od -A n --output-duplicates --skip-bytes=0x000a0120 --read-bytes=0x3cd0 -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_info
echo "8bits .debug_info" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000a3df0 --read-bytes=0x12d9cd -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_info" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000a3df0 --read-bytes=0x12d9cd -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "32bits .debug_info" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000a3df0 --read-bytes=0x12d9cd -t x4 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "64bits .debug_info" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000a3df0 --read-bytes=0x12d9cd -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_abbrev
echo "8bits .debug_abbrev" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001d17bd --read-bytes=0x2b52b -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_abbrev" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001d17bd --read-bytes=0x2b52b -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_line
echo "8bits .debug_line" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001fcce8 --read-bytes=0xd8079 -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_line" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001fcce8 --read-bytes=0xd8079 -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "32bits .debug_line" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001fcce8 --read-bytes=0xd8079 -t x4 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "64bits .debug_line" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0001fcce8 --read-bytes=0xd8079 -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_str
echo "8bits .debug_str" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0002dd920 --read-bytes=0x1d4e9 -c $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_loc
echo "8bits .debug_loc" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0002fae09 --read-bytes=0x107abe -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_loc" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0002fae09 --read-bytes=0x107abe -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "32bits .debug_loc" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0002fae09 --read-bytes=0x107abe -t x4 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "64bits .debug_loc" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0002fae09 --read-bytes=0x107abe -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_ranges
echo "8bits .debug_ranges" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0004028c7 --read-bytes=0x301b0 -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_ranges" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0004028c7 --read-bytes=0x301b0 -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "32bits .debug_ranges" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0004028c7 --read-bytes=0x301b0 -t x4 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "64bits .debug_ranges" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x0004028c7 --read-bytes=0x301b0 -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

# .debug_macro
echo "8bits .debug_macro" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000432a77 --read-bytes=0x3b10 -t x1 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "16bits .debug_macro" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000432a77 --read-bytes=0x3b10 -t x2 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "32bits .debug_macro" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000432a77 --read-bytes=0x3b10 -t x4 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt

echo "64bits .debug_macro" >> hex.txt
od -A n --output-duplicates --skip-bytes=0x000432a77 --read-bytes=0x3b10 -t x8 $file | awk '{ printf("%06x %s\n",NR*16-16,$0) }' >> hex.txt
echo >> hex.txt