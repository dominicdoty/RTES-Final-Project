// Servo Control Service
// ECEN 5623 RTES
// Final Project

#ifndef SERVO_H
#define SERVO_H

/* HEADERS */
#include <stdint.h>


/* DEFINES AND TYPEDEFS */
#define SERVO_SEM "servo"

/* GLOBALS */


/* FUNCTION DECLARATIONS */
void servo_plan(void);

void servo_execute(void);


#endif // SERVO_H