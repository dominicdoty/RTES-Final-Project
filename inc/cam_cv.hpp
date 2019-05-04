// Cam CV service
// 

#ifndef CAMCV_H
#define CAMCV_H

#include <iostream>
#include "opencv2/opencv.hpp"
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <vector>

#define CANNY_LOW_THRESH (60)
#define CANNY_HIGH_THRESH (3*CANNY_LOW_THRESH)

// resolution of the image
#define HRES 	(640)
#define VRES	(480)

// defining the colorscale range (HSV) that is "blue"
#define LOW_H	(100)
#define LOW_S	(150)
#define LOW_V	(0)
#define HIGH_H	(140)
#define HIGH_S	(255)
#define HIGH_V	(255)

// defining the region of interest (ROI). (0,0) is the top-left corner of the image
#define START_X		(0)
#define START_Y		(360)	// 360 pixels from the top
#define WIDTH_X		(640)	// take the whole width of the image
#define HEIGHT_Y 	(120)	// take the bottom quarter of the image

/*  Function declarations  */

extern cv::VideoCapture cap;

void* cam_lines(void* args);

//void* cam_circles(void* args);


#endif
