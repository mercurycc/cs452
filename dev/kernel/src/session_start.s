	.file	"session.s"
	.text
	.align	2
	.global	session_start
	.type	session_start, %function
session_start:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ a0: Target task pc
	@ a1: Target task sp
	@ Retrive next task's sp from a1
	mov	sp, a1
	@ ==============================================================
	@ Enter System mode
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #31
	msr	cpsr_c, ip
	@ ==============================================================
	@ Setup target task sp
	mov	sp, a1
	@ ==============================================================
	@ Enter supervisor mode
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #19
	msr	cpsr_c, ip
	@ ==============================================================
	@ Change spsr to user mode, based on current cpsr, assuming current cpsr have everything setup properly already
	@ Default CPSR should be a good start
	@ ==============================================================
	@ Enter user mode
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #0x10
	msr	spsr_c, ip
	@ ==============================================================
	@ Go back to user mode
	movs	pc, lr
	.size	trap_handler_end, .-trap_handler_end
	.ident	"GCC: (GNU) 4.0.2"
