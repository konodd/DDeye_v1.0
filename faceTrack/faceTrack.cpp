#include <opencv2\opencv.hpp>
#include <opencv2\tracking\tracker.hpp>
#include <string>
#include <vector>
#include <xl320.hpp>
#include <smoothDel.hpp>

XL320 mydxl = XL320();
const int radius_threshold = 50;

void moveDXL(int8_t id, int16_t position, int16_t velocity)
{
	mydxl.setGoalVelocity(id, velocity);
	mydxl.setGoalPosition(id, position);
}

int fps()
{
	static int fps = 0;
	static int lastTime = cv::getTickCount();
	static int frameCount = 0;

	++frameCount;

	int curTime = cv::getTickCount();
	if ((curTime - lastTime) / cv::getTickFrequency() > 1.0) // ȡ�̶�ʱ����Ϊ1��
	{
		fps = frameCount;
		frameCount = 0;
		lastTime = curTime;
	}
	return fps;
}

int main(int argc, char **argv)
{
	cv::CascadeClassifier cas("../haarcascades/haarcascade_frontalface_alt.xml");  //�������
	cv::VideoCapture cap(1);
	std::vector<cv::Rect> face_boxes;                                              //������
	cv::Rect2d tracking_box;                                                       //����Ŀ��
	cv::Mat frame;
	cv::Ptr<cv::TrackerKCF> face_tracker;                                          //KCF����
	cv::TrackerKCF::Params face_tracker_params;                                    //KCF���ٲ���
	face_tracker_params.detect_thresh = 0.2;                                       //KCF�������Ŷ�
	face_tracker_params.resize = true;
	cv::Point lastPoint = cv::Point(320, 240);                                     //���ڱ�����һ֡�е�Ŀ��λ��
	cv::Point center;                                                              //��֡��Ŀ���λ��
	int dxl1, dxl2;                                                                //���1��2��λ��
	int dxl1_center, dxl2_center;                                                  //�궨���У���������Ķ��λ��
	smoothDel sd;
	bool isCenter = false, autoTrack = false;
	char str[20];
	bool is_detected = false;

	if (mydxl.init() && sd.setDataPath("../dataset/auto_calibration.txt"))
	{
		mydxl.setLED(1, 5);
		mydxl.setLED(2, 5);
		sd.getPoints();
		//init center
		sd.getPosition(320, 240);
		sd.getDXLPosition(dxl1_center, dxl2_center);
		moveDXL(1, dxl1_center, (abs(dxl1_center - mydxl.getPresentPosition(1)) / 2));
		moveDXL(2, dxl2_center, (abs(dxl2_center - mydxl.getPresentPosition(2)) / 2));
		while (cap.isOpened())
		{
			cap >> frame;
			if (frame.empty())
			{
				continue;
			}
			//���
			if (!is_detected)
			{
				cv::putText(frame, "detecting face", cv::Point(0, 20), cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 0, 0));
				cas.detectMultiScale(frame, face_boxes);
				//��⵽Ŀ��󣬳�ʼ������ģ��
				if (!face_boxes.empty())
				{
					tracking_box = face_boxes[0];
					face_tracker = cv::TrackerKCF::create(face_tracker_params);
					face_tracker->init(frame, tracking_box);
					is_detected = true;
				}
			}
			//����
			else
			{
				if (face_tracker->update(frame, tracking_box))
				{
					cv::putText(frame, "tracking face", cv::Point(0, 20), cv::FONT_HERSHEY_COMPLEX, 0.6, cv::Scalar(255, 0, 0));
					cv::rectangle(frame, tracking_box, cv::Scalar(0, 0, 255));
					cv::putText(frame, "tracking", cv::Point(tracking_box.x, tracking_box.y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255));
					if (autoTrack)
					{
						center = cv::Point(tracking_box.x + tracking_box.width / 2, tracking_box.y + tracking_box.height / 2);
						//����ƫ�Χ
						int dx = center.x - 320;
						int dy = center.y - 240;
						//������ֵ��Χ�����ֹͣ�˶�
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
								lastPoint = center;
								sd.getDXLPosition(dxl1, dxl2);
								moveDXL(1, dxl1, (abs(dxl1 - mydxl.getPresentPosition(1)) / 2));
								moveDXL(2, dxl2, (abs(dxl2 - mydxl.getPresentPosition(2)) / 2));
							}
						}
					}
				}
				else
				{
					face_tracker.release();
					is_detected = false;
				}
			}
			//��ʾͼƬ����������
			sprintf(str, "FPS:%d", fps());
			cv::putText(frame, str, cv::Point(540, 30), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0));
			cv::imshow("test", frame);
			int opt = cv::waitKey(10);
			switch (opt)
			{
			case 'p':                                //��ͣ
				cv::waitKey(0);
				break;
			case 'a':                                //�޸�ģʽ���Զ�/���Զ�
				autoTrack = !autoTrack;
				break;
			case 'r':                                //��ԭ����ʼλ��
				mydxl.setGoalTorque(1, 1023);
				mydxl.setGoalTorque(2, 1023);
				moveDXL(1, dxl1_center, 40);
				moveDXL(2, dxl2_center, 40);
				lastPoint = cv::Point(320, 240);
				break;
			case ' ':                                //���¼��
				is_detected = false;
				/*if (sd.getPosition(960 - (center.x + lastPoint.x), 720 - (center.y + lastPoint.y)))
				{
					lastPoint = center;
					sd.getDXLPosition(dxl1, dxl2);
					moveDXL(1, dxl1, (abs(dxl1 - mydxl.getPresentPosition(1)) / 2));
					moveDXL(2, dxl2, (abs(dxl2 - mydxl.getPresentPosition(2)) / 2));
				}*/
				break;
			case 'q':                                //�Ƴ�
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