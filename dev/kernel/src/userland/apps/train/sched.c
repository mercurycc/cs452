#include <types.h>
#include <user/syscall.h>
#include <user/semaphore.h>
#include <lib/rbuf.h>
#include <err.h>
#include "inc/config.h"
#include "inc/train.h"

enum Train_sched_request_types {
	TRAIN_SCHED_NEW_TRAIN,
	TRAIN_SCHED_ADD,
	TRAIN_SCHED_CANCEL,
	TRAIN_SCHED_SUSPEND,
	TRAIN_SCHED_RESUME,
	TRAIN_SCHED_UPDATE
};

typedef struct Train_sched_request_s {
	int type;
	int ticket;
	track_node* dest;
	int dest_dist;
	track_node* src;
	int src_dist;
	int deadline;
	Train* train;
} Sched_request;

typedef struct Train_sched_reply_s {
	int ticket;
} Sched_reply;

typedef struct Train_assign_s {
	Train* train;
	track_node* dest;
	int dest_dist;
	track_node* src;
	int src_dist;
} Train_assign;

void train_sched()
{
	Sched_request request;
	Sched_reply reply;
	Train_assign trains[ MAX_NUM_TRAINS ] = { 0 };
	Train* current_train;
	Sched_request request_buf[ SCHEDULER_BUFFER ];
	Rbuf ring_body;
	Rbuf* request_pool = &ring_body;
	int available_trains = 0;
	int ticket = 1;
	int status;
	int control = 1;
	int auto_tid;
	int i;
	int tid;

	status = RegisterAs( TRAIN_SCHED_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	auto_tid = WhoIs( TRAIN_AUTO_NAME );
	assert( auto_tid > 0 );

	status = rbuf_init( request_pool, ( uchar* )request_buf, sizeof( Sched_request ), sizeof( Sched_request ) * SCHEDULER_BUFFER );
	assert( status == ERR_NONE );

	while( 1 ){
		status = Receive( &tid, ( uchar* )&request, sizeof( request ) );
		assert( status == sizeof( request ) );

		switch( request.type ){
		case TRAIN_SCHED_NEW_TRAIN:
			trains[ available_trains ].train = request.train;
			available_trains += 1;
			break;
		case TRAIN_SCHED_ADD:
			request.ticket = ticket;
			ticket += 1;
			rbuf_put( request_pool, ( uchar* )&request );
			break;
		case TRAIN_SCHED_CANCEL:
			{
				int temp = 0;
				int request_ticket = request.ticket;
				while( ! rbuf_empty( request_pool ) ){
					status = rbuf_get( request_pool, ( uchar* )&request );
					if( ! temp ){
						temp = request.ticket;
					} else if( temp == request.ticket ){
						break;
					}
					if( request.ticket == request_ticket ){
						break;
					} else {
						rbuf_put( request_pool, ( uchar* )&request );
					}
				}
			}
			break;
		case TRAIN_SCHED_SUSPEND:
			control = 0;
			break;
		case TRAIN_SCHED_RESUME:
			control = 1;
			break;
		case TRAIN_SCHED_UPDATE:
		default:
			break;
		}

		status = Reply( tid, ( uchar* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( control ){
			for( i = 0; i < available_trains; i += 1 ){
				current_train = trains[ i ].train;
				sem_acquire_all( current_train->sem );
				while( ! train_planner_have_control( current_train ) ){
					if( current_train->src ){
						train_auto_plan( auto_tid, current_train->train->id,
								 current_train->src->group,
								 current_train->src->id,
								 current_train->src_dist );
						current_train->src = 0;
					} else if( current_train->dest ){
						train_auto_plan( auto_tid, current_train->train->id,
								 current_train->dest->group,
								 current_train->dest->id,
								 current_train->dest_dist );
						current_train->dest = 0;
					} else if( !rbuf_empty( request_pool ) ){
						rbuf_get( request_pool, ( uchar* )&request );
						current_train->src = request.src;
						current_train->src_dist = request.src_dist;
						current_train->dest = request.dest;
						current_train->dest_dist = request.dest_dist;
						continue;
					}
					break;
				}
				sem_release( current_train->sem );
			}					
		}
	}
}

static inline int train_sched_request( int tid, int type, int ticket,
				       track_node* dest, track_node* src,
				       int dest_dist, int src_dist,
				       int deadline, Train* train, Sched_reply* reply )
{
	Sched_request request;
	Sched_reply local_reply;
	int status;

	request.type = type;
	request.ticket = ticket;
	request.dest = dest;
	request.dest_dist = dest_dist;
	request.src = src;
	request.src_dist = src_dist;
	request.deadline = deadline;
	request.train = train;

	if( !reply ){
		reply = &local_reply;
	}

	status = Send( tid, ( uchar* )&request, sizeof( Sched_request ), ( uchar* )reply, sizeof( Sched_reply ) );
	assert( status == sizeof( Sched_reply ) );

	return ERR_NONE;
}

int train_sched_new_train( int tid, Train* train )
{
	return train_sched_request( tid, TRAIN_SCHED_NEW_TRAIN, 0, 0, 0, train, 0 );
}

int train_sched_add( int tid, track_node* dest, int dest_dist, track_node* src, int src_dist, int deadline, Train* train )
{
	Sched_reply reply;
	int status;

	status = train_sched_request( tid, TRAIN_SCHED_ADD, 0, dest, dest_dist, src, src_dist, deadline, train, &reply );
	assert( status == ERR_NONE );

	return reply.ticket;
}

int train_sched_cancel( int tid, int ticket )
{
	return train_sched_request( tid, TRAIN_SCHED_CANCEL, ticket, 0, 0, 0, 0, 0, 0, 0 );
}

int train_sched_suspend( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_SUSPEND, 0, 0, 0, 0, 0, 0, 0, 0 );
}

int train_sched_resume( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_RESUME, 0, 0, 0, 0, 0, 0, 0, 0 );
}

int train_sched_update( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_UPDATE, 0, 0, 0, 0, 0, 0, 0, 0 );
}
