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
	@ Save kernel context
	stmdb	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
	@ Push context onto kernel stack
	stmdb	sp!, {a1}
	@ Pass sp to trap_handler_end, according to trap.s
	mov	a4, sp
	@ Exit kernel through trap_handler_end
	b	trap_handler_end
	.size	session_start, .-session_start
	.ident	"GCC: (GNU) 4.0.2"
