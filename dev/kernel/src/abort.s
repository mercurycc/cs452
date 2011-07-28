	.file	"abort.s"
	.text
	.align	2
	.global	abort_trap
	.type	abort_trap, %function
abort_trap:
	@ Check spsr
	mrs	ip, spsr
	and	ip, ip, #0x1f
	cmp	ip, #0x1b
	beq	abort_trace_end
	cmp	ip, #0x17
	beq	abort_trace_end
	@ Pass in the PC of the faulting instruction
	mov	a1, lr
	@ Correct the pc
	sub	a1, a1, #4
	@ ==============================================================
	@ Enter System mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #31
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	@ Pass in the fp of the faulting function
	mov	ip, fp
	@ Enter supervisor mode to print the shit
	@ ==============================================================
	@ Enter supervisor mode
	mrs	ip, cpsr
	@ Clear mode
	bic	ip, ip, #0x1f
	@ Set system mode
	orr	ip, ip, #19
	@ move into cpsr
	msr	cpsr_c, ip
	@ ==============================================================
	bl	abort_handle
	@ Make a stack trace
	mov	a1, ip
	@ v1 as fp, v2 as sp (so it doesn't matter), v3 as lr
	bl	abort_trace_begin
	@ Assign again
	mov	a1, fp
abort_trace_start:
	sub	a1, a1, #12
	ldmia	a1, {v1, v2, v3}
	mov	a2, a1
	mov	a1, v3
	sub	v4, v4, #1
	cmp	a1, #0
	@ The initial lr should be 0, so the created task should lr to 0 and thus is the trace bottom
	beq	abort_trace_end
	bl	abort_trace_print
	mov	a1, v1
	b	abort_trace_start
abort_trace_end:
	bl	abort_trace_complete
	@ Turn off
	mov	a1, sp
	b	kernel_shutdown
	.size	abort_trap, .-abort_trap
	.ident	"GCC: (GNU) 4.0.2"
