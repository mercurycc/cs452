#ifndef _TRAIN_TRACK_NODE_H_
#define _TRAIN_TRACK_NODE_H_

typedef enum {
	NODE_NONE,
	NODE_SENSOR,
	NODE_BRANCH,
	NODE_MERGE,
	NODE_ENTER,
	NODE_EXIT
} node_type;

#define DIR_AHEAD 0
#define DIR_STRAIGHT 0
#define DIR_CURVED 1
#define TRACK_NUM_NODES   150

struct track_node;
typedef struct track_node track_node;
typedef struct track_edge track_edge;

struct track_edge {
	track_edge *reverse;
	track_node *src, *dest;
	int dist;             /* in millimetres */
};

struct track_node {
	const char *name;
	int group;
	int id;
	node_type type;
	int num;              /* sensor or switch number */
	track_node *reverse;  /* same location, but opposite direction */
	track_edge edge[2];
};

#endif /* _TRAIN_TRACK_NODE_H_ */
