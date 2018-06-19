#include <fstream>
#include <string>
#include <xl320.hpp>
#include <vector>
#include <opencv2\opencv.hpp>
using namespace std;

XL320 mydxl = XL320();
int threshold = 12;

void moveDXL(int id1, int id2, int position1, int position2, int velocity1, int velocity2)
{
	mydxl.setGoalVelocity(id1, velocity1);
	mydxl.setGoalPosition(id1, position1);
	mydxl.setGoalVelocity(id2, velocity2);
	mydxl.setGoalPosition(id2, position2);
}

bool saveTable(double time)
{
	static int lastTime = cv::getTickCount();

	int curTime = cv::getTickCount();
	if ((curTime - lastTime) / cv::getTickFrequency() > time) // 取固定时间间隔为time秒
	{
		lastTime = curTime;
		return true;
	}
	return false;
}

int main(int argc, char **argv)
{
	cv::VideoCapture cap(1);
	cv::Mat frame;
	cv::Mat frame_gray;
	vector<cv::Vec3f> circles;
	ifstream in("../标定范围表6.0.txt");
	ofstream out("../dataset/auto_calibration.txt");

	char a[20];
	int i = 0;

	int dxl1;
	int dxl2_start, dxl2_end;

	bool isBack = false;
	bool startCalibration = false;

	if (mydxl.init() && cap.isOpened() && in.is_open() && out.is_open())
	{
		//初始阶段，将舵机移到初始位置
		in >> dxl1 >> dxl2_start >> dxl2_end;
		moveDXL(1, 2, dxl1, dxl2_end, 40, 40);
		while (!in.eof())
		{
			cap >> frame;
			cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
			GaussianBlur(frame_gray, frame_gray, cv::Size(9, 9), 2, 2);
			HoughCircles(frame_gray, circles, CV_HOUGH_GRADIENT, 1, frame_gray.rows / 8, 60, 40, 0, 0);
			if (!circles.empty())
			{
				circle(frame, cv::Point(circles[0][0], circles[0][1]), circles[0][2], cv::Scalar(0, 0, 255), 3);
			}

			cv::circle(frame, cv::Point(320, 240), 1, cv::Scalar(0, 0, 255), 2);
			cv::imshow("autoCalibration", frame);
			if (startCalibration)
			{
				//每次移动轨迹：end->start->end
				if (abs(mydxl.getPresentPosition(1) - dxl1) < threshold && abs(mydxl.getPresentPosition(2) - dxl2_end) < threshold && !isBack)
				{
					moveDXL(1, 2, dxl1, dxl2_start, 40, 40);
				}
				else if (abs(mydxl.getPresentPosition(1) - dxl1) < threshold && abs(mydxl.getPresentPosition(2) - dxl2_start) < threshold && !isBack)
				{
					moveDXL(1, 2, dxl1, dxl2_end, 40, 40);
					isBack = true;
				}
				else if (abs(mydxl.getPresentPosition(1) - dxl1) < threshold && abs(mydxl.getPresentPosition(2) - dxl2_end) < threshold && isBack)
				{
					isBack = false;
					//从范围表中读取数据
					in >> dxl1 >> dxl2_start >> dxl2_end;
					if (in.fail())
					{
						break;
					}
					printf("dxl1 position：%d  dxl2 from %d to %d\n", dxl1, dxl2_start, dxl2_end);
					moveDXL(1, 2, dxl1, dxl2_end, 40, 40);
				}
			}
			
			//隔一定时间取样,此处为0.5
			//if (saveTable(0.5) && !circles.empty())
			if(saveTable(0.5) && circles[0][0] && circles[0][1])
			{
				//sprintf(a, "../dataset/%05d.jpg", i++);
				//cv::imwrite(a, frame);
				//out << a << " " << mydxl.getPresentPosition(1) << " " << mydxl.getPresentPosition(2) << " " << circles[0][0] << " " << circles[0][1] << "\n";
				out << mydxl.getPresentPosition(1) << " " << mydxl.getPresentPosition(2) << " " << circles[0][0] << " " << circles[0][1] << "\n";
			}

			int opt = cv::waitKey(30);
			switch (opt)
			{
			case 'q':
				return 1;
			case ' ':
				startCalibration = !startCalibration;
				break;
			case 'r':
				mydxl.setGoalTorque(1, 1023);
				mydxl.setGoalTorque(2, 1023);
				moveDXL(1, 2, 450, 400, 40, 40);
				break;
			default:
				break;
			}
		}
	}

	return 0;
}