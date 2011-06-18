#include <types.h>
#include <err.h>
#include <user/assert.h>
#include <user/syscall.h>
#include <user/clock_server.h>
#include <user/devices/clock.h>
#include <user/train.h>
#include <user/uart.h>
#include <user/display.h>
#include <user/lib/sync.h>

#define LIST_SIZE 8

#define TRAIN_SENSOR_MAGIC 0x4778B8FB


typedef struct Sensor_name_s {
	char group;
	char number[2];
} Sensor_name;

int parse_sensor( char data, int group, Sensor_name* list, int head ){
	if ( !data ) {
		return head;
	}
	int i;
	int n;
	for ( i = 0; i < 8; i++ ) {
		if ( data & ((0x1 << 7) >> i) ){
			head = (head + LIST_SIZE - 1) % LIST_SIZE;
			n = group % 2 * 8 + i + 1;
			list[head].group = (group / 2) + 'A';
			list[head].number[0] = n / 10 + '0';
			list[head].number[1] = n % 10 + '0';
		}
	}
	return head;
}

void train_sensor() {
	int quit = 0;
	int status;
	int i;
	int tid;

	char sensor_table[10];
	for ( i = 0; i < 10; i++ ) {
		sensor_table[i] = 0;
	}

	Sensor_name sensor_list[LIST_SIZE];
	int head = 0;
	for ( i = 0; i < LIST_SIZE; i++ ){
		sensor_list[i].group = ' ';
		sensor_list[i].number[0] = ' ';
		sensor_list[i].number[1] = ' ';
	}

	char buffer[40];
	for ( i = 0; i < 40; i++ ){
		if ( (i+1) % 5 ) {
			buffer[i] = ' ';
		}
		else {
			buffer[i] = '\n';
		}
	}

	Region sensor_rect = { 59, 4, 11, 16, 0, 1 };
	Region *sensor_region = &sensor_rect;
	status = region_init( sensor_region );
	assert( status == ERR_NONE );

	while ( !quit ) {

		status = train_all_sensor();
		assert( status == 0 );
		
		for ( i = 0; i < 10; i++ ) {
			char c = Getc( COM_1 );
			if ( sensor_table[i] != c ) {
				sensor_table[i] = c;
				//sensor_table[i] = Getc( COM_1 );
				head = parse_sensor( sensor_table[i], i, sensor_list, head );
			}
		}

		int magic = 0;
		status = Receive( &tid, (char*)&magic, sizeof(magic) );
		assert( status == sizeof(magic) );
		assert( magic == TRAIN_SENSOR_MAGIC );

		int reply = 0;
		if ( sensor_list[head].group != ' ' ){
			reply = (sensor_list[head].group - 'A')*32 + ((sensor_list[head].number[0]-'0')*10 + (sensor_list[head].number[1] - '0'));
		}
		status = Reply( tid, (char*)&reply, sizeof(reply) );
		assert( status == 0 );

		status = region_printf( sensor_region, "Sensors:\n %c%c%c\n %c%c%c\n %c%c%c\n %c%c%c\n %c%c%c\n %c%c%c\n %c%c%c\n %c%c%c\n",
			sensor_list[head%LIST_SIZE].group, sensor_list[head%LIST_SIZE].number[0], sensor_list[head%LIST_SIZE].number[1],
			sensor_list[(head+1)%LIST_SIZE].group, sensor_list[(head+1)%LIST_SIZE].number[0], sensor_list[(head+1)%LIST_SIZE].number[1],
			sensor_list[(head+2)%LIST_SIZE].group, sensor_list[(head+2)%LIST_SIZE].number[0], sensor_list[(head+2)%LIST_SIZE].number[1],
			sensor_list[(head+3)%LIST_SIZE].group, sensor_list[(head+3)%LIST_SIZE].number[0], sensor_list[(head+3)%LIST_SIZE].number[1],
			sensor_list[(head+4)%LIST_SIZE].group, sensor_list[(head+4)%LIST_SIZE].number[0], sensor_list[(head+4)%LIST_SIZE].number[1],
			sensor_list[(head+5)%LIST_SIZE].group, sensor_list[(head+5)%LIST_SIZE].number[0], sensor_list[(head+5)%LIST_SIZE].number[1],
			sensor_list[(head+6)%LIST_SIZE].group, sensor_list[(head+6)%LIST_SIZE].number[0], sensor_list[(head+6)%LIST_SIZE].number[1],
			sensor_list[(head+7)%LIST_SIZE].group, sensor_list[(head+7)%LIST_SIZE].number[0], sensor_list[(head+7)%LIST_SIZE].number[1]
 );
		assert( status == 0 );

	}

	Exit();
}

int sensor_last( int tid ){
	int result = TRAIN_SENSOR_MAGIC;
	int status = Send( tid, (char*)&result, sizeof(result), (char*)&result, sizeof(result) );
	assert( status == sizeof(result) );
	return result;
}

