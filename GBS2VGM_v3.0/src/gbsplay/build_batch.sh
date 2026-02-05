#!/bin/bash
# Build gbs2vgm_batch with 7z and zlib support

cd "$(dirname "$0")"

echo "Compiling 7z SDK..."
gcc -c -O2 7z/7zAlloc.c -o 7z/7zAlloc.o
gcc -c -O2 7z/7zArcIn.c -o 7z/7zArcIn.o
gcc -c -O2 7z/7zBuf.c -o 7z/7zBuf.o
gcc -c -O2 7z/7zCrc.c -o 7z/7zCrc.o
gcc -c -O2 7z/7zCrcOpt.c -o 7z/7zCrcOpt.o
gcc -c -O2 7z/7zDec.c -o 7z/7zDec.o
gcc -c -O2 7z/7zExtract.c -o 7z/7zExtract.o
gcc -c -O2 7z/7zFile.c -o 7z/7zFile.o
gcc -c -O2 7z/7zStream.c -o 7z/7zStream.o
gcc -c -O2 7z/Bcj2.c -o 7z/Bcj2.o
gcc -c -O2 7z/Bra.c -o 7z/Bra.o
gcc -c -O2 7z/Bra86.c -o 7z/Bra86.o
gcc -c -O2 7z/Delta.c -o 7z/Delta.o
gcc -c -O2 7z/Lzma2Dec.c -o 7z/Lzma2Dec.o
gcc -c -O2 7z/LzmaDec.c -o 7z/LzmaDec.o
gcc -c -O2 7z/Ppmd7.c -o 7z/Ppmd7.o
gcc -c -O2 7z/Ppmd7Dec.c -o 7z/Ppmd7Dec.o

echo "Compiling zlib..."
gcc -c -O2 zlib/adler32.c -o zlib/adler32.o
gcc -c -O2 zlib/compress.c -o zlib/compress.o
gcc -c -O2 zlib/crc32.c -o zlib/crc32.o
gcc -c -O2 zlib/deflate.c -o zlib/deflate.o
gcc -c -O2 zlib/inffast.c -o zlib/inffast.o
gcc -c -O2 zlib/inflate.c -o zlib/inflate.o
gcc -c -O2 zlib/inftrees.c -o zlib/inftrees.o
gcc -c -O2 zlib/trees.c -o zlib/trees.o
gcc -c -O2 zlib/zutil.c -o zlib/zutil.o

echo "Compiling archive_utils..."
gcc -c -O2 archive_utils.c -I. -I7z -Izlib -o archive_utils.o

echo "Compiling gbs2vgm_batch..."
gcc -c -O2 gbs2vgm_batch.c -I. -o gbs2vgm_batch.o
gcc -c -O2 filename_parser.c -o filename_parser.o

echo "Linking..."
gcc -o gbs2vgm_batch.exe \
	gbs2vgm_batch.o \
	filename_parser.o \
	archive_utils.o \
	vgm_writer.o \
	m3u_parser.o \
	gbcpu.o \
	gbhw.o \
	gblfsr.o \
	mapper.o \
	gbs.o \
	crc32.o \
	util.o \
	7z/*.o \
	zlib/*.o \
	-lm

echo "Done! gbs2vgm_batch.exe created"
