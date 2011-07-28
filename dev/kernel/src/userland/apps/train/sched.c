#include <types.h>
#include <user/syscall.h>
#include <user/semaphore.h>
#include <user/name_server.h>
#include <user/assert.h>
#include <lib/rbuf.h>
#include <lib/str.h>
#include <err.h>
#include "inc/config.h"
#include "inc/train.h"
#include "inc/sched.h"

#define LOCAL_DEBUG
#include <user/dprint.h>

enum Train_sched_request_types {
	TRAIN_SCHED_NEW_TRAIN,
	TRAIN_SCHED_ADD,
	TRAIN_SCHED_CANCEL,
	TRAIN_SCHED_SUSPEND,
	TRAIN_SCHED_RESUME,
	TRAIN_SCHED_UPDATE
};

typedef struct Train_assign_s {
	Train* train;
	int dest_assign;
	int dest_group;
	int dest_id;
	int dest_dist;
	int src_assign;
	int src_group;
	int src_id;
	int src_dist;
} Train_assign;

typedef struct Train_sched_request_s {
	int type;
	int ticket;
	Train_assign assign;
	int deadline;
} Sched_request;

typedef struct Train_sched_reply_s {
	int ticket;
} Sched_reply;

static void train_sched_alarm()
{
	int ptid = MyParentTid();

	while( 1 ){
		Delay( 100 );

		train_sched_update( ptid );
	}
}

void train_sched()
{
	Sched_request request;
	Sched_reply reply;
	Train_assign trains[ MAX_NUM_TRAINS ];
	Train_assign* current_assign;
	Sched_request request_buf[ SCHEDULER_BUFFER ];
	Rbuf ring_body;
	Rbuf* request_pool = &ring_body;
	int available_trains = 0;
	int ticket = 1;
	int status;
	int control = 1;
	int auto_tid;
	int i;
	int temp;
	int tid;

	dnotice( "Sched launched\n" );

	status = RegisterAs( TRAIN_SCHED_NAME );
	assert( status == REGISTER_AS_SUCCESS );

	auto_tid = WhoIs( TRAIN_AUTO_NAME );
	assert( auto_tid > 0 );

	status = rbuf_init( request_pool, ( uchar* )request_buf, sizeof( Sched_request ), sizeof( Sched_request ) * SCHEDULER_BUFFER );
	assert( status == ERR_NONE );

	status = Create( TRAIN_SCHED_PRIORITY + 1, train_sched_alarm );
	assert( status > 0 );

	temp = 1;
	control = 1;

	while( 1 ){
		status = Receive( &tid, ( char* )&request, sizeof( request ) );
		ASSERT_M( status == sizeof( request ), "expect %d, got %d\n", sizeof( request ), status );

		switch( request.type ){
		case TRAIN_SCHED_NEW_TRAIN:
			memcpy( ( uchar* )( trains + available_trains ), ( uchar* )&request.assign, sizeof( request.assign ) );
			available_trains += 1;
			dprintf( "Received registration of train %d\n", request.assign.train->id );
			break;
		case TRAIN_SCHED_ADD:
			request.ticket = ticket;
			ticket += 1;
			rbuf_put( request_pool, ( uchar* )&request );
			break;
		case TRAIN_SCHED_CANCEL:
			{
				int request_ticket = request.ticket;
				temp = 0;
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

		status = Reply( tid, ( char* )&reply, sizeof( reply ) );
		assert( status == SYSCALL_SUCCESS );

		if( control ){
			for( i = 0; i < available_trains; i += 1 ){
				current_assign = trains + i;
				while( 1 ){
					sem_acquire_all( current_assign->train->sem );
					temp = train_planner_have_control( current_assign->train );
					sem_release( current_assign->train->sem );

					if( temp ){
						break;
					}
				
					if( current_assign->src_assign ){
						train_auto_plan( auto_tid, current_assign->train->id,
								 current_assign->src_group,
								 current_assign->src_id,
								 current_assign->src_dist );
						dprintf( "Plan source trip for train %d\n", current_assign->train->id );
						current_assign->src_assign = 0;
					} else if( current_assign->dest_assign ){
						train_auto_plan( auto_tid, current_assign->train->id,
								 current_assign->dest_group,
								 current_assign->dest_id,
								 current_assign->dest_dist );
						dprintf( "Plan destination trip for train %d\n", current_assign->train->id );
						current_assign->dest_assign = 0;
					} else if( !rbuf_empty( request_pool ) ){
						rbuf_get( request_pool, ( uchar* )&request );
						request.assign.train = current_assign->train;
						request.assign.dest_assign = 1;
						request.assign.src_assign = 1;
						dprintf( "Assigning trip %d for train %d\n", request.ticket, current_assign->train->id );
						memcpy( ( uchar* )current_assign, ( uchar* )&request.assign, sizeof( request.assign ) );
						continue;
					}
					break;
				}
			}					
		}
	}
}

static inline int train_sched_request( int tid, int type, int ticket, Train_assign* assign, Sched_reply* reply )
{
	Sched_request request;
	Sched_reply local_reply;
	int status;

	request.type = type;
	request.ticket = ticket;

	if( assign ){
		memcpy( ( uchar* )&request.assign, ( uchar* )assign, sizeof( request.assign ) );
	}

	if( ! reply ){
		reply = &local_reply;
	}

	status = Send( tid, ( char* )&request, sizeof( request ), ( char* )reply, sizeof( local_reply ) );
	assert( status == sizeof( Sched_reply ) );

	return ERR_NONE;
}

int train_sched_new_train( int tid, Train* train )
{
	Train_assign assign = { 0 };

	assign.train = train;
	
	return train_sched_request( tid, TRAIN_SCHED_NEW_TRAIN, 0, &assign, 0 );
}

int train_sched_add( int tid, int dest_group, int dest_id, int dest_dist, int src_group, int src_id, int src_dist, Train* train )
{
	Sched_reply reply;
	Train_assign assign;
	int status;

	assign.dest_group = dest_group;
	assign.dest_id = dest_id;
	assign.dest_dist = dest_dist;
	assign.src_group = src_group;
	assign.src_id = src_id;
	assign.src_dist = src_dist;

	status = train_sched_request( tid, TRAIN_SCHED_ADD, 0, &assign, &reply );
	assert( status == ERR_NONE );

	return reply.ticket;
}

int train_sched_cancel( int tid, int ticket )
{
	return train_sched_request( tid, TRAIN_SCHED_CANCEL, ticket, 0, 0 );
}

int train_sched_suspend( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_SUSPEND, 0, 0, 0 );
}

int train_sched_resume( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_RESUME, 0, 0, 0 );
}

int train_sched_update( int tid )
{
	return train_sched_request( tid, TRAIN_SCHED_UPDATE, 0, 0, 0 );
}
