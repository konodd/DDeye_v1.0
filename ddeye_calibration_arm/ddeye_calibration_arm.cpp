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

bool isCircleContour(vector<Point>& contour, double minRadius)
{
	bool isCircle = false;

	//double perimeter = arcLength(contour,TRUE);
	double perimeter = arcLength(contour, false);
	double perimeter2 = arcLength(contour, true);
	Rect rect = boundingRect(contour);
	double r = (rect.width + rect.height) / 4.0;    //半径约等于轮廓外接矩的边长/2

	double area = contourArea(contour);

	double pi = 3.1415926;

	double perimeterDiffRatio = fabs(2 * pi*r - perimeter)*2.0 / (2 * pi*r + perimeter) * 100;
	double areaDiffRatio = fabs(pi*r*r - area)*2.0 / (pi*r*r + area) * 100;
	double sideDiffRatio = abs(rect.width - rect.height)*2.0 / (rect.width + rect.height) * 100;

	const double areaDiffThresh = 5 * 2;        //目前统计的，最大的面积差2.58%
	const double perimeterDiffThresh = 7.25 * 2; //目前统计的，最大的周长差5.65%
	const double sideDiffThresh = 10 * 2;       //目前统计的，最大的边长差7.3%
	const double avgDiffThresh = 5.0 * 2;       //三个差异比值的加权平均要求小于5.0;

	double avdDiffRatio = (areaDiffRatio + perimeterDiffRatio + sideDiffRatio) / 3.0;

	//计算出的轮廓周长是半径的2*pi倍数
	if ((areaDiffRatio <areaDiffThresh)
		&& (perimeterDiffRatio <perimeterDiffThresh)
		&& (sideDiffRatio <sideDiffThresh)
		&& (avdDiffRatio<avgDiffThresh)
		&& (r>minRadius))
	{
		isCircle = true;
	}
	return isCircle;
}

int main(int argc, char **argv)
{
    ifstream in("../dataset/dxl_angle.txt");
    ofstream out("../dataset/point_test.txt");
    string filename;
    string angle1;
    string angle2;
    Mat frame;
    Mat frame_hsv;
    Mat frame_gray;
    vector<Vec3f> circles;
    vector<vector<Point>> contours;

    if(in.is_open())
    {
        while (!in.eof())
        {
            in >> filename;
            in >> angle1;
            in >> angle2;
            if(in.fail())
            {
                break;
            }
            //图像预处理
            frame = cv::imread(filename);
            cvtColor(frame, frame_hsv, COLOR_BGR2HSV);
            cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

            Mat mask;
            inRange(frame_hsv, Scalar(35, 43, 46), Scalar(77, 255, 255), mask);
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

            if(!contours.empty())
            {
                for(auto i : contours)
                {
                    if(contourArea(i) > maxArea && isCircleContour(i, 5))
                    {
                        maxArea = contourArea(i);
                        contour = i;
                    }
                    //minEnclosingCircle(i, center, radius);
                    //mu = moments(i);
                    //center = Point(int(mu.m10 / mu.m00), int(mu.m01 / mu.m00));
                    //if(radius > 0)
                    //{
                    //    circle(frame, center, radius, Scalar(0, 0, 255), 3);
                    //}
                }
                //cout << contours.size() << endl;
                //for(int i = 0; i < contours.size(); i++)
                //{
                //    drawContours(frame, contours, i, Scalar(0, 0, 255));
                //}
                if(!contour.empty())
                {
                    minEnclosingCircle(contour, center, radius);
                    mu = moments(contour);
                    center = Point(int(mu.m10 / mu.m00), int(mu.m01 / mu.m00));
                    cout << contours.size() << " " << contour.size() << " " << center.x << " " << center.y << " " << radius << endl;
                }
            }
            if(radius > 0)
            {
                circle(frame, center, radius, Scalar(0, 0, 255), 3);
            }

            imshow( windowName, frame);

            int opt = waitKey(30);
            if(opt == 'p')
            {
                waitKey(0);
            }
			else if (opt == 'q')
			{
				break;
			}
        }
    }

    in.close();
    out.close();
    return 0;
}