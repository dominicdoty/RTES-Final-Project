// Servo Control Service
// ECEN 5623 RTES
// Final Project


/* HEADER */
#include "servo.h"

#define DEBUG 1
#include "debug.h"

/* DEFINES AND TYPEDEFS */
#define PERIOD				20		// period in ms
#define NSEC_PER_MSEC		1000000
#define NSEC_PER_SEC		1000000000

#define IMAGE_WIDTH			960		// image width in pixels
#define CENTERING_TOLERANCE	20		// ± around center of image that's considered "centered"
#define SLOPE_TOLERANCE		500		// Slope deviation allowed before turning

#define IMAGE_CENTER_X			(IMAGE_WIDTH/2)
#define CENTERED_RIGHT_THRESH	(IMAGE_CENTER_X + CENTERING_TOLERANCE)
#define CENTERED_LEFT_THRESH	(IMAGE_CENTER_X - CENTERING_TOLERANCE)
#define SLOPE_POS_THRESH		(SLOPE_TOLERANCE)
#define SLOPE_NEG_THRESH		(-SLOPE_TOLERANCE)

// Point type
// Sized to hold up to a 3280x2464 (max RasPiCam resolution)
typedef struct
{
	uint_fast16_t x;
	uint_fast16_t y;
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
	int_fast16_t m;
	int_fast16_t b;
}line_slope_int_t;

// Line in Point Slope ((y-y1) = m(x-x1))
typedef struct
{
	int_fast16_t m;
	point_t p;
}line_point_slope_t;


/* GLOBALS */


/* STATIC FUNCTION DECLARATIONS */
line_point_slope_t find_centerline(line_xy_t line_0, line_xy_t line_1);


/* FUNCTION DEFINITIONS */
void* servo_plan(void* args)
{
	// Get timing set up
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	time.tv_sec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
	time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;

	while(1)
	{
		// Sleep to achieve the set period	
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);

		debug_print_time();
		debug_print(" - servo_planner start\n");

		// Need to add something here about how the data gets into the service
		// Message queue or something?

		// assume I know have these from image service
		line_xy_t line_0 = {{5,2}, {200,800}};
		line_xy_t line_1 = {{1200,150},{1600,900}};

		// Get the centerline
		line_point_slope_t centerline = find_centerline(line_0, line_1);
		debug_print("Centerline slope %d, point (%d, %d)\n", centerline.m, centerline.p.x, centerline.p.y);

		// Determine what to do
		if((centerline.p.x > CENTERED_RIGHT_THRESH) || (centerline.p.x < CENTERED_LEFT_THRESH))
		{
			debug_print("Not Centered, go fwd\n");
			// We're not centered between the lines
			// Drive forward
		}
		else if(centerline.m > SLOPE_POS_THRESH)
		{
			debug_print("Turn Right\n");
			// We're centered, but not going straight between the lines
			// Turn right proportional to the slope
		}
		else if(centerline.m < SLOPE_NEG_THRESH)
		{
			debug_print("Turn Left\n");
			// We're centered, but not going straight between the lines
			// Turn left proportional to the slope
		}
		else
		{
			debug_print("Centered and straight, go fwd\n");
			// We're centered and going straight
			// go forwards
		}

		debug_print_time();
		debug_print(" - servo planner end\n");
		time.tv_sec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
		time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;
	}
}



line_point_slope_t find_centerline(line_xy_t line_0, line_xy_t line_1)
{
	// find equation of line_0
	line_slope_int_t line_0_ps;
	line_0_ps.m = (line_0.p0.y - line_0.p1.y)/(line_0.p0.x - line_0.p1.x);
	line_0_ps.b = line_0.p0.y - (line_0_ps.m * line_0.p0.x);

	// find the equation of line_1
	line_slope_int_t line_1_ps;
	line_1_ps.m = (line_1.p0.y - line_1.p1.y)/(line_1.p0.x - line_1.p1.x);
	line_1_ps.b = line_1.p0.y - (line_1_ps.m * line_1.p0.x);

	// find the intersection point
	line_point_slope_t centerline;

	// check for parallel
	if(line_0_ps.m == line_1_ps.m)
	{
		// parallel
		centerline.p.x = (line_0.p0.x + line_0.p1.x + line_1.p0.x + line_1.p1.x)/4;
		centerline.p.y = (line_0.p0.y + line_0.p1.y + line_1.p0.y + line_1.p1.y)/4;
		centerline.m = line_0_ps.m;
	}
	else
	{
		// intersection can be found
		centerline.p.x = (line_0_ps.m - line_1_ps.m)/(line_1_ps.b - line_0_ps.b);
		centerline.p.y = (line_0_ps.m * centerline.p.x) + line_0_ps.b;
		centerline.m = (line_0_ps.m + line_1_ps.m)/2;
	}

	return centerline;
}