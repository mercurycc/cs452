# Makefile for X-Compiling ARMv4

KERNELEXEC = bin/pinball.elf

MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}	# makefile name

XCC     = gcc
AS	= as
AR	= ar
MKDIR   = mkdir -p
CP      = cp
RMRF    = rm -rf

XCCLIB = /u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2

ARCH = arm920t
BOARD = ts7200

INCLUDE = inc $(ARCH)/inc $(ARCH)/$(BOARD)/inc

# Directory storing important build files
BUILD := build

# Directory to store built objects/.s
BUILT := built

# List of all source codes
SRC :=

CFLAGS  = -c -fPIC -Wall -I. -I$(INCLUDE) -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point
ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

LDFLAGS = -init main -Map $(BUILT)/$(KERNELEXEC).map -N -T $(BUILD)/orex.ld -L$(XCCLIB) -L$(BUILD) -lgcc

# ===============
# File Lists
# ===============

# Kernel file list
KERNELSRC :=
include src/kernel.mk
SRC += $(addprefix src/, $(KERNELSRC))

# Architecture specific file list
ARCHSRC :=
include arch/arch.mk
SRC += $(addprefix arch/, $(ARCHSRC))

CSRC = $(filter %.c, $(SRC))
ASMSRC = $(filter %s, $(SRC))

CBUILTS = $(addprefix $(BUILT)/, $(basename $(CSRC)))
ASMTARGET = $(addprefix $(BUILT)/, $(ASMSRC))
ABUILTS = $(basename $(ASMTARGET))
BULITDIRS = $(dir $(BUILTS))

ASSEMBLY = $(addsuffix .s, $(CBUILTS))
OBJECTS = $(addsuffix .o, $(CBUILTS) $(ABUILTS))

# ===============
# Build Rules
# ===============

all: $(KERNELEXEC)

$(KERNELEXEC): $(OBJECTS)
	$(LD) -o $@ $^ $(LDFLAGS)

$(OBJECTS): %.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

$(ASSEMBLY): %.s: %.c
	$(XCC) -S $(CFLAGS) $< -o $@

$(ASMTARGET): :
	$(CP) $< $@

$(ASSEMBLY) $(OBJECTS): $(BUILTDIRS)

$(BUILTDIRS):
	$(MKDIR) $@

.PHONY: clean

clean:
	$(RMRF) $(ASSEMBLY) $(OBJECTS) $(KERNELEXEC)