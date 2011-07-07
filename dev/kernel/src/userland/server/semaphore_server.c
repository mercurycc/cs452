#include <user/syscall.h>
#include <user/semaphore.h>
#include <user/name_server.h>
#include <user/assert.h>
#include <lib/rbuf.h>
#include <config.h>
#include <err.h>

enum Sem_request_type {
	SEM_ACQUIRE,
	SEM_RELEASE,
	SEM_AC_ALL
};

typedef struct Sem_request_s {
	uint type;
	Semaphore* sem;
} Sem_request;

void sem_server()
{
	Sem_request request;
	Semaphore* sem;
	int tid;
	int status;

	status = RegisterAs( SEMAPHORE_SERVER_NAME );
	assert( status == REGISTER_AS_SUCCESS );
	
	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );

		sem = request.sem;

		switch( request.type ){
		case SEM_RELEASE:
			sem->count += 1;
			status = Reply( tid, ( char* )&status, sizeof( status ) );
			assert( status == SYSCALL_SUCCESS );
			break;
		case SEM_AC_ALL:
			if( sem->count > 0 ){
				sem->count = 1;
			}
			// Fall through
		case SEM_ACQUIRE:
			status = rbuf_put( &sem->wait_queue, ( uchar* )&tid );
			assert( status == ERR_NONE );
			break;
		}

		while( sem->count > 0 && ! rbuf_empty( &sem->wait_queue ) ){
			sem->count -= 1;
			rbuf_get( &sem->wait_queue, ( uchar* )&tid );
			status = Reply( tid, ( char* )&status, sizeof( status ) );
			assert( status == SYSCALL_SUCCESS );
		}
	}
}

int sem_init( Semaphore* sem, int count )
{
	sem->srv_tid = WhoIs( SEMAPHORE_SERVER_NAME );
	sem->count = count;
	rbuf_init( &sem->wait_queue, ( uchar* )sem->wait_queue_buf, sizeof( int ), sizeof( int ) * SEMAPHORE_WAIT_QUEUE_SIZE );

	return ERR_NONE;
}

static int sem_request( Semaphore* sem, uint type )
{
	Sem_request request;
	int status;

	request.type = type;
	request.sem = sem;

	status = Send( sem->srv_tid, ( char* )&request, sizeof( request ), ( char* )&status, sizeof( status ) );
	assert( status == sizeof( status ) );

	return ERR_NONE;
}

int sem_acquire( Semaphore* sem )
{
	return sem_request( sem, SEM_ACQUIRE );
}

int sem_release( Semaphore* sem )
{
	return sem_request( sem, SEM_RELEASE );
}

int sem_acquire_all( Semaphore* sem )
{
	return sem_request( sem, SEM_AC_ALL );
}
