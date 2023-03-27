# There are a lot of toolchains, huh
ifeq ($(MDROOT),)
	ifeq ($(shell which m68k-elf-gcc),)
		ifeq ($(MARSDEV),)
			ifeq ($(GDK),)
				ifeq ($(GENDEV),)
					$(error "Can't find any installed toolchain!")
				else
					MDROOT := $(GENDEV)/m68k-elf
				endif
			else
				MDROOT := $(GDK)
			endif
		else
			MDROOT := $(MARSDEV)/m68k-elf
		endif
	else
		MDROOT := $(subst /bin/m68k-elf-gcc,,$(shell which m68k-elf-gcc))
	endif
endif

MDBIN  := $(MDROOT)/bin
MDLIB  := $(MDROOT)/lib
MDINC  := $(MDROOT)/include

CC     := $(MDBIN)/m68k-elf-gcc
AS     := $(MDBIN)/m68k-elf-as
LD     := $(MDBIN)/m68k-elf-ld
NM     := $(MDBIN)/m68k-elf-nm
OBJC   := $(MDBIN)/m68k-elf-objcopy

# The tools this repo builds in the parent directory
# You might want to change the path in your own projects, like a tools/ directory
XGMTOOL := ../bin/xgmtool
WAV2RAW := ../bin/wav2raw
SJASM   := ../bin/sjasm

MDTILER := mdtiler/mdtiler

# Some files needed are in a versioned directory
GCC_VER := $(shell $(CC) -dumpversion)
PLUGIN  := $(MDROOT)/libexec/gcc/m68k-elf/$(GCC_VER)
LTO_SO  := liblto_plugin.so

REL_FLAGS = -O3 -flto=auto
DBG_FLAGS = -Og -g

# Flags for compiler, assembler and linker
CCFLAGS  = -m68000 -ffreestanding $(DBG_FLAGS) -Wall -Wextra
ASFLAGS  = -m68000 -I../m68k-src --register-prefix-optional --bitwise-or
LDFLAGS  = -T megadrive.ld -nostartfiles -L$(MDLIB)/gcc/m68k-elf/$(GCC_VER) -lgcc

# Tilesets to convert without compression
TSETS  = $(wildcard res/*.png)
TSETO  = $(TSETS:.png=.pat)

# VGM files to convert to XGC
VGMS  = $(wildcard res/*.vgm)
XGCS  = $(VGMS:.vgm=.xgc)

# WAV files to convert to raw PCM
WAVS  = $(wildcard res/*.wav)
PCMS  = $(WAVS:.wav=.pcm)

# Source files
CS    = $(wildcard *.c)
SS    = $(wildcard *.s)
SS   += $(wildcard ../m68k-src/*.s)
OBJS  = $(CS:.c=.o)
OBJS += $(SS:.s=.o)

TARGET := xgm-test

.SECONDARY: $(TARGET).elf

.PHONY: all clean

all: prereq $(TARGET).gen $(TARGET).lst

prereq: $(MDTILER) $(TSETO) $(XGCS) $(PCMS)

# Cross reference symbol list with the addresses displayed in the crash handler
%.lst: %.elf
	$(NM) --plugin=$(PLUGIN)/$(LTO_SO) -n $< > $@

%.gen: %.elf
	@echo "Stripping ELF header, pad to 512K"
	@$(OBJC) -O binary $< temp.bin
	@dd if=temp.bin of=$@ bs=512K conv=sync
	@rm -f temp.bin

%.elf: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS)

%.o: %.c
	@echo "CC $<"
	@$(CC) $(CCFLAGS) $(OPTIONS) $(WARNINGS) $(INCS) -c $< -o $@

%.o: %.s
	@echo "AS $<"
	@$(AS) $(ASFLAGS) $< -o $@


%.pat: %.png
	$(MDTILER) -t "$<" "$@"

# Convert VGM
%.xgc: %.vgm
	$(XGMTOOL) "$<" "$@" -s

# Convert WAV
%.pcm: %.wav
	$(WAV2RAW) "$<" "$@" 14000


$(MDTILER):
	cc mdtiler/*.c -Imdtiler -o $@ -lpng16 -lz


clean:
	rm -f $(TSETO) $(XGCS) $(PCMS) $(OBJS)
	rm -f $(TARGET).elf $(TARGET).gen $(TARGET).lst
