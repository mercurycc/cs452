	@ a1: trap reason, 0 for interrupt
	.file	"trap.s"
	.text
	.align	2
	.global	trap_handler_begin
	.type	trap_handler_begin, %function
trap_handler_begin:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Store ip onto kernel stack, as we will clobber it
	stmdb	sp!, {ip}
	@ ==============================================================
	@ Enter System mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #31
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Save out user data
	stmdb	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, lr}
	@ Put sp into a2
	mov	a2, sp
	@ ==============================================================
	@ Enter IRQ mode to save IRQ handler state
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set IRQ mode
	orr	ip, ip, #12
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	mrs	ip, spsr
	@ Save out IRQ handler data
	stmdb	a2!, {ip, sp, lr}
	@ ==============================================================
	@ Enter supervisor mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #19
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Put original ip onto user stack
	ldmia	sp!, {ip}
	stmdb	a2!, {ip}
	@ Save kernel sp to a4 (TODO: This is pretty sketchy, but currently we rely on it to obtain the kernel context)
	mov	a4, sp
	@ Store the spsr, also pass spsr to trap_handler
	mrs	a3, spsr
	@ Store user pc and cpsr onto user stack
	stmdb	a2!, {a3, lr}
	@ Call trap_handler
	b	trap_handler
	.size	trap_handler_begin, .-trap_handler_begin
	.align	2
	.global	trap_handler_end
	.type	trap_handler_end, %function
	
	@ a2: current/next task's sp
	@ a4: kernel stack pointer, for trap_handler and trap_handler_end
trap_handler_end:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Restore user task pc and cpsr
	ldmia	a2!, {ip, lr}
	@ Restore cpsr
	msr	spsr, ip
	@ Restore kernel stack pointer
	mov	sp, a4
	@ Take original ip onto kernel stack
	ldmia	a2!, {ip}
	stmdb	sp!, {ip}
	@ ==============================================================
	@ Enter IRQ mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set IRQ mode
	orr	ip, ip, #12
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Restore IRQ handler states
	ldmia	a2!, {ip, sp, lr}
	msr	spsr, ip
	@ ==============================================================
	@ Enter System mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #31
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Retrive next task's sp from a2
	mov	sp, a2
	@ Restore
	ldmia	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, lr}
	@ ==============================================================
	@ Enter supervisor mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #19
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Restore original ip from kernel stack
	ldmia	sp!, {ip}
	@ Go back to saved mode
	movs	pc, lr
	.size	trap_handler_end, .-trap_handler_end
	.ident	"GCC: (GNU) 4.0.2"
