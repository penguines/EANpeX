/**********************************************

Author: penguine

Date: 2020-09-28

Description: Provide functions to parse EAN code in images with openCV.

**********************************************/

#ifndef EANpeX_cv_H
#define EANpeX_cv_H

#include "EANpeX.h"
#include "cv_chart.h"
#include <opencv2/opencv.hpp>
#include <imgproc/imgproc_c.h>

#define DEG2RAD 0.017453292519943295769236907684886

/*
Parse EAN Code to demical string from a cv Mat image.
*/
int parseEANMat_DEC(cv::Mat& InputMat, std::string& ean, int EANLength, int frontCode, int thresholdType = EAN_THRESHOLD_CONST, float threshold = 128.0);

/*
Find EAN Code area with HoughLinesP().
For conditions where EAN code area color is not so different from color of other parts.
A general mathod.
*/
int findEANArea_Hough(cv::Mat& InputMat, cv::Mat& OutputMat, bool showDetectedLines = false, bool showDetectedContours = false);

/*
Find EAN Code area with findContours().
For conditions where EAN code area color is obviously different from color of other parts.
If contours are not so obvious, please try to use cvContrastBright() to make contours obvious��
Or use findEANArea_Hough() instead.
*/
int findEANArea_Contours(cv::Mat& InputMat, cv::Mat& OutputMat, bool showDetectedContours = false);

/*
Rebuild EAN Code image.
*/
void rebuildEAN(cv::Mat& OutputArray, int OutputWidth, int OutputHeight, int* thresholdedData, int n);

/*

*/
void genEANBarcode(cv::Mat& OutputArray, 
				   int width, 
				   int height, 
				   const char* ean, 
				   int midIndex, 
				   bool isContainCheck = true, 
				   bool genFirstNum = false, 
				   bool showNumbers = true);

/*
Add angle1 and angle2.
*/
int angleAdd(int angle1, int angle2);

/*
Change contrast and bright of image.
	contrastValue: contrast value(percent). If input contrastValue = 10, all pixels will be multiplied by 10%(0.1).
	brightValue: brightness value needed to add/minus.
*/
void cvContrastBright(cv::Mat& srcArray, cv::Mat& dstArray, int contrastValue, int brightValue);

#endif