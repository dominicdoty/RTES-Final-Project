
#include "servo.h"
#include "monitoring.h"
#include "debug.h"
#include <stdio.h>
#include <syslog.h>

#define DATA_POINTS (100)
#define PERIOD	10

#define MAX_CHARS_PER_LINE (5 + 1 + 5 + 1 + 10 + 1)
unsigned char fileData[DATA_POINTS * MAX_CHARS_PER_LINE];
unsigned char* fileDataPtr = &fileData[0];

void* monitor(void* args)
{
	// Get timing set up
	struct timespec time;
	struct timespec time_ex;
	clock_gettime(CLOCK_MONOTONIC, &time);
	time.tv_sec += (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
	time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;
	
	FILE* file = fopen("Monitor.txt", "wb");
	if(file == NULL)
	{
		debug_print("Error Creating File\n");
		return (void*)(-1);
	}

	fileDataPtr = &fileData[0];
	
	for(int i = 0; i < DATA_POINTS; i++)
	{
//		clock_gettime(CLOCK_MONOTONIC, &time_ex);
//		syslog(LOG_CRIT, "monitor start time @ sec=%d.%9d\n", (int)(time_ex.tv_sec), (int)(time_ex.tv_nsec)); 

		IntToAscii(speed_right_servo);
		
		*fileDataPtr = ' ';
		fileDataPtr++;
		
		IntToAscii(speed_left_servo);
		
		*fileDataPtr = ' ';
		fileDataPtr++;
		
		IntToAscii(centerline.p.x);
		
		*fileDataPtr = '\n';
		fileDataPtr++;
		
		// centerline.m // slope
		// to be added if there is time

//		clock_gettime(CLOCK_MONOTONIC, &time_ex);
//		syslog(LOG_CRIT, "monitor end time @ sec=%d.%9d\n", (int)time_ex.tv_sec, (int)time_ex.tv_nsec);


		time.tv_sec += (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
		time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
	}
	
	fwrite(&fileData[0], sizeof(char), fileDataPtr - &fileData[0],file);
	
	fclose(file);
	file = NULL;
	// never return
	while(1){}
}

void IntToAscii(int n)
{
	if(n >= 10)
	{
		IntToAscii(n/10);
		n = n%10;
	}

	*fileDataPtr = n + '0';
	fileDataPtr++;
}
