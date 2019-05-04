// Servo Control Service
// ECEN 5623 RTES
// Final Project


/* HEADER */
#include "servo.h"

//#define DEBUG 1
#include "debug.h"

/* DEFINES AND TYPEDEFS */
#define NSEC_PER_MSEC 	(1000000)
#define NSEC_PER_SEC	(1000000000)

#define PERIOD				50		// period in ms
#define IMAGE_WIDTH			960		// image width in pixels
#define IMAGE_CENTER_X		(IMAGE_WIDTH/2)

// Servo Pins
#define SERVO_L		26
#define SERVO_R		12

// Servo Speeds
#define SERVO_ZERO				1500
#define SERVO_STANDARD_SPEED	100

// Servo Direction Modifiers
#define SERVO_L_FW	(1)
#define SERVO_R_FW	(SERVO_L_FW * (-1))


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


/* STATIC FUNCTION DECLARATIONS */
// Find the slope intercept equation of a line
line_slope_int_t find_line_equation(line_xy_t line);

// Find the centerline of two lines
line_point_slope_t find_centerline(line_slope_int_t line_0, line_slope_int_t line_1);

// Perform PID control with given parameters
float pid_calculate(uint_fast8_t Kp,
							uint_fast8_t Ki,
							uint_fast8_t Kd,
							uint_fast16_t setpoint,
							float measurement);

// Output Control Values to the servos.
// Takes a signed number indicating steering direction and magnitude
void servo_command(uint_fast16_t steering_input);


/* FUNCTION DEFINITIONS */
void* servo_plan(void* args)
{
	// Init GPIO
	gpioInitialise();


	// Set up Double Buffers


	// Get timing set up
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	time.tv_sec += (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
	time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;

	while(1)
	{
		// Sleep to achieve the set period	
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
		debug_print_time();
		debug_print(" - servo_planner start\n");

		// Need to add something here about how the data gets into the service
		// Message queue or something?
		// Get array of lines, number of lines
		uint8_t num_lines = 6;
		line_xy_t lines[6] = {
			{{40,118},{183,2}},
			{{498,1},{637,54}},
			{{459,21},{627,118}},
			{{3,67},{131,2}},
			{{435,6},{627,117}},
			{{40,119},{183,3}},
		};

		// These store an accumulating average of slopes
		float average_slope_pos;
		int32_t average_intercept_pos;
		float average_slope_neg;
		int32_t average_intercept_neg;

		uint_fast8_t num_lines_pos = 0;
		uint_fast8_t num_lines_neg = 0;
		
		// Loop through finding all the line equations and average them in two groups, neg or pos slope
		for(uint_fast8_t i = 0; i < num_lines; i++)
		{
			line_slope_int_t line = find_line_equation(lines[i]);

			if(line.m >= 0)
			{
				average_slope_pos += line.m;
				average_intercept_pos += line.b;
				num_lines_pos++;
			}
			else
			{
				average_slope_neg += line.m;
				average_intercept_neg += line.b;
				num_lines_neg++;
			}

			debug_print("Line %d Slope %f Int %d | avg_slope_pos %f, avg_int_pos %d | avg_slope_neg %f, avg_int_neg %d\n",
						i,
						line.m,
						line.b,
						average_slope_pos,
						average_intercept_pos,
						average_slope_neg,
						average_intercept_neg);
		}

		// Divide out the accumulated line equations to get avg
		line_slope_int_t pos_line;
		pos_line.m = average_slope_pos / num_lines_pos;
		pos_line.b = average_intercept_pos / num_lines_pos;
		debug_print("pos line slope %f int %d\n", pos_line.m, pos_line.b);

		line_slope_int_t neg_line;
		neg_line.m = average_slope_neg / num_lines_neg;
		neg_line.b = average_intercept_neg / num_lines_neg;
		debug_print("neg line slope %f int %d\n", neg_line.m, neg_line.b);

		// Get the centerline
		line_point_slope_t centerline = find_centerline(pos_line, neg_line);
		debug_print("Centerline slope %f, point (%d, %d)\n", centerline.m, centerline.p.x, centerline.p.y);

		// PID Calculate
		float command = pid_calculate(5, 2, 1, 0, centerline.m);
		debug_print("PID output %f\n", command);

		// Servo Output
		servo_command((uint_fast16_t)command);

		// Update Time
		debug_print_time();
		debug_print(" - servo planner end\n");
		time.tv_sec += (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
		time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;
	}
}


line_slope_int_t find_line_equation(line_xy_t line)
{
	// Find the slope intercept formula of a line
	line_slope_int_t line_si;
	line_si.m = ((float)line.p0.y - line.p1.y)/((float)line.p0.x - line.p1.x);
	line_si.b = line.p0.y - (line_si.m * line.p0.x);

	return line_si;
}

line_point_slope_t find_centerline(line_slope_int_t line_0, line_slope_int_t line_1)
{
	// find the intersection point
	line_point_slope_t centerline;

	// intersection can be found
	centerline.p.x = (line_1.b - line_0.b)/(line_0.m - line_1.m);
	centerline.m = (line_0.m + line_1.m)/2;

	return centerline;
}


float pid_calculate(uint_fast8_t Kp,
							uint_fast8_t Ki,
							uint_fast8_t Kd,
							uint_fast16_t setpoint,
							float measurement)
{
	// Static Terms
	static float integral = 0;
	static float last_measurement = 0;

	// Calculate Error
	float error = setpoint - measurement;

	// Calculate New Integral
	integral += error;

	// Calculate New Derivative
	float derivative = measurement - last_measurement;

	// Calculate P Term
	float P = Kp * error;

	// Calculate I Term
	float I = Ki * integral;

	// Calculate D Term
	float D = Kd * derivative;

	// Sum for Output
	float output = P + I + D;

	// Return
	return output;
}

void servo_command(uint_fast16_t steering_input)
{
	// Limit Speed Input
	if(steering_input > SERVO_STANDARD_SPEED)
	{
		steering_input = SERVO_STANDARD_SPEED;
	}

	// Calculate the servo outputs to achieve the differential steering required
	uint_fast16_t speed_left_servo = SERVO_ZERO + SERVO_L_FW*(SERVO_STANDARD_SPEED + steering_input);
	uint_fast16_t speed_right_servo = SERVO_ZERO + SERVO_R_FW*(SERVO_STANDARD_SPEED - steering_input);

	// Output the values
	gpioServo(SERVO_L, speed_left_servo);
	gpioServo(SERVO_R, speed_right_servo);
}