# XGM Standalone

The XGM driver (and xgmtool) from SGDK by itself.


## Structure

 - `z80-xgm` - Contains Z80 source for the sound driver
 - `m68k-xgm` - Contains m68k assembly source and C header to drop into your Mega Drive project
 - `xgmtool` - Source code to xgmtool. Used to convert VGM files to XGM data
 - `wavtoraw` - Tool that can convert sample rate of wav files for sound effects
 - `example` - Example project that demonstrates how to use this thing


## Compiling

Assuming you have a C compiler `make` is all you need. This does the following:
 - Build the `xgmtool` and `wavtoraw` utilities.
 - Download and compile `sjasm` (z80 assembler) if you don't have it already.
 - Assemble the Z80 driver to `z80-xgm.bin`.


## Installing / Using

Once you have the `z80-xgm.bin` copy that into your project, along with the
contents of the `m68k-src` directory.

Give `m68k-src/xgm.h` and `example/main.c` a read for further instructions.

You may need to modify these paths in `m68k-src/xgm.s`. 
Just point them to where the files actually are, relative to your project root.

    /* XGM Driver blob */
    BIN z80_xgm,    "z80_xgm.bin"
    z80_xgm_end:
    
    BIN stop_xgm,   "stop_xgm.bin"

As for installing the tools to your system, you could just copy them to
`/usr/local/bin`, or include compiling them as a step in building your
own Mega Drive project.


## How do I get sound data in my game without Rescomp?

TL;DR - By hand.


### For Music:

Rescomp just runs xgmtool under the hood.
You can script this with Bash, or add a step to your projects Makefile like so:

    # VGM files to convert to XGC. This assumes your VGM files are in res/bgm
    VGMS  = $(wildcard res/bgm/*.vgm)
    XGCS  = $(VGMS:.vgm=.xgc)
    
    # Make it a dependency somewhere, as long as it builds before the C/ASM code
    all: $(XGCS) (...other stuff...) mygame.bin
    
    # Convert VGM. Replace or define $(XGMTOOL) with the actual path to run xgmtool
    %.xgc: %.vgm
        $(XGMTOOL) "$<" "$@" -s
    
    # Clean up after yourself
    clean:
        rm -f $(XGCS)

As for including the output XGC files in the game, put something like this in an asm file:

        .align 2
        .globl BGM_MySong
    BGM_MySong:
        .incbin "res/bgm/mysong.xgc"

And in a C header:

    extern const uint8_t BGM_MySong[];


### For PCM Sound Effects:

XGM samples are signed 8-bit 14000Hz PCM streams with no header.
You could convert your samples to this format in Audacity manually,
or build the wavtoraw tool and convert them like this:

    # WAV files to convert to PCM. This assumes your sound files are in res/sfx
    WAVS  = $(wildcard res/sfx/*.wav)
    PCMS  = $(VGMS:.wav=.pcm)
    
    # Make it a dependency somewhere, as long as it builds before the C/ASM code
    all: $(PCMS) (...other stuff...) mygame.bin
    
    # Convert WAV
    %.pcm: %.wav
        $(WAVTORAW) "$<" "$@" 14000
    
    # Clean up after yourself
    clean:
        rm -f $(PCMS)

Then just incbin each of them in an asm file like so:

        .align 256
        .globl PCM_MySound
    PCM_MySound:
        .incbin "res/sfx/mysound.pcm"
    PCM_MySound_end:

And in a C header:

    extern const uint8_t PCM_MySound[], PCM_MySound_end[];

Why is the _end part needed? You have to tell XGM the size of every SFX sample.
You can see how this comes together in the example project.


## Miscellanea

The example project can be built with the `-mshort` compiler option.
However, you will need to enable the lines in `xgm.s` with a (-mshort) comment,
and disable the lines beneath them that they replace, then do the same for
`dma.s` (dma_queue routine) or freaky things will happen.
Due to the ABI change with `-mshort` the length of the stack used for those routines
becomes smaller and the parameters need to be read from a different offset.

Since xgm.s is the only m68k source file that "does things", you could potentially
include this standalone driver in assembly projects without C as well.
But since it is written in GNU syntax, you will have to convert it to Asm68k,
AS, or asmx if you use any of those. 
