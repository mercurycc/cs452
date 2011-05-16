	.file	"task_init.s"
	.text
	.align	2
	.global	task_init
	.type	task_init, %function
task_init:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ a1: Target task pc
	@ a2: Target task sp
	@ Setup initial stack
	sub	a2, a2, #56
	@ Push task pc onto task stack
	stmdb	a2!, {a1}
	@ Return the new stack pointer
	mov	a1, a2
	mov	pc, lr
	.size	task_init, .-task_init
	.ident	"GCC: (GNU) 4.0.2"
