// Servo Control Service
// ECEN 5623 RTES
// Final Project

#ifndef SERVO_H
#define SERVO_H

/* HEADERS */
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <pigpio.h>

/* DEFINES AND TYPEDEFS */


/* GLOBALS */


/* FUNCTION DECLARATIONS */
void* servo_plan(void* args);

void* servo_execute(void* args);


#endif // SERVO_H