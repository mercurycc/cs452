	.file	"kernel_shutdown.s"
	.text
	.align	2
	.global	kernel_shutdown
	.type	kernel_shutdown, %function
kernel_shutdown:
	@ a1: kernel sp
	mov	sp, a1
	@ Pop kernel context
	ldmia	sp!, {a1}
	@ Pop all kernel context
	ldmia	sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, v6, sl, fp, ip, lr}
	@ Return to kernel main
	mov	pc, lr
	.size	kernel_shutdown, .-kernel_shutdown
	.ident	"GCC: (GNU) 4.0.2"

