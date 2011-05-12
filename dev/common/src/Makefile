#
# Makefile for busy-wait IO library
#

include ../common.mk

# Library pre-requisite

CFILES = $(foreach name, $(LIBS), $(name).c)
SFILES = $(foreach name, $(LIBS), $(name).s)
OFILES = $(foreach name, $(LIBS), $(name).o)
AFILES = $(foreach name, $(LIBS), $(name).a)
LIBFILES = $(foreach name, $(AFILES), $(LIBDIR)/lib$(name))

all:   $(LIBFILES)

# Library build
$(SFILES): $(CFILES)
	$(XCC) -S $(CFLAGS) $(subst .s,.c,$@) -o $@

$(OFILES): $(SFILES)
	$(AS) $(ASFLAGS) $(subst .o,.s,$@) -o $@

$(AFILES): $(OFILES)
	$(AR) $(ARFLAGS) $@ $(subst .a,.o,$@)

$(LIBFILES): $(AFILES)
	cp $(subst $(LIBDIR)/lib,,$@) $@

clean:
	-rm -f $(SFILES) $(OFILES) $(AFILES)
