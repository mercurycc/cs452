#ifndef _USER_SEMAPHORE_H_
#define _USER_SEMAPHORE_H_

#include <types.h>
#include <lib/rbuf.h>
#include <config.h>

typedef struct Semaphore_s {
	int srv_tid;
	int count;
	Rbuf wait_queue;
	int wait_queue_buf[ SEMAPHORE_WAIT_QUEUE_SIZE ];
} Semaphore;

void sem_server();
int sem_init( Semaphore* sem, int count );
int sem_acquire( Semaphore* sem );
int sem_release( Semaphore* sem );
int sem_acquire_all( Semaphore* sem );

#endif /* _USER_SEMAPHORE_H_ */
