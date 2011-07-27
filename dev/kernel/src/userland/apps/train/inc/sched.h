#ifndef _TRAIN_SCHED_H_
#define _TRAIN_SCHED_H_

#include "train_types.h"
#include "track_node.h"

void train_sched();
/* Return a positive ticket.  0 for error */
int train_sched_add( int tid, int dest_group, int dest_id, int dest_dist, int src_group, int src_id, int src_dist, Train* train );
/* Cancel a trip that has not be assigned */
int train_sched_cancel( int tid, int ticket );
int train_sched_suspend( int tid );
int train_sched_resume( int tid );
int train_sched_update( int tid );

#endif /* _TRAIN_SCHED_H_ */
