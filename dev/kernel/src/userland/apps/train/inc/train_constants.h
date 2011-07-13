#ifndef _TRAIN_CONSTANTS_H_
#define _TRAIN_CONSTANTS_H_

/* 999 means not measured yet */
#define TRAIN_HEAD_LENGTH( i ) ( i > 22 ? ( i == 23 ? 30 : 25 ) : ( i == 21 ? 20 : 999 ) )
#define TRAIN_TAIL_LENGTH( i ) ( i > 22 ? ( i == 23 ? 140 : 145 ) : ( i == 21 ? 120 : 999 ) )

#define TRAIN_SPEED_PRED_FACTOR( i ) ( i > 21 ? ( i == 23 ? 55 : 0 ) : ( i == 21 ? 22 : 0 ) )

/* Measurements:
 * pickup = 50mm;
 * 21 : head = 20mm; tail = 120mm;
 * 23 : head = 30mm; tail = 140mm;
 * 24 : head = 25mm; tail = 145mm;
 */


static inline int train_head_length( int id ){
	switch ( id ) {
	case 21:
		return 20;
	case 23:
		return 30;
	case 24:
		return 25;
	default:
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
	}
	return 0;
}

#endif /* _TRAIN_CONSTANTS_H_ */
