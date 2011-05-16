	@ a1: trap reason
	@ a2: current/next task's sp
	@ a3: mode, for trap_handler
	@ a4: kernel stack pointer, for trap_handler and trap_handler_end
	@ TODO: mode is not processed.  Later with IRQ it will mess up everything
	.file	"trap.s"
	.text
	.align	2
	.global	trap_handler_begin
	.type	trap_handler_begin, %function
trap_handler_begin:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
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
	stmdb	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
	@ Put sp into a2
	mov	a2, sp
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
	@ Save kernel sp to a4 (TODO: This is pretty sketchy, but currently we rely on it to obtain the kernel context)
	mov	a4, sp
	@ Store user pc onto user stack
	stmdb	a2!, {lr}
	@ Call trap_handler
	b	trap_handler
	.size	trap_handler_begin, .-trap_handler_begin
	.align	2
	.global	trap_handler_end
	.type	trap_handler_end, %function
trap_handler_end:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Restore user task pc
	ldmia	a2!, {lr}
	@ Restore kernel stack pointer
	mov	sp, a4
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
	ldmia	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
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
	@ Go back to user mode
	movs	pc, lr
	.size	trap_handler_end, .-trap_handler_end
	.ident	"GCC: (GNU) 4.0.2"
