#ifndef _REGOPTS_H_
#define _REGOPTS_H_

#include <types.h>

/* FIXME: Can I assume all registers are 32 bits? */
/* Answer: no.  WDT uses 8 bit register */
#define HW_ADDR( base, offset )			\
	(volatile uint*)( base + offset )

#define HW_WRITE( base, offset, value )		\
	*( HW_ADDR( base, offset ) ) = (uint)value

#define HW_READ( base, offset )			\
	*( HW_ADDR( base, offset ) )

#endif /* _REGOPTS_H_ */
