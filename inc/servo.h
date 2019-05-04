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
// Point type
// Sized to hold up to a 3280x2464 (max RasPiCam resolution)
typedef struct
{
	uint32_t x;
	uint32_t y;
}point_t;

// Line type - (0,0) is bottom left corner of image
typedef struct
{
	point_t p0;
	point_t p1;
}line_xy_t;

// Line in Slope Intercept (y = mx + b)
typedef struct
{
	float m;
	int_fast32_t b;
}line_slope_int_t;

// Line in Point Slope ((y-y1) = m(x-x1))
typedef struct
{
	float m;
	point_t p;
}line_point_slope_t;


/* GLOBALS */
extern line_point_slope_t centerline;
extern uint_fast16_t speed_left_servo;
extern uint_fast16_t speed_right_servo;


/* FUNCTION DECLARATIONS */
void* servo_plan(void* args);

void* servo_execute(void* args);


#endif // SERVO_H