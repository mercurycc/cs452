# Initialization
KERNELSRC :=

# Core
KERNELSRC += entry.c
KERNELSRC += init.c
KERNELSRC += session_start.s
KERNELSRC += trap.s
KERNELSRC += trap_handler.c

# Devices
KERNELSRC += devices/console.c
KERNELSRC += devices/clock.c
KERNELSRC += devices/bwio.c

# Libraries
KERNELSRC += lib/str.c

# Userland init
