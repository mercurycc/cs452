#
# Makefile for a0 run
#

exename = clk_test.elf
exeout = ../bin/$(exename)

mainName = main

# Include common.mk for applications after exename is declared
include ../../common/common.mk

all:  $(mainName).s $(exeout)

$(mainName).s: $(mainName).c 
	$(XCC) -S $(CFLAGS) $(mainName).c

$(mainName).o: $(mainName).s
	$(AS) $(ASFLAGS) -o $(mainName).o $(mainName).s

$(exeout): $(mainName).o
	$(LD) -o $@ $(mainName).o $(LDFLAGS) 

clean:
	-rm -f *.s *.o $(exename).map $(exeout)
