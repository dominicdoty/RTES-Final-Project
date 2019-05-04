// Program Main
// ECEN 5623 RTES
// Final Project


/* HEADER */
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include "servo.h"
#include "cam_cv.hpp"

#define DEBUG 1
#include "debug.h"

#include "data.h"

/* DEFINES AND TYPEDEFS */
typedef void*(*pthread_routine_t)(void*);
typedef void* pthread_argument_t;

typedef struct {
	pthread_t thread;
	pthread_attr_t attr;
	pthread_routine_t function;
	pthread_argument_t argument;
	struct sched_param sched_param;
}pthread_container_t;

#define NUM_THREADS 2


/* GLOBALS */

//extern int *buffer0;
//extern int *buffer1;

//extern pthread_mutex_t mutex0;
//extern pthread_mutex_t mutex1;


/* FUNCTION DEFINITIONS */

int main(void)
{
	int* buffer0 = new int[40];
	int* buffer1 = new int[40];
	pthread_mutex_t mutex0;
	pthread_mutex_t mutex1;

	pthread_mutex_init(&mutex0, NULL);
	pthread_mutex_init(&mutex1, NULL);

	// Elevate the Main Thread
	struct sched_param main_param = {.sched_priority = sched_get_priority_max(SCHED_FIFO)};
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &main_param);

	// init the mutex
	//sem_init(&cv_sem, 0, 0);

	// Thread Stuff
	pthread_container_t threads[NUM_THREADS] = {0};

	// Set thread functions, arguments (default to null), and priorities here.
	// Servo Planning Thread
	threads[0].function = servo_plan;
	threads[0].sched_param.sched_priority = 95;

	// setting the CV functions to be same priority as servo task
	threads[1].function = cam_lines;
	threads[1].sched_param.sched_priority = 95;

	//threads[2].function = cam_circles;
	//threads[2].sched_param.sched_priority = 95;

	// Set up default attributes for all threads
	for(uint_fast8_t i = 0; i < NUM_THREADS; i++)
	{
		pthread_attr_init(&threads[i].attr);
		pthread_attr_setschedpolicy(&threads[i].attr, SCHED_FIFO);
		pthread_attr_setschedparam(&threads[i].attr, &threads[i].sched_param);
	}
	debug_print("Attributes setup\n");

	// Do any thread attribute modifications here

	// Create all the threads
	for(uint_fast8_t i = 0; i < NUM_THREADS; i++)
	{
		pthread_create(&threads[i].thread, &threads[i].attr, threads[i].function, &threads[i].argument);
	}
	debug_print("All threads created\n");

	// Done
	debug_print("Main going to sleep\n");
	while(1)
	{
		pause();
	}


}
