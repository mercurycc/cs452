	.file	"cache.s"
	.text
	.align	2
	.global	cache_init
	.type	cache_init, %function
cache_init:
	@ Move CP15 to ip
	mrc	p15, 0, ip, c1, c0, 0
	@ Enable I-cache and D-cache
	orr	ip, ip, #0x1000
	@ orr	ip, ip, #0x4
	@ Write back
	mcr	p15, 0, ip, c1, c0, 0
	@ Return
	mov	pc, lr
	.size	cache_init, .-cache_init
	.ident	"GCC: (GNU) 4.0.2"

