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
KERNELSRC += sched.c
KERNELSRC += kernel_shutdown.s
KERNELSRC += interrupt.s
KERNELSRC += interrupt_handler.c

# Devices
KERNELSRC += devices/console.c
KERNELSRC += devices/clock.c
KERNELSRC += devices/bwio.c

# Libraries
KERNELSRC += lib/str.c
KERNELSRC += lib/rbuf.c
KERNELSRC += lib/list.c
KERNELSRC += lib/hashtable.c

# ==================================================
# Userland (too much trouble to modify the Makefile)

# Applications
KERNELSRC += userland/apps/init/main.c
KERNELSRC += userland/apps/init_user/main.c
KERNELSRC += userland/apps/noise/main.c
KERNELSRC += userland/apps/rps_client/main.c
KERNELSRC += userland/apps/RPSServer/main.c
KERNELSRC += userland/apps/rps_game/main.c
KERNELSRC += userland/apps/srr_timing/main.c

# Libraries
KERNELSRC += userland/lib/syscall.c
KERNELSRC += userland/lib/name_server.c
KERNELSRC += userland/lib/prng.c
