release: all
	psp-strip vshmain.elf
	mv vshmain.elf vshmain.prx


TARGET = vshmain
OBJS = main.o imports.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

USE_PSPSDK_LIBC = 1

LIBDIR = ../libs
LIBS =
LDFLAGS += -Telf_mipsallegrexel_psp_2.x -mno-crt0 -nostartfiles

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
