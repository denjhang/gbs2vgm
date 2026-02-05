# GBS2VGM Batch Converter Makefile
# Converts Game Boy Sound (GBS) files to VGM format with automatic loop detection

CC = gcc
CFLAGS = -std=c99 -O2 -Wall -Igbsplay -Igbsplay/7z
LDFLAGS = -lm -lz

# Source directories
SRCDIR = gbsplay
LZMADIR = gbsplay/7z

# Output
TARGET = gbs2vgm_batch.exe
TEST_M3U = test_m3u_parser.exe
VGM_TRIM = vgm_trim.exe

# Source files
SOURCES = \
	$(SRCDIR)/gbs2vgm_batch.c \
	$(SRCDIR)/m3u_parser.c \
	$(SRCDIR)/filename_parser.c \
	$(SRCDIR)/archive_utils.c \
	$(SRCDIR)/vgm_writer.c \
	$(SRCDIR)/gbs.c \
	$(SRCDIR)/gbcpu.c \
	$(SRCDIR)/gbhw.c \
	$(SRCDIR)/gblfsr.c \
	$(SRCDIR)/mapper.c \
	$(SRCDIR)/cfgparser.c \
	$(SRCDIR)/crc32.c \
	$(SRCDIR)/filewriter.c

# LZMA SDK sources (for 7z support)
LZMA_SOURCES = \
	$(LZMADIR)/7zAlloc.c \
	$(LZMADIR)/7zArcIn.c \
	$(LZMADIR)/7zBuf.c \
	$(LZMADIR)/7zCrc.c \
	$(LZMADIR)/7zCrcOpt.c \
	$(LZMADIR)/7zDec.c \
	$(LZMADIR)/7zExtract.c \
	$(LZMADIR)/7zFile.c \
	$(LZMADIR)/7zStream.c \
	$(LZMADIR)/Bcj2.c \
	$(LZMADIR)/Bra.c \
	$(LZMADIR)/Bra86.c \
	$(LZMADIR)/Delta.c \
	$(LZMADIR)/Lzma2Dec.c \
	$(LZMADIR)/LzmaDec.c \
	$(LZMADIR)/Ppmd7.c \
	$(LZMADIR)/Ppmd7Dec.c

# Test programs
TEST_M3U = test_m3u_parser.exe

# Utility programs
VGM_TRIM = vgm_trim.exe

.PHONY: all clean test utils

all: $(TARGET)

$(TARGET): $(SOURCES) $(LZMA_SOURCES)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(TEST_M3U): test_m3u_parser.c $(SRCDIR)/m3u_parser.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TEST_M3U)"

$(VGM_TRIM): vgm_trim.c $(SRCDIR)/vgm_trml.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(VGM_TRIM)"

utils: $(VGM_TRIM)
	@echo "Utility programs built"

test: $(TEST_M3U)
	@echo "Running M3U parser test..."
	./$(TEST_M3U) test_intro_loop.m3u

clean:
	rm -f $(TARGET) $(TEST_M3U) $(VGM_TRIM)
	@echo "Clean complete"

help:
	@echo "GBS2VGM Batch Converter - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build gbs2vgm_batch.exe (default)"
	@echo "  test    - Build and run test programs"
	@echo "  clean   - Remove built executables"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Usage:"
	@echo "  make                    # Build main program"
	@echo "  make test               # Build and run tests"
	@echo "  make clean              # Clean build artifacts"
