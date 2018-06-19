//#include "xl320.hpp"
//#include <iostream>
//using namespace std;
//
//int _main(int argc, char **argv)
//{
//	XL320 mydxl = XL320();
//	if (mydxl.init())
//	{
//		cout << mydxl.getPresentPosition(2) << endl;
//		mydxl.setLED(2, 0);
//		mydxl.setGoalVelocity(2, 40);
//		mydxl.setGoalPosition(2, 1000);
//		while (mydxl.isMoving(2))
//		{
//			cout << mydxl.getPresentPosition(2) << endl;
//		}
//	}
//	return 0;
//}