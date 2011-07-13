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
KERNELSRC += kill.s
KERNELSRC += mem.c
KERNELSRC += context.c
KERNELSRC += sched.c
KERNELSRC += kernel_shutdown.s
KERNELSRC += interrupt.s
KERNELSRC += interrupt_handler.c
KERNELSRC += watchdog.c
KERNELSRC += perf.c
KERNELSRC += cache.s

# Devices
KERNELSRC += devices/console.c
KERNELSRC += devices/clock.c
KERNELSRC += devices/bwio.c

# Libraries
KERNELSRC += lib/str.c
KERNELSRC += lib/rbuf.c
KERNELSRC += lib/list.c
KERNELSRC += lib/hashtable.c
KERNELSRC += lib/strprintf.c

# ==================================================
# Userland (too much trouble to modify the Makefile)

# Applications
KERNELSRC += userland/apps/init/main.c
# KERNELSRC += userland/apps/init_user/main.c
# KERNELSRC += userland/apps/noise/main.c
# KERNELSRC += userland/apps/rps_client/main.c
# KERNELSRC += userland/apps/RPSServer/main.c
# KERNELSRC += userland/apps/rps_game/main.c
# KERNELSRC += userland/apps/lazy_dog/main.c
KERNELSRC += userland/apps/train/train_control.c
KERNELSRC += userland/apps/train/train_module.c
KERNELSRC += userland/apps/train/train_module_executor.c
KERNELSRC += userland/apps/train/train_auto.c
KERNELSRC += userland/apps/train/sensor.c
KERNELSRC += userland/apps/train/track_data.c
KERNELSRC += userland/apps/train/train_location.c
# KERNELSRC += userland/apps/train/speed.c
KERNELSRC += userland/apps/train/planner.c
KERNELSRC += userland/apps/train/train_tracking.c
KERNELSRC += userland/apps/train/error_tolerance.c
KERNELSRC += userland/apps/train/ui/clock.c
KERNELSRC += userland/apps/train/ui/sensor_monitor.c
KERNELSRC += userland/apps/train/ui/switch_monitor.c
KERNELSRC += userland/apps/train/ui/train_tracking.c

# Libraries
KERNELSRC += userland/lib/syscall.c
KERNELSRC += userland/lib/name_server.c
KERNELSRC += userland/lib/prng.c
KERNELSRC += userland/lib/heap.c
KERNELSRC += userland/lib/sync.c
KERNELSRC += userland/lib/event.c
KERNELSRC += userland/lib/uart.c
KERNELSRC += userland/lib/cursor_control.c
KERNELSRC += userland/lib/courier.c
KERNELSRC += userland/lib/parser.c
KERNELSRC += userland/lib/math.c

# Drivers
KERNELSRC += userland/drivers/clock_drv.c
KERNELSRC += userland/drivers/uart_drv.c

# Services
KERNELSRC += userland/server/time_server.c
KERNELSRC += userland/server/display_server.c
KERNELSRC += userland/server/semaphore_server.c
