#ifndef _TRAIN_TRACK_NODE_H_
#define _TRAIN_TRACK_NODE_H_

#include "config.h"

struct Train_data_s;

typedef enum {
	NODE_NONE,
	NODE_SENSOR,
	NODE_BRANCH,
	NODE_MERGE,
	NODE_ENTER,
	NODE_EXIT
} node_type;

enum Graph_node_group {
	GROUPA = 0,
	GROUPB,
	GROUPC,
	GROUPD,
	GROUPE,
	GROUPMR,
	GROUPBR,
	GROUPEX,
	GROUPEN,
	GROUP_COUNT
};

#define TRACK_GRAPH_NODES_PER_GROUP   22

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1
#define TRACK_NUM_NODES   144

struct track_node;
//struct track_edge;
typedef struct track_node track_node;
typedef struct track_edge track_edge;
typedef struct Track_reserve_s Track_reserve;

struct Track_reserve_s {
	struct Train_data_s* train;
	int version;
	int from;
	int to;
};

struct track_edge {
	track_edge *reverse;
	track_node *src, *dest;
	uint dist;             /* in millimetres */
	
	/* Near-distance reservation */
	Track_reserve close_reserves[ MAX_NUM_TRAINS ];

	/* Long term reservation */
	int reserve_from;     /* Reserve time period */
	int reserve_to;
};

struct track_node {
	const char *name;
	int group;
	int id;
	int index;
	int broken;
	node_type type;
	int num;              /* sensor or switch number */
	track_node *reverse;  /* same location, but opposite direction */
	track_edge edge[2];
};

/* track node name conversion */
int track_node_id2name( char* str, int group, int id );
int track_node_name2id( const char* str, int* group, int* id );

#endif /* _TRAIN_TRACK_NODE_H_ */
