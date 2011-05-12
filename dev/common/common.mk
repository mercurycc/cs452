COMMON := $(dir $(lastword $(MAKEFILE_LIST)))
LIBDIR := $(COMMON)/lib
BUILT := $(COMMON)/built

LIBS += ui/ui sensorctrl ui/sensor ui/status ui/switches ui/prompt ui/timer command regionio train time bufio rbuf console bwio clock str

# Common Makefile for X-Compiling ARMv4

XCC     = gcc
AS	= as
AR	= ar

XCCLIB = /u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2

CFLAGS  = -c -fPIC -Wall -I. -I$(COMMON)/inc -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

LDFLAGS = -init main -Map $(exename).map -N -T $(COMMON)/orex.ld -L$(XCCLIB) -L$(LIBDIR) $(foreach name, $(LIBS), -l$(name)) -lgcc


