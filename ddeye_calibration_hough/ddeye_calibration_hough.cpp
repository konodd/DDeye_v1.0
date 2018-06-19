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
	ifstream in("../dataset/dxl_angle.txt");
	ofstream out("../dataset/point1.txt");
	string filename;
	string angle1;
	string angle2;
	Mat frame;
	Mat frame_gray;
	std::vector<Vec3f> circles;
	int cannyThreshold = cannyThresholdInitialValue;
	int accumulatorThreshold = accumulatorThresholdInitialValue;

	if (in.is_open())
	{
		while (!in.eof())
		{
			in >> filename;
			in >> angle1;
			in >> angle2;
			if (in.fail())
			{
				break;
			}
			//ÕºœÒ‘§¥¶¿Ì
			frame = cv::imread(filename);
			cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
			GaussianBlur(frame_gray, frame_gray, Size(9, 9), 2, 2);

			namedWindow(windowName, WINDOW_AUTOSIZE);
			createTrackbar(cannyThresholdTrackbarName, windowName, &cannyThreshold, maxCannyThreshold);
			createTrackbar(accumulatorThresholdTrackbarName, windowName, &accumulatorThreshold, maxAccumulatorThreshold);

			cannyThreshold = std::max(cannyThreshold, 1);
			accumulatorThreshold = std::max(accumulatorThreshold, 1);

			//ªÙ∑Ú‘≤ºÏ≤‚£¨Ω´Õº÷–µƒ‘≤¥Ê»Îcircles÷–
			HoughCircles(frame_gray, circles, CV_HOUGH_GRADIENT, 1, frame_gray.rows / 8, cannyThreshold, accumulatorThreshold, 0, 0);

			//            for(size_t i = 0; i < circles.size(); i++ )
			//            {
			//                Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));       //Center °™°™£®x£¨y£©
			//                int radius = cvRound(circles[i][2]);                                //∞Îæ∂°™°™r
			//
			//                circle(frame, center, 3, Scalar(0,255,0), -1, 8, 0 );            //°æ÷––ƒ‘≤°ø
			//                circle(frame, center, radius, Scalar(0,0,255), 3, 8, 0 );        //°æÕ‚¬÷¿™‘≤°ø
			//            }
			if (circles[0][0] && circles[0][1])
			{
				circle(frame, Point(circles[0][0], circles[0][1]), circles[0][2], Scalar(0, 0, 255), 3);
				//out << filename << " " << angle1 << " " << angle2 << " " << circles[0][0] << " " << circles[0][1] << endl;
				out << angle1 << " " << angle2 << " " << circles[0][0] << " " << circles[0][1] << endl;
			}

			imshow(windowName, frame);

			int opt = waitKey(30);
			if (opt == 'p')
			{
				waitKey(0);
			}
		}
	}

	in.close();
	out.close();
	return 0;
}
