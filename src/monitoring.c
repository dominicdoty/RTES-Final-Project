
#include "servo.h"
#include <stdio.h>

#define DATA_POINTS (100)

void IntToAscii(uint_fast16_t n);

#define MAX_CHARS_PER_LINE (5 + 1 + 5 + 1 + 10 + 1)
unsigned char fileData[DATA_POINTS * MAX_CHARS_PER_LINE];
unsigned char* fileDataPtr; = &fileData[0];

void* monitor(void* args)
{
  // Init GPIO (probably already called by servo service)
  // gpioInitialise();
	
	// gpioSetMode(5, PI_OUTPUT); // Set pin 5 (reset) as an output
	// gpioWrite(5, 1); // force high to prevent reset 
	
	FILE* file = fopen("Monitor.txt", "wb");
	if(file == NULL)
	{
		debug_print("Error Creating File\n");
    return -1;
	}

	fileDataPtr = &fileData[0];
	
	for(int i = 0; i < DATA_POINTS; i+=1)
	{
		IntToAscii(speed_right_servo);
		
		*fileDataPtr = ' ';
		fileDataPtr++;
		
		IntToAscii(speed_left_servo);
		
		*fileDataPtr = ' ';
		fileDataPtr++
		
		IntToAscii(centerline.p.x);
		
		*fileDataPtr = '\n';
		fileDataPtr++;
		
		// centerline.m // slope
		// to be added if there is time
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
	}
	
	fwrite(&fileData[0], sizeof(char), fileDataPtr - &fileData[0],file);
	
	fclose(file);
	file = NULL;
	// never return
	while(1){}
}

void IntToAscii(int n){
  if(n >= 10){
    IntToAscii(n/10);
    n = n%10;
  }
	*fileDataPtr = n + '0';
  fileDataPtr++;
}