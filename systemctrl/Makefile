TARGET = systemctrl
OBJS = main.o systemctrl.o nidresolver.o

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

INCDIR =
CFLAGS = -O2 -G0 -Wall -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

all: rebootex.h $(TARGET).prx

rebootex.h:
	bin2c ../rebootex/rebootex.bin rebootex.h rebootex_buffer

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak