# Add board source
BOARDSRC :=
include $(BOARD)/$(BOARD).mk
ARCHSRC += $(addprefix $(BOARD), $(BOARDSRC))

