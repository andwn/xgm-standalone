XGMTOOL := bin/xgmtool
WAV2RAW := bin/wav2raw
SJASM   := bin/sjasm

.PHONY: all clean

all: bin $(XGMTOOL) $(WAV2RAW) z80-xgm.bin

# Create bin dir
bin:
	mkdir -p bin

# Grab sjasm git repo
sjasm:
	git clone https://github.com/konamiman/sjasm --depth=1 --branch v0.39

$(XGMTOOL): bin
	cd xgmtool && cc *.c -o xgmtool -lm
	cp -f xgmtool/xgmtool $(XGMTOOL)

$(WAV2RAW): bin
	cd wavtoraw && cc wavtoraw.c -o wavtoraw -lm
	cp -f wavtoraw/wavtoraw $(WAV2RAW)

$(SJASM): bin sjasm
	cd sjasm/Sjasm && $(MAKE)
	cp -f sjasm/Sjasm/sjasm $(SJASM)

z80-xgm.bin: $(SJASM)
	$(SJASM) -iz80-src z80-src/z80_xgm.s80 $@ z80-xgm.lst


clean:
	rm -rf bin sjasm
	rm -f z80-xgm.bin z80-xgm.lst
