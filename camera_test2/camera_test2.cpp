//
// Created by kono on 18-5-16.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;
using namespace cv;

// windows and trackbars name
const std::string windowName = "Calibration Demo";
const std::string cannyThresholdTrackbarName = "Canny threshold";
const std::string accumulatorThresholdTrackbarName = "Accumulator Threshold";

// initial and max values of the parameters of interests.
const int cannyThresholdInitialValue = 60;
const int accumulatorThresholdInitialValue = 40;
const int maxAccumulatorThreshold = 200;
const int maxCannyThreshold = 255;

int main(int argc, char **argv)
{
	VideoCapture cap(1);
	Mat frame;
	Mat frame_hsv;
	Mat frame_gray;
	vector<Vec3f> circles;
	int cannyThreshold = cannyThresholdInitialValue;
	int accumulatorThreshold = accumulatorThresholdInitialValue;

	while (cap.isOpened())
	{
		//图像预处理
		cap >> frame;
		cvtColor(frame, frame_hsv, COLOR_BGR2HSV);
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		GaussianBlur(frame_gray, frame_gray, Size(9, 9), 2, 2);

		namedWindow(windowName, WINDOW_AUTOSIZE);
		createTrackbar(cannyThresholdTrackbarName, windowName, &cannyThreshold, maxCannyThreshold);
		createTrackbar(accumulatorThresholdTrackbarName, windowName, &accumulatorThreshold, maxAccumulatorThreshold);
		imshow("gray", frame_gray);
		cannyThreshold = std::max(cannyThreshold, 1);
		accumulatorThreshold = std::max(accumulatorThreshold, 1);

		//霍夫圆检测，将图中的圆存入circles中
		HoughCircles(frame_gray, circles, CV_HOUGH_GRADIENT, 1, frame_gray.rows / 8, cannyThreshold, accumulatorThreshold, 0, 0);

		Point2f center;
		float radius;
		Moments mu;
		Vec3f circle1;
		float maxArea = 0.0;

		if (!circles.empty())
		{
			for (auto i : circles)
			{
				circle(frame, Point(i[0], i[1]), i[2], Scalar(0, 0, 255), 3);
			}
		}

		imshow(windowName, frame);

		int opt = waitKey(30);
		if (opt == 'p')
		{
			waitKey(0);
		}
		else if (opt == 'q')
		{
			break;
		}
	}
	return 0;
}