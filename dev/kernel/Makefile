# Makefile for X-Compiling ARMv4

KERNELEXEC = bin/pinball.elf
KERNELEXECMAP = $(KERNELEXEC).map

MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}	# makefile name

XCC     = gcc
AS	= as
AR	= ar
MKDIR   = mkdir -p
CP      = cp
RMRF    = rm -rf

XCCLIB = /u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2

INCLUDE = inc

# Directory storing important build files
BUILD := build

# Directory to store built objects/.s
BUILT := built

# Directory holding the kernel source code
SOURCE := src

CFLAGS  = -c -fPIC -Wall -I. -I$(INCLUDE) -mcpu=arm920t -msoft-float -finline-functions -finline-functions-called-once -fgcse -fexpensive-optimizations -fif-conversion -fif-conversion2 -ftree-pre -ftree-ccp
# 
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point
ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

LDFLAGS = -init main -Map $(KERNELEXECMAP) -N -T $(BUILD)/orex.ld -L$(XCCLIB) -L$(BUILD) -lgcc

# ===============
# File Lists
# ===============

# Kernel file list
KERNELSRC :=
include $(SOURCE)/kernel.mk

CSRC = $(filter %.c, $(KERNELSRC))
ASMSRC = $(filter %s, $(KERNELSRC))

CBUILTS = $(addprefix $(BUILT)/, $(basename $(CSRC)))
ASMTARGET = $(addprefix $(BUILT)/, $(ASMSRC))
ABUILTS = $(basename $(ASMTARGET))
BUILTDIRS = $(addprefix $(BUILT)/, $(sort $(dir $(KERNELSRC)))) bin

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

$(ASSEMBLY): $(BUILT)/%.s: $(SOURCE)/%.c
	$(XCC) -S $(CFLAGS) $< -o $@

$(ASMTARGET): $(BUILT)/%.s: $(SOURCE)/%.s
	$(CP) $< $@

$(ASSEMBLY) $(OBJECTS): $(BUILTDIRS)

$(BUILTDIRS):
	$(MKDIR) $@

.PHONY: clean

clean:
	$(RMRF) $(ASSEMBLY) $(ASMTARGET) $(OBJECTS) $(KERNELEXEC) $(KERNELEXECMAP)