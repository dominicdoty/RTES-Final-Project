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


#define DEBUG 1
#include "debug.h"

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

#define NUM_THREADS 1
#define NSEC_PER_MSEC 1000000000

/* GLOBALS */


/* FUNCTION DEFINITIONS */

int main(void)
{
	// Elevate the Main Thread
	struct sched_param main_param = {.sched_priority = sched_get_priority_max(SCHED_FIFO)};
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &main_param);

	// Thread Stuff
	pthread_container_t threads[NUM_THREADS] = {0};

	// Set thread functions, arguments (default to null), and priorities here.
	// Servo Planning Thread
	threads[0].function = servo_plan;
	threads[0].sched_param.sched_priority = 95;

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