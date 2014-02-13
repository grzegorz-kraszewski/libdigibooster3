#
# Makefile for libdigibooster3
#

# Commons

CFLAGS = -W -Wall -O2 -g -Wpointer-arith -Wno-parentheses
CFLAGS += -fno-strict-aliasing -fno-builtin -I../include/ -L./
OBJS  = loader.o player.o
OBJS += dsp_wavetable.o dsp_linresampler.o dsp_fetchinstr.o dsp_panoramizer.o dsp_echo.o
DOC = libdigibooster3.txt
LIB = libdigibooster3.a
TOOLS = dbminfo dbm2wav

################################################################################

.PHONY: clean mos os3 os4 linux

default:
	@echo "Available targets:"
	@echo "    mos     (morphos/ppc)"
	@echo "    os3     (amigaos3/m68k)"
	@echo "    os4     (amigaos4/ppc)"
	@echo "    linux   (linux/generic)"
	@echo "    clean   (deletes objects and executables)"

clean:
	rm -vf *.o $(LIB) $(DOC) $(TOOLS)

os3: CC = m68k-amigaos-gcc
os3: AR = m68k-amigaos-ar
os3: CFLAGS += -fbaserel -m68030 -DTARGET_AMIGAOS3 -noixemul
os3: $(LIB) $(TOOLS)

os4: CC = ppc-amigaos-gcc
os4: AR = ppc-amigaos-ar
os4: CFLAGS += -D__USE_INLINE__ -DTARGET_AMIGAOS4
os4: $(LIB) $(TOOLS)

mos: CC = ppc-morphos-gcc-4
mos: AR = ar
mos: CFLAGS += -DUSE_INLINE_STDARG -DTARGET_MORPHOS -noixemul
mos: $(LIB) $(DOC) $(TOOLS)

linux: CC = gcc
linux: AR = ar
linux: CFLAGS += -DTARGET_LINUX
linux: $(LIB) $(TOOLS)

################################################################################

$(LIB): $(OBJS)
	$(AR) rs $@ $(OBJS)

dbminfo: $(LIB) dbminfo.o
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -o dbminfo dbminfo.o -ldigibooster3
	@strip dbminfo

dbm2wav: $(LIB) dbm2wav.o
	@echo "Building $@..."
	@$(CC) $(CFLAGS) -o dbm2wav dbm2wav.o -ldigibooster3
	@strip dbm2wav

$(DOC): loader.c player.c
	cat $^ >tempfile
	robodoc tempfile $@ TABSIZE 4 TOC SORT ASCII
	rm tempfile

%.o: %.c
	@echo "Compiling $@..."
	@$(CC) -c $(CFLAGS) -o $@ $<

################################################################################

dbm2wav.o: dbm2wav.c libdigibooster3.h musicmodule.h
dbminfo.o: dbminfo.c libdigibooster3.h musicmodule.h
ddsp_echo.o: dsp_echo.c libdigibooster3.h musicmodule.h dsp.h lists.h
dsp_fetchinstr.o: dsp_fetchinstr.c libdigibooster3.h musicmodule.h dsp.h lists.h
dsp_linresampler.o: dsp_linresampler.c libdigibooster3.h musicmodule.h dsp.h lists.h
dsp_panoramizer.o: dsp_panoramizer.c libdigibooster3.h musicmodule.h dsp.h lists.h
dsp_wavetable.o: dsp_wavetable.c libdigibooster3.h musicmodule.h dsp.h lists.h
loader.o: loader.c libdigibooster3.h musicmodule.h
player.o: player.c libdigibooster3.h musicmodule.h dsp.h lists.h player.h
