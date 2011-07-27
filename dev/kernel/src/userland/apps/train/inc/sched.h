#ifndef _TRAIN_SCHED_H_
#define _TRAIN_SCHED_H_

#include "train_types.h"
#include "track_node.h"

void train_sched();
/* Return a positive ticket.  0 for error */
int train_sched_add( int tid, track_node* dest, int dest_dist, track_node* src, int src_dist, int deadline, Train* train );
/* Cancel a trip that has not be assigned */
int train_sched_cancel( int tid, int ticket );
int train_sched_suspend( int tid );
int train_sched_resume( int tid );

#endif /* _TRAIN_SCHED_H_ */
