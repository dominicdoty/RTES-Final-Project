// Program Main
// ECEN 5623 RTES
// Final Project


/* HEADER */
#include <pthread.h>
#include <stdint.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "servo.h"


/* DEFINES AND TYPEDEFS */
typedef void*(*pthread_routine_t)(void*);
typedef void* pthread_argument_t;

typedef struct {
	pthread_t thread;
	pthread_attr_t attr;
	pthread_routine_t function;
	pthread_argument_t argument;
	struct sched_param sched_param;
	char* semaphore_name;
}pthread_container_t;


#define NUM_THREADS 1

/* GLOBALS */


/* FUNCTION DEFINITIONS */

int main(void)
{
	// Thread Stuff
	pthread_container_t threads[NUM_THREADS] = {0};

	// Set thread functions, arguments (default to null), and priorities here.
	threads[0].function = servo_plan;
	threads[0].sched_param.sched_priority = 5;
	threads[0].semaphore_name = SERVO_SEM;

	// Set up default attributes for all threads
	for(uint_fast8_t i = 0; i < NUM_THREADS; i++)
	{
		pthread_attr_init(&threads[i].attr);
		pthread_attr_setschedpolicy(&threads[i].attr, SCHED_FIFO);
		pthread_attr_setschedparam(&threads[i].attr, &threads[i].sched_param);
	}

	// Do any thread attribute modifications here

	// Create all the threads
	for(uint_fast8_t i = 0; i < NUM_THREADS; i++)
	{
		sem_open(threads[i].semaphore_name, O_RDWR | O_CREAT, S_IRWXU, 0);
		pthread_create(&threads[i], &threads[i].attr, &threads[i].function, &threads[i].argument);
	}

	// Dispatcher
}