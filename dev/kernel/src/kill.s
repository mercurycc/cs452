	.file	"kill.s"
	.text
	.align	2
	.global	task_kill
	.type	task_kill, %function
task_kill:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ a1: Target task sp
	@ a2: Exit (stupid idea, but I do not know how to load an address into a register)
	@ Save v1, v2
	stmdb	sp!, {v1, v2}
	@ Load user spsr, lr => v1, v2
	ldmia	a1!, {v1, v2}
	mov     v2, a2
	stmdb	a1!, {v1, v2}
	@ Restore v1, v2
	ldmia	sp!, {v1, v2}
	mov	pc, lr
	.size	task_kill, .-task_kill
	.ident	"GCC: (GNU) 4.0.2"
