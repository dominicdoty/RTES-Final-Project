// Cam CV transforms service
// ECEN 5623

#include "cam_cv.hpp"

#include "debug.h"
#define DEBUG 1

extern sem_t cv_sem;

using namespace cv;
using namespace std;
/// how to get the vector out of the thread?

void* cam_lines(void* args)
{
	// give the mutex once here so that cam_lines runs first
	sem_wait(&cv_sem);
	
	vector<Vec4i> lines;
	Mat frame, canny_frame, line_gray;

	while(1)
	{
		// wait until the mutex is released by 
		sem_wait(&cv_sem);

		VideoCapture cap(0); //capture the video from webcam
	
		 if ( !cap.isOpened() )  // if not success, exit program
		 {
			cout << "Failed to open the web cam in cam_lins()" << endl;
			lines.clear();
			return lines;
	         }
	
		cap >> frame;
		flip(frame, frame, 0);
		Mat mat_lines(frame);
	
		cvtColor(mat_lines, line_gray, COLOR_BGR2GRAY);
		GaussianBlur(line_gray, canny_frame, Size(7,7), 1.5, 1.5);
		Canny(canny_frame, canny_frame, CANNY_LOW_THRESH, CANNY_HIGH_THRESH, 3);
		HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);	
	
		cout << "lines.size" << lines.size() << endl;	
	
		for(size_t i = 0; i < lines.size(); i++)
		{
			Vec4i l = lines[i];
			line(mat_lines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
		}
	
	//	imwrite("edges.jpg", canny_frame);
	//	imwrite("lines.jpg", mat_lines);
	
		// post the cv-function related mutex, should be immediately taken by cv_circles()
		sem_post(&cv_sem);

		return lines;
	}
}

void* cam_circles(void *args)
{
	vector<Vec3f> circles;
	Mat frame, circle_gray;


	while(1)
	{
		sem_wait(&cv_sem);

		VideoCapture cap(0);
	
		if( !cap.isOpened())
		{
			cout << "Failed to open the camera in cam_circles" << endl;	
			circles.clear();
			return circles;
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
	
		// give up the cv-function related mutex (should be immediately taken by cam_lines()
		sem_post(&cv_sem);
	        
		return circles;
	}
}
