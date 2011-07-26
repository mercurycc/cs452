#ifndef _TRAIN_CONSTANTS_H_
#define _TRAIN_CONSTANTS_H_


/* Measurements:
 * pickup = 50mm;
 * 21 : head = 20mm; tail = 120mm;
 * 23 : head = 30mm; tail = 140mm;
 * 24 : head = 25mm; tail = 145mm;
 */

#define PICKUP_SIZE 50

static inline int train_head_length( int id ){
	switch ( id ) {
	case 21:
		return 20;
	case 23:
		return 30;
	case 24:
		return 25;
	default:
		break;
	}
	return 0;
}

static inline int train_tail_length( int id ){
	switch ( id ) {
	case 21:
		return 120;
	case 23:
		return 140;
	case 24:
		return 145;
	default:
		break;
	}
	return 0;
}

static inline int train_full_length( int id ){
	return PICKUP_SIZE + train_head_length( id ) + train_tail_length( id );
}

#endif /* _TRAIN_CONSTANTS_H_ */
