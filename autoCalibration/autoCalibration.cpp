//
// Created by kono on 18-6-5.
//

#include <fstream>
#include <xl320.hpp>
#include <opencv2/opencv.hpp>
using namespace std;

XL320 mydxl = XL320();
const int threshold = 10;       //舵机位置误差阈值
const int maxload = 70;         //舵机最大负载
int dxl1_start = 450;                 //舵机1初始位置
int dxl1_end = 850;                   //舵机1终点位置
int dxl2_start = 350;                 //舵机2初始位置

									  //移动舵机
void moveDXL(int id1, int id2, int position1, int position2, int velocity1, int velocity2)
{
	mydxl.setGoalVelocity(id1, velocity1);
	mydxl.setGoalPosition(id1, position1);
	mydxl.setGoalVelocity(id2, velocity2);
	mydxl.setGoalPosition(id2, position2);
}

//判断经过一定时间
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
	ofstream out("../dataset/auto_table.txt");

	char a[20];
	int i = 1;

	int dxl1 = dxl1_start;

	bool isBack = false;
	bool startCalibration = false;
	if (mydxl.init() && cap.isOpened() && out.is_open())
	{
		//初始阶段，将舵机移到初始位置
		moveDXL(1, 2, dxl1_start, dxl2_start, 40, 40);
		while (1)
		{
			cap >> frame;
			cv::circle(frame, cv::Point(320, 240), 1, cv::Scalar(0, 0, 255), 2);
			cv::imshow("getDataSet", frame);
			if (startCalibration)
			{
				int curLoad1 = mydxl.getPresentLoad(1);
				int curLoad2 = mydxl.getPresentLoad(2);
				//舵机2位置从大到小
				if (abs(mydxl.getPresentPosition(1) - dxl1) <= threshold && curLoad2 % 1024 <= maxload && !isBack)
				{
					printf("curLoad1:%4d\tcurLoad2:%4d\tfrom max to min\n", curLoad1, curLoad2);
					moveDXL(1, 2, dxl1, mydxl.getPresentPosition(2) - 10, 40, 40);
				}
				//舵机2位置从小到大
				else if (abs(mydxl.getPresentPosition(1) - dxl1) <= threshold && curLoad2 % 1024 <= maxload && isBack)
				{
					printf("curLoad1:%4d\tcurLoad2:%4d\tfrom min to max\n", curLoad1, curLoad2);
					moveDXL(1, 2, dxl1, mydxl.getPresentPosition(2) + 10, 40, 40);
					isBack = true;
				}
				//舵机负载到达阈值时，转向
				else if (abs(mydxl.getPresentPosition(1) - dxl1) <= threshold && (curLoad2 % 1024 > maxload))
				{
					printf("curLoad1:%4d\tcurLoad2:%4d\tturn\n", curLoad1, curLoad2);
					//isBack = !isBack;
					if (isBack)
					{
						isBack = false;
						dxl1 += 20;
						if (dxl1 > dxl1_end)
						{
							break;
						}
						moveDXL(1, 2, dxl1, mydxl.getPresentPosition(2) + 20, 40, 40);
					}
					else
					{
						isBack = true;
						//达到最大值
						if ((curLoad2 % 1024 > maxload && curLoad2 / 1024) || (curLoad1 % 1024 > maxload && !(curLoad1 / 1024)))
						{
							moveDXL(1, 2, dxl1, mydxl.getPresentPosition(2) - 15, 40, 40);
						}
						else
						{
							moveDXL(1, 2, dxl1, mydxl.getPresentPosition(2) + 15, 40, 40);
						}
					}
				}
			}
			//隔一定时间取样,此处为0.5
			if (saveTable(0.5))
			{
				sprintf(a, "../dataset/%05d.jpg", i++);
				cv::imwrite(a, frame);
				out << a << " " << mydxl.getPresentPosition(1) << " " << mydxl.getPresentPosition(2) << "\n";
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
				moveDXL(1, 2, 450, 375, 40, 40);
				break;
			default:
				break;
			}
		}
	}

	return 0;
}
