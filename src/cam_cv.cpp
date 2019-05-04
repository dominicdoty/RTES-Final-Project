// Cam CV transforms service
// ECEN 5623

#include "cam_cv.hpp"
#include "debug.h"
#include "data.h"


#define DEBUG 1



#define PERIOD (50)


using namespace cv;
using namespace std;



void* cam_lines(void* args)
{
	int buffer_Val  = 0;

	// setting up timing for 50[ms] period
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	debug_print("initial time in cam: %ld.%ld\n", time.tv_sec, time.tv_nsec);
	time.tv_sec += (time.tv_nsec + (PERIOD*NSEC_PER_MSEC))/NSEC_PER_SEC;
	time.tv_nsec = (time.tv_nsec + (PERIOD*NSEC_PER_MSEC)) % NSEC_PER_SEC;
	debug_print("new time in cam: %ld.%ld\n", time.tv_sec, time.tv_nsec);
	debug_print("entered cam_lines\n");
	
	vector<Vec4i> lines;
	Mat frame, canny_frame, line_gray, line_HSV, threshold_HSV, cropped_HSV;

	// initialize the camera
	cv::VideoCapture cap(0); 	// capture the video from camera

	// set up the double buffer

	int *buffer0;
	int *buffer1;
	pthread_mutex_t mutex0;
	pthread_mutex_t mutex1;

	while(1)
	{
		if(buffer_Val == 0)
			pthread_mutex_lock(&mutex0);
		else if(buffer_Val == 1)
			pthread_mutex_lock(&mutex1);

		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);

		debug_print_time();
		debug_print(" - cam_lines start\n");
	
		// wait until the mutex is released by 
		//sem_wait(&cv_sem);

		 if ( !cap.isOpened() )  // if not success, exit program
		 {
			cout << "Failed to open the web cam in cam_lins()" << endl;
			lines.clear();
			break;
			//return lines;
	         }


		bool success = cap.grab();
		cap.retrieve(frame);

		if(!success)
			debug_print("failed to grab frame\n");

		flip(frame, frame, 0);
		Mat mat_lines(frame);
	
		cvtColor(mat_lines, line_HSV, COLOR_BGR2HSV);
		inRange(line_HSV, Scalar(LOW_H, LOW_S, LOW_V), Scalar(HIGH_H, HIGH_S, HIGH_V), threshold_HSV);

		Mat image_roi(threshold_HSV, Rect(START_X, START_Y, WIDTH_X, HEIGHT_Y));

		image_roi.copyTo(cropped_HSV);

		GaussianBlur(cropped_HSV, canny_frame, Size(7,7), 1.5, 1.5);
		Canny(canny_frame, canny_frame, CANNY_LOW_THRESH, CANNY_HIGH_THRESH, 3);
		HoughLinesP(canny_frame, lines, 3, 3*CV_PI/180, 50, 125, 50);	
	
		debug_print("lines.size: %d\n", lines.size());

		for(size_t i = 0; i < lines.size(); i++)
		{
			Vec4i l = lines[i];
			line(mat_lines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
			debug_print("Line[%d], Point 1. x=%d y=%d\n", i, l[0], l[1]);
			debug_print("Line[%d]: Point 2. x=%d y=%d\n", i, l[2], l[3]);
		}
		

		// write the line data and count of line to the shared buffers
		if(buffer_Val == 0)
		{
			for(size_t j = 0; j < lines.size(); j++)
			{
				Vec4i line = lines[j];
				
				buffer0[0] = (int)lines.size();

				buffer0[4*j+1] = line[0];
				buffer0[4*j+2] = line[1];
				buffer0[4*j+3] = line[2];
				buffer0[4*j+4] = line[2];
			}

			pthread_mutex_unlock(&mutex0);
			buffer_Val = 1;
		}
		else if(buffer_Val == 1)
		{
			for(size_t k = 0; k < lines.size(); k++)
			{
				Vec4i line = lines[k];
				buffer1[0] = (int)lines.size();

				buffer1[4*k+1] = line[0];
				buffer1[4*k+2] = line[1];
				buffer1[4*k+3] = line[2];
				buffer1[4*k+4] = line[3];		
			}

			pthread_mutex_unlock(&mutex1);
			buffer_Val = 0;		
		}

		time.tv_sec += (time.tv_nsec + (PERIOD * NSEC_PER_MSEC))/NSEC_PER_SEC;
		time.tv_nsec = (time.tv_nsec + (PERIOD * NSEC_PER_MSEC)) % NSEC_PER_SEC;

		debug_print_time();
		debug_print(" - cam_lines end\n");
	}
	return 0;
}



/*
void* cam_circles(void *args)
{
	debug_print("entered cam_circles\n");

	vector<Vec3f> circles;
	Mat frame, circle_gray;


	while(1)
	{
		sem_wait(&cv_sem);

		debug_print_time();
		debug_print(" - cam_circles got semaphore\n");

		VideoCapture cap(0);
	
		if( !cap.isOpened())
		{
			cout << "Failed to open the camera in cam_circles" << endl;	
			circles.clear();
			break;
			//return -1;
			//return circles;
		}	
	
		cap >> frame;
		flip(frame, frame, 0);
		Mat mat_circles(frame);
	
		cvtColor(mat_circles, circle_gray, COLOR_BGR2GRAY);
		GaussianBlur(circle_gray, circle_gray, Size(9,9), 2, 2);
		HoughCircles(circle_gray, circles, CV_HOUGH_GRADIENT, 1, circle_gray.rows/8, 200, 25, 0, 0);
	
		cout << "circles.size" << circles.size() << endl;
	
		for(size_t j = 0; j < circles.size(); j++)
		{
			Point center(cvRound(circles[j][0]), cvRound(circles[j][1]));
			int radius = cvRound(circles[j][2]);
	
			circle(mat_circles, center, 3, Scalar(0,255,0), -1, 8, 0);
			circle(mat_circles, center, radius, Scalar(255,0,0), 3, 8, 0);
		}
	
	//	imwrite("circles.jpg", mat_circles);
	
		debug_print_time();
		debug_print(" - cam_circles end, giving up semaphore\n");

		// give up the cv-function related mutex (should be immediately taken by cam_lines()
		sem_post(&cv_sem);
	       
		//return circles;
	}
	return 0;
}
*/
