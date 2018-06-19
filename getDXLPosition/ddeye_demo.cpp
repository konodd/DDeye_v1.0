//
// Created by kono on 18-5-16.
//
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <xl320.hpp>
#include <smoothDel.hpp>
using namespace std;
using namespace cv;

// windows and trackbars name
const std::string windowName = "Demo";
XL320 mydxl = XL320();

void moveDXL(int8_t id, int16_t position, int16_t velocity)
{
	mydxl.setGoalVelocity(id, velocity);
	mydxl.setGoalPosition(id, position);
}

// trackbars name
const std::string cannyThresholdTrackbarName = "Canny threshold";
const std::string accumulatorThresholdTrackbarName = "Accumulator Threshold";

// initial and max values of the parameters of interests.
const int cannyThresholdInitialValue = 60;
const int accumulatorThresholdInitialValue = 40;
const int maxAccumulatorThreshold = 200;
const int maxCannyThreshold = 255;

const int radius_threshold = 40;

int main(int argc, char **argv)
{
	VideoCapture cap(1);
	Mat frame;
	Mat frame_gray;
	vector<Vec3f> circles;
	vector<vector<Point>> contours;
	smoothDel sd;
	Point lastPoint = Point(320, 240);
	Point center;
	int dxl1, dxl2;
	int dxl1_center, dxl2_center;
	bool isCenter = false, autoTrack = false;
	int cannyThreshold = cannyThresholdInitialValue;
	int accumulatorThreshold = accumulatorThresholdInitialValue;

	if (mydxl.init() && sd.setDataPath("../dataset/auto_calibration.txt"))
	{
		mydxl.setLED(1, 5);
		mydxl.setLED(2, 5);
		sd.getPoints();
		//init center
		sd.getPosition(320, 240);
		lastPoint = center;
		sd.getDXLPosition(dxl1_center, dxl2_center);
		moveDXL(1, dxl1_center, (abs(dxl1_center - mydxl.getPresentPosition(1)) / 2));
		moveDXL(2, dxl2_center, (abs(dxl2_center - mydxl.getPresentPosition(2)) / 2));
		while (cap.isOpened())
		{
			//ÕºœÒ‘§¥¶¿Ì
			cap >> frame;
			cv::circle(frame, cv::Point(320, 240), 1, cv::Scalar(0, 0, 255), 2);
			cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

			GaussianBlur(frame_gray, frame_gray, Size(9, 9), 2, 2);

			namedWindow(windowName, WINDOW_AUTOSIZE);
			createTrackbar(cannyThresholdTrackbarName, windowName, &cannyThreshold, maxCannyThreshold);
			createTrackbar(accumulatorThresholdTrackbarName, windowName, &accumulatorThreshold, maxAccumulatorThreshold);
			cannyThreshold = std::max(cannyThreshold, 1);
			accumulatorThreshold = std::max(accumulatorThreshold, 1);

			//ªÙ∑Ú‘≤ºÏ≤‚£¨Ω´Õº÷–µƒ‘≤¥Ê»Îcircles÷–
			HoughCircles(frame_gray, circles, CV_HOUGH_GRADIENT, 1, frame_gray.rows / 8, cannyThreshold, accumulatorThreshold, 0, 0);

			if (circles[0][0] && circles[0][1])
			{
				/*for (auto i : circles)
				{
					center = Point(i[0], i[1]);
					circle(frame, center, i[2], Scalar(0, 0, 255), 3);
				}*/
				center = Point(circles[0][0], circles[0][1]);
				circle(frame, center, circles[0][2], Scalar(0, 0, 255), 3);
			}
			//…Ë÷√∆´≤Ó∑∂Œß
			int dx = center.x - 320;
			int dy = center.y - 240;
			if (dx * dx + dy * dy <= radius_threshold * radius_threshold && !isCenter)
			{
				moveDXL(1, mydxl.getPresentPosition(1), 40);
				moveDXL(2, mydxl.getPresentPosition(2), 40);
				isCenter = true;
			}
			else if (autoTrack && dx * dx + dy * dy > radius_threshold * radius_threshold)
			{
				isCenter = false;
				if (sd.getPosition(960 - (center.x + lastPoint.x), 720 - (center.y + lastPoint.y)))
				{
					//sd.getPosition(320, 240);
					lastPoint = center;
					sd.getDXLPosition(dxl1, dxl2);
					moveDXL(1, dxl1, (abs(dxl1 - mydxl.getPresentPosition(1)) / 2));
					moveDXL(2, dxl2, (abs(dxl2 - mydxl.getPresentPosition(2)) / 2));
				}
			}
			//circle(frame, Point(320, 240), radius_threshold, Scalar(0, 255, 0));

			imshow(windowName, frame);

			int opt = waitKey(30);
			switch (opt)
			{
			case 'p':
				waitKey(0);
				break;
			case 'a':
				autoTrack = !autoTrack;
				break;
			case 'r':
				mydxl.setGoalTorque(1, 1023);
				mydxl.setGoalTorque(2, 1023);
				moveDXL(1, dxl1_center, 40);
				moveDXL(2, dxl2_center, 40);
				lastPoint = Point(320, 240);
				break;
			case ' ':
				if (sd.getPosition(960 - (center.x + lastPoint.x), 720 - (center.y + lastPoint.y)))
				{
					//sd.getPosition(320, 240);
					lastPoint = center;
					sd.getDXLPosition(dxl1, dxl2);
					moveDXL(1, dxl1, (abs(dxl1 - mydxl.getPresentPosition(1)) / 2));
					moveDXL(2, dxl2, (abs(dxl2 - mydxl.getPresentPosition(2)) / 2));
				}
				break;
			case 'q':
				mydxl.setLED(1, 0);
				mydxl.setLED(2, 0);
				return 1;
			default:
				break;
			}
		}
	}
	return 0;
}