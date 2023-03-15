TARGET = flasher
OBJS = main.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = 
LDFLAGS =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_SFO = PARAM.SFO

all: systemctrlmod.h vshexmod.h vshmainmod.h $(TARGET).prx

systemctrlmod.h:
	bin2c ../systemctrl/systemctrl.prx systemctrlmod.h systemctrl_buffer

vshexmod.h:
	bin2c ../vshex/vshex.prx vshexmod.h vshex_buffer

vshmainmod.h:
	bin2c ../vshmain/vshmain.prx vshmainmod.h vshmain_buffer

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
