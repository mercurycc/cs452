# Initialization
KERNELSRC :=

# Core
KERNELSRC += entry.c
KERNELSRC += init.c
KERNELSRC += session_start.s
KERNELSRC += trap.s
KERNELSRC += trap_handler.c
KERNELSRC += task.c
KERNELSRC += task_init.s
KERNELSRC += mem.c
KERNELSRC += context.c
# KERNELSRC += sched.c

# Devices
KERNELSRC += devices/console.c
KERNELSRC += devices/clock.c
KERNELSRC += devices/bwio.c

# Libraries
KERNELSRC += lib/str.c
KERNELSRC += lib/rbuf.c
KERNELSRC += lib/list.c

# ==================================================
# Userland (too much trouble to modify the Makefile)

# Libraries
KERNELSRC += userland/lib/syscall.c