#ifndef _TRACK_RESERVE_H_
#define _TRACK_RESERVE_H_

#include <types.h>
#include "track_node.h"

/* Server */
void track_reserve();

int track_reserve_init( int tid, track_node* track_graph, int* switch_table );
int track_reserve_get_range( int tid, track_node* node, int dist );
int track_reserve_get( int tid, track_node* node );
int track_reserve_put( int tid, track_node* node );

#endif /* _TRACK_RESERVE_H_ */
