	.file	"session.s"
	.text
	.align	2
	.global	session_start
	.type	session_start, %function
session_start:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Push context pointer onto kernel stack, setup user cpsr, and fallback to usermode through trap_handler_end
	@ a1: Kernel context
	@ a2: First task sp
	@ Push context onto kernel stack
	stmdb	sp!, {a1}
	@ Pass sp to trap_handler_end, according to trap.s
	mov	a4, sp
	@ ==============================================================
	@ Setup user mode cpsr in spsr
	mrs	ip, cpsr
	bic	ip, ip, #0x1f
	orr	ip, ip, #0x10
	msr	spsr_c, ip
	@ ==============================================================
	@ Exit kernel through trap_handler_end
	b	trap_handler_end
	.size	session_start, .-session_start
	.ident	"GCC: (GNU) 4.0.2"
