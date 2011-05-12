ARCHSRC :=
include $(ARCH)/$(ARCH).mk
ARCHSRC = $(addprefix $(ARCH), $(ARCHSRC))