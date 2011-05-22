#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include <types.h>

struct Syscall_s {
	uint code;
	/* Target tig */
	uint target_tid;
	/* For obtaining the input */
	void* data;
	uint datalen;
	/* For returning the result */
	void* buffer;
	uint bufferlen;
	/* Return value */
	int result;
};

#endif /* _SYSCALL_H_ */
