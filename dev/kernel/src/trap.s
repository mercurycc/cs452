	.file	"trap.s"
	.text
	.align	2
	.global	trap_handler_begin
	.type	trap_handler_begin, %function
trap_handler_begin:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	trap_handler_begin, .-trap_handler_begin
	.align	2
	.global	trap_handler_end
	.type	trap_handler_end, %function
trap_handler_end:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	mov	ip, sp
	stmfd	sp!, {fp, ip, lr, pc}
	sub	fp, ip, #4
	sub	sp, sp, #8
	str	r0, [fp, #-16]
	str	r1, [fp, #-20]
	sub	sp, fp, #12
	ldmfd	sp, {fp, sp, pc}
	.size	trap_handler_end, .-trap_handler_end
	.ident	"GCC: (GNU) 4.0.2"
