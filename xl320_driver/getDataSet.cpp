#include <xl320.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>
using namespace std;

XL320 mydxl = XL320();

void moveDXL(int8_t id, int16_t position, int16_t velocity)
{
	mydxl.setGoalVelocity(id, velocity);
	mydxl.setGoalPosition(id, position);
}

int main(int argc, char **argv)
{
	cv::VideoCapture cap;
	cv::Mat frame;
	ofstream out("../dataset/dxl_angle.txt");
	char a[20];
	int i = 1;
	int j = 0;
	int k = 0;
	int opt;
	if (mydxl.init() && out.is_open())
	{
		//mydxl.setLED(1, 5);
		//mydxl.setLED(2, 5);
		//打开摄像头
		if (!cap.open(1))
		{
			return -1;
		}
		//获取数据
		while (1)
		{
			cap >> frame;
			cv::circle(frame, cv::Point(320, 240), 1, cv::Scalar(0, 0, 255), 2);
			cv::imshow("cap", frame);
			//wasd控制舵机转动
			opt = cv::waitKey(30);
			if (opt != -1)
			{
				switch (opt)
				{
				case 'w':
					moveDXL(1, mydxl.getPresentPosition(1) + 20, 40);
					break;
				case 's':
					moveDXL(1, mydxl.getPresentPosition(1) - 20, 40);
					break;
				case 'a':
					moveDXL(2, mydxl.getPresentPosition(2) + 20, 40);
					break;
				case 'd':
					moveDXL(2, mydxl.getPresentPosition(2) - 20, 40);
					break;
				case 'r':
					mydxl.setGoalTorque(1, 1023);
					mydxl.setGoalTorque(2, 1023);
					moveDXL(1, 612, 80);
					moveDXL(2, 412, 80);
					break;
				case 'z':
					mydxl.setLED(1, (j++) % 8);
					break;
				case 'x':
					mydxl.setLED(2, (k++) % 8);
					break;
				case ' ':
					sprintf(a, "../dataset/%05d.jpg", i++);
					cv::imwrite(a, frame);
					out << a << " " << mydxl.getPresentPosition(1) << " " << mydxl.getPresentPosition(2) << "\n";
					break;
				case 'q':
					mydxl.setLED(1, 0);
					mydxl.setLED(2, 0);
					out.close();
					return 1;
				default:
					break;
				}
			}

		}
	}
	return 0;
}