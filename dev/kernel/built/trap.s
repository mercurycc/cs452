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
	@ Save out user data, a0 is the trap reason, no need to save
	stmdb	sp!, {a1, a2, a3, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
	@ Put sp into a1
	mov	a1, sp
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
	@ Call trap_handler
	b	trap_handler
	.size	trap_handler_begin, .-trap_handler_begin
	.align	2
	.global	trap_handler_end
	.type	trap_handler_end, %function
trap_handler_end:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Retrive next task's sp from a1
	mov	sp, a1
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
	@ Restore
	ldmia	sp!, {a1, a2, a3, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
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
