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
const std::string windowName = "Demo";

bool isCircleContour(vector<Point>& contour, double minRadius)
{
	bool isCircle = false;

	double perimeter = arcLength(contour, false);
	Rect rect = boundingRect(contour);
//	double r = (rect.width + rect.height) / 4.0;    //半径约等于轮廓外接矩的边长/2
	double r = min(rect.width, rect.height) / 2.0;

	double area = contourArea(contour);

	double pi = 3.1415926;

	double circle_area = pi * r * r;

	if (fabs(circle_area - area) / area < 0.1
		&& fabs(rect.width - rect.height) / rect.width < 0.1)
	{
		isCircle = true;
	}

	//double perimeterDiffRatio = fabs(2 * pi * r - perimeter) * 2.0 / (2 * pi * r + perimeter) * 100;
	//double areaDiffRatio = fabs(pi * r * r - area) * 2.0 / (pi * r * r + area) * 100;
	//double sideDiffRatio = abs(rect.width - rect.height) * 2.0 / (rect.width + rect.height) * 100;

	//const double areaDiffThresh = 5;        //目前统计的，最大的面积差2.58%
	//const double perimeterDiffThresh = 7.25; //目前统计的，最大的周长差5.65%
	//const double sideDiffThresh = 10;       //目前统计的，最大的边长差7.3%
	//const double avgDiffThresh = 5.0;       //三个差异比值的加权平均要求小于5.0;

	//double avdDiffRatio = (areaDiffRatio + perimeterDiffRatio + sideDiffRatio) / 3.0;

	////计算出的轮廓周长是半径的2*pi倍数
	//if ((areaDiffRatio < areaDiffThresh)
	//	&& (perimeterDiffRatio < perimeterDiffThresh)
	//	&& (sideDiffRatio < sideDiffThresh)
	//	&& (avdDiffRatio < avgDiffThresh)
	//	&& (r > minRadius))
	//{
	//	isCircle = true;
	//}
	return isCircle;
}

int main(int argc, char **argv)
{
	VideoCapture cap(0);
	Mat frame;
	Mat frame_hsv;
	Mat frame_gray;
	vector<Vec3f> circles;
	vector<vector<Point>> contours;

	while (cap.isOpened())
	{
		//图像预处理
		cap >> frame;
		cvtColor(frame, frame_hsv, COLOR_BGR2HSV);
		cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

		Mat mask;
		inRange(frame_hsv, Scalar(0, 0, 0), Scalar(190, 255, 50), mask);
		imshow("hsv", frame_hsv);
		imshow("mask", mask);
		erode(mask, mask, NULL, Point(-1, -1), 2);
		imshow("erode", mask);
		dilate(mask, mask, NULL, Point(-1, -1), 2);
		imshow("dilate", mask);
		findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		Point2f center;
		float radius;
		Moments mu;
		vector<Point> contour;
		float maxArea = 0.0;

		if (!contours.empty())
		{
			for (auto i : contours)
			{
				if (contourArea(i) > maxArea && isCircleContour(i, 1))
				{
					maxArea = contourArea(i);
					contour = i;
					minEnclosingCircle(i, center, radius);
					mu = moments(i);
					center = Point(int(mu.m10 / mu.m00), int(mu.m01 / mu.m00));
					if (radius > 0)
					{
						circle(frame, center, radius, Scalar(0, 0, 255), 3);
					}
				}
			}
			//            cout << contours.size() << endl;
			for (int i = 0; i < contours.size(); i++)
			{
				drawContours(frame, contours, i, Scalar(255, 0, 0));
			}
			//if (!contour.empty())
			//{
			//	minEnclosingCircle(contour, center, radius);
			//	mu = moments(contour);
			//	center = Point(int(mu.m10 / mu.m00), int(mu.m01 / mu.m00));
			//}
		}
		//if (radius > 0)
		//{
		//	circle(frame, center, radius, Scalar(0, 0, 255), 3);
		//}


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