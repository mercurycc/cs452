#
# Makefile for busy-wait IO tests
#

exename = iotest.elf
exeout = ../bin/$(exename)

# Include common.mk for applications after exename is declared
include ../../common/common.mk

all:  iotest.s $(exeout)

iotest.s: iotest.c iotest.h
	$(XCC) -S $(CFLAGS) iotest.c

iotest.o: iotest.s
	$(AS) $(ASFLAGS) -o iotest.o iotest.s

$(exeout): iotest.o
	$(LD) -o $@ iotest.o $(LDFLAGS)

clean:
	-rm -f iotest.elf *.s *.o $(exename).map $(exeout)
