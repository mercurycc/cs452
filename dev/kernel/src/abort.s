	.file	"task_init.s"
	.text
	.align	2
	.global	abort_trap
	.type	abort_trap, %function
abort_trap:
	@ Pass in the PC of the faulting instruction
	mov	a1, lr
	b	abort_handle
	.size	abort_trap, .-abort_trap
	.ident	"GCC: (GNU) 4.0.2"
