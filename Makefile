TITLE_ID = MEMDMP001
TARGET = mDump
PSVITAIP = 192.168.1.78

PLUGIN_OBJS = user.o
HEADERS = $(wildcard *.h)

PLUGIN_LIBS = -ltaihen_stub -lSceSysclibForDriver_stub -lSceModulemgrForKernel_stub -lSceIofilemgr_stub -lSceLibc_stub -lSceSysmemForDriver_stub -lSceSysmemForKernel_stub -lSceLibKernel_stub

PREFIX  = arm-vita-eabi
CC      = $(PREFIX)-gcc
CFLAGS  = -Wl,-q -Wall -O3
ASFLAGS = $(CFLAGS)

all: mDump.suprx

mDump.suprx: mDump.velf
	vita-make-fself $< $@

mDump.velf: mDump.elf
	vita-elf-create -e exports.yml $< $@

mDump.elf: $(PLUGIN_OBJS)
	$(CC) $(CFLAGS) $^ $(PLUGIN_LIBS) -o $@ -nostdlib

clean:
	@rm -rf *.velf *.elf *.vpk *.skprx $(MAIN_OBJS) $(PLUGIN_OBJS) param.sfo eboot.bin

send: mDump.suprx
	curl -T mDump.suprx ftp://$(PSVITAIP):1337/ux0:/dump/
	@echo "Sent."
