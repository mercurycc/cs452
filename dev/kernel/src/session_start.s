	.file	"session.s"
	.text
	.align	2
	.global	session_start
	.type	session_start, %function
session_start:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ a1: Target task pc
	@ a2: Target task sp
	@ ==============================================================
	@ Enter System mode
	mrs	ip, cpsr
	orr	ip, ip, #0x1f
	msr	cpsr_c, ip
	@ ==============================================================
	@ Setup target task sp
	mov	sp, a2
	@ ==============================================================
	@ Enter supervisor mode
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #0x13
	msr	cpsr_c, ip
	@ ==============================================================
	@ ==============================================================
	@ Setup user mode cpsr in spsr
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #0x10
	msr	spsr_c, ip
	@ ==============================================================
	@ Go back to user mode
	movs	pc, a1
	.size	session_start, .-session_start
	.ident	"GCC: (GNU) 4.0.2"
