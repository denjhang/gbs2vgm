#!/bin/bash
# GBS2VGM v2.0 Build Script
# Supports Linux, macOS, and Windows (MSYS2/MinGW)

set -e

echo "=== Building GBS2VGM v2.0 ==="
echo ""

# Detect platform
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "mingw"* ]]; then
    PLATFORM="Windows"
    OUTPUT="gbs2vgm_batch.exe"
else
    PLATFORM="Unix"
    OUTPUT="gbs2vgm_batch"
fi

echo "Platform: $PLATFORM"
echo "Output: $OUTPUT"
echo ""

# Compiler flags
CFLAGS="-Wall -O2 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L"

# Source files
SOURCES="gbs2vgm_batch.c m3u_parser.c vgm_writer.c \
         gbcpu.c gbhw.c gblfsr.c mapper.c gbs.c \
         crc32.c util.c filename_parser.c archive_utils.c"

# Dependency files
DEPS_7Z="../dependencies/7z/*.c"
DEPS_ZLIB="../dependencies/zlib/*.c"

# Libraries
LIBS="-lm"

echo "Compiling..."
gcc $CFLAGS -o $OUTPUT $SOURCES $DEPS_7Z $DEPS_ZLIB $LIBS

if [ $? -eq 0 ]; then
    echo ""
    echo "=== Build successful! ==="
    echo "Executable: $OUTPUT"
    echo ""
    echo "Usage: ./$OUTPUT <input-file>"
    echo "Example: ./$OUTPUT \"Game Name.7z\""
else
    echo ""
    echo "=== Build failed! ==="
    exit 1
fi
