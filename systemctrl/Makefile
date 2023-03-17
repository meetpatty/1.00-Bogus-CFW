TARGET = systemctrl
OBJS = main.o systemctrl.o nidresolver.o

BUILD_PRX=1
PRX_EXPORTS=exports.exp

USE_PSPSDK_LIBC = 1

INCDIR =
CFLAGS = -O2 -G0 -Wall -nostartfiles
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak