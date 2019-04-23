/*
 * File: services.cpp
 * Authors: Adrian Unkeles and Domoinic Doty
 * ECEN 5623 final project
 * 4/19/2019
 */

// Code has been written with reference to previous projects
// In particular: Exercise 4, Part 5
//

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/param.h>
#include <syslog.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define NUM_THREADS (4)
#define HRES 640
#define VRES 480
#define NSEC_PER_SEC (1000000000)

#define THRESHOLD_CANNY (25)
#define THRESHOLD_HOUGH (42)
#define THRESHOLD_ELLIPTICAL (15)

#define ESC_KEY (27)

using namespace cv;
using namespace std;

// POSIX thread declarations and scheduling attributes
// thread1: canny-transformation
// thread2: elliptical transformation
// thread3: servo control
// thread4: ??? path-finding, or is it as we discussed, in servo control?

pthread_t thread_1, thread_2, thread_3, thread_4;
pthread_attr_t att_1, att_2, att_3, att_4;
pthread_attr_t main_att;

sem_t sem1, sem2, sem3, sem4;

char img_window_name[] = "Transform Display";

int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size=3;	// need to remember what this is used for
int edgeThresh=1;
int ratio = 3;
int dev = 0;

Mat canny_frame, cdst, img_gray, img_grad;

CvCapture* capture;
IplImage* frame;

int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
	int dt_sec=stop->tv_sec - start->tv_sec;
	int dt_nsec=stop->tv_nsec - start->tv_nsec;

	if(dt_sec >= 0)
	{
		if(dt_nsec >= 0)
		{
			delta_t->tv_sec=dt_sec;
			delta_t->tv_nsec=dt_nsec;
		}
		else
		{
			delta_t->tv_sec=dt_sec-1;
			delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
		}
	}
	else
	{
		if(dt_nsec >= 0)
		{
			delta_t->tv_sec=dt_sec;
			delta_t->tv_nsec=dt_sec;
		}
		else
		{
			delta_t->tv_sec=dt_sec-1;
			delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
		}
	}
	return(1);
}

void CannyThreshold(int, void*)
{
	Mat mat_frame(frame);

	cvtColor(mat_frame, img_gray, CV_RGB2GRAY);

	blur(img_gray, canny_frame, Size(3,3) );

	Canny(canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size);

	img_grad = Scalar::all(0);

	mat_frame.copyTo(img_grad, canny_frame);

	imshow(img_window_name, img_grad);	
}

void *canny(void *threadp)
{
	int cnt1=0, capture_flag1=0;
	float frames1;
	struct timespec start_time, stop_time, diff_time;
	
	while(1)
	{
		sem_wait(&sem1);
		while(cnt1<50)
		{
			clock_gettime(CLOCK_REALTIME, &start_time);
			cnt1++;
			frame=cvQueryFrame(capture);
			if(!frame)
			{
				break;
			}

			CannyThreshold(0,0);
			char q	= cvWaitKey(33);
			if(q=='q')
			{
				printf("got quit\n");
				break;		
			}
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
	
			frames1 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			
			printf("Canny frame rate = %f \n", frames1);
			printf("Canny jitter = %f \n", (THRESHOLD_CANNY - frames1));
	
			if((THRESHOLD_CANNY - frames1) < 0)
			{
				printf("\n MISSED canny DEADLINE\n");
			}			
		}
		cnt1=0;
//		sem_post(&sem2);
	}
}

void *hough(void *threadp)
{
	struct timespec start_time, stop_time, diff_time;
	IplImage* frame;
	int cnt2=0, capture_flag2=0;
	float frames2;
	Mat gray, canny_frame, cdst;
	vector<Vec4i> lines;

	while(1)
	{
		sem_wait(&sem2);
		while(cnt2<80)
		{
			clock_gettime(CLOCK_REALTIME, &start_time);
			cnt2++;
			frame=cvQueryFrame(capture);

			Mat mat_frame(frame);
			Canny(mat_frame, canny_frame, 50, 200, 3);

			HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

			for(size_t i=0; i < lines.size(); i++)
			{
				Vec4i l = lines[i];
				line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar (0,0,255), 3, CV_AA);
			}

			if(!frame) break;
			cvShowImage("Capture Example", frame);

			char c = cvWaitKey(10);
			if(c == ESC_KEY)
				break;
			
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
			frames2 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			if((THRESHOLD_HOUGH - frames2) < 0)
			{
				printf("\n *** Missed hough deadline ***\n");
			}
		}
		cnt2=0;
		sem_post(&sem3);
	}
}

void *ellip(void* threadp)
{
	struct timespec start_time, stop_time, diff_time;
	IplImage* frame;
	int cnt3=0, capture_flag3=0;
	float frames3;
	Mat gray;
	vector<Vec3f> circles;

	while(1)
	{
		sem_wait(&sem3);
		while(cnt3 < 80)
		{
			cnt3++;
			frame=cvQueryFrame(capture);
			Mat mat_frame(frame);
			cvtColor(mat_frame, gray, CV_BGR2GRAY);
			GaussianBlur(gray, gray, Size(9,9), 2, 2);
			HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);
	
			printf("circles.size = %d\n", circles.size());
	
			for(size_t i = 0; i < circles.size(); i++)
			{
				Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
				int radius = cvRound(circles[i][2]);
				// circle ctner
				circle (mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0);
				// circle outline
				circle(mat_frame, center, radius, Scalar(255,0,0), 3, 8, 0);
			}
	
			if(!frame) break;
	
			cvShowImage("Capture Example", frame);
	
			char c = cvWaitKey(10);
			if(c == ESC_KEY) break;
	
			clock_gettime(CLOCK_REALTIME, &stop_time);
			delta_t(&stop_time, &start_time, &diff_time);
			frames3 = 1/((float)(((float)diff_time.tv_nsec))/NSEC_PER_SEC);
			if((THRESHOLD_ELLIPTICAL - frames3) < 0)
			{
				printf("\n *** Missed elliptical Deadline ***\n");
			}
		}
		cnt3=0;
		sem_post(&sem1);
	}
}

int main(int argc, char *argv[])
{
	int rc;
	int i, scope;
//	useconds_t Tdeadline1 = framerate of camera
//	useconds_t Tdeadline2 = framerate of camera
//	useconds_t Tdeadline3 = deadline for servo control

	sem_init(&sem1,0,1);
//	sem_init(&sem2,0,0);
//	sem_init(&sem3,0,0);

	rc=pthread_attr_init(&att_1);
//	rc=pthread_attr_init(&att_2);
//	rc=pthread_attr_init(&att_3);

	capture=(CvCapture*)cvCreateCameraCapture(dev);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

	pthread_create(&thread_1, &att_1, canny, (void*) 0);
//	pthread_create(&thread_2, &att_2, hough, (void*) 0);
//	pthread_create(&thread_3, &att_3, ellip, (void*) 0);

	pthread_join(thread_1, NULL);
//	pthread_join(thread_2, NULL);
//	pthread_join(thread_3, NULL);

	pthread_attr_destroy(&att_1);
//	pthread_attr_destroy(&att_2);
//	pthread_attr_destroy(&att_3);

	sem_destroy(&sem1);
//	sem_destroy(&sem2);
//	sem_destroy(&sem3);

	printf("\nTest Complete\n");
}



