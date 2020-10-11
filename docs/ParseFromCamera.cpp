#include <iostream>
#include <opencv2/opencv.hpp>
#include "EANpeX.h"
#include "EANpeX_cv.h"

int main() {
	cv::Mat frame, frameTemp, EANGray;
	cv::Mat EANImg(300, 800, CV_8UC3);
	cv::VideoCapture cap(0);
	if (!cap.isOpened())return -1;
	std::string ean;

	while (1) {
		cap >> frame;
		frameTemp = frame.clone();
		int findArea = findEANArea_Hough(frame, EANImg, true, true);
		int isParse = -1;
		//if area is found successfully
		if (findArea == 0) {
			cv::imshow("HoughFind", EANImg);
			cv::cvtColor(EANImg, EANGray, CV_BGR2GRAY);
			isParse = parseEANMat_DEC(EANGray, ean, 13, -1, EAN_THRESHOLD_AVERAGE);
		}
		if (isParse == 0)std::cout << "Parse succeeded by HoughFind: " << ean << std::endl;
		cv::imshow("camera_hough", frame);

		isParse = -1;
		frame = frameTemp.clone();
		findArea = findEANArea_Contours(frame, EANImg, true);
		if (findArea == 0) {
			cv::imshow("ContoursFind", EANImg);
			cv::cvtColor(EANImg, EANGray, CV_BGR2GRAY);
			isParse = parseEANMat_DEC(EANGray, ean, 13, -1, EAN_THRESHOLD_AVERAGE);
		}
		if (isParse == 0)std::cout << "Parse succeeded by ContoursFind: " << ean << std::endl;
		cv::imshow("camera_contours", frame);
		if (cv::waitKey(30) >= 0)break;
	}
	return 0;
}