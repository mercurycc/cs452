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
	@ TODO: We could find a way to use the trap_handler_begin to do this.  However be warned that trap.s might need renovation in order to carry out this task.
	@ Save v1, v2 as we are going to use them
	stmdb	sp!, {v1, v2}
	@ Use v2 to store user pc
	mov	v2, a1
	@ Setup initial stack
	sub	a2, a2, #80
	@ ==============================================================
	@ Setup user mode initial cpsr in spsr
	mrs	v1, cpsr
	@ Clear nzcv bits and mode bits
	bic	v1, v1, #0xf0000000
	bic	v1, v1, #0x1f
	@ Set user mode in cpsr
	orr	v1, v1, #0x10
	@ ==============================================================
	@ Push task cpsr and pc onto task stack
	stmdb	a2!, {v1, v2}
	@ Return the new stack pointer
	mov	a1, a2
	@ Restore v1, v2
	ldmia	sp!, {v1, v2}
	mov	pc, lr
	.size	task_init, .-task_init
	.ident	"GCC: (GNU) 4.0.2"
