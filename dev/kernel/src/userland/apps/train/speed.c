#include <err.h>
#include <user/assert.h>
#include <lib/str.h>
#include "inc/config.h"
#include "inc/speed.h"
#include "inc/train_location.h"



int get_avg_speed( Speed* spd1, Speed* spd2, Speed* ret ){
/*
	int n1 = spd1->numerator;
	int d1 = spd1->denominator;
	int n2 = spd2->numerator;
	int d2 = spd2->denominator;
	
	long long int top = n1 * d2 + n2 * d1;
	long long int bot = 2 * d1 * d2;
	
	while ( top > 10000 || bot > 10000 ){
		top = top / 10;
		bot = bot / 10;
	}

	int s1 = n1 * 100 / d1;
	int s2 = n2 * 100 / d2;
	int s = top * 100 / bot;
	assert(( s1 <= s && s <= s2 )||( s1 >= s && s >= s2 ))
	ret->numerator = top;
	ret->numerator = bot;
*/
	return get_inter_speed( spd1, spd2, ret, 1, 2);
}

int get_inter_speed( Speed* spd1, Speed* spd2, Speed* ret, int factor, int total ){
	int a = spd1->numerator;
	int b = spd1->denominator;
	int c = spd2->numerator;
	int d = spd2->denominator;
	int f = factor;
	int t = total;
	
	long long int top = f * b * c - f * a * d + a * d * t;
	long long int bot = b * d * t;
	
	while ( top > 10000 || bot > 10000 ){
		top = top / 10;
		bot = bot / 10;
	}
	
	int s1 = a * 100 / b;
	int s2 = c * 100 / d;
	int s = top * 100 / bot;
	assert(( s1 <= s && s <= s2 )||( s1 >= s && s >= s2 ));
	ret->numerator = top;
	ret->denominator = bot;
	
	return 0;
}

int get_distance( Speed* spd, int time ){
	long long int ret =  spd->numerator * time / spd->denominator;
	assert( ret < INFINITY );
	return ret;
}

int get_distance_changing_speed( Speed* spd1, Speed* spd2, int time ){
	int a = spd1->numerator;
	int b = spd1->denominator;
	int c = spd2->numerator;
	int d = spd2->denominator;
	int t = time;
	
	long long int top = b * c * t + a * d * t;
	long long int bot = 2 * b * d;
	
	long long ret = top / bot;
	assert( ret < INFINITY );
	
	return ret;
}

int get_distance_changed_speed( Speed* spd1, Speed* spd2, int changetime, int totaltime ){
	assert( changetime <= totaltime );
	int a = spd1->numerator;
	int b = spd1->denominator;
	int c = spd2->numerator;
	int d = spd2->denominator;
	int ct = changetime;
	int tt = totaltime;
	
	long long int top = (b * c + a * d) * ct / 2 + b * c * (tt - ct);
	long long int bot = b * d;
	
	long long int ret = top / bot;
	
	assert( ret < INFINITY );
	
	return ret;
}

int get_stop_distance( Speed* spd ){
	return get_distance( spd, SPEED_CHANGE_TIME / 2 );
}
