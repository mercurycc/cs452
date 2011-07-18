#ifndef _TRACK_RESERVE_H_
#define _TRACK_RESERVE_H_

#include <types.h>
#include "track_node.h"
#include "train_types.h"

enum Track_reserve_return {
	RESERVE_SUCCESS,
	RESERVE_FAIL_SAME_DIR,
	RESERVE_FAIL_AGAINST_DIR,
	RESERVE_FAIL_EXIT
};

enum Track_reserve_holds_return {
	RESERVE_NOT_HOLD,
	RESERVE_HOLD
};

/* Server */
void track_reserve();

int track_reserve_get_range( int tid, Train* train, int dist );
int track_reserve_get( int tid, Train* train, track_node* node );
int track_reserve_may_i( int tid, Train* train, track_node* node, int direction );
int track_reserve_put( int tid, Train* train, track_node* node );
int track_reserve_free( int tid, Train* train );

/* check if a block is hold, returns RESERVE_HOLD or RESERVE_NOT_HOLD */
int track_reserve_holds( int tid, Train* train, track_node* node );

#endif /* _TRACK_RESERVE_H_ */
