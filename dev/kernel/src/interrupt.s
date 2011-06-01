	.file	"interrupt.s"
	.text
	.align	2
	.global	interrupt_trap
	.type	interrupt_trap, %function
interrupt_trap:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 1, uses_anonymous_args = 0
	@ Enter trap
	swi	0
	@ Return to user program
	subs	pc, lr, #4
	.size	interrupt_trap, .-interrupt_trap
	.align	2
	.global	interrupt_set_stack
	.type	interrupt_set_stack, %function


