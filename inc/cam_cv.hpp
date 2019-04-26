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

/*  Function declarations  */

void* cam_lines(void* args);

void* cam_circles(void* args);


#endif
