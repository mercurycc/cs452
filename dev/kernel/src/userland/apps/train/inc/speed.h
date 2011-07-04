#ifndef _TRAIN_SPEED_C_
#define _TRAIN_SPEED_C_

#include <types.h>

typedef struct Speed_s {
	int numerator;
	uint denominator;
} Speed;


/* ret = (spd1 + spd2) / 2 */
int get_avg_speed( Speed* spd1, Speed* spd2, Speed* ret );
/* ret = (spd2 - spd1) * factor / total + spd1 */
int get_inter_speed( Speed* spd1, Speed* spd2, Speed* ret, int factor, int total );

/* distance = spd * time */
int get_distance( Speed* spd, int time );
/* distance = (spd1 + spd2) * time / 2 */
int get_distance_changing_speed( Speed* spd1, Speed* spd2, int time );
/* distance = (spd1 + spd2) * changetime / 2 + spd2 * (totaltime - changetime) */
int get_distance_changed_speed( Speed* spd1, Speed* spd2, int changetime, int totaltime );
/* stop_distance = spd * stop_time / 2 */
int get_stop_distance( Speed* spd );


#endif

